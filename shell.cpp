#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include <wordexp.h>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <cassert>
#include <cstring>
#include <memory>
#include "object.h"
#include "module.h"
#include "machine.h"
#include "commands.h"

#define PERM_FILE		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define FD_CHECK

namespace rps
{
    static void fd_check(void)
    {
        int fd;
        bool ok = true;

        for (fd = 3; fd < 20; fd++)
        {
            if (fcntl(fd, F_GETFL) != -1 || errno != EBADF) {
                ok = false;
                fprintf(stderr, "*** fd %d is open ***\n", fd);
            }
        }
        if (!ok)
            _exit(EXIT_FAILURE);
    }

    void wait_and_display(pid_t wpid)
    {
        int status;

        pid_t pid = -1;
        while (wpid != pid)
        {
            pid = wait(&status);
            if (pid < 0)
            {
                if (errno == ECHILD)
                {
                    return;
                }
            }
            //if (WIFEXITED(status))
                //printf("Exit value %d\n", WEXITSTATUS(status));
            if (WIFSIGNALED(status))
            {
                printf("Process %ld: ", (long)pid);
                printf (" - signal %d\n", WTERMSIG(status));
            }
            if (WCOREDUMP(status))
            {
                printf("Process %ld: ", (long)pid);
                printf(" - core dumped\n");
            }
            if (WIFSTOPPED(status))
            {
                printf("Process %ld: ", (long)pid);
                printf(" (stopped)\n");
            }
            if (WIFCONTINUED(status))
            {
                printf("Process %ld: ", (long)pid);
                printf(" (continued)\n");
            }
        }
    }

#define IsPipe(x) ((x&PIPE_IN)!=0 || (x&PIPE_OUT)!=0)
#define IsFileIn(x) ((x&FILE_IN)!=0)
#define IsFileOut(x) ((x&FILE_OUT)!=0)

    enum redir_t {NONE = 0
                , PIPE_IN  = 0x01
                , PIPE_OUT = 0x02
                , FILE_IN = 0x04
                , FILE_OUT = 0x08
    };

    struct CommandItem
    {
        CommandItem()
        :fd_in(STDIN_FILENO)
        ,fd_out(STDOUT_FILENO)
         ,redir(NONE)
        ,append(false)
        ,background(false)
        {}

        int fd_in;
        int fd_out;
        std::string file_in;
        std::string file_out;
        int redir;
        bool append;
        bool background;
        std::vector<std::string> args;
    };

    struct CommandLine
    {
        CommandLine()
        {
            commands.emplace_back(CommandItem());
        }

        void reset()
        {
            commands.clear();
            commands.emplace_back(CommandItem());
        }
        std::vector<CommandItem> commands;
    };

    CommandLine commandLine;

    void Invoke(Machine& machine, CommandLine& cl, pid_t& waitPid)
    {
        int idx = commandLine.commands.size()-1; 

        waitPid = -1;
        while(idx >= 0)
        {
            CommandItem& cmd = commandLine.commands[idx];
            if (cmd.args[0] == "cd")
            {
                if (cmd.args.size() == 1)
                    return;

               if (chdir(cmd.args[1].c_str()) != 0)
                   std::cout << "cd: " << cmd.args[1] << ": " << strerror(errno) << std::endl; 
            }
            else if (cmd.args[0] == "exit")
            {
                machine.SetProperty("shellExit", 1);
            }
            else if (cmd.args[0] == "stack")
            {
                int depth(4);
                if (cmd.args.size() > 1)
                    depth = std::strtol(cmd.args[1].c_str(), nullptr, 10);
                VIEW(machine, depth);
            }
            else
            {
                pid_t pid = fork();
                switch(pid)
                {
                case -1:
                    throw std::runtime_error("fork() failed");
                    break;
                case 0: //child
                    if (IsPipe(cmd.redir))
                    {
                        if ((cmd.redir & PIPE_IN) == PIPE_IN)
                        {
                            assert(cmd.fd_in != STDIN_FILENO);
                            dup2(cmd.fd_in, STDIN_FILENO);
                        }
                        if ((cmd.redir & PIPE_OUT) == PIPE_OUT)
                        {
                            assert(cmd.fd_out != STDOUT_FILENO);
                            dup2(cmd.fd_out, STDOUT_FILENO);
                        }
                        for (size_t n = 0; n < commandLine.commands.size(); ++n)
                        {
                            if (commandLine.commands[n].fd_in != STDIN_FILENO)
                                close(commandLine.commands[n].fd_in);
                            if (commandLine.commands[n].fd_out != STDOUT_FILENO)
                                close(commandLine.commands[n].fd_out);
                        }
                    }
                    if (IsFileOut(cmd.redir))
                    {
                        int flags = O_WRONLY | O_CREAT;
                        if (cmd.append)
                            flags |= O_APPEND;
                        else
                            flags |= O_TRUNC;
                        cmd.fd_out = open(cmd.file_out.c_str(), flags, PERM_FILE);
                        if (cmd.fd_out < 0)
                        {
                            std::cout << "Unable to open " << cmd.file_out << " for writing, err: " << strerror(errno) << std::endl;
                            _exit(EXIT_FAILURE);
                        }
                        if (dup2(cmd.fd_out, STDOUT_FILENO) < 0)
                        {
                            std::cout << "dup2 failed" << std::endl;
                            _exit(EXIT_FAILURE);
                        }
                        close(cmd.fd_out);
                    }
                    if (IsFileIn(cmd.redir))
                    {
                        cmd.fd_in = open(cmd.file_in.c_str(), O_RDONLY);
                        if (cmd.fd_in < 0)
                        {
                            std::cout << "Unable to open \"" << cmd.file_in << "\" for reading, err: " << strerror(errno) << std::endl;
                            _exit(EXIT_FAILURE);
                        }
                        if (dup2(cmd.fd_in, STDIN_FILENO) < 0)
                        {
                            std::cout << "dup2 failed" << std::endl;
                            _exit(EXIT_FAILURE);
                        }
                        close(cmd.fd_in);
                    }
                    const char *argv[cmd.args.size()+1];
                    for (size_t n = 0; n < cmd.args.size(); ++n)
                    {
                        //std::cout << "=== cmd[" << n << "]: " << cmd.args[n] << std::endl;
                        argv[n] = cmd.args[n].c_str();
                    }
                    argv[cmd.args.size()] = '\0';
                    const char *cmdname = strrchr(argv[0], '/');
                    if (cmdname == nullptr)
                        cmdname = argv[0];
                    else
                        cmdname++;
                    const char *cmdpath = argv[0];
#if defined(FD_CHECK)
                    fd_check();
#endif
                    execvp(cmdpath, (char *const *)argv);
                    std::cout << "Cannot execute " << cmdpath << std::endl;
                    _exit(EXIT_FAILURE);
                    break;
                }
                //parent
                if (waitPid == -1)
                    waitPid = pid;
                if (cmd.fd_in != STDIN_FILENO)
                    close(cmd.fd_in);
                if (cmd.fd_out != STDOUT_FILENO)
                    close(cmd.fd_out);
            }
            --idx;
        }
    }

    //TODO: Need support for stderror
    void PushWord(Machine& machine, const char *w)
    {
        CommandItem& cmd = commandLine.commands.back();
        if (IsFileIn(cmd.redir))
            cmd.file_in = w;
        else if (IsFileOut(cmd.redir))
            cmd.file_out = w;
        else 
        {
            wordexp_t p;
            char **exp_words;
            int i;

            wordexp(w, &p, 0);
            exp_words = p.we_wordv;
            for (i = 0; i < p.we_wordc; i++)
                commandLine.commands.back().args.emplace_back(exp_words[i]);
            wordfree(&p);
        }
    }

    void PushBar(Machine& machine)
    {
        CommandItem& cmd = commandLine.commands.back();
        if (cmd.fd_out < 0)
            throw std::runtime_error("Output already redirected");
        int pfd[2];
        pipe(pfd);
        cmd.fd_out = pfd[1];
        cmd.redir |= PIPE_OUT;
        commandLine.commands.emplace_back(CommandItem());
        CommandItem& cmd2 = commandLine.commands.back();
        cmd2.fd_in = pfd[0];
        cmd2.redir |= PIPE_IN;
    }

    void PushLT(Machine& machine)
    {
        CommandItem& cmd = commandLine.commands.back();
        if (cmd.fd_in < 0)
            throw std::runtime_error("Input already redirected");
        cmd.redir |= FILE_IN;
        cmd.fd_in = -1;
    }

    void PushGT(Machine& machine)
    {
        CommandItem& cmd = commandLine.commands.back();
        if (cmd.fd_out < 0)
            throw std::runtime_error("Output already redirected");
        cmd.redir |= FILE_OUT;
        cmd.fd_out = -1;
    }

    void PushGTGT(Machine& machine)
    {
        CommandItem& cmd = commandLine.commands.back();
        if (cmd.fd_out < 0)
            throw std::runtime_error("Output already redirected");
        cmd.redir |= FILE_OUT;
        cmd.fd_out = -1;
        cmd.append = true;
    }

    void PushAmp(Machine& machine)
    {
        CommandItem& cmd = commandLine.commands.back();
        cmd.background = true;
        std::cout << "Background not supported" << std::endl;
        commandLine.reset();
    }

    void PushSemi(Machine& machine)
    {
        pid_t w;
        Invoke(machine, commandLine, w);
        commandLine.reset();
    }

    void PushNL(Machine& machine)
    {
        pid_t w;
        if (commandLine.commands[0].args.size() == 0)
            return;
        Invoke(machine, commandLine, w);
        wait_and_display(w);
        commandLine.reset();
    }
}

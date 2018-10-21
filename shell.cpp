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
#include "parser.h"
#include "utilities.h"

#define PERM_FILE		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define FD_CHECK

namespace rps
{

    void stack(Machine&, std::vector<std::string>& args);
    void clrstk(Machine&, std::vector<std::string>& args);
    void echo(Machine&, std::vector<std::string>& args);
    void swap(Machine&, std::vector<std::string>& args);
    void drop(Machine&, std::vector<std::string>& args);
    void dropn(Machine&, std::vector<std::string>& args);
    void roll(Machine&, std::vector<std::string>& args);
    void rolld(Machine&, std::vector<std::string>& args);
    void pick(Machine&, std::vector<std::string>& args);
    void get(Machine&, std::vector<std::string>& args);
    void exit(Machine&, std::vector<std::string>& args);
    void pwd(Machine&, std::vector<std::string>& args);
    void cd(Machine&, std::vector<std::string>& args);
    void dup(Machine&, std::vector<std::string>& args);
    void fromlist(Machine&, std::vector<std::string>& args);
    void tolist(Machine&, std::vector<std::string>& args);
    void reverse(Machine&, std::vector<std::string>& args);

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
#define IsListIn(x) ((x&LIST_IN)!=0)

    enum redir_t {NONE = 0
                , PIPE_IN  = 0x01
                , PIPE_OUT = 0x02
                , FILE_IN = 0x04
                , FILE_OUT = 0x08
                , LIST_IN = 0x10
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
            if (IsListIn(cmd.redir))
            {
                --idx;
                continue;
            }
            if (cmd.args[0] == "cd")
                cd(machine, cmd.args);
            else if (cmd.args[0] == "pwd")
                pwd(machine, cmd.args);
            else if (cmd.args[0] == "exit")
                exit(machine, cmd.args);
            else if (cmd.args[0] == "drop")
                drop(machine, cmd.args);
            else if (cmd.args[0] == "dropn")
                dropn(machine, cmd.args);
            else if (cmd.args[0] == "roll")
                roll(machine, cmd.args);
            else if (cmd.args[0] == "rolld")
                rolld(machine, cmd.args);
            else if (cmd.args[0] == "pick")
                pick(machine, cmd.args);
            else if (cmd.args[0] == "swap")
                swap(machine, cmd.args);
            else if (cmd.args[0] == "get")
                get(machine, cmd.args);
            else if (cmd.args[0] == "echo")
                echo(machine, cmd.args);
            else if (cmd.args[0] == "stack")
                stack(machine, cmd.args);
            else if (cmd.args[0] == "dup")
                dup(machine, cmd.args);
            else if (cmd.args[0] == "clrstk")
                clrstk(machine, cmd.args);
            else if (cmd.args[0] == "fromlist")
                fromlist(machine, cmd.args);
            else if (cmd.args[0] == "tolist")
                tolist(machine, cmd.args);
            else if (cmd.args[0] == "reverse")
                reverse(machine, cmd.args);
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
        if (strcmp(w, "\\") == 0)
        {
            // wordexp will crash if a '\' is passed with a char
            // to be escaped
            return;
        }
        //std::cout << "=== PushWord: " << w << std::endl;
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
            {
                //std::cout << "=== PushWord exp_word: " << exp_words[i] << std::endl;
                commandLine.commands.back().args.emplace_back(exp_words[i]);
            }
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

    void PushBang(Machine& machine)
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
        cmd2.redir |= LIST_IN;

        pid_t w;
        Invoke(machine, commandLine, w);

        FILE *fp = fdopen(cmd2.fd_in, "r");
        if (fp == nullptr)
            std::cout << "! operator, " <<  strerror(errno) << std::endl;
        if (fp)
        {
            ListPtr ret = MakeList();
            char buf[10240];
            int64_t limit(1000000);
            for (int count = 0; count < limit && !feof(fp); ++count)
            {
                if (bInterrupt)
                    break;
                if (fgets(buf, sizeof(buf), fp))
                {
                    size_t l = strlen(buf);
                    if (buf[l-1] == '\n')
                        buf[l-1] = '\0';
                    StringPtr sp = MakeString();
                    sp->set(buf);
                    ret->items.push_back(sp);
                }
            }
            machine.push(ret);
        }
        wait_and_display(w);
        if (fp)
            fclose(fp);
        commandLine.reset();
        VIEW(machine, 4);
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
        wait_and_display(w);
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

    void cd(Machine& machine, std::vector<std::string>& args)
    {
        if (args.size() == 1)
            return;

       if (chdir(args[1].c_str()) != 0)
           std::cout << "cd: " << args[1] << ": " << strerror(errno) << std::endl; 
    }

    void pwd(Machine& machine, std::vector<std::string>& args)
    {
        char *p = getcwd(nullptr, 0);
        std::cout << p << std::endl;
        free(p);
    }

    void exit(Machine& machine, std::vector<std::string>& args)
    {
        machine.SetProperty("rpsExit", 1);
        machine.SetProperty("shellExit", 1);
    }

    void drop(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            DROP(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void dropn(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            if (args.size() == 1)
            {
                std::cout << "usage: dropn <int>" << std::endl;
                return;
            }
            int64_t n = std::strtoull(args[1].c_str(), nullptr, 10);
            machine.push(n);
            DROPN(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void roll(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            if (args.size() == 1)
            {
                std::cout << "usage: roll <int>" << std::endl;
                return;
            }
            int64_t n = std::strtoull(args[1].c_str(), nullptr, 10);
            machine.push(n);
            ROLL(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void pick(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            if (args.size() == 1)
            {
                std::cout << "usage: pick <int>" << std::endl;
                return;
            }
            int64_t n = std::strtoull(args[1].c_str(), nullptr, 10);
            machine.push(n);
            PICK(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void rolld(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            if (args.size() == 1)
            {
                std::cout << "usage: rolld <int>" << std::endl;
                return;
            }
            int64_t n = std::strtoull(args[1].c_str(), nullptr, 10);
            machine.push(n);
            ROLLD(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void swap(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            SWAP(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void get(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            if (args.size() == 1)
            {
                std::cout << "usage: get <int>" << std::endl;
                return;
            }
            int64_t n = std::strtoull(args[1].c_str(), nullptr, 10);
            machine.push(n);
            GET(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void dup(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            DUP(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void fromlist(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            FROMLIST(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void tolist(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            if (args.size() == 1)
            {
                std::cout << "usage: tolist nitems" << std::endl;
            }
            int64_t n = strtoll(args[1].c_str(), nullptr, 10);
            machine.push(n);
            TOLIST(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void reverse(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            REVERSE(machine);
            VIEW(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void clrstk(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            CLRSTK(machine);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void echo(Machine& machine, std::vector<std::string>& args)
    {
        try
        {
            for (int i = 1; i < args.size(); ++i)
            {
                std::cout << args[i] << " ";
            }
            std::cout << std::endl;
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }

    void stack(Machine& machine, std::vector<std::string>& args)
    {
        int depth(4);
        if (args.size() > 1)
            depth = std::strtol(args[1].c_str(), nullptr, 10);
        try
        {
            VIEW(machine, depth);
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }
}

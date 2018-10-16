CC=gcc
CDEBUG = -g -O0
CPPFLAGS = $(CDEBUG) -I.
LDFLAGS=-g
LIBS = -lstdc++ -lreadline
DEPS=machine.h object.h module.h parser.h token.h commands.h utilities.h shell.h

SRC	= main.cpp machine.cpp object.cpp module.cpp parser.cpp math_commands.cpp variables_commands.cpp stack_commands.cpp control_commands.cpp utilities.cpp list_commands.cpp logical_commands.cpp functional_commands.cpp io_commands.cpp string_commands.cpp type_commands.cpp execution_commands.cpp environment_commands.cpp shell.cpp

OBJS	= $(SRC:.cpp=.o) 

rps : $(OBJS) $(DEPS)
	$(CC) $(CPPFLAGS) -o $@ $(OBJS) $(LIBS)

clean: 
	rm $(OBJS) rps 


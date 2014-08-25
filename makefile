CC=g++
CFLAGS=-g -Wall -Werror -std=c++11 -I.
EXECUTIBLE=clisp
SOURCES=main.cpp parser.cpp lexer.cpp error.cpp environment.cpp
# replace all appearance of .cpp with .o
OBJECTS=$(SOURCES:.cpp=.o)

all: $(EXECUTIBLE)

# $@ is automatic variable for target name
$(EXECUTIBLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -c 

clean:
	rm -rf *o clisp

test: $(EXECUTIBLE)
	valgrind -q --track-origins=yes ./$(EXECUTIBLE)

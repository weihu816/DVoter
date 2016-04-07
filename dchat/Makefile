EXE = dchat

CC	= g++

HEADERS		= BlockingQueue.h DNet.h DNode.h Handler.h Member.h stdincludes.h

C_SRCS		= dchat.cpp DNet.cpp DNode.cpp Handler.cpp Member.cpp

C_OBJS		= dchat.o DNet.o DNode.o Handler.o Member.o

OBJS		= ${C_OBJS}

#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CC_FLAGS	= -std=c++11 -g -pthread -Wall
LD_FLAGS	= -std=c++11 -g -pthread -Wall

.cpp.o:
	@echo "Compiling each source file separately ..."
	$(CC) $(CC_FLAGS) $<
	@echo ""

#
# Default executable
#
all:
	g++ -std=c++11 -pthread -g -Wall *.cpp

dchat: $(EXE)

#
# executable
#
$(EXE):	$(OBJS)
	@echo "Linking all object modules ..."
	$(CC) -o $(EXE) $(LD_FLAGS) $(OBJS)
	@echo "Done."

${C_OBJS}: ${HEADERS}


#
# Test executable
#
# change-password-tcp:	change-password-tcp.c
# 	@echo "Compiling change-password-tcp.c"
# 	$(CC) -o change-password-tcp change-password-tcp.c
# 	@echo "Done."


#
# clean and new
#
clean:
	@echo "Cleaning up project directory ..."
	rm -rf *.o $(EXE) *.ln core a.out *.dSYM
	@echo ""
	@echo "Clean."

new:
	make clean
	make
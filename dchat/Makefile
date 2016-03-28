EXE = dchat

GCC	= gcc

#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
GCC_FLAGS	= -c -g -Wall
LD_FLAGS	= -g -Wall

#
# Default executable
#
dchat: $(EXE)

#
# Server executable
#
server-tcp:	xxx.o xxx.h
	@echo "Linking all object modules ..."
	$(GCC) -o server-tcp $(LD_FLAGS) server-tcp.o
	@echo ""
	@echo "Done."

#
# Client executable
#
# change-password-tcp:	change-password-tcp.c
# 	@echo "Compiling change-password-tcp.c"
# 	$(GCC) -o change-password-tcp change-password-tcp.c
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
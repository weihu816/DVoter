EXE = dchat

GCC	= gcc

#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
GCC_FLAGS	= -std=c++11 -g -Wall
LD_FLAGS	= -g -Wall

#
# Default executable
#
dchat: $(EXE)

#
# executable
#
dchat:	xxx.o xxx.h
	@echo "Linking all object modules ..."
	$(GCC) -o dchat $(LD_FLAGS) dchat.o
	@echo ""
	@echo "Done."

#
# Test executable
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
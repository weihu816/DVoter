#
# Default executable
#
dchat:
	g++ -std=c++11 -pthread -g -Wall *.cpp -o dchat

#
# clean and new
#
clean:
	@echo "Cleaning up project directory ..."
	rm -rf *.o $(EXE) *.ln core dchat *.dSYM
	@echo ""
	@echo "Clean."

new:
	make clean
	make

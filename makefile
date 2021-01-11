#Edward Brace Slime Volleyball
#makefile assumes compiling on Mac, with SDL2.framework in /Library/Frameworks/ directory
#modify framework path as required
targets: main.cpp
	g++ -o main main.cpp -F/Library/Frameworks/ -framework SDL2
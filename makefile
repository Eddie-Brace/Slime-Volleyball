#Edward Brace Slime Volleyball
#makefile assumes compiling on Mac, with SDL2.framework & SDL2_ttf.framework in /Library/Frameworks/ directory
#modify framework path as required
targets: main.cpp
	g++ -o main main.cpp -F/Library/Frameworks/ -framework SDL2 -F/Library/Frameworks/ -framework  SDL2_ttf
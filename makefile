all: main.cpp
	g++ main.cpp -std=c++17 -O0 -ggdb -o main -lSDL2 -mavx2
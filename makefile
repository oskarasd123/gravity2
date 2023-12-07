all: main.cpp
	g++ main.cpp -std=c++17 -Ofast -ggdb -o main -lSDL2 -mavx2 -mfma -Wall -Wextra -pedantic
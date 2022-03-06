all: compile link

compile:
	g++ -I src -c main.cpp

link:
	g++ main.o -o main -L src/lib -lsfml-graphics -lsfml-window -lsfml-system

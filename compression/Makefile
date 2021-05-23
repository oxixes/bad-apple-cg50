CXX=g++
CC=gcc

all: compress.exe

compress.exe: main.o fastlz.o
	$(CXX) -o compress.exe main.o fastlz.o

main.o: main.cpp
	$(CXX) -c main.cpp

fastlz.o: fastlz.c
	$(CC) -c fastlz.c

clean:
	del main.o fastlz.o compress.exe

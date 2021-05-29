all: main.cpp
	g++ -o a main.cpp -lm -lGL -lX11 -lpng -lpthread -lasound
	./a

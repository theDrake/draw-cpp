all: draw

draw: src/*
	g++ src/*.cc -lglut -lGL -lGLU -o draw

.PHONY: all clean

clean:
	rm -f draw

CC = /usr/bin/g++
CFLAGS = -lm -pthread -O3 -march=native -lgsl -lm -lgslcblas  -I/opt/local/include -L/opt/local/lib
INCLUDES = -I/usr/local/include
LIBS = -L/usr/local/lib

all: pte

pte : ransampl.o linelib.o main.o
	$(CC) $(CFLAGS) -o pte ransampl.o linelib.o main.o $(INCLUDES) $(LIBS)

ransampl.o : ransampl.c
	$(CC) $(CFLAGS) -c ransampl.c $(INCLUDES) $(LIBS)

linelib.o : linelib.cpp ransampl.h
	$(CC) $(CFLAGS) -c linelib.cpp $(INCLUDES) $(LIBS)

main.o : main.cpp linelib.o
	$(CC) $(CFLAGS) -c main.cpp $(INCLUDES) $(LIBS)

clean :
	rm -rf *.o pte

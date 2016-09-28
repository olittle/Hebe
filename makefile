CC = /usr/bin/g++
CFLAGS = -lgsl -lgslcblas -DHAVE_INLINE -pthread -Wall -O3

all: tensor2vec distance pair_distance

distance : distance.c 
	$(CC) distance.c -o distance $(CFLAGS)

pair_distance : pair_distance.c
	$(CC) pair_distance.c -o pair_distance $(CFLAGS)

tensor2vec : tensor2vec.c
	$(CC) tensor2vec.c -o tensor2vec $(CFLAGS)

clean:
	rm -rf tensor2vec pair_distance distance

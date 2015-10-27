all: my_calls.o bbfs.o log.o
	g++ -g `pkg-config fuse --cflags --libs` bbfs.o log.o my_calls.o -o my_fs.out

my_calls.o: my_calls.cc my_calls.h
	g++ my_calls.cc -c

bbfs.o: bbfs.c
	gcc -g `pkg-config fuse --cflags --libs` bbfs.c -c

log.o: log.c
	gcc -g `pkg-config fuse --cflags --libs` log.c -c

clean:
	rm *.o *.out *.gch

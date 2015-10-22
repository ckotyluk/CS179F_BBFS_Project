all: my_calls.o bbfs.o
	g++ -g `pkg-config fuse --cflags --libs` bbfs.o log.o my_calls.o -o my_fs.out

my_calls.o: my_calls.cc my_calls.h
	g++ my_calls.cc -c
	@echo "Built my_calls.o"

bbfs.o: bbfs.c
	gcc -g `pkg-config fuse --cflags --libs` bbfs.c log.c -c
	@echo "Built bbfs.o"

clean:
	rm *.o *.out *.gch

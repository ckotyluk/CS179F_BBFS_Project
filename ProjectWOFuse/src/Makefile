all:
	g++ -std=c++11 -g my_stubs.cc -D HAVE_SYS_XATTR_H -o my_fs.out

run: all
	./my_fs.out

clean:
	rm *.out *.gch

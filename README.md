CS179F File Systems Project
====================

How to compile code in src2 directory:
>```g++ -std=c++11 -g my_stubs.cc -o my_fs.out```  

How to compile bbfs code (in src directory):
>```gcc -g `pkg-config fuse --cflags --libs` bbfs.c log.c my_calls.c -o bbfs```  

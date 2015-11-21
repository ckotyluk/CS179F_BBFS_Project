CS179F File Systems Project with FUSE
====================

We were having issues with getting FUSE to run our system calls. We stopped our development with FUSE and are leaving these files here so we can add FUSE to the project at a later date.

How to compile bbfs code (in src directory):
```
gcc -g `pkg-config fuse --cflags --libs` bbfs.c log.c my_calls.c -o bbfs
```  
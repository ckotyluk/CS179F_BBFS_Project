CS179F File Systems Project without FUSE
====================

We were having issues with getting FUSE to run our system calls. Due to time contraints, we decided to continue developing the project without FUSE. 

How to compile code in src directory:
```
source /opt/rh/devtoolset-2/enable
g++ -std=c++11 -g my_stubs.cc -o my_fs.out
```  
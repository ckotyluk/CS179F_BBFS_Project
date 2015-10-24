#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <utime.h>
#include "my_calls.h"
#include <string>
#include <iostream>
#include <vector>
#include <map>

//inode functions
unsigned long inode_init(mode_t i_m)
{
	//Create new inode
	my_inode in;
	
	unsigned long max_inode = 0;
	
	for(std::map<unsigned long, my_inode>::iterator it = my_ilist.begin(); it != my_ilist.end(); ++it)
	{
		if(it->second.i_ino > max_inode)
		{
			max_inode = it->second.i_ino;
		}
	}
	
	in.i_ino = max_inode+1;
	
	in.i_mode = i_m;
	//in.i_size = 0;

	//Add to ilist
    my_ilist.insert( std::pair<unsigned long, my_inode>(in.i_ino, in) );
    
    return in.i_ino;
}

int my_access(const char *pathname, int mode)
{
	return -1;
}

int my_chmod(const char *path, mode_t mode)
{
	return -1;
}

int my_chown(const char *path, uid_t owner, gid_t group)
{
	return -1;
}

int my_close(int fd)
{
	return -1;
}

int my_closedir(DIR *dirp)
{
	return -1;
}

//added function toverify the validity of the
//path to a file or a directory
//bool is_valid(string fullpath){
	/*vector<string> folders;	
	istringstream ss(fullpath);
	string token;
	while (getline(ss,token, '/')){
		folders.push_back(token);
	}*/
	//return (dir_table[fullpath]) ? true : false;
//}

int my_creat(const char *pathname, mode_t mode)
{
	//Create and initialize new inode
	unsigned long inum = inode_init(mode);

	//split pathname

	//grab the root inode number which is 0

	//find the corresponding inode from the ilist using the inode number

	//look up the vector of dirents and find the dirent

	//use the inode number in that dirent to find its corr. inode in ilist

	//when we have the last dirent push the new_dirent onto it
	my_dirent new_dirent;
	new_dirent.d_ino = inum;
	//new_dirent.d_name = fname;
	//parentdir_inode.dirent_buf.push_back(new_dirent);

	return inum;
}

int my_fdatasync(int fd)
{
	return -1;
}

int my_fstat(int fd, struct stat *buf)
{
	return -1;
}

int my_fsync(int fd)
{
	return -1;
}

int my_ftruncate(int fd, off_t length)
{
	return -1;
}

ssize_t my_lgetxattr(const char *path, const char *name, void *value, size_t size)
{
	return -1;
}

int my_link(const char *oldpath, const char *newpath)
{
	return -1;
}

ssize_t my_llistxattr(const char *path, char *list, size_t size)
{
	return -1;
}

int my_lremovexattr(const char *path, const char *name)
{
	return -1;
}

int my_lsetxattr(const char *path, const char *name, const void *value, size_t size, int flags)
{
	return -1;
}

int my_lstat(const char *path, struct stat *buf)
{
	return -1;
}

int my_mkdir(const char *pathname, mode_t mode)
{
	return -1;
}

int my_mkfifo(const char *pathname, mode_t mode)
{
	return -1;
}

int my_mknod(const char *pathname, mode_t mode, dev_t dev)
{
	return -1;
}

int my_open(const char *pathname, int flags)
{
	return -1;
}

int my_open(const char *pathname, int flags, mode_t mode)
{
	return -1;
}

DIR *my_opendir(const char *name)
{
	//return -1;
}

ssize_t my_pread(int fd, void *buf, size_t count, off_t offset)
{
	return -1;
}

ssize_t my_pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	return -1;
}

struct dirent *my_readdir(DIR *dirp)
{
	//return -1;
}

ssize_t my_readlink(const char *path, char *buf, size_t bufsiz)
{
	return -1;
}

int my_rename(const char *oldpath, const char *newpath)
{
	return -1;
}

int my_rmdir(const char *pathname)
{
	return -1;
}

int my_statvfs(const char *path, struct statvfs *buf)
{
	return -1;
}

int my_symlink(const char *target, const char *linkpath)
{
	return -1;
}

int my_truncate(const char *path, off_t length)
{
	return -1;
}

int my_unlink(const char *pathname)
{
	return -1;
}

int my_utime(const char *filename, const struct utimbuf *times)
{
	return -1;
}



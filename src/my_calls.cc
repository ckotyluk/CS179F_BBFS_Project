#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <utime.h>
#include "my_calls.h"
#include <string>
#include <iostream>
#include <vector>
//#include "my_file.h"


//inode functions
void inode_init(my_inode &in, mode_t i_m)
{
	in.i_ino = ilist.size();
	in.i_mode = i_m;
	in.i_size = 0;

	//Last thing to do
	ilist.push_back(in);
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
	/*string pth(pathname);
	if (pth [0] != '/')
		return -1;
	
	int pos = pth.find_last_of("/");
	string fname = pth.substr(pos+1);
	string rpath = pth.substr(0, pos);
	
	if !is_valid(rpath)
		return -1;

	my_file new_file = {
		.data = new char[1024],
		.fh = file_table.size(),
		.file_name = fname
	};
	
	my_dir parent_dir = dir_table[rpath];
	//pushing inside the file list of the current working directory
	
	parent_dir.file_list.push_back(new_file);
	//pushing into the table of files
	file_table[new_file.fh] = new_file;
    */

	//Create new inode
	my_inode a;
	//Initialize inode
	inode_init(a, mode);

	//Create dirent for inode
	//Initialize dirent
	//Add dirent to parent directory

    return -1;
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



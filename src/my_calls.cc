#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <utime.h>
#include "my_calls.h"
#include <string>
#include <iostream>
//#include "my_file.h"

int my_access(const char *pathname, int mode)
{

}

int my_chmod(const char *path, mode_t mode)
{
	
}

int my_chown(const char *path, uid_t owner, gid_t group)
{
	
}

int my_close(int fd)
{
	
}

int my_closedir(DIR *dirp)
{
	
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
}

int my_fdatasync(int fd)
{
	
}

int my_fstat(int fd, struct stat *buf)
{
	
}

int my_fsync(int fd)
{
	
}

int my_ftruncate(int fd, off_t length)
{
	
}

ssize_t my_lgetxattr(const char *path, const char *name, void *value, size_t size)
{
	
}

int my_link(const char *oldpath, const char *newpath)
{
	
}

ssize_t my_llistxattr(const char *path, char *list, size_t size)
{
	
}

int my_lremovexattr(const char *path, const char *name)
{
	
}

int my_lsetxattr(const char *path, const char *name, const void *value, size_t size, int flags)
{
	
}

int my_lstat(const char *path, struct stat *buf)
{
	
}

int my_mkdir(const char *pathname, mode_t mode)
{
	
}

int my_mkfifo(const char *pathname, mode_t mode)
{
	
}

int my_mknod(const char *pathname, mode_t mode, dev_t dev)
{
	
}

int my_open1(const char *pathname, int flags)
{
	
}

int my_open2(const char *pathname, int flags, mode_t mode)
{
	
}

DIR *my_opendir(const char *name)
{
	
}

ssize_t my_pread(int fd, void *buf, size_t count, off_t offset)
{
	
}

ssize_t my_pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	
}

struct dirent *my_readdir(DIR *dirp)
{
	
}

ssize_t my_readlink(const char *path, char *buf, size_t bufsiz)
{
	
}

int my_rename(const char *oldpath, const char *newpath)
{
	
}

int my_rmdir(const char *pathname)
{
	
}

int my_statvfs(const char *path, struct statvfs *buf)
{
	
}

int my_symlink(const char *target, const char *linkpath)
{
	
}

int my_truncate(const char *path, off_t length)
{
	
}

int my_unlink(const char *pathname)
{
	
}

int my_utime(const char *filename, const struct utimbuf *times)
{
	
}



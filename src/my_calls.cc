#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <utime.h>
//#include "my_calls.h"
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <string.h>

#ifdef _cplusplus
extern "C" {
#endif

#include "my_calls.h"

struct my_inode
{
    //dev_t           i_dev;      // device number
    unsigned long   i_ino;      // inode number
    mode_t          i_mode;     // mode(protection bits) also explains file type
    //nlink_t         i_nlink;    // number of hard links
    //uid_t           i_uid;      // user id of owner
    //gid_t           i_gid;      // group id of owner
    //dev_t           i_rdev;     // device ID (if special file)
    //off_t           i_size;     // total size of file, in bytes
    //unsigned long   i_blksize;  // block size
    //unsigned long   i_blocks;   // blocks
    //time_t          i_atime;    // time of last access
    //time_t          i_mtime;    // time of last modification
    //time_t          i_ctime;    // time of last status change
    
    std::vector<char>    buf;   //File array (later convert to blocks)
    std::vector<my_dirent> dirent_buf;
	my_inode(unsigned long x, mode_t y) : i_ino(x), i_mode(y) {}
};

std::map<unsigned long, my_inode> my_ilist;

//Helper Functions
std::vector<std::string> split(const std::string s, const std::string pat)
{
	std::string c;
	std::vector<std::string> v;
	int i = 0;

	for(;;)
	{
		int t = s.find(pat,i); //Finds ith pat in s
		//If end of string, j = s.size(), otherwise j=t
		int j = ( ( t == std::string::npos) ? s.size() : t);

		//Create a string from i to j-i
		std::string c = s.substr(i,j-i);
		//Add this string to the vector
		if(!c.empty())
			v.push_back(c);
		
		i = j+pat.size();

		//If the end of the string is reached, return v
		if( t == std::string::npos) return v;
	}
}

//inode functions
unsigned long inode_init(mode_t i_m)
{
	//Create new inode
	//my_inode in;
	
	//Find the current highest inode value used
	unsigned long max_inode = 0;
	for(std::map<unsigned long, my_inode>::iterator it = my_ilist.begin(); it != my_ilist.end(); ++it)
	{
		if(it->second.i_ino > max_inode)
		{
			max_inode = it->second.i_ino;
		}
	}
	
	//Create a new inode with max+1 inode number and i_m mode
	my_inode in(max_inode+1, i_m);

	//Add to inode to the ilist
    my_ilist.insert( std::pair<unsigned long, my_inode>(in.i_ino, in) );
    
    //Return the inode number
    return in.i_ino;
}

//my_inode root_inode(0, 222);
//my_ilist.insert( std::pair<unsigned long, my_inode>(0, root_inode) );


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

	//Splits the string into seperate strings at "/"
	//Last string is the file name, others are directories
	std::vector<std::string> dir_names = split(pathname, "/");

	//Grab filename and remove from list of directory names
	std::string fname = dir_names.at(dir_names.size()-1);
	dir_names.pop_back();
	//Adds filename to new dirent
	strcpy(new_dirent.d_name, fname.c_str());

	//Get root inode, assuming root inum is 0
	my_inode root = (my_ilist.find(0)->second);

	if(dir_names.size() == 0)
	//dir_names is empty, so parent dir is root(/)
	{
		root.dirent_buf.push_back(new_dirent);
	}
	else
	{
		my_inode temp = root;
		std::vector<std::string>::iterator it;

		//Go through list of dir_names
		for(it = dir_names.begin(); it != dir_names.end(); it++)
		{
			//Look through current directory dirents for current dir_name
			unsigned long inum_temp = -1;
			for(int i = 0; i < temp.dirent_buf.size(); i++)
			{
				//Compare cur dir_name to dirent names in dirent_buf
				if( std::string(*it) == std::string(temp.dirent_buf[i].d_name) )
				{
					//Found dirent, so set inum and break
					inum_temp = temp.dirent_buf[i].d_ino;
					break;
				}
			}
			if(inum_temp == -1)
			{
				//A matching dirent was not found, so inum_temp was never set
				std::cout << "The path: " << pathname << " is invalid" << std::endl;
				return -1; //Error ocurred
			}

			//Assign temp to the new inode based on the inum_temp
			temp = my_ilist.find(inum_temp)->second;
		}

		//Finished traversing the path, parent dir found and stored in temp
		//temp.dirent_buf.push_back(new_dirent);
		//Make changes to the inode that is stored in the ilist
		my_ilist.find(temp.i_ino)->second.dirent_buf.push_back(new_dirent);
	}
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

#ifdef _cplusplus
}
#endif



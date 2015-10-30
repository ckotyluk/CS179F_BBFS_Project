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

extern "C" {

#include "my_calls.h"

//Struct which stores information and the data of the file
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

//Struct for an open file
struct my_file
{
	unsigned long	f_desc;			// File descriptor
	mode_t 			f_mode;			// Permissions modes for the file
	unsigned long	f_pos;			// Current position(offset) in the file
	unsigned short 	flags;			// Access flags for the file
	unsigned short 	f_count;		// Access count

	struct my_inode *f_inode;		// Pointer to the files inode
	my_file(unsigned long fd, mode_t m, unsigned long pos, unsigned short fl, struct my_inode* i) 
		: f_desc(fd), f_mode(m), f_pos(pos), flags(fl), f_count(0), f_inode(i) {}
};

std::map<unsigned long, my_inode> my_ilist;

std::map<unsigned long, my_file> my_openFT; // Open file table

//Helper Functions
//---------------------------------------------------------------------
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

long get_inode_number(char* pathname){
	std::vector<std::string> dir_names = split(pathname, "/");
	
	//my_inode root = my_ilist.find(0)->second;

	//gets the root inode and return -1 if not found
	
	if(my_ilist.find(0) == my_ilist.end())
		return -1;
	my_inode root = my_ilist.find(0)->second;
	
	//when the dir_names is empty / was the pathname
	if(dir_names.size() == 0)
		return root.i_ino;
	
	else{
		my_inode temp = root;
		std::vector<std::string>::iterator it;
		
		for(it = dir_names.begin(); it != dir_names.end(); it++){
			long inum_temp = -1;
			
			for(int i = 0; i < temp.dirent_buf.size(); i++){
				
				if(std::string (*it) == std::string(temp.dirent_buf[i].d_name)){
					
					inum_temp = temp.dirent_buf[i].d_ino;
					break;
				}
			}
			if(inum_temp == -1){
				
				std::cout << "The path: " << pathname << " is invalid" <<std::endl;
				return -1;
			}

			//temp = my_ilist.find(inum_temp)->second;
			//Update to the current inode, return -1 if not found
			if( my_ilist.find(inum_temp) == my_ilist.end() )
				return -1;
			temp = my_ilist.find(inum_temp)->second;
		}
		return temp.i_ino;
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

unsigned long file_init(mode_t m, unsigned long pos, unsigned short flags, my_inode* i)
{
	
	//Find the current highest fd value used
	unsigned long max_fd = 0;
	for(std::map<unsigned long, my_file>::iterator it = my_openFT.begin(); it != my_openFT.end(); ++it)
	{
		if(it->second.f_desc > max_fd)
		{
			max_fd = it->second.f_desc;
		}
	}

	//Create new file
	my_file fi(max_fd+1, m, pos, flags, i);

	//Add the open file to the open file table
	my_openFT.insert(std::pair<unsigned long,my_file>(fi.f_desc,fi));

	//Return the file descriptor
	return fi.f_desc;
}

//END OF HELPER FUNCTIONS
//---------------------------------------------------------------------------

int my_access(const char *pathname, int mode)
{
	return -1;
}

//Changes the mod of the file by modifing the inode
int my_chmod(const char *path, mode_t mode)
{
	unsigned long i_num;
	
	//Get the inode number and return -1 if invalid
	if ( (i_num = get_inode_number((char*)path)) == -1)
		return 0;

	//Set the inode mode to the passed in mode
	my_ilist.find(i_num)->second.i_mode = (mode_t)mode;
	return 0;
}

int my_chown(const char *path, uid_t owner, gid_t group)
{
	return -1;
}

int my_close(int fd)
{
	std::map<unsigned long, my_file>::iterator it = my_openFT.find((unsigned long)fd);
	if (it == my_openFT.end())
		return -1;
	my_openFT.erase(it);
	return 0;
}

int my_closedir(DIR *dirp)
{
	return -1;
}

int my_creat(const char *pathname, mode_t mode)
{
	//Create and initialize new inode
	unsigned long inum = inode_init(mode);

	my_dirent new_dirent;
	new_dirent.d_ino = inum;

	//Splits the string into seperate strings at "/"
	//Last string is the file name, others are directories
	std::vector<std::string> dir_names = split(pathname, "/");

	//Grab filename and remove from list of directory names
	std::string fname = dir_names.at(dir_names.size()-1);
	dir_names.pop_back();

	//construction of the path to parent folder
	std::string str = "/";
	for(int i = 0; i < dir_names.size(); i++)
		str += dir_names.at(i) + "/";
	//Adds filename to new dirent
	strcpy(new_dirent.d_name, fname.c_str());

	//Get root inode, assuming root inum is 0
	//my_inode root = (my_ilist.find(0)->second);
	if( my_ilist.find(0) == my_ilist.end() )
		return -1;
	my_inode root = my_ilist.find(0)->second;

	if(dir_names.size() == 0)
	//dir_names is empty, so parent dir is root(/)
	{
		root.dirent_buf.push_back(new_dirent);
	}
	else
	{
		unsigned long temp_inode = get_inode_number((char*)str.c_str());
		//Make changes to the inode that is stored in the ilist
		my_ilist.find(temp_inode)->second.dirent_buf.push_back(new_dirent);
	}
	return inum;
}

int my_fdatasync(int fd)
{
	return -1;
}

int my_fstat(int fd, struct stat *buf)
{
	//Get the file from the file descriptor
	if( my_openFT.find(fd) == my_openFT.end() )
		return -1;
	my_file fi = my_openFT.find(fd)->second;

	//Update fields in the stat struct from info in the inode struct
	//buf.st_dev = fi.f_inode.
	buf->st_ino = fi.f_inode->i_ino;
	buf->st_mode = fi.f_inode->i_mode;
	//buf.st_nlink
	//buf.st_uid
	//buf.st_gid
	//buf.st_dev
	//buf.st_size
	//buf.st_blksize
	//buf.st_blocks
	//buf.st_atime
	//buf.st_mtime
	//buf.st_ctime

	return 0;
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
	//Gets inode number
	long inum = get_inode_number((char*)path);

	//Gets inode from inum
	if(my_ilist.find(inum) == my_ilist.end())
		return -1;
	my_inode temp_inode = my_ilist.find(inum)->second;

	//Update fields in the stat struct from info in the inode struct
	//buf.st_dev = fi.f_inode.
	buf->st_ino = temp_inode.i_ino;
	buf->st_mode = temp_inode.i_mode;
	//buf.st_nlink
	//buf.st_uid
	//buf.st_gid
	//buf.st_dev
	//buf.st_size
	//buf.st_blksize
	//buf.st_blocks
	//buf.st_atime
	//buf.st_mtime
	//buf.st_ctime

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

int my_open1(const char *pathname, int flags)
{
	long temp_inum = get_inode_number((char *)pathname);
	
	//Check whether or not the pathname is valid
	//If invalid pathname, return -1
	//Otherwise return fd
	if(temp_inum == -1){
		std::cout<<"The file: " << pathname << "doesn't exist."<< std::endl;
		return -1;
	}

	//Get inode for path
	//my_inode *temp_inode = &my_ilist.find(temp_inum)->second;
	if( my_ilist.find(temp_inum) == my_ilist.end())
		return -1;
	my_inode *temp_inode = &my_ilist.find(temp_inum)->second;


	//Inode or inum?
	unsigned long fd = file_init(/*mode_t*/ 0x644, /*pos*/ 0, /*flags*/ flags, /*my_inode**/ temp_inode);

	//Return new file descriptor
	return fd;
}

int my_open2(const char *pathname, int flags, mode_t mode)
{
	long temp_inum = get_inode_number((char *)pathname);
	
	//Check whether or not the pathname is valid
	//If invalid pathname, return -1
	//Otherwise return fd
	if(temp_inum == -1){
		std::cout<<"The file: " << pathname << "doesn't exist."<< std::endl;
		return -1;
	}

	//Get inode for path
	if(my_ilist.find(temp_inum) == my_ilist.end() )
		return -1;
	my_inode *temp_inode = &my_ilist.find(temp_inum)->second;


	//Inode or inum?
	unsigned long fd = file_init(/*mode_t*/ mode, /*pos*/ 0, /*flags*/ flags, /*my_inode**/ temp_inode);

	//Return new file descriptor
	return fd;

}

DIR *my_opendir(const char *name)
{
	//return -1;
}
// my_pread: reads up to COUNT bytes from file descriptor FD
// at offset OFFSET (from the start of the file) into the buffer
// starting at BUF.
ssize_t my_pread(int fd, void *buf, size_t count, off_t offset)
{
	// grab the my_file corresponding to the opened file whose FD is 
	// passed through parameter fd
	if(my_openFT.find((unsigned long)fd) == my_openFT.end() )
		return -1;
	my_file tempFile = my_openFT.find((unsigned long)fd)->second;

	std::string str = "";
	int index;
	std::vector<char> tempBuf = tempFile.f_inode->buf;
	
	// if count > tempBuf.size() then read the whole file and
	// return the file size as the number of bytes read
	int bytes_read = ( (int)count > tempBuf.size() ? tempBuf.size() : (int)count );
	
	//Move buf contents into str
	for(index = (int)offset; index < bytes_read + (int)offset; index++)
		str += tempBuf.at(index);
	
	//Copy str into the passed in buf
	strcpy((char*)buf, str.c_str());

	//Return the number of bytes read
	return (ssize_t)bytes_read;
}

//Write count bytes from buf into the file starting at file[offset]
ssize_t my_pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	//Get the open file using the file descriptor
	if( my_openFT.find((unsigned long) fd) == my_openFT.end() )
		return -1;
	my_file tempFile = my_openFT.find((unsigned long) fd)->second;
	//Get the file buffer from the inode
	std::vector<char> tempBuf = tempFile.f_inode->buf;

	//Calculalte the possible number of bytes we can write
	int bytes_to_write = ( (int)count > tempBuf.size() ? strlen((char*)buf) : (int)count );

	//Write from buf to file[offset:offset+count]
	//for(index = (int)offset; index < offset + bytes_to_write; index++)
	for(int index = 0; index < bytes_to_write; index++)
	{
		tempFile.f_inode->buf[index + offset] = ((char*)buf)[index];
	}

	//Return the number of bytes we wrote
	return (ssize_t)bytes_to_write;
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

}




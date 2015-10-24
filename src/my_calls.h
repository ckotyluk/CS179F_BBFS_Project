#ifndef _MY_CALLS_H_
#define _MY_CALLS_H_

#include <vector>
#include <map>
#include <string>

//Helper Functions
std::vector<std::string> split(const std::string s, const std::string pat);

struct my_dirent
{
    ino_t           d_ino;      // inode number
    char            d_name[256];// file name
};

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

unsigned long inode_init(mode_t i_m);

//std::vector<my_inode> ilist;    //List of inodes
std::map<unsigned long, my_inode> my_ilist;

struct my_stat
{
    dev_t           s_dev;      // ID of device containing file
    ino_t           s_ino;      // inode number
    mode_t          s_mode;     // protection
    nlink_t         s_nlink;    // number of hard links
    uid_t           s_uid;      // user ID of owner
    gid_t           s_gid;      // group ID of owner
    dev_t           s_rdev;     // device ID (if special file)
    off_t           s_size;     // total size, in bytes
    blksize_t       s_blksize;  // blocksize for filesystem I/O
    blkcnt_t        s_blocks;   // number of blocks allocated
    time_t          s_atime;    // time of last access
    time_t          s_mtime;    // time of last modification
    time_t          s_ctime;    // time of last status change
};

int my_access(const char *pathname, int mode);
int my_chmod(const char *path, mode_t mode);
int my_chown(const char *path, uid_t owner, gid_t group);
int my_close(int fd);
int my_closedir(DIR *dirp);
int my_creat(const char *pathname, mode_t mode);
int my_fdatasync(int fd);
int my_fstat(int fd, struct stat *buf);
int my_fsync(int fd);
int my_ftruncate(int fd, off_t length);
ssize_t my_lgetxattr(const char *path, const char *name, void *value, size_t size);
int my_link(const char *oldpath, const char *newpath);
ssize_t my_llistxattr(const char *path, char *list, size_t size);
int my_lremovexattr(const char *path, const char *name);
int my_lsetxattr(const char *path, const char *name, const void *value, size_t size, int flags);
int my_lstat(const char *path, struct stat *buf);
int my_mkdir(const char *pathname, mode_t mode);
int my_mkfifo(const char *pathname, mode_t mode);
int my_mknod(const char *pathname, mode_t mode, dev_t dev);
int my_open(const char *pathname, int flags);
int my_open(const char *pathname, int flags, mode_t mode);
DIR *my_opendir(const char *name);
ssize_t my_pread(int fd, void *buf, size_t count, off_t offset);
ssize_t my_pwrite(int fd, const void *buf, size_t count, off_t offset);
struct dirent *my_readdir(DIR *dirp);
ssize_t my_readlink(const char *path, char *buf, size_t bufsiz);
int my_rename(const char *oldpath, const char *newpath);
int my_rmdir(const char *pathname);
int my_statvfs(const char *path, struct statvfs *buf);
int my_symlink(const char *target, const char *linkpath);
int my_truncate(const char *path, off_t length);
int my_unlink(const char *pathname);
int my_utime(const char *filename, const struct utimbuf *times);


#endif

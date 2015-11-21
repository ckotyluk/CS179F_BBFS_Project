#ifndef _MY_CALLS_H_
#define _MY_CALLS_H_


#ifdef _cplusplus
extern "C" {
#endif

struct my_dirent
{
    ino_t           d_ino;      // inode number
    char            d_name[256];// file name
};

unsigned long inode_init(mode_t i_m);


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
int my_open1(const char *pathname, int flags);
int my_open2(const char *pathname, int flags, mode_t mode);
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


#ifdef _cplusplus
}
#endif

#endif

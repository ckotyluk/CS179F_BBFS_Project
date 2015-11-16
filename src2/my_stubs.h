#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ino_t fh;
	int index;
	int offset;
	int max_offset;
} MY_DIR;
/*
typedef struct {
	ino_t fh;
	struct dirent *base;
	struct dirent *offset;
	struct dirent *max_offset;
} MY_DIR;
*/
int my_lstat( const char* path, struct stat *statbuf );
int my_readlink( const char *path, char *link, size_t size );
int my_mknod( const char *path, mode_t mode, dev_t dev );
int my_mkdir( const char *path, mode_t mode );
int my_unlink( const char *path );
int my_rmdir( const char *path );
int my_symlink(const char *path, const char *link);
int my_rename( const char *path, const char *newpath );
int my_link(const char *path, const char *newpath);
int my_chmod(const char *path, mode_t mode);
int my_chown(const char *path, uid_t uid, gid_t gid);
int my_truncate(const char *path, off_t newsize);
int my_utime(const char *path, struct utimbuf *ubuf);
int my_open( const char *path, int flags ); 
int my_pread( int fh, char *buf, size_t size, off_t offset );
int my_pwrite( int fh, const char *buf, size_t size, off_t offset );
int my_statvfs(const char *fpath, struct statvfs *statv);
int my_close( int fh );
int my_fdatasync( ino_t fh );
int my_fsync( ino_t fh );
int my_lsetxattr( const char *fpath, const char *name, const char *value, size_t size, int flags );
int my_lgetxattr( const char *fpath, const char *name, char *value, size_t size, int flags );
int my_llistxattr( const char *path, char *list, size_t size );
int my_lremovexattr( const char *path, const char *name );
MY_DIR* my_opendir( char fpath[PATH_MAX] );
struct dirent *my_readdir( MY_DIR *dp );
int my_closedir( MY_DIR *dp );
int my_access( const char *fpath, int mask );
int my_creat( const char *fpath, mode_t mode );
int my_ftruncate( ino_t fh, off_t offset );
int my_fstat( ino_t fh, struct stat* statbuf );

#ifdef __cplusplus
}
#endif

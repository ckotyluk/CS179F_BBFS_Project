
// Note: this code is a work-in-progress.  This version: 2:00PM PST 11/14/15

// I intend that the prototypes and behaviors of these functions be
// identical to those of their glibc counterparts, with three minor
// exceptions:

//   *  open(...) returns the handle of a file rather than the
//      descriptor of an open file.

//   *  pread(...) and pwrite(...) take
//      the handle of a file rather than the descriptor of an open file
//      as their respective first arguments.

// Recall that openfiles are to files what stringstreams are to strings. 

// In our case, we have no need to create an open file from the file
// in question; that gets done at a higher level. 


// Josef Pfeiffer's bbfs.c includes:
//#include "params.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
//#include <fuse.h>  
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/statvfs.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#include <attr/xattr.h>
#endif

//#include <sys/xattr.h>
//#include <attr/xattr.h>


// Here we include
// C stuff
#include "my_stubs.h"
#include </usr/include/linux/fs.h>  // needed for compilation on vagrant
#include <sys/stat.h>  // this has our official definition of stat
#include <dirent.h>    // this has our official definition of dirent
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <err.h>
// C++ stuff
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <vector>
#include <string.h>
#include <string>
#include <map>
#include <list>
#include <iostream>  
#include <errno.h>
#include <iomanip>

using namespace std;

// Prototypes for local functions.  There's no need to put these into
// my_stubs.H, since these functions are local to my_stubs.cc.
void show_stat( struct stat& root );
void show_statvfs( struct statvfs *buf);
void initialize();                  
ino_t find_real_ino( string path );
ino_t find_ino( string path );
ino_t lookup( string name, ino_t fh );
void print_xattr_list(char* buf, int size);

// Here is a convenient macro for debugging.  cdbg works like cerr but 
// prefixes the error message with its location: line number and function
// name.
#define cdbg cerr <<"\nLn "<<dec<<  __LINE__ << " of "  << __FUNCTION__ << ": "


string cwd;   // to hold name of current working directory but not used yet.

struct dirent_frame { // The official definition of dirent puts its users
    // into "undefined behavior"
    dirent the_dirent;
    char overflow[NAME_MAX]; // to catch overflow from one-char name field.
};


class File {
public:
    //inode metadata;                     // for use by all files
    struct stat metadata;                 // for use by all files
    // one or the other of the following (data and frame) should be empty.
    string data;                            // for use by regular files
    vector<dirent_frame> dentries;          // for use by directories
    // list<dirent_frame> dentries;        // it'd be cleaner to make this a list
    map<string, string> xattr;              // Map to hold a list of xattr of form (name, value)
};


class Ilist {
public:
    int count;
    map<ino_t,File> entry;
    int next() {
        static int count = 2;  // ino counter stats at 2.
        return count++;
    }
    
} ilist; 


void initialize() { // now called from main() but could be called from
                                        // Ilist constructor (I think).
    my_mkdir("/", 0700);
    char tmp[PATH_MAX];
    getcwd(tmp,998);
    cwd = string(tmp);  
    // cwd now hold the path to the initial current working directory.
    // But, we don't yet use it.
    
}

void show_stat( struct stat& root ) {
    // Displays the entries in a stat in same formats that Pfeiffer uses.
    cerr << "SHOW\n";
    cerr << "st_dev     = " << dec << root.st_dev     << endl;  
    cerr << "st_ino     = "        << root.st_ino     << endl;  
    cerr << "st_mode    = " << oct << root.st_mode    << endl;  
    cerr << "st_nlink   = "        << root.st_nlink   << endl;  
    cerr << "st_uid     = " << dec << root.st_uid     << endl;  
    cerr << "st_gid     = " << dec << root.st_gid     << endl;  
    cerr << "st_rdev    = "        << root.st_rdev    << endl;  
    cerr << "st_size    = "        << root.st_size    << endl;  
    cerr << "st_blksize = "        << root.st_blksize << endl;  
    cerr << "st_blocks  = "        << root.st_blocks  << endl;  
    cerr << "st_atime   = " << dec << root.st_atime   << endl;  
    cerr << "st_mtime   = " << dec << root.st_mtime   << endl;  
    cerr << "st_ctime   = " << dec << root.st_ctime   << endl;  
}; 

void show_statvfs( struct statvfs *buf)
{
    // Displays the enteries in statvfs
    cerr << "SHOW_STATVFS\n";
    cerr << "f_bsize    = " << buf->f_bsize      << endl;
    cerr << "f_frsize   = " << buf->f_frsize     << endl;
    cerr << "f_blocks   = " << buf->f_blocks     << endl;
    cerr << "f_bfree    = " << buf->f_bfree      << endl;
    cerr << "f_bavail   = " << buf->f_bavail     << endl;
    cerr << "f_files    = " << buf->f_files      << endl;
    cerr << "f_ffree    = " << buf->f_ffree      << endl;
    cerr << "f_favail   = " << buf->f_favail     << endl;
    cerr << "f_fsid     = " << buf->f_fsid       << endl;
    cerr << "f_flag     = " << buf->f_flag       << endl;
    cerr << "f_namemax  = " << buf->f_namemax    << endl;
}


inline  // a simple utility for splitting strings at a find-pattern.
vector<string>
split(const string s, const string pat ) {
    string c;
    vector<string> v;
    int i = 0;
    for (;;) {
        int t = s.find(pat,i);
        int j = ( t == string::npos ) ? s.size() : t;
        string c = s.substr(i,j-i);
        v.push_back(c);
        i = j+pat.size();
        if ( t == string::npos ) return v;
    }
}

inline  // joins vector of strings separating via a given pattern.
string
join( const vector<string> v, const string pat, int start=0, int end=-1 ) {
    if ( end < 0 ) end += v.size();
    //  assert ( start < v.size() );   // should throw an exception.
    if ( start >= v.size() ) return "";
    string s = v[start++];
    for ( int i = start; i <= end; ++i )  s += pat + v[i];
    return s;
}

const int ok = 0;
const int an_err = -1;
// Note that for pointer and ino_t return values, 0 indicates an error.


// lstat is called at line #95 of bbfs.c
int my_lstat( const char* path, struct stat *statbuf ) {
    //cdbg << "lstat has been called and is calling find_ino on " << path << endl;
    ino_t fh = find_real_ino(path);
    //cdbg << "fh = " << fh << endl;
    int retstat;
    if ( fh == 0 ) {
        errno = EBADF; // fd is bad
        retstat = an_err;
    } else {
        retstat = my_fstat(fh,statbuf);
    }  
    return retstat;
}  

// called at line #125 of bbfs.c
int my_readlink( const char *path, char *link, size_t size ) {

    int fh = find_real_ino(path);
    if (fh == 0)
    {
        cout << path << " does not exist" << endl;
        return an_err;
    }

    if( !S_ISLNK(ilist.entry[fh].metadata.st_mode) )
    {
        cout << "Not a link" << endl;
        return an_err;
    }

    //int size = ilist.entry[fh].metadata.data.size();

    int err = my_pread(fh, link, size, 0);
    if(err == -1)
    {
        cout << "Pread error" << endl;
        return an_err;
    }
    return ok;  
}  

// called at line #168 of bbfs.c.  See line #151.
int my_mknod( const char *path, mode_t mode, dev_t dev ) {
    //cdbg <<"mknod been called with " << path << " " << oct << mode 
    //       << " " << dev <<endl;
    // Returns error code only.  It should make a file and install it
    // into ilist and instrall a dentry for it in the parent directory.

    // To later get the handle for that inode use find_ino().  To get a
    // pointer to that file use find_file.  IMHO, we should use it to
    // creat both directories and regular files, but the man page leaves
    // out directories.  So, I've not used it, and it's not been tested.

    dev = 100;   // a device number that's unlikely to be used on vagrant

    // Now we create and configure this File's metadata (inode/struct).
    int the_ino = ilist.next();     // Get and record this directory's ino
    struct stat& md = ilist.entry[the_ino].metadata;   // Create this file
    mode_t old_umask = umask(0);  // sets umask to 0 and returns old value
    umask(old_umask);                       // restores umask to old value
    md.st_dev     = dev;                /* ID of device containing file */
    md.st_ino     = the_ino;                            /* inode number */
    //cdbg << "mode = " << oct << mode << " old_umask = " 
    //     << oct << old_umask << endl;
    md.st_mode    = ( mode & ~ old_umask);                /* protection */
    md.st_nlink   = 1;                          /* number of hard links */
    md.st_uid     = geteuid();                      /* user ID of owner */
    md.st_gid     = getegid();              /* group ID of owning group */
    md.st_rdev    = 0;                   /* device ID (if special file) */
    md.st_size    = 0;       /* size in bytes for reg-files and symlinks*/
    md.st_blksize = 4096;              /* blocksize for file system I/O */
    md.st_blocks  = 0;               /* number of 512B blocks allocated */
    md.st_atime   = time(0);                     /* time of last access */
    md.st_mtime   = time(0);               /* time of last modification */
    md.st_ctime   = time(0);              /* time of last status change */

    // The man page on STAT(2) is an excellent resource for configuring
    // a stat, e.g., http://man7.org/linux/man-pages/man2/stat.2.html


    // Now we push a dentry for this directory onto the parent directory.
    // except for root directory
    vector<string> v = split( path, "/" );  // split the path
    string the_name = v.back();
    v.pop_back();

    // Create, configure, and install a dentry for this new directory's
    // in its parent directory.  But, omit this the first time, since
    // the root directory has no parent.
    // cdbg << "the_ino is " << the_ino << endl;
    ino_t parent_ino = find_ino(join(v,"/"));  
    if(parent_ino == 0)
    {
        cout << "Error: Parent does not exist." << endl;
        return an_err;
    }
    static bool first_time = true;  // static vars get initialized once
    if ( first_time ) {                    
        first_time = false;
    } else {    // Omit this on first invocation, i.e., when making root
        if(find_ino(path) != 0)
        {
            cout << "Error: " << path << " already exists." << endl;
            errno = EEXIST;
            return an_err;
        }
        ino_t fh = find_ino(join(v,"/"));
        dirent_frame df;                    
        df.the_dirent.d_ino = the_ino; 
        strcpy(df.the_dirent.d_name, the_name.c_str());  
        //cdbg << "Inserting dentry for " << the_ino << " w name " \
                     << the_name << " into directory " << join(v,"/");
        ilist.entry[fh].dentries.push_back(df);  // push df onto the back of parent
    }

    // Branch around this if not a directory
    if ( S_ISDIR(mode) ) {
        // Install dentries for . and .. within this new directory
        dirent_frame df;

        strcpy(df.the_dirent.d_name, ".");
        df.the_dirent.d_ino  = the_ino;  
        ilist.entry[the_ino].dentries.push_back(df);

        strcpy(df.the_dirent.d_name, "..");
        df.the_dirent.d_ino  = parent_ino;  // needs to be adjusted for root directory
        ilist.entry[the_ino].dentries.push_back(df);
    }

    return ok;

}  


// called at line #186 of bbfs.c
int my_mkdir( const char *path, mode_t mode ) {
    // returns an error code.
    //cdbg << "mkdir has been called with path \"" << path<< "\" and mode " \
    //     << oct <<  mode << endl;
    return my_mknod(path, (S_IFDIR | mode), 100 );
}  // my_mkdir

// called at line #203 of bbfs.cg
int my_unlink( const char *path ) {
	
	ino_t fh = find_ino(path);
	File file_in_dir = ilist.entry[fh];

    if( fh == 0)
    {
        cout << "Error: " << path << " does not exist." << endl;
        errno = ENOTDIR;
        return an_err; 
    }

    //Remove dirent

    vector<string> v = split(path, "/");
    string tail = v.back();
    v.pop_back();
    string parent = join(v, "/");
    cdbg << "Parent path is: " << parent << endl;
    ino_t parent_fh = find_ino(parent.c_str());

    for(int i = 0; i < ilist.entry[parent_fh].dentries.size(); i++){
        cdbg << ilist.entry[parent_fh].dentries.at(i).the_dirent.d_name << " ?== " << path << endl;
        if((string)ilist.entry[parent_fh].dentries.at(i).the_dirent.d_name == (string)tail){
            cout << "Deleting dentry with name: " << path << endl;
            ilist.entry[parent_fh].dentries.erase(ilist.entry[parent_fh].dentries.begin()+i);
            break;
        }
    }

    ilist.entry[fh].metadata.st_nlink--;
    if(ilist.entry[fh].metadata.st_nlink <= 0)
    {
	   ilist.entry.erase(fh);
	}

    return ok;   

}  

// called at line #220 of bbfs.c
int my_rmdir( const char *path ) {
    // See http://linux.die.net/man/2/rmdir for a full list of all 13
    // possible errors for rmdir.

    ino_t fh = find_ino( path );
    File the_dir = ilist.entry[fh];
    if ( ! S_ISDIR(the_dir.metadata.st_mode) ) { 
        cdbg << "does not exist\n";
        errno = ENOTDIR;
        return an_err;
    }
    if ( the_dir.dentries.size() > 2 ) {  // for . and ..
        cdbg << "not empty\n";
        errno = ENOTEMPTY;
        return an_err;
    }
    vector<string> vs = split( path, "/" );
    vs.pop_back();
    string parent_path = join(vs,"/");
    parent_path = "/" + parent_path;  // FIX this hack
    // cdbg << "Parent path is " << parent_path << endl;
    ino_t parent = find_ino(parent_path);
    // cdbg << "Parent ino is " << parent << endl;
    vector<dirent_frame>& v = ilist.entry[parent].dentries;
    for(auto it = v.begin(); it != v.end(); ++it ) {
        // We erase him from his parent directory.
        if ( it->the_dirent.d_ino == fh ) {
            // cdbg << "erasing " << fh << " from " << parent << endl;
            v.erase(it);  
            break;  // Must stop iterating now!
        }
    }    
    if ( --the_dir.metadata.st_nlink == 0 ) {
        ilist.entry.erase(fh);  // get rid of this file
    }
    return ok;
}  

// called at line #241 of bbfs.c
int my_symlink(const char *path, const char *link) {
    //Makes a softlink of link that points to path
    //File at link contains path

    int link_fh = find_ino(link);
    if(link_fh != 0)
    {
        cout << "Error: " << link << " already exists." << endl;
        errno = EEXIST;
        return an_err;
    }

    // Create file
    int err = my_creat(link, 666);
    if(err == -1)
    {
        cout << "Creat Error" << endl;
        return an_err;
    }

    //Writes the path into the link file
    link_fh = find_ino(link);
    my_pwrite(link_fh, path, strlen(path), 0);


    // Setup permissions correctly and set symlink flags
    int len = strlen(path);
    err = my_chmod(link, ilist.entry[link_fh].metadata.st_mode | S_IFLNK);
    if(err == -1)
    {
        cout << "Chmod Error" << endl;
        return an_err;
    }
	return ok;
}

// called at line #261 of bbfs.c
int my_rename( const char *path, const char *newpath ) {
    //checking for collisions with the new path name
    vector<string> v_newpath = split(string(newpath), "/");
    string newpath_tail = v_newpath.back();
    v_newpath.pop_back();
    string parent_newpath = join(v_newpath, "/");
    ino_t parent_newpath_fh = find_ino(parent_newpath.c_str());

    if(lookup(newpath_tail, parent_newpath_fh)) //File with same name exists in newpath dir
    {
        cdbg << "File with same name exists in newpath dir" << endl;
        //return an_err;

        for(int i = 0; i < ilist.entry[parent_newpath_fh].dentries.size(); i++)
        {
            cdbg << "Checking " << ilist.entry[parent_newpath_fh].dentries.at(i).the_dirent.d_name << " ==? " << (string)newpath_tail << endl;
            if((string)ilist.entry[parent_newpath_fh].dentries.at(i).the_dirent.d_name == (string)newpath_tail)
            {
                cdbg << "Removing newpath: " << ilist.entry[parent_newpath_fh].dentries.at(i).the_dirent.d_name << endl;
                ilist.entry[parent_newpath_fh].dentries.erase(ilist.entry[parent_newpath_fh].dentries.begin()+i);
                break;
            } 
        }
    }

    ino_t original_fh = find_ino(path);
    vector<string> v_path = split(string(path), "/");
    string path_tail = v_path.back();
    v_path.pop_back();
    string parent_path = join(v_path, "/");
    ino_t parent_path_fh = find_ino(parent_path.c_str());

    for(int i = 0; i < ilist.entry[parent_path_fh].dentries.size(); i++)
    {
        cdbg << "Checking " << ilist.entry[parent_path_fh].dentries.at(i).the_dirent.d_name << " ==? " << (string)path_tail << endl;
        if((string)ilist.entry[parent_path_fh].dentries.at(i).the_dirent.d_name == (string)path_tail)
        {
            cdbg << "Removing " << ilist.entry[parent_path_fh].dentries.at(i).the_dirent.d_name << endl;
            ilist.entry[parent_path_fh].dentries.erase(ilist.entry[parent_path_fh].dentries.begin()+i);
            break;
        } 
    }

    dirent_frame df;

    strcpy(df.the_dirent.d_name, newpath_tail.c_str());
    df.the_dirent.d_ino = original_fh;

    ilist.entry[parent_newpath_fh].dentries.push_back(df);

    cdbg << "Pushing new dirent with [" << df.the_dirent.d_name << ", " << df.the_dirent.d_ino << "] onto " << parent_newpath_fh << endl;

    return ok;
}  

// called at line #279 of bbfs.c
int my_link(const char *path, const char *newpath) {
	ino_t fh = find_ino(path);
    if(fh == 0) // Check that path exists
    {
		errno = EPERM;
		cout << "Source file " << path << " does not exist." << endl;
		return an_err;
	}
	
	if ( !S_ISREG( ilist.entry[fh].metadata.st_mode ) ) // Stop if not a file
	{
        errno = EMLINK;  // only one link to each directory.
        return an_err; 
    }
    
    // Split up new path to create parent path
    vector<string> v = split(string(newpath),"/");
    string tail = v.back();
    v.pop_back(); // get rid of tail
    string parent_path = join(v, "/"); // the remaining part of newpath

	ino_t parent_fh = find_ino(parent_path);
	if(parent_fh == 0) // Check that newpath has a valid parent directory
    {
		errno = EPERM;
		cout << "Destination parent directory " << parent_path << " noes not exist." << endl;
		return an_err;
	}

    ino_t new_fh = find_ino(newpath);
	if(new_fh != 0) // New file already exists
    {
		errno = EEXIST;
		cout << "Destination file " << newpath << " already exists." << endl;
		return an_err;
	}
	
    // if dirpath didn't name a directory, find_ino() would have failed.
    cout << "n_link was: " << ilist.entry[fh].metadata.st_nlink;
    ++ilist.entry[fh].metadata.st_nlink; // no overflow since fh is regular.
    cout << " and is now: " << ilist.entry[fh].metadata.st_nlink << endl;
    
    dirent_frame df;
    strcpy(df.the_dirent.d_name, tail.c_str() );
    df.the_dirent.d_ino  = fh;  
    ilist.entry[parent_fh].dentries.push_back(df);
    return ok;
}  

// called at line #296 of bbfs.c
int my_chmod(const char *path, mode_t mode) {
    ino_t fh = find_ino(path);
    if(fh == -1)
        return an_err;  

    ilist.entry[fh].metadata.st_mode = mode;
    return ok;
}  

// called at line #314 of bbfs.c
int my_chown(const char *path, uid_t uid, gid_t gid) {
    
    ino_t fh = find_ino(path);
    if(fh == -1)
        return an_err;

    ilist.entry[fh].metadata.st_uid = uid;
    ilist.entry[fh].metadata.st_gid = gid;

    return ok;
}  

// called at line #349 of bbfs.c
int my_utime(const char *path, struct utimbuf *ubuf) {
    if(ubuf == NULL)
    {
        errno = EACCES;
        return an_err;
    }

    ino_t fh = find_ino(path);
    if(fh == 0)
    {
        errno = ENOENT;
        return an_err;
    }

    ilist.entry[fh].metadata.st_atime = (ubuf ? (*ubuf).actime : time(0) );
    ilist.entry[fh].metadata.st_mtime = (ubuf ? (*ubuf).modtime : time(0) );

    return ok;  
}  

// called at line #376 of bbfs.c.  Returns file handle not a file descriptor
int my_open( const char *path, int flags ) {
    // Write a function called "lookup" that takes a full path and the
    // handle of a directory and returns the handle of the inode having
    // that name.  To do so, split full path.  Then starting with the
    // handle of the root directory, look up each segment in the
    // directory whose handle you currently have.  Use a linear search
    // that consists of successive calls to readdir() to find the
    // corresponding directory entry, which has type struct dirent.
    // Return its d_fileno, unless there's an error and then return -1.

    // FINISH THIS
    //static int filecount = 0;
    ino_t fh = find_ino(path);  // 
    if ( fh >= 0 ) {
    	ilist.entry[fh].metadata.st_nlink++;
        return fh;
    } //else if ( flags & CREAT ) {
    //   // create a new inode with ino_t filecount++;
    //}
	else if (flags & O_CREAT) 
    {
		int err = my_creat(path, 666);
		if(err == -1)
		{
			return an_err;	
		}
		return find_ino(path);
	}
    else
    {
        return an_err;
    }
}  

// called at line #411 of bbfs.c  Note that our firt arg is an fh not an fd
int my_pread( int fh, char *buf, size_t size, off_t offset ) {
    //cdbg << "File[" << fh << "] size: " << ilist.entry[fh].data.length() << endl;
    //cdbg << "File[" << fh << "] contains: [" << ilist.entry[fh].data << "]" << endl;
    
    //int bufLen = strlen(buf);
    int bufLen = size;
	int fileLen = ilist.entry[fh].data.length();

    //cdbg << "bufLen: " << bufLen << " & fileLen: " << fileLen << endl;
    //cdbg << "bufLen new: " << sizeof(buf) << endl;

	if(fileLen < offset)
    {
        //cdbg << "Case 1" << endl;
        return an_err;
	}
    else if (fileLen < (offset + size - 1) ){
		//cdbg << "Case 2" << endl;
        strcpy(buf, (char *)ilist.entry[fh].data.substr(offset).c_str());
		return fileLen - offset;
	}
	else{ // copy everything
		//cdbg << "Case 3" << endl;
        strcpy(buf, (char*)ilist.entry[fh].data.substr(offset, size).c_str());
		return size;
	}
}  

// called at line #439 of bbfs.c  Note that our firt arg is an fh not an fd
int my_pwrite( int fh, const char *buf, size_t size, off_t offset ) {
    ilist.entry[fh].data.resize(size);

    string str((char*)buf);
	
	int j = 0;
	for(int i = offset; i < offset + size; i++)
    {
		ilist.entry[fh].data.at(i) = str[j++];
    }

    //cdbg << "Wrote [" << buf << "] to file" << endl; 

    ilist.entry[fh].metadata.st_size = str.length();

	return str.length();
}  

// called at line #463 of bbfs.c
int my_statvfs(const char *fpath, struct statvfs *statv) {
    
    ino_t fh = find_ino(fpath);
    if(fh == 0)
    {
        errno = ENOENT;
        return an_err;
    }

    statv->f_bsize      = 0;
    statv->f_frsize     = 0;
    statv->f_blocks     = 0;
    statv->f_bfree      = 0;
    statv->f_bavail     = 0;
    statv->f_files      = ilist.entry.size();
    statv->f_ffree      = 0;
    statv->f_favail     = 0;
    statv->f_fsid       = 0;
    statv->f_flag       = 0;
    statv->f_namemax    = NAME_MAX;

    return ok;  
}  

// called at line #530 of bbfs.c
int my_close( int fh ) {
    //Since we are closing a file, we decremente the nlink count
    ilist.entry[fh].metadata.st_nlink--;

    //Whenever the nlink count is 0, we need to delete that data
    if( ilist.entry[fh].metadata.st_nlink == 0 )
        ilist.entry.erase(fh);

    return ok;
}  

// called at line #553 of bbfs.c
int my_fdatasync( ino_t fh ) {
    return an_err;  
}  

// called at line #556 of bbfs.c
int my_fsync( ino_t fh ) {
    return an_err;  
}  

// called at line #575 of bbfs.c
//Stores (name:value) into fpath
int my_lsetxattr( const char *fpath, const char *name, const char *value, size_t size, int flags )
{
    ino_t fh = find_real_ino(fpath);

    string x_name = (string)name;
    string x_val = (string)value;

    std::pair<std::map<string, string>::iterator,bool> ret;

    ret = ilist.entry[fh].xattr.insert(std::pair<string,string> (x_name, x_val) );
    if(ret.second == false)
    {
        errno = EEXIST;
        return an_err;
    }

    return ok;  
}  

// called at line #592 of bbfs.c
int my_lgetxattr( const char *fpath, const char *name, char *value, size_t size, int flags )
{
    ino_t fh = find_real_ino(fpath);

    std::map<string,string>::iterator it;

    it = ilist.entry[fh].xattr.find(name);
    if(it == ilist.entry[fh].xattr.end() )
    {
        cout << "Error: Attribute does not exist" << endl;
        return an_err;
    }

    value = (char*)(it->second).c_str();

    return strlen(value); 
}  

void print_xattr_list(char* buf, int size)
{
    cout << "Attribute List" << endl;
    for(int i = 0; i < size; i++)
    {
        if(!buf[i-1]) cout << '\t';
        cout << (buf[i] ? buf[i] : '\n');
    }
}

// called at line #613 of bbfs.c
int my_llistxattr( const char *path, char *list, size_t size ) {
    ino_t fh = find_real_ino(path);
    if(fh == 0)
    {
        errno = ENOENT;
        return an_err;
    }

    int tmpsize;
    int curPos = 0;

    std::map<string,string>::iterator it;
    for(it = ilist.entry[fh].xattr.begin(); it != ilist.entry[fh].xattr.end(); it++)
    //Goes through each attribute in the xattr list
    {
        tmpsize = strlen((it->first).c_str());
        if((curPos + tmpsize + 1) < (size))
        {
            //list[curPos] = (it->first).c_str();
            for(int i = 0; i < tmpsize; i++)
            {
                list[curPos+i] = (it->first).at(i);
            }
            list[curPos+tmpsize] = '\0'; //Should be null, 0 for debugging
            curPos = curPos + tmpsize + 1;
        }
        else{
            break;
        }
    }
    return curPos;
}  

// called at line #634 of bbfs.c
int my_lremovexattr( const char *path, const char *name ) {
    ino_t fh = find_ino(path);
    if(fh == 0)
    {
        errno = ENOENT;
        return an_err;
    }

    std::map<string,string>::iterator it;
    it = ilist.entry[fh].xattr.find(name);
    if(it == ilist.entry[fh].xattr.end())
    {
        errno = ENOATTR;
        return an_err;
    }
    ilist.entry[fh].xattr.erase(it);
    return ok;  
}  


// called at line #826 of bbfs.c
//Checks file perms with your perms
//If all perms match, return 0
//If not, returns -1
int my_access( const char *fpath, int mask ) {
    
    ino_t fh = find_ino(fpath);
    vector<string> v = split(fpath,"/");
    string filename = v.back();

    //Get stat to get the perms
    struct stat st;
    if ( my_fstat( fh, &st ) != 0 )
    {
        cerr << "Cannot stat file: " << filename
             << ": " << strerror(errno) << endl;
        return an_err;
    }

    //Check that the perms in mask match the user perms for the file
    if( (st.st_mode & S_IRUSR) != (mask & S_IRUSR) ) return an_err;
    if( (st.st_mode & S_IWUSR) != (mask & S_IWUSR) ) return an_err;
    if( (st.st_mode & S_IXUSR) != (mask & S_IXUSR) ) return an_err;

    return ok;  
}  

// called at line #856 of bbfs.c
int my_creat( const char *fpath, mode_t mode ) {
    // we can create a file by using the right flags to open
    
    return my_mknod(fpath, (S_IFREG | mode), 100 );
}  


// called at line #887 of bbfs.c
int my_ftruncate( ino_t fh, off_t offset ) {
    if(fh == 0)
    {
        cout << "Error: Bad fh" << endl;
        errno = EBADF;
        return an_err;
    }
    //Changes size of data, adding null if the new size is bigger
    ilist.entry[fh].data.resize(offset,'\0');
    ilist.entry[fh].metadata.st_size = ilist.entry[fh].data.length();
    return ok;
}  

// called at line #331 of bbfs.c
int my_truncate(const char *path, off_t newsize) {
    if(newsize < 0)
    {
        errno = EINVAL;
        return an_err;
    }

    ino_t fh = find_ino(path);
    cout << "fh: " << fh << endl;
    if(fh == 0)
    {
        cout << "Error: Bad fh" << endl;
        errno = ENOENT;
        return an_err;
    }
    
    if(S_ISDIR(ilist.entry[fh].metadata.st_mode))
    {
        errno = EISDIR;
        return an_err;
    }

    return my_ftruncate(fh, newsize);
}  

// called at line #921 of bbfs.c
int my_fstat( ino_t fh, struct stat* statbuf ) {

    //cdbg << "my_fstat has been called on " << fh << " " << long(statbuf) << endl;
    if ( ilist.entry.count(fh) == 0 ) {
        errno = ENOENT;
        return an_err;
    }  
    *statbuf = ilist.entry[fh].metadata;
    // cdbg << "and here's *statbuf for " << endl;
    // show_stat(*statbuf);
    return ok;
}  


// Here are my helper functions ==================================

// In the code below  opendir is defined in terms of find_ino,
// which is defined in terms of lookup,
// which is defined in terms of opendir and readdir.
// The header file takes care of that circularity.

// It doesn't say so anywhere that I can find, but a MY_DIR has to be
// similar to an open file.  However, rather than creating a stream of
// bytes (chars) from the file, it creates a stream of directory
// entries.  The key difference is how much you increment the offset
// counter each time you move to the next item.

// Note that at line #742 of bbfs.c, Professor Pfeiffer converts a
// file handle into a DIR* via "(uintptr_t) fh."  But, his file handles
// are indices of byte streams, while our are the addresses of inodes.


MY_DIR* fopendir( ino_t fh ) {  // not exported
    if ( ! S_ISDIR( ilist.entry[fh].metadata.st_mode ) ) {
        return 0;  // null pointer indicates err
    }
    MY_DIR * tmp = new MY_DIR;
    // Initialize both members of tmp*
    tmp->fh    = fh;
    tmp->index = 0;
    return tmp;
}


// called at lines #707 and #726 of bbfs.c
dirent* my_readdir( MY_DIR* dirp ) {  
    vector<dirent_frame> v = ilist.entry[dirp->fh].dentries;
    int tmp = dirp->index++;  // post increment dirp*'s index.
    return tmp == v.size() ? 0 : & v[tmp].the_dirent;
}


// called at line #742 of bbfs.
int my_closedir( MY_DIR* dirp ) {
    delete dirp;  
}


ino_t lookup( string name, ino_t fh ) {
    // Searches for and returns ino of file of a given name within the 
    // directory given by fh.  This function will be used by find_ino().

    MY_DIR* dirp = fopendir( fh ); 
    // fopendir() will return 0 if fh isn't the handle of a directory.
    if ( ! dirp ) return 0;                   // And so will lookup().
    while ( dirent* dp = my_readdir(dirp) ) {  
        //cdbg << dp->d_name << " =? " <<name<< " then return " << dp->d_ino << endl;
        if ( string(dp->d_name) == name ) { // comparision of C++ strings
            my_closedir(dirp);
            return dp->d_ino;  
        } 
    }
    my_closedir(dirp);  // close MY_DIR asap, to reset internal data
    return 0;  // name-not-found
}  

ino_t find_real_ino( string path ) {

    //cdbg << "find_ino() has been called with path \"" << path << "\"\n"; 
    vector<string> v = split( path, "/" );  // The members of v are "segments."
    // Here are the exact details concerning path resolution:
    // http://manpages.ubuntu.com/manpages/lucid/en/man7/path_resolution.7.html

    // The plan: if the initial segment is not null prepend the full
    // pathname for the current working directory to path and start
    // over.  Process out segments of the form"." or ".." (along with
    // its predecessor).  Delete any other null segments.  Among other
    // things, we want the path name "/" to designate the root
    // directory.

    if ( v[0] != "" ) path = cwd + "/" + path;  // cwd == "" for now
    ino_t fh = 2;    // I've read that 2 is the ino of a filesystem's 
                                     // root directory.  
    for ( auto it : v ) {
        if ( it == "" ) {
             continue;                  // ignore null segments.
        } else if ( ilist.entry.count(fh) == 0 ) {  // no such directory
            cdbg << "no such entry as " << it << " in " << path << endl;
            errno = ENOENT;
            fh = 0;
            break;
        } else if ( S_ISDIR(ilist.entry[fh].metadata.st_mode) ) {
            //cdbg << "lookup(" << it << "," << fh << ") yields ...";
            fh = lookup( it, fh );
            // cerr << "yields ... this value " << fh << endl;
        } else {
            cdbg << "In " << path <<  ", " << it << " is not a directory \n";
            // errno = ENOTDIR;
            fh = 0;
            break;   
        }
        //cdbg << v[i] << " " << fh << end;
    } 
    // cdbg << "and is returning " << fh << endl;
    return fh;
} 


ino_t find_ino( string path ) {
    int fh = find_real_ino(path);
    if(S_ISLNK(ilist.entry[fh].metadata.st_mode) )
    {
        char link[50];
        int err = my_readlink(path.c_str(), link, 50);
        if(err == -1)
        {
            cout << "Error: Readlink fails" << endl;
            return an_err;
        }

        fh = find_real_ino(link);
    }
    return fh;
} 
/*
ino_t find_ino( string path )
{
    int fh = get_ino(path);
    if(S_ISLNK(ilist.entry[fh].metadata.st_mode) )
    {
        char link[50];
        int err = my_readlink(path.c_str(), link, 50);
        if(err == -1)
        {
            cout << "Error: Readlink fails" << endl;
            return an_err;
        }

        fh = get_ino(link);
    }
    return fh;
}*/

File* find_file( ino_t ino ) { // could improve readability of code
    return & ilist.entry[ino];
}


File* find_file( string s ) {  // could improve readability of code
    return find_file(find_ino(s));
}


// called at line #659 of bbfs.c
MY_DIR * my_opendir( const char path[PATH_MAX] ) {
    return fopendir( find_ino( path ) ); 
    // Note that fopendir checks whether its input is the handle of a
    // directory.
}


int describe_file( string pathname );

int ls(string path) {
    // diagnostic tool to see what's in a directory
    // cdbg << "ls has been called on \"" << path << "\" and ilist has "   
    // << ilist.entry.size() << " entries\n";
    ino_t fh = find_real_ino(path.c_str());
    if ( fh == 0 ) return an_err;
    File f = ilist.entry[fh];
    if ( ! S_ISDIR(f.metadata.st_mode) ) {
        errno = ENOTDIR;
        cdbg << "ilist entry " << fh << " isn't a directory: mode = " 
                 << oct << f.metadata.st_mode << endl;
        //show_stat( f.metadata );
        describe_file(path);
        return ok;
    }
    vector<struct dirent_frame> v = f.dentries;
    //cdbg << "The directory (ino" << fh << ") has " << v.size() << " entries:\n";
    for ( auto it : v ) {
        struct dirent d = it.the_dirent;
        //cout << d.d_ino << " " << d.d_name << endl;
        describe_file( path+"/"+d.d_name );
    }
}

// ===========================================


template< typename T1, typename T2>   // An idea
string pickle(map<T1,T2> m) {
    typename map<T1,T2>::iterator it;
    string s;
    stringstream ss(s);
    for ( auto it : m ) {
        // This requires some kind of separation/terminaton symbol at the end of each.
        ss << it.first;  
        ss << it.second;
    }  
}  

    
// MY_DIR is typedef'd in my_stubs.H.  It's essentially an iterator for 
// directories.

// Per this GNU document (http://www.delorie.com/gnu/docs/dirent/dirent.5.html): 
// "The dirent structure is defined below.
// struct dirent {
//   long           d_ino;
//   off_t          d_off;
//   unsigned short d_reclen;
//   char d_name[1];
// };
// The field d_ino is a number which is unique for each file in the
// file system. The field d_off represents an offset of that directory
// entry in the actual file system directory. The field d_name is the
// beginning of the character array giving the name of the directory
// entry. This name is null terminated and may have at most NAME_MAX
// characters in addition to the null terminator. This results in file
// system independent directory entries being variable-length
// entities. The value of d_reclen is the record length of this
// entry. This length is defined to be the number of bytes between the
// beginning of the current entry and the next one, adjusted so that
// the next entry will start on a long boundary."

// But, per the relevant GNU manual pages on dirent,
// (http://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html),
// it may have many additional fields and even d_reclen is not
// required.  So far as I can tell, we don't need d_off and d_reclen
// but may need to maintain them for compatibility with FUSE, etc.

// Per a creator of FUSE (Millos Szeredit) regarding the offset field of dirents:
// http at fuse.996288.n3.nabble.com/issues-in-readdir-and-fuse-fill-dir-t-filler-td10397.html:

// "For directories it's completely irrelevant.  It can go forward,
// backward or jump around randomly.  Ancient unix systems allowed
// directories to be read as regular files (i.e. cat would work) and the
// offset was really an offset into this file.

// But now filesystems are free to implement directories in any way and
// the "offset" is not necessarily an offset any more, just a opaque
// cookie, unique for each directory entry within the directory,
// indicating the position within the directory stream.

// Thanks,
// Miklos "

// IMHO, this validates our use of maps for directories (thp).

// INODES: There is a good definition of struct inode at
// http://www.makelinux.net/books/lkd2/ch12lev1sec6.  But
// we are not using them.  Rather we are using struct stat
// for the metadata of a file.


// Here are details on lstat() and struct stat from
// http://linux.die.net/man/2/stat:

// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// These functions return information about a file. No permissions are
// required on the file itself, but-in the case of stat() and lstat()
// - execute (search) permission is required on all of the directories
// in path that lead to the file.

// stat() stats the file pointed to by path and fills in buf.

// lstat() is identical to stat(), except that if path is a symbolic
// link, then the link itself is stat-ed, not the file that it refers
// to.

// fstat() is identical to stat(), except that the file to be stat-ed
// is specified by the file descriptor fd.

// All of these system calls return a stat structure, which contains
// the following fields:

//     struct stat {
//         dev_t     st_dev;     /* ID of device containing file */
//         ino_t     st_ino;     /* inode number */
//         mode_t    st_mode;    /* protection */
//         nlink_t   st_nlink;   /* number of hard links */
//         uid_t     st_uid;     /* user ID of owner */
//         gid_t     st_gid;     /* group ID of owner */
//         dev_t     st_rdev;    /* device ID (if special file) */
//         off_t     st_size;    /* total size, in bytes */
//         blksize_t st_blksize; /* blocksize for file system I/O */
//         blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
//         time_t    st_atime;   /* time of last access */
//         time_t    st_mtime;   /* time of last modification */
//         time_t    st_ctime;   /* time of last status change */
//     };

// The st_dev field describes the device on which this file
// resides. (The major(3) and minor(3) macros may be useful to
// decompose the device ID in this field.)

// The st_rdev field describes the device that this file (inode) represents.

// The st_size field gives the size of the file (if it is a regular
// file or a symbolic link) in bytes. The size of a symbolic link is
// the length of the pathname it contains, without a terminating null
// byte.

// The st_blocks field indicates the number of blocks allocated to the
// file, 512-byte units. (This may be smaller than st_size/512 when
// the file has holes.)

// The st_blksize field gives the "preferred" blocksize for efficient
// file system I/O. (Writing to a file in smaller chunks may cause an
// inefficient read-modify-rewrite.)

// Not all of the Linux file systems implement all of the time
// fields. Some file system types allow mounting in such a way that
// file and/or directory accesses do not cause an update of the
// st_atime field. (See noatime, nodiratime, and relatime in mount(8),
// and related information in mount(2).) In addition, st_atime is not
// updated if a file is opened with the O_NOATIME; see open(2).

// The field st_atime is changed by file accesses, for example, by
// execve(2), mknod(2), pipe(2), utime(2) and read(2) (of more than
// zero bytes). Other routines, like mmap(2), may or may not update
// st_atime.

// The field st_mtime is changed by file modifications, for example,
// by mknod(2), truncate(2), utime(2) and write(2) (of more than zero
// bytes). Moreover, st_mtime of a directory is changed by the
// creation or deletion of files in that directory. The st_mtime field
// is not changed for changes in owner, group, hard link count, or
// mode.

// The field st_ctime is changed by writing or by setting inode
// information (i.e., owner, group, link count, mode, etc.).

// The following POSIX macros are defined to check the file type using
// the st_mode field:

//     S_ISREG(m)

//     is it a regular file?

//     S_ISDIR(m)

//     directory?

//     S_ISCHR(m)

//     character device?

//     S_ISBLK(m)

//     block device?

//     S_ISFIFO(m)

//     FIFO (named pipe)?

//     S_ISLNK(m)

//     symbolic link? (Not in POSIX.1-1996.)

//     S_ISSOCK(m)

//     socket? (Not in POSIX.1-1996.) 
// The following flags are defined for the st_mode field:
// The set-group-ID bit (S_ISGID) has several special uses. For a
// directory it indicates that BSD semantics is to be used for that
// directory: files created there inherit their group ID from the
// directory, not from the effective group ID of the creating process,
// and directories created there will also get the S_ISGID bit
// set. For a file that does not have the group execution bit
// (S_IXGRP) set, the set-group-ID bit indicates mandatory file/record
// locking.

// The sticky bit (S_ISVTX) on a directory means that a file in that
// directory can be renamed or deleted only by the owner of the file,
// by the owner of the directory, and by a privileged process.
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



// The following gives information on configuring the root directory:
// "The root directory has a '.' and a '..' entry in it, and the inode
// number for each is the same. Traditionally, the inode number is 2;
// it still is on MacOS X, Linux, Solaris. So, when you do 'cd /; cd
// ..', you end up at the same location.

// In the 1980s, there was a system called Newcastle Connection that
// treated networked computers as being above the root of your local
// computer. Thus, on such a machine, you would type:

// cd /../othermachine/path/to/interesting/place

// to change directory to a remote file system.

// (You can find the paper via a Google search of 'Newcastle
// Connection' - the URL is intractable.)"
// See http://superuser.com/questions/186184/the-parent-of-the-root-directory

         // struct stat {
         //     unsigned int  st_dev;      /* ID of device containing file */
         //     unsigned int  st_ino;      /* inode number */
         //     mode_t        st_mode;     /* type and protection of file */
         //     unsigned int  st_nlink;    /* number of hard links */
         //     unsigned int  st_uid;      /* user ID of owner */
         //     unsigned int  st_gid;      /* group ID of owner */
         //     unsigned int  st_rdev;     /* device type (if inode device) */
         //     unsigned long st_size;     /* total size, in bytes */
         //     unsigned long st_blksize;  /* blocksize for filesystem I/O */
         //     unsigned long st_blocks;   /* number of 512B blocks allocated */
         //     time_t        st_atime;    /* time of last access */
         //     time_t        st_mtime;    /* time of last modification */
         //     time_t        st_ctime;    /* time of last change */
         // };
// Note that this shows the real types of all inode components except
// mode_t and time_t.
// for moredetails see: 
//   http://manpages.ubuntu.com/manpages/hardy/man2/stat.2.html

// Under development: Function to format ls lines in format of "ls -d"
int describe_file( string pathname ) {
    //string stream ccout;
    struct stat st;

    //struct stat st;       // "struct stat" because stat() is defined
    if ( my_lstat( pathname.c_str(), &st ) != 0 ) {
        errno = ENOENT;
        cdbg << "Cannot stat file [" << pathname
                 << "]: " << strerror(errno) << endl;
        return -1;
    }

    ino_t fh = find_ino(pathname);
    if(fh == 0)
    {
        return an_err;
    }

    cout << st.st_ino << ": "
             << ( (S_ISDIR(st.st_mode) != 0) ? 'd' : (  (S_ISLNK(st.st_mode) != 0) ? 'l' : '-' ) )
             << ( (st.st_mode & S_IRUSR) ? 'r' : '-' )
             << ( (st.st_mode & S_IWUSR) ? 'w' : '-' )
             << ( (st.st_mode & S_IXUSR) ? 'x' : '-' )
             << ( (st.st_mode & S_IRGRP) ? 'r' : '-' )
             << ( (st.st_mode & S_IWGRP) ? 'w' : '-' )
             << ( (st.st_mode & S_IXGRP) ? 'x' : '-' )
             << ( (st.st_mode & S_IROTH) ? 'r' : '-' )
             << ( (st.st_mode & S_IWOTH) ? 'w' : '-' )
             << ( (st.st_mode & S_IXOTH) ? 'x' : '-' )
             << ( ilist.entry[fh].xattr.size() ? '+' : ' ')
    ;

    char date[64];
    strftime( date, 15, "%b %d %H:%M  ", localtime( &st.st_mtime ) );

    //Sets value to be outputted
    //Name if the id exists
    //Or just id if the id does not exist
    struct passwd *uid = getpwuid(st.st_uid);
    struct group *gid = getgrgid(st.st_gid);
    string uid_out = ( uid == NULL ? to_string(st.st_uid) : uid->pw_name );
    string gid_out = ( gid == NULL ? to_string(st.st_gid) : gid->gr_name );

    printf( 
        "%2i %7s %7s %8ld %8s ",          // format string
        st.st_nlink,                      // number of links
        uid_out.c_str(),
        //getpwuid(st.st_uid)->pw_name,     // password name
        gid_out.c_str(),
        //getgrgid(st.st_gid)->gr_name,     // group name
        st.st_size,                       // size of file
        date                              // time of last modification
    );

    vector<string> v = split(pathname, "/");
    if(S_ISLNK(st.st_mode) )
    {
        cout << v.back() << " -> " << ilist.entry[find_real_ino(pathname)].data << endl;
    }
    else
    {
        cout << v.back() << endl;  
    }
}



// man readdir(3), lstat, opendir, closedir.

// Format of a directory entry: 
// http://www.delorie.com/gnu/docs/glibc/libc_270.html

// Testing the type of a file: 
// http://www.delorie.com/gnu/docs/glibc/libc_286.html
// (Test the st_mode field returned by stat on a given file.)

// To get Linux to use ascii ordering "export LANG=us.ascii"

// handy macro for iterating through an stl container
#define each(I) \
    for( auto it=(I).begin(); it!=(I).end(); ++it )


int visit( string root ) { // recursive visitor function, implements lslr

    // OPEN root
    MY_DIR* dirp;                                          // open DIR
    //if ( ! ( dirp = my_opendir( root.c_str() ) ) ) {
    if ( ! ( dirp = my_opendir( root.c_str() ) ) ) {
        cerr << "Cannot open directory " << root << ".\n";
        return -1;
    }

    // CREATE TWO LISTS OF FILE NAMES: file and hardSubdirectory
    list<string> file;          // names of each file in this dirctory
    list<string> hardSubdirectory;    // each hard-linked subdirectory
    while ( dirent* dp = my_readdir(dirp) ) {
        string s = dp->d_name;        // converts C string to C++ string
        if ( s == "." || s == ".." ) continue;             // skip these
        s = root + ( root.back() == '/'? "" : "/") + s;  // prepend the current path
        file.push_back( s ); 
        struct stat st;
        cdbg << "d_ino = " << dp->d_ino << endl;
        int error = my_fstat(dp->d_ino, &st);
        //show_stat(st);
     
        cdbg << "Considering " 
                 << s << dp->d_ino << " of mode " 
                 << oct << st.st_mode << " for hard directory.\n";
        if ( S_ISDIR(st.st_mode) ) {
        //if ( ( dp->d_type & DT_DIR ) && !(dp->d_type & DT_LNK) ) {
            hardSubdirectory.push_back( s );
        }
    }
    my_closedir(dirp);         // close DIR asap, to reset internal data
    cdbg << "hardSubdirectory has " << hardSubdirectory.size() << " entries.\n";
    for( auto it : hardSubdirectory ) cdbg << it << endl;

    // EMIT root's HEADER, INCLUDING ITS TOTAL SIZE
    cout << root << ":" << endl;
    cout << "total ";
    int size = 0;
    each( file ) {
        string filename = *it;
        struct stat st;
        if ( my_lstat( filename.c_str(), &st ) == 0 ) size += st.st_blocks;
    }
    cout << size/2 << endl;    // kilobytes-per-block correcton factor

    // lstat() AND REPORT ON EACH FILE WHOSE NAME IS IN root
    file.sort();  
    each( file ) {
        string filename = *it;    

        struct stat st;       // "struct stat" because stat() is defined
        if ( my_lstat( filename.c_str(), &st ) != 0 ) {
            cerr << "Cannot stat file " << filename 
                     << ": " << strerror(errno) << endl;
            return -1;
        }

        cout << st.st_ino << ": "     // added for CS179F
                 << ( (S_ISDIR(st.st_mode) != 0) ? 'd' : '-' )
                 << ( (st.st_mode & S_IRUSR) ? 'r' : '-' )
                 << ( (st.st_mode & S_IWUSR) ? 'w' : '-' )
                 << ( (st.st_mode & S_IXUSR) ? 'x' : '-' )
                 << ( (st.st_mode & S_IRGRP) ? 'r' : '-' )
                 << ( (st.st_mode & S_IWGRP) ? 'w' : '-' )
                 << ( (st.st_mode & S_IXGRP) ? 'x' : '-' )
                 << ( (st.st_mode & S_IROTH) ? 'r' : '-' )
                 << ( (st.st_mode & S_IWOTH) ? 'w' : '-' )
                 << ( (st.st_mode & S_IXOTH) ? 'x' : '-' )
        ;

        char date[64];
        strftime( date, 15, "%b %d %H:%M  ", localtime( &st.st_mtime ) );

        printf( 
            "%2i %7s %7s %8ld %8s ",          // format string
            st.st_nlink,                      // number of links
            getpwuid(st.st_uid)->pw_name,     // password name
            getgrgid(st.st_gid)->gr_name,     // group name
            st.st_size,                       // size of file
            date                              // time of last modification
        );

        cout << *it << endl;

    }

    // RECURSE THROUGH root's HARD-LINKED SUBDIRECTORIES AND RETURN
    each( hardSubdirectory ) {
        cout << endl;
        visit( *it );   
    }

    return 0;                                        // return success

}


// int main( int argc, char* argv[] ) { 
//   return visit( argc > 1 ? argv[1] : "." ); 
// }




int main(int argc, char* argv[] ) {
    //int cwd = 2;  // ino of current working directory;
    // The place for testing of functions.
    // cdbg << "Now we call initialize()" << endl;
    initialize();
    stringstream record;
    ifstream myin;
    if ( argc ) myin.open( argv[1] );
    for(;;) { // Idiom for infinite loop
        string op, file;
        // if ( myin.eof() ) exit(0);
        cout << "Which op and file? " << endl;
        (myin.good() ? myin : cin) >> op >> file;
        if ( op != "exit" ) record << op << " " << file << endl;
        if        ( op == "help" ) // lists available ops
        { 
        }
        else if (op == "play"  ) // accepts input from file instead of keyboard
        { 
        }
        else if (op == "save"  ) // saves dialog to specified file
        { 
        }
        else if (op == "mkdir" ) // prompts for protection mode
        { 
            cout << "Specify file permissions in octal: ";
            mode_t mode; 
            // cin >> oct >> mode;
            (myin.good()? myin : cin) >> oct >> mode;
            record << oct << mode << endl;
            my_mkdir(file.c_str(), mode );
        }
        else if (op == "rmdir"  ) // shows file's metadata
        {
            my_rmdir(file.c_str() );
        }
        else if (op == "show"  ) // shows file's metadata
        { 
            show_stat( ilist.entry[ int(find_ino(file)) ].metadata );
        }
        else if (op == "ls"  ) // lists the specified directory. 
        { 
            ls(file);
        }
        else if (op == "lstat"  ) // lists the specified directory. 
        { 
            struct stat a_stat;
            my_lstat(file.c_str(), &a_stat);
            show_stat(a_stat);
        }
        else if (op == "exit"  ) // quits 
                                 // save dialog so far to specified file.
        { 
            ofstream myfile;
            myfile.open (file);
            myfile << record.str();
            myfile.close();
            return 0;
        }
        else if (op == "break"  ) // executes the rest of main()
        { 
            break;
        }
        else if (op == "lslr"  ) // executes visit()
        { 
            visit(file);
        }
        else if (op == "creat" || op == "touch" ) //Creates a regular file
        {
            cout << "Specify file permissions in octal: ";
            mode_t mode; 
            // cin >> oct >> mode;
            (myin.good()? myin : cin) >> oct >> mode;
            record << oct << mode << endl;
            my_creat(file.c_str(), mode );
        }
        else if (op == "chown" ) //Changes owner of file
        {
            uid_t u;
            gid_t g;

            cout << "Specify user id: ";
            (myin.good() ? myin : cin) >> dec >> u;
            record << dec << u << endl;

            cout << "Specify group id: ";
            (myin.good() ? myin : cin) >> dec >> g;
            record << dec << g << endl;

            my_chown(file.c_str(), u, g);
        }
        else if (op == "chmod" )
        {
            cout << "Specify file permissions in octal: ";
            mode_t mode; 
            // cin >> oct >> mode;
            (myin.good()? myin : cin) >> oct >> mode;
            record << oct << mode << endl;
            my_chmod(file.c_str(), mode );
        }
        else if (op == "access" )
        {
            cout << "Specify permissions to check: ";
            mode_t mode; 
            // cin >> oct >> mode;
            (myin.good()? myin : cin) >> oct >> mode;
            record << oct << mode << endl;
            int ret = my_access(file.c_str(), mode);
            cout << ( ret == 0 ? "All requested permissions granted" : "Some permissions failed" ) << endl;
        }
		else if (op == "link")
        {
			cout << "Specify new path: ";
            string newpath;
            (myin.good()? myin : cin) >> newpath;
            record << newpath << endl;
            cout << "file: [" << file << "] newpath: [" << newpath << "]" << endl;
            my_link(file.c_str(), newpath.c_str() );
		}
        else if (op == "unlink")
        {
            cout << "Unlinking file: " << file << endl;
            my_unlink(file.c_str() );
        }
		else if(op == "cwd")
		{
			// Print current working directory
			cout << "cwd = " << cwd << endl;
		}
        else if (op == "rename")
        {
            cout << "Specify new file name: ";
            string newpath;
            (myin.good() ? myin : cin) >> newpath;
            record << newpath << endl;
            cout << "Renaming " << file << " to " << newpath << endl;
            my_rename(file.c_str(), newpath.c_str());
        }
        else if (op == "pread")
        {
            int offset, size;
            cout << "Specify starting position: ";
            (myin.good() ? myin : cin) >> offset;
            cout << "Specify number of characters to read: ";
            (myin.good() ? myin : cin) >> (dec) >> size;

            //cdbg << "MAIN: offset: " << (dec) << offset << (dec) << " size: " << size << endl;

            char data[size+1];
            //char* data = (char*)malloc(size+1);
            //cdbg << "buf len: " << sizeof(data) << endl;
            my_pread(find_ino(file), data, size, offset);
            //int my_pread( int fh, char *buf, size_t size, off_t offset )
            cout << "Read [" << (string)data << "] from " << file << endl;
        }
        else if (op == "pwrite")
        {
            int offset;
            cout << "Specify starting position: ";
            (myin.good() ? myin : cin) >> offset;
            cout << "Specify file data: " << endl;
            string data;
            cin.ignore();
            getline(cin,data);
            
            record << data << endl;
            cout << "Writing [" << data << "] to " << file << endl;
            my_pwrite(find_ino(file), data.c_str(), data.size(), offset);
            //my_pwrite( int fh, const char *buf, size_t size, off_t offset )
        }
        else if (op == "symlink")
        {
            string link;

            //cout << "Specifiy path: ";
            //(myin.good() ? myin : cin) >> path;
            cout << "Specifiy link: ";
            (myin.good() ? myin : cin) >> link;

            my_symlink(file.c_str(),link.c_str());
        }
        else if (op == "readlink")
        {
            char link[20];
            int err = my_readlink(file.c_str(),link,20);
            if(err != -1)
            {
                cout << file << "->" << link << endl;
            }
        }
        else if (op == "utime")
        {
            struct utimbuf utime;
            int atime, mtime;
            cout << "Specify new access time in decimal seconds: " << endl;
            cin >> utime.actime;
            cout << "Specifiy new mod time in decimal seconds: " << endl;
            cin >> utime.modtime;

            my_utime(file.c_str(),&utime);
        }
        else if (op == "truncate")
        {
            cout << "Specify new file size: ";
            int n;
            cin >> n;
            my_truncate(file.c_str(), n);
        }
        else if (op == "statvfs")
        {
            struct statvfs statbuf;
            int err = my_statvfs(file.c_str(), &statbuf);
            show_statvfs(&statbuf);
        }
        else if (op == "lsetxattr")
        {
            string name, val;
            cout << "Specify name: ";
            cin >> name;
            cout << "Specify value: ";
            cin >> val;

            my_lsetxattr(file.c_str(),name.c_str(),val.c_str(),0,0);
        }
        else if (op == "lgetxattr")
        {
            string name, val;
            int size;
            cout << "Specify name: ";
            cin >> name;
            cout << "Specify size: ";
            cin >> size;

            val.resize(size+1,'\0');

            my_lgetxattr(file.c_str(), name.c_str(), (char*)val.c_str(), size, 0);

        }
        else if (op == "lremovexattr")
        {
            string name;
            cout << "Specify name: ";
            cin >> name;

            my_lremovexattr(file.c_str(), name.c_str());          
        }
        else if (op == "llistxattr")
        {
            int size = 100;
            char buf[size];
            size = my_llistxattr(file.c_str(), buf, size);
            print_xattr_list(buf, size);
        }
        else
        {
            cout << "Correct usage is: op pathname,\n"; 
            cout << "where \"op\" is one of the following:\n";
            cout << setw(10) << left << "help" <<
                    setw(10) << "play" <<
                    setw(10) <<  "save" <<
                    setw(10) <<  "mkdir" <<
                    setw(10) << "rmdir" << endl <<
                    setw(10) << "show " <<
                    setw(10) << "ls " << 
                    setw(10) << "lstat " << 
                    setw(10) << "break" << 
                    setw(10) << " lslr" << endl <<
                    setw(10) << "exit" <<
                    setw(10) << "creat" <<
                    setw(10) << "touch" <<
                    setw(10) << "chown" <<
                    setw(10) << "chmod" << endl <<
                    setw(10) << "access" <<
                    setw(10) << "link" <<
                    setw(10) << "unlink" <<
                    setw(10) << "cwd" <<
                    setw(10) << "rename" << endl <<
                    setw(10) << "pread" <<
                    setw(10) << "pwrite" <<
                    setw(10) << "symlink" <<
                    setw(10) << "readlink" <<
                    setw(10) << "utime" << endl <<
                    setw(10) << "truncate" <<
                    setw(10) << "statvfs" <<
                    setw(10) << "lsetxattr" <<
                    setw(10) << "lgetxattr" <<
                    setw(10) << "lremovexattr " << setw(10) << "llistxattr" << endl;
            cout << "For example, type \"exit now\" to exit.\n";
        }
    }  

    // Continuation of main(), which is reacable via the "break" op.
    show_stat( ilist.entry[2].metadata );  // all looks good here.
    cdbg << "Now we call lstat on \"/\" and &mystat" << endl;
    struct stat mystat;
    my_lstat("/",&mystat);
    cdbg << "Now we call ls on \"/\"" << endl;
    ls("/"); 
    cdbg << "Now we call mkdir on \"/junk\" and 0700" << endl;
    my_mkdir("/junk", 0700);
    cdbg << "Now we call ls on \"/\"" << endl;
    ls("/");

    cdbg << "now we call ls on \"/junk\"" << endl;
    //  cout << endl;
    ls("/junk");
    
    cdbg << "Now we call mkdir on \"/junk/stuff\" and 0700" << endl;
    my_mkdir("/junk/stuff", 700);
    cdbg << "now we call ls on \"/junk\"" << endl;
    ls("/junk");
    cdbg << "now we call ls on \"/junk/stuff\"" << endl;
    ls("/junk/stuff");

    cout << endl;
    describe_file( "/junk" );
    cout << endl;
}


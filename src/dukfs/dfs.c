/*
Filesystem library for Duktape.

Based on the LuaFilesystem library
https://github.com/keplerproject/luafilesystem

define BUILD_AS_DLL to build as a duktape module.
*/

#include <duktape.h>

#include <string.h>

#if defined( _WIN32 )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <direct.h>

#include <sys/types.h>
#include <sys/stat.h>

#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))

#include <dirent.h>

#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#endif


#if defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )
	#define chdir _chdir
	#define getcwd _getcwd
	#define mkdir _mkdir
	#define rmdir _rmdir
  #define snprintf _snprintf
#endif



static duk_ret_t dfs_chdir(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	int ret = chdir(path);

	duk_push_boolean(ctx, ret == 0);
	return 1;
}


static duk_ret_t dfs_getcwd(duk_context *ctx) {
	char *buffer = getcwd(NULL, 0);

	if (buffer == NULL) {
		duk_push_null(ctx);
	} else {
		duk_push_string(ctx, buffer);
		free(buffer);
	}

	return 1;
}


static duk_ret_t dfs_mkdir(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

#ifdef _WIN32
	int ret = mkdir(path);
#else
	int ret = mkdir (path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
                             S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH );
#endif

	duk_push_boolean(ctx, ret == 0);
	return 1;
}


static duk_ret_t dfs_rmdir(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	int ret = rmdir(path);

	duk_push_boolean(ctx, ret == 0);
	return 1;
}


static duk_ret_t dfs_dir(duk_context *ctx) {
  duk_idx_t arr_idx;
  arr_idx = duk_push_array(ctx);
  int i = 0;

  const char *path = duk_require_string(ctx, 0);

#ifdef _WIN32

  WIN32_FIND_DATA ffd;
  char directory[MAX_PATH];
  HANDLE hFind = INVALID_HANDLE_VALUE;
  DWORD dwError=0;

  snprintf(directory, MAX_PATH, "%s\\*", path);

  hFind = FindFirstFile(directory, &ffd);
  if (hFind == INVALID_HANDLE_VALUE) {
    duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open directory %s", path);
    return -1;
  }

  do {

    if (!strcmp(ffd.cFileName, ".") || !strcmp(ffd.cFileName, "..")) {
      continue;
    }

    duk_push_string(ctx, ffd.cFileName);
    duk_put_prop_index(ctx, arr_idx, i++);

  } while (FindNextFile(hFind, &ffd) != 0);

  dwError = GetLastError();
  if (dwError != ERROR_NO_MORE_FILES) 
  {
    duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not read directory %s", path);
    return -1;
  }

  FindClose(hFind);

#else

  struct dirent *entry;
  DIR *dp;

  dp = opendir(path);
  if (dp == NULL) {
    duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open directory %s", path);
    return -1;
  }

  while((entry = readdir(dp))) {

    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
      continue;
    }
    
    duk_push_string(ctx, entry->d_name);
    duk_put_prop_index(ctx, arr_idx, i++);
  }

  closedir(dp);

#endif

  return 1;
}


// ---------------------------------------------------------------------------------------------------

/*
stat serializing methods copied from luafilesystem
adapted by armornick for use with Duktape

licensed under the MIT License
Copyright © 2003-2014 Kepler Project.
*/


#ifdef _WIN32
 #ifdef __BORLANDC__
  #define STAT_STRUCT struct stati64
 #else
  #define STAT_STRUCT struct _stati64
 #endif
#define STAT_FUNC _stati64
#define LSTAT_FUNC STAT_FUNC
#else
#define STAT_STRUCT struct stat
#define STAT_FUNC stat
#define LSTAT_FUNC lstat
#endif


#ifdef _WIN32
 #ifndef S_ISDIR
   #define S_ISDIR(mode)  (mode&_S_IFDIR)
 #endif
 #ifndef S_ISREG
   #define S_ISREG(mode)  (mode&_S_IFREG)
 #endif
 #ifndef S_ISLNK
   #define S_ISLNK(mode)  (0)
 #endif
 #ifndef S_ISSOCK
   #define S_ISSOCK(mode)  (0)
 #endif
 #ifndef S_ISFIFO
   #define S_ISFIFO(mode)  (0)
 #endif
 #ifndef S_ISCHR
   #define S_ISCHR(mode)  (mode&_S_IFCHR)
 #endif
 #ifndef S_ISBLK
   #define S_ISBLK(mode)  (0)
 #endif
#endif
/*
** Convert the inode protection mode to a string.
*/
#ifdef _WIN32
static const char *mode2string (unsigned short mode) {
#else
static const char *mode2string (mode_t mode) {
#endif
  if ( S_ISREG(mode) )
    return "file";
  else if ( S_ISDIR(mode) )
    return "directory";
  else if ( S_ISLNK(mode) )
        return "link";
  else if ( S_ISSOCK(mode) )
    return "socket";
  else if ( S_ISFIFO(mode) )
        return "named pipe";
  else if ( S_ISCHR(mode) )
        return "char device";
  else if ( S_ISBLK(mode) )
        return "block device";
  else
        return "other";
}

 /*
** Convert the inode protection mode to a permission list.
*/

#ifdef _WIN32
static const char *perm2string (unsigned short mode) {
  static char perms[10] = "---------";
  int i;
  for (i=0;i<9;i++) perms[i]='-';
  if (mode  & _S_IREAD)
   { perms[0] = 'r'; perms[3] = 'r'; perms[6] = 'r'; }
  if (mode  & _S_IWRITE)
   { perms[1] = 'w'; perms[4] = 'w'; perms[7] = 'w'; }
  if (mode  & _S_IEXEC)
   { perms[2] = 'x'; perms[5] = 'x'; perms[8] = 'x'; }
  return perms;
}
#else
static const char *perm2string (mode_t mode) {
  static char perms[10] = "---------";
  int i;
  for (i=0;i<9;i++) perms[i]='-';
  if (mode & S_IRUSR) perms[0] = 'r';
  if (mode & S_IWUSR) perms[1] = 'w';
  if (mode & S_IXUSR) perms[2] = 'x';
  if (mode & S_IRGRP) perms[3] = 'r';
  if (mode & S_IWGRP) perms[4] = 'w';
  if (mode & S_IXGRP) perms[5] = 'x';
  if (mode & S_IROTH) perms[6] = 'r';
  if (mode & S_IWOTH) perms[7] = 'w';
  if (mode & S_IXOTH) perms[8] = 'x';
  return perms;
}
#endif

/* inode protection mode */
static void push_st_mode (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_string(ctx, mode2string (info->st_mode));
}
/* device inode resides on */
static void push_st_dev (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t) info->st_dev);
}
/* inode's number */
static void push_st_ino (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t) info->st_ino);
}
/* number of hard links to the file */
static void push_st_nlink (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t)info->st_nlink);
}
/* user-id of owner */
static void push_st_uid (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t)info->st_uid);
}
/* group-id of owner */
static void push_st_gid (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t)info->st_gid);
}
/* device type, for special file inode */
static void push_st_rdev (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t) info->st_rdev);
}
/* time of last access */
static void push_st_atime (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t) info->st_atime);
}
/* time of last data modification */
static void push_st_mtime (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t) info->st_mtime);
}
/* time of last file status change */
static void push_st_ctime (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t) info->st_ctime);
}
/* file size, in bytes */
static void push_st_size (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t)info->st_size);
}
#ifndef _WIN32
/* blocks allocated for file */
static void push_st_blocks (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t)info->st_blocks);
}
/* optimal file system I/O blocksize */
static void push_st_blksize (duk_context *ctx, STAT_STRUCT *info) {
        duk_push_int (ctx, (duk_int_t)info->st_blksize);
}
#endif

/* permssions string */
static void push_st_perm (duk_context *ctx, STAT_STRUCT *info) {
    duk_push_string (ctx, perm2string (info->st_mode));
}


typedef void (*_push_function) (duk_context *ctx, STAT_STRUCT *info);

struct _stat_members {
        const char *name;
        _push_function push;
};

struct _stat_members members[] = {
        { "mode",         push_st_mode },
        { "dev",          push_st_dev },
        { "ino",          push_st_ino },
        { "nlink",        push_st_nlink },
        { "uid",          push_st_uid },
        { "gid",          push_st_gid },
        { "rdev",         push_st_rdev },
        { "access",       push_st_atime },
        { "modification", push_st_mtime },
        { "change",       push_st_ctime },
        { "size",         push_st_size },
        { "permissions",  push_st_perm },
#ifndef _WIN32
        { "blocks",       push_st_blocks },
        { "blksize",      push_st_blksize },
#endif
        { NULL, NULL }
};


static duk_ret_t _dfs_attributes(duk_context *ctx, int (*st)(const char*, STAT_STRUCT*)) {
	STAT_STRUCT info;
	const char *path = duk_require_string(ctx, 0);
	int i;

	if (st(path, &info)) {
		duk_push_null(ctx);
		return 1;
	}

	if (duk_is_string(ctx, 1)) {
		const char *member = duk_require_string(ctx, 1);

		for (i = 0; members[i].name; i++) {
            if (strcmp(members[i].name, member) == 0) {
                /* push member value and return */
                members[i].push (ctx, &info);
                return 1;
    	    }
    	}

    	duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "invalid attribute name %s", member);
    	return -1;
	}

	duk_push_object(ctx);

	/* stores all members in table on top of the stack */
    for (i = 0; members[i].name; i++) {
        members[i].push (ctx, &info);
        duk_put_prop_string(ctx, -2, members[i].name);
    }
    return 1;
}

static duk_ret_t dfs_attributes(duk_context *ctx) {
	return _dfs_attributes(ctx, STAT_FUNC);
}


// ---------------------------------------------------------------------------------------------------

static const duk_function_list_entry dfs_module[] = {
	{ "chdir", dfs_chdir, 1 },
	{ "currentdir", dfs_getcwd, 0 },
	{ "mkdir", dfs_mkdir, 1 },
	{ "rmdir", dfs_rmdir, 1 },
  { "attributes", dfs_attributes, 2 },
  { "dir", dfs_dir, 1 },
	{ NULL, NULL, 0}
};

static int dfs_core(duk_context *ctx) {
	int mod = duk_push_object(ctx);
	duk_put_function_list(ctx, -1, dfs_module);
	return mod;
}

#ifdef BUILD_AS_DLL

#if defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

DLL_EXPORT duk_ret_t dukopen_dfs(duk_context *ctx) {
	dfs_core(ctx);
	return 1;
}

#else

void register_dfs(duk_context *ctx) {
	dfs_core(ctx);
	duk_put_global_string(ctx, "dfs");

	duk_eval_string_noresult(ctx, "Duktape.modLoaded['dfs'] = dfs");
}

#endif
/*
OS module for Duktape.
Implements a subset of the os module of Node.js
see: https://nodejs.org/api/os.html
*/

#include "duknode.h"

/*
------------------------------------------------------------------------------------
*/

#define DFS_STATS_PROTOTYPE "fs.Stats"

static duk_ret_t dfs_stats_isfile(duk_context *ctx) {
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "mode");

	int mode = duk_require_int(ctx, -1);
	duk_pop_2(ctx);
	duk_push_boolean(ctx, (mode & S_IFREG) == S_IFREG);

	return 1;
}

static duk_ret_t dfs_stats_isdirectory(duk_context *ctx) {
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "mode");

	int mode = duk_require_int(ctx, -1);
	duk_pop_2(ctx);
	duk_push_boolean(ctx, (mode & S_IFDIR) == S_IFDIR);

	return 1;
}

static duk_ret_t dfs_stats_isblockdevice(duk_context *ctx) {
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "mode");

	int mode = duk_require_int(ctx, -1);
	duk_pop_2(ctx);
#ifdef S_IFBLK
	duk_push_boolean(ctx, (mode & S_IFBLK) == S_IFBLK);
#else
	duk_push_boolean(ctx, 0);
#endif

	return 1;
}

static duk_ret_t dfs_stats_ischaracterdevice(duk_context *ctx) {
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "mode");

	int mode = duk_require_int(ctx, -1);
	duk_pop_2(ctx);
#ifdef S_IFCHR
	duk_push_boolean(ctx, (mode & S_IFCHR) == S_IFCHR);
#else
	duk_push_boolean(ctx, 0);
#endif

	return 1;
}

static duk_ret_t dfs_stats_issocket(duk_context *ctx) {
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "mode");

	int mode = duk_require_int(ctx, -1);
	duk_pop_2(ctx);
#ifdef S_IFSOCK
	duk_push_boolean(ctx, (mode & S_IFSOCK) == S_IFSOCK);
#else
	duk_push_boolean(ctx, 0);
#endif

	return 1;
}

static duk_ret_t dfs_stats_isfifo(duk_context *ctx) {
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "mode");

	int mode = duk_require_int(ctx, -1);
	duk_pop_2(ctx);
#ifdef S_IFIFO
	duk_push_boolean(ctx, (mode & S_IFIFO) == S_IFIFO);
#else
	duk_push_boolean(ctx, 0);
#endif

	return 1;
}

static const duk_function_list_entry dfs_stats_methods[] = {
	{ "isFile", dfs_stats_isfile, 0 },
	{ "isDirectory", dfs_stats_isdirectory, 0 },
	{ "isBlockDevice", dfs_stats_isblockdevice, 0 },
	{ "isCharacterDevice", dfs_stats_ischaracterdevice, 0 },
	{ "isFIFO", dfs_stats_isfifo, 0 },
	{ "isSocket", dfs_stats_issocket, 0 },
	{ NULL, NULL, 0}
};

/*
------------------------------------------------------------------------------------
*/

static int dfs_make_file_array(duk_context *ctx, const char *path) {
	int i = 0;
	duk_idx_t arr_idx = duk_push_array(ctx);

#if DUKNODE_PLATFORM_WINDOWS
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;

	char searchpath[DUKNODE_MAX_PATH];
	snprintf(searchpath, DUKNODE_MAX_PATH, "%s\\*", path);

	hFind = FindFirstFile(searchpath, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
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
	if (dwError != ERROR_NO_MORE_FILES) {
		return -1;
	}

	FindClose(hFind);
#else
	DIR *dirp;
	struct dirent *dp;

	if ((dirp = opendir(path)) == NULL) {
		return -1;
	}

	do {
		if ((dp = readdir(dirp)) != NULL) {
			if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
				continue;
			}

			duk_push_string(ctx, dp->d_name);
			duk_put_prop_index(ctx, arr_idx, i++);
		}
	} while (dp != NULL);

	(void) closedir(dirp);
#endif

	return 0;
}

/*
------------------------------------------------------------------------------------
*/

static duk_ret_t dfs_rename(duk_context *ctx) {
	const char *oldPath = duk_require_string(ctx, 0);
	const char *newPath = duk_require_string(ctx, 1);

	if (!duk_is_function(ctx, 2)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as third argument");
		return -1;
	}
	duk_dup(ctx, 2);

	if (rename(oldPath, newPath)) {
		// duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not rename %s to %s", oldPath, newPath);
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not rename %s to %s: %s", oldPath, newPath, strerror(errno));
	} else {
		duk_push_null(ctx);
	}

	duk_call(ctx, 1);
	return 0;
}

static duk_ret_t dfs_rename_sync(duk_context *ctx) {
	const char *oldPath = duk_require_string(ctx, 0);
	const char *newPath = duk_require_string(ctx, 1);

	if (rename(oldPath, newPath)) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not rename %s to %s: %s", oldPath, newPath, strerror(errno));
		return -1;
	}

	duk_push_undefined(ctx);
	return 1;
}

static void dfs_push_stat(duk_context *ctx, struct stat *buf) {
	duk_push_object(ctx);

	duk_push_int(ctx, buf->st_size);
	duk_put_prop_string(ctx, -2, "size");

	duk_push_int(ctx, buf->st_mode);
	duk_put_prop_string(ctx, -2, "mode");

	duk_push_int(ctx, buf->st_dev);
	duk_put_prop_string(ctx, -2, "dev");

	duk_push_int(ctx, buf->st_gid);
	duk_put_prop_string(ctx, -2, "gid");

	duk_push_int(ctx, buf->st_ino);
	duk_put_prop_string(ctx, -2, "ino");

	duk_push_int(ctx, buf->st_nlink);
	duk_put_prop_string(ctx, -2, "nlink");

	duk_push_int(ctx, buf->st_rdev);
	duk_put_prop_string(ctx, -2, "rdev");

	duk_push_int(ctx, buf->st_uid);
	duk_put_prop_string(ctx, -2, "uid");

	duk_push_int(ctx, buf->st_atime);
	duk_put_prop_string(ctx, -2, "atime");

	duk_push_int(ctx, buf->st_ctime);
	duk_put_prop_string(ctx, -2, "ctime");

	duk_push_int(ctx, buf->st_mtime);
	duk_put_prop_string(ctx, -2, "mtime");

	duk_get_global_string(ctx, DFS_STATS_PROTOTYPE);
	duk_set_prototype(ctx, -2);
}

static duk_ret_t dfs_stat(duk_context *ctx) {
	struct stat buf;
	const char *path = duk_require_string(ctx, 0);

	if (!duk_is_function(ctx, 1)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as second argument");
		return -1;
	}
	duk_dup(ctx, 1);

	int result = stat(path, &buf);
	if (result != 0) {
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not get file information for %s: %s", path, strerror(errno));
		duk_push_null(ctx);
	} else {
		duk_push_null(ctx);
		dfs_push_stat(ctx, &buf);
	}

	duk_call(ctx, 2);
	return 0;
}

static duk_ret_t dfs_stat_sync(duk_context *ctx) {
	struct stat buf;
	const char *path = duk_require_string(ctx, 0);

	int result = stat(path, &buf);
	if (result != 0) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not get file information for %s: %s", path, strerror(errno));
		return -1;
	}

	dfs_push_stat(ctx, &buf);
	return 1;
}

static duk_ret_t dfs_lstat(duk_context *ctx) {
	struct stat buf;
	const char *path = duk_require_string(ctx, 0);

	if (!duk_is_function(ctx, 1)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as second argument");
		return -1;
	}
	duk_dup(ctx, 1);

	int result = lstat(path, &buf);
	if (result != 0) {
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not get file information for %s: %s", path, strerror(errno));
		duk_push_null(ctx);
	} else {
		duk_push_null(ctx);
		dfs_push_stat(ctx, &buf);
	}

	duk_call(ctx, 2);
	return 0;
}

static duk_ret_t dfs_lstat_sync(duk_context *ctx) {
	struct stat buf;
	const char *path = duk_require_string(ctx, 0);

	int result = lstat(path, &buf);
	if (result != 0) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not get file information for %s: %s", path, strerror(errno));
		return -1;
	}

	dfs_push_stat(ctx, &buf);
	return 1;
}

static duk_ret_t dfs_realpath(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	if (!duk_is_function(ctx, 1)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as second argument");
		return -1;
	}
	duk_dup(ctx, 1);

#if DUKNODE_PLATFORM_WINDOWS 
	char fullpath[DUKNODE_MAX_PATH];
	DWORD retval = 0;

	retval = GetFullPathName(path, DUKNODE_MAX_PATH, fullpath, NULL);

	if (retval == 0) {
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not get realpath for %s: %s", path, GetLastError());
		duk_push_null(ctx);
	} else {
		duk_push_null(ctx);
		duk_push_string(ctx, fullpath);
	}
#else
	char fullpath[PATH_MAX+1];

	char *res = realpath(path, fullpath);

	if (res == NULL) {
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not get realpath for %s: %s", path, strerror(errno));
		duk_push_null(ctx);
	} else {
		duk_push_null(ctx);
		duk_push_string(ctx, fullpath);
	}
#endif

	duk_call(ctx, 2);
	return 0;
}

static duk_ret_t dfs_realpath_sync(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

#if DUKNODE_PLATFORM_WINDOWS 
	char fullpath[DUKNODE_MAX_PATH];
	DWORD retval = 0;

	retval = GetFullPathName(path, DUKNODE_MAX_PATH, fullpath, NULL);

	if (retval == 0) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not get realpath for %s: %s", path, GetLastError());
		return -1;
	}
#else
	char fullpath[PATH_MAX+1];

	char *res = realpath(path, fullpath);

	if (res == NULL) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not get realpath for %s: %s", path, strerror(errno));
		return -1;
	}
#endif

	duk_push_string(ctx, fullpath);
	return 1;
}

static duk_ret_t dfs_remove(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	if (!duk_is_function(ctx, 1)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as second argument");
		return -1;
	}
	duk_dup(ctx, 1);

	if (remove(path)) {
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not remove %s: %s", path, strerror(errno));
	} else {
		duk_push_null(ctx);
	}

	duk_call(ctx, 1);
	return 0;
}

static duk_ret_t dfs_remove_sync(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	if (remove(path)) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not remove %s: %s", path, strerror(errno));
		return -1;
	}

	duk_push_undefined(ctx);
	return 1;
}

static duk_ret_t dfs_mkdir(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	if (!duk_is_function(ctx, 1)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as second argument");
		return -1;
	}
	duk_dup(ctx, 1);

	if (mkdir(path, 0777)) {
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not create directory %s: %s", path, strerror(errno));
	} else {
		duk_push_null(ctx);
	}

	duk_call(ctx, 1);
	return 0;
}

static duk_ret_t dfs_mkdir_sync(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	if (mkdir(path, 0777)) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not create directory %s: %s", path, strerror(errno));
		return -1;
	}

	duk_push_undefined(ctx);
	return 1;
}

// dfs_make_file_array
static duk_ret_t dfs_readdir(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	if (!duk_is_function(ctx, 1)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as second argument");
		return -1;
	}
	duk_dup(ctx, 1);

	duk_push_null(ctx); /* pre-emptively push null in case we succeed */
	if (dfs_make_file_array(ctx, path)) {
		duk_pop_2(ctx); /* pop null and array */
#if DUKNODE_PLATFORM_WINDOWS
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not read directory %s: %s", path, GetLastError());
#else
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not read directory %s: %s", path, strerror(errno));
#endif
		duk_push_null(ctx);
	}

	duk_call(ctx, 2);
	return 0;
}

static duk_ret_t dfs_readdir_sync(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	if (dfs_make_file_array(ctx, path)) {
#if DUKNODE_PLATFORM_WINDOWS
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not read directory %s: %s", path, GetLastError());
#else
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not read directory %s: %s", path, strerror(errno));
#endif
		return -1;
	}

	return 1;
}

static duk_ret_t dfs_readfile(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	FILE *inputf = NULL;

	if (!duk_is_function(ctx, 1)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as second argument");
		return -1;
	}
	duk_dup(ctx, 1);

	duk_push_null(ctx); /* pre-emptively push null in case we succeed */
	inputf = fopen(filename, "rb");
	if (inputf == NULL) {
		duk_pop(ctx);
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file %s: %s", filename, strerror(errno));
		duk_push_null(ctx);
		duk_call(ctx, 2);
		return 0;
	}

	fseek(inputf, 0, SEEK_END);
	long size = ftell(inputf);
	fseek(inputf, 0, SEEK_SET);
	void *bytes = duk_push_fixed_buffer(ctx, size);
	fread(bytes, 1, size, inputf);
	fclose(inputf);

	duk_call(ctx, 2);
	return 0;
}

static duk_ret_t dfs_readfile_sync(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	FILE *inputf = NULL;

	inputf = fopen(filename, "rb");
	if (inputf == NULL) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file %s: %s", filename, strerror(errno));
		return -1;
	}

	fseek(inputf, 0, SEEK_END);
	long size = ftell(inputf);
	fseek(inputf, 0, SEEK_SET);
	void *bytes = duk_push_fixed_buffer(ctx, size);
	fread(bytes, 1, size, inputf);
	fclose(inputf);

	return 1;
}

static duk_ret_t dfs_writefile(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	FILE *outputf = NULL;

	if (!duk_is_function(ctx, 2)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as third argument");
		return -1;
	}
	duk_dup(ctx, 2);

	outputf = fopen(filename, "wb");
	if (outputf == NULL) {
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file %s: %s", filename, strerror(errno));
		duk_call(ctx, 1);
		return 0;
	}

	if (duk_is_buffer(ctx, 1)) {
		
		void *buffer; duk_size_t sz;
		buffer = duk_get_buffer(ctx, 1, &sz);
		fwrite(buffer, 1, sz, outputf);

	} else if (duk_is_string(ctx, 1)) {

		const char *outputs = duk_get_string(ctx, 1);
		fputs(outputs, outputf);

	} else {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "string or buffer expected as second argument");
		return -1;
	}

	duk_push_null(ctx);
	duk_call(ctx, 1);
	return 0;
}

static duk_ret_t dfs_writefile_sync(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	FILE *outputf = NULL;

	outputf = fopen(filename, "wb");
	if (outputf == NULL) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file %s: %s", filename, strerror(errno));
		return -1;
	}

	if (duk_is_buffer(ctx, 1)) {
		
		void *buffer; duk_size_t sz;
		buffer = duk_get_buffer(ctx, 1, &sz);
		fwrite(buffer, 1, sz, outputf);

	} else if (duk_is_string(ctx, 1)) {

		const char *outputs = duk_get_string(ctx, 1);
		fputs(outputs, outputf);

	} else {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "string or buffer expected as second argument");
		return -1;
	}

	duk_push_undefined(ctx);
	return 1;
}

static duk_ret_t dfs_appendfile(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	FILE *outputf = NULL;

	if (!duk_is_function(ctx, 2)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "function expected as third argument");
		return -1;
	}
	duk_dup(ctx, 2);

	outputf = fopen(filename, "ab");
	if (outputf == NULL) {
		duk_push_error_object(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file %s: %s", filename, strerror(errno));
		duk_call(ctx, 1);
		return 0;
	}

	if (duk_is_buffer(ctx, 1)) {
		
		void *buffer; duk_size_t sz;
		buffer = duk_get_buffer(ctx, 1, &sz);
		fwrite(buffer, 1, sz, outputf);

	} else if (duk_is_string(ctx, 1)) {

		const char *outputs = duk_get_string(ctx, 1);
		fputs(outputs, outputf);

	} else {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "string or buffer expected as second argument");
		return -1;
	}

	duk_push_null(ctx);
	duk_call(ctx, 1);
	return 0;
}

static duk_ret_t dfs_appendfile_sync(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	FILE *outputf = NULL;

	outputf = fopen(filename, "ab");
	if (outputf == NULL) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file %s: %s", filename, strerror(errno));
		return -1;
	}

	if (duk_is_buffer(ctx, 1)) {
		
		void *buffer; duk_size_t sz;
		buffer = duk_get_buffer(ctx, 1, &sz);
		fwrite(buffer, 1, sz, outputf);

	} else if (duk_is_string(ctx, 1)) {

		const char *outputs = duk_get_string(ctx, 1);
		fputs(outputs, outputf);

	} else {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "string or buffer expected as second argument");
		return -1;
	}

	duk_push_undefined(ctx);
	return 1;
}

/*
------------------------------------------------------------------------------------
*/

static const duk_function_list_entry dfs_module[] = {
	{ "rename", dfs_rename, 3 },
	{ "renameSync", dfs_rename_sync, 2 },
	{ "stat", dfs_stat, 2 },
	{ "statSync", dfs_stat_sync, 1 },
	{ "lstat", dfs_lstat, 2 },
	{ "lstatSync", dfs_lstat_sync, 1 },
	{ "realpath", dfs_realpath, 2 },
	{ "realpathSync", dfs_realpath_sync, 1 },
	{ "unlink", dfs_remove, 2 },
	{ "unlinkSync", dfs_remove_sync, 1 },
	{ "rmdir", dfs_remove, 2 },
	{ "rmdirSync", dfs_remove_sync, 1 },
	{ "mkdir", dfs_mkdir, 2 },
	{ "mkdirSync", dfs_mkdir_sync, 1 },
	{ "readdir", dfs_readdir, 2 },
	{ "readdirSync", dfs_readdir_sync, 1 },
	{ "readFile", dfs_readfile, 2 },
	{ "readFileSync", dfs_readfile_sync, 1 },
	{ "writeFile", dfs_writefile, 3 },
	{ "writeFileSync", dfs_writefile_sync, 2 },
	{ "appendFile", dfs_appendfile, 3 },
	{ "appendFileSync", dfs_appendfile_sync, 2 },
	{ NULL, NULL, 0}
};

static void dfs_core(duk_context *ctx) {
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, dfs_stats_methods);
	duk_put_global_string(ctx, DFS_STATS_PROTOTYPE);

	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, dfs_module);
}

#ifdef BUILD_AS_DLL

DLL_EXPORT duk_ret_t dukopen_fs(duk_context *ctx) {
	dfs_core(ctx);
	return 1;
}

#else

void register_dfs(duk_context *ctx) {
	dfs_core(ctx);
	duk_put_global_string(ctx, "fs");
}

void preload_dfs(duk_context *ctx) {
	duk_get_global_string(ctx, "package");
	duk_get_prop_string(ctx, -1, "preload");
	dfs_core(ctx);
	duk_put_prop_string(ctx, -2, "fs");
	duk_pop_2(ctx);
}

#endif
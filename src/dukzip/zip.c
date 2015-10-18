/*
Zip library for Duktape.

Uses zlib and minizip.

define BUILD_AS_DLL to build as a duktape module.
*/

#include <duktape.h>

#include <zip.h>
#include <unzip.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/* ---------------------------------------------------------- */

#define ZIPHANDLE_PROP "$$zfile"
#define ZIPFILENAME_PROP "path"
#define DUKZIP_UNZ_PROTOTYPE "DukzipArchiveReadablePrototype"
#define DUKZIP_ZIP_PROTOTYPE "DukzipArchiveWritablePrototype"

/* ---------------------------------------------------------- */

static unzFile dukzip_require_unz(duk_context *ctx, int index) {
	unzFile result;

	duk_get_prop_string(ctx, index, ZIPHANDLE_PROP);
	result = duk_get_pointer(ctx, -1);
	duk_pop(ctx);
	if (!result) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "Expected dukzip archive at index %d", index);
		return NULL;
	}

	return result;
}

static unzFile dukzip_unz_from_this(duk_context *ctx) {
	unzFile f;
	duk_push_this(ctx);
	f = dukzip_require_unz(ctx, -1);
	duk_pop(ctx);
	return f;
}

static void dukzip_push_unzfile(duk_context *ctx, unzFile archive, const char *filename) {
	/* create object with readable Dukzip archive prototype */
	duk_push_object(ctx);
	duk_get_global_string(ctx, DUKZIP_UNZ_PROTOTYPE);
	duk_set_prototype(ctx, -2);

	/* set the archive pointer data */
	duk_push_pointer(ctx, archive);
	duk_put_prop_string(ctx, -2, ZIPHANDLE_PROP);

	/* set path property */
	duk_push_string(ctx, filename);
	duk_put_prop_string(ctx, -2, ZIPFILENAME_PROP);
}

static duk_ret_t dukzip_unz_finalizer(duk_context *ctx) {
	unzFile archive = dukzip_require_unz(ctx, 0);
	unzClose(archive);

	return 0;
}

/* ---------------------------------------------------------- */

static duk_ret_t dukzip_unz_listfiles(duk_context *ctx) {
	unzFile archive = dukzip_unz_from_this(ctx);

	unzGoToFirstFile(archive);
	int i = 0, res;
	duk_idx_t arr_idx = duk_push_array(ctx);

	do {
		unz_file_info fileInfo;
		unzGetCurrentFileInfo(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);

		char fileName[fileInfo.size_filename];
		unzGetCurrentFileInfo(archive, &fileInfo, fileName, fileInfo.size_filename, NULL, 0, NULL, 0);

		duk_push_lstring(ctx, fileName, fileInfo.size_filename);
		duk_put_prop_index(ctx, arr_idx, i++);

		res = unzGoToNextFile(archive);

	} while (res != UNZ_END_OF_LIST_OF_FILE || res == UNZ_OK);

	return 1;
}

static duk_ret_t dukzip_unz_getfirstfile(duk_context *ctx) {
	unzFile archive = dukzip_unz_from_this(ctx);
	int res = unzGoToFirstFile(archive);

	if (res == UNZ_OK) {
		duk_push_true(ctx);
	} else {
		duk_push_false(ctx);
	}
	return 1;
}

static duk_ret_t dukzip_unz_getnextfile(duk_context *ctx) {
	unzFile archive = dukzip_unz_from_this(ctx);
	int res = unzGoToNextFile(archive);

	if (res == UNZ_OK) {
		duk_push_true(ctx);
	} else {
		duk_push_false(ctx);
	}
	return 1;
}

static duk_ret_t dukzip_unz_getfile(duk_context *ctx) {
	unzFile archive = dukzip_unz_from_this(ctx);
	const char *filename = duk_require_string(ctx, 0);
	int res = unzLocateFile(archive, filename, 0);

	if (res == UNZ_OK) {
		duk_push_true(ctx);
	} else {
		duk_push_false(ctx);
	}
	return 1;
}

static duk_ret_t dukzip_unz_getfilename(duk_context *ctx) {
	unz_file_info fileInfo;
	unzFile archive = dukzip_unz_from_this(ctx);

	unzGetCurrentFileInfo(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);

	char fileName[fileInfo.size_filename];
	unzGetCurrentFileInfo(archive, &fileInfo, fileName, fileInfo.size_filename, NULL, 0, NULL, 0);

	duk_push_lstring(ctx, fileName, fileInfo.size_filename);
	return 1;
}

static duk_ret_t dukzip_unz_getfileinfo(duk_context *ctx) {
	duk_idx_t info_obj;
	unz_file_info fileInfo;
	unzFile archive = dukzip_unz_from_this(ctx);

	unzGetCurrentFileInfo(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
	char fileName[fileInfo.size_filename], 
		extraField[fileInfo.size_file_extra], 
		commentString[fileInfo.size_file_comment];
	unzGetCurrentFileInfo(archive, &fileInfo, 
		fileName, fileInfo.size_filename,
		extraField, fileInfo.size_file_extra,
		commentString, fileInfo.size_file_comment);

	info_obj = duk_push_object(ctx);

	duk_push_int(ctx, fileInfo.compressed_size);
	duk_put_prop_string(ctx, info_obj, "compressed");

	duk_push_int(ctx, fileInfo.uncompressed_size);
	duk_put_prop_string(ctx, info_obj, "uncompressed");

	duk_push_lstring(ctx, fileName, fileInfo.size_filename);
	duk_put_prop_string(ctx, info_obj, "filename");

	duk_push_lstring(ctx, extraField, fileInfo.size_file_extra);
	duk_put_prop_string(ctx, info_obj, "extra");

	duk_push_lstring(ctx, commentString, fileInfo.size_file_comment);
	duk_put_prop_string(ctx, info_obj, "comment");
}


/* ---------------------------------------------------------- */

static duk_ret_t dukzip_open(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	const char *filemode;
	unzFile archive;

	if (duk_is_string(ctx, 1)) {
		filemode = duk_require_string(ctx, 1);
	} else {
		filemode = "r";
	}

	if (filemode[0] == 'r') {

		archive = unzOpen(filename);
		if (archive == NULL) {
			duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file '%s'", filename);
		}
		
		dukzip_push_unzfile(ctx, archive, filename);
		return 1;

	} else if (filemode[0] == 'w') {

		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "writable zip files not implemented yet");

	} else {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "%s is not a valid file mode (valid modes: 'r' or 'w')", filemode);
	}
}

/* ---------------------------------------------------------- */

static const duk_function_list_entry dukzip_unz_prototype[] = {
	{ "listFiles", dukzip_unz_listfiles, 0 },
	{ "getFirstFile", dukzip_unz_getfirstfile, 0 },
	{ "getNextFile", dukzip_unz_getnextfile, 0 },
	{ "getFile", dukzip_unz_getfile, 1 },
	{ "getFileName", dukzip_unz_getfilename, 0 },
	{ "getFileInfo", dukzip_unz_getfileinfo, 0 },
	{ NULL, NULL, 0 }
};

static const duk_function_list_entry dukzip_module[] = {
	{ "open", dukzip_open, 2 },
	{ NULL, NULL, 0}
};


static int dukzip_core(duk_context *ctx) {
	int mod = duk_push_object(ctx);

	/* create readable archive prototype */
	duk_push_object(ctx);
	duk_push_c_function(ctx, dukzip_unz_finalizer, 1);
	duk_set_finalizer(ctx, -2);
	duk_put_function_list(ctx, -1, dukzip_unz_prototype);
	duk_put_global_string(ctx, DUKZIP_UNZ_PROTOTYPE);

	duk_put_function_list(ctx, -1, dukzip_module);
	return mod;
}


#ifdef BUILD_AS_DLL

#if defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

DLL_EXPORT duk_ret_t dukopen_zip(duk_context *ctx) {
	dukzip_core(ctx);
	return 1;
}

#else

void register_dukzip(duk_context *ctx) {
	dukzip_core(ctx);
	duk_put_global_string(ctx, "zip");

	duk_eval_string_noresult(ctx, "Duktape.modLoaded['zip'] = zip");
}

#endif
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
		// unz_file_info fileInfo;
		// unzGetCurrentFileInfo(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
		unz_file_info64 fileInfo;
		unzGetCurrentFileInfo64(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);

		char fileName[fileInfo.size_filename];
		// unzGetCurrentFileInfo(archive, &fileInfo, fileName, fileInfo.size_filename, NULL, 0, NULL, 0);
		unzGetCurrentFileInfo64(archive, &fileInfo, fileName, fileInfo.size_filename, NULL, 0, NULL, 0);

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
	// unz_file_info fileInfo;
	unz_file_info64 fileInfo;
	unzFile archive = dukzip_unz_from_this(ctx);

	// unzGetCurrentFileInfo(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
	unzGetCurrentFileInfo64(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);

	char fileName[fileInfo.size_filename];
	// unzGetCurrentFileInfo(archive, &fileInfo, fileName, fileInfo.size_filename, NULL, 0, NULL, 0);
	unzGetCurrentFileInfo64(archive, &fileInfo, fileName, fileInfo.size_filename, NULL, 0, NULL, 0);

	duk_push_lstring(ctx, fileName, fileInfo.size_filename);
	return 1;
}

static duk_ret_t dukzip_unz_getfileinfo(duk_context *ctx) {
	duk_idx_t info_obj;
	// unz_file_info fileInfo;
	unz_file_info64 fileInfo;
	unzFile archive = dukzip_unz_from_this(ctx);

	// unzGetCurrentFileInfo(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
	unzGetCurrentFileInfo64(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
	char fileName[fileInfo.size_filename], 
		extraField[fileInfo.size_file_extra], 
		commentString[fileInfo.size_file_comment];

	// unzGetCurrentFileInfo(archive, &fileInfo, 
	// 	fileName, fileInfo.size_filename,
	// 	extraField, fileInfo.size_file_extra,
	// 	commentString, fileInfo.size_file_comment);
	unzGetCurrentFileInfo64(archive, &fileInfo, 
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

static duk_ret_t dukzip_unz_readfile(duk_context *ctx) {
	unz_file_info fileInfo;
	unzFile archive = dukzip_unz_from_this(ctx);
	int ret = UNZ_OK;

	unzGetCurrentFileInfo(archive, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
	void *bytes = duk_push_fixed_buffer(ctx, fileInfo.uncompressed_size);

	ret = unzOpenCurrentFile(archive);
	if (ret != UNZ_OK) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "unable to open file in archive");
		return -1;
	}

	ret = unzReadCurrentFile(archive, bytes, fileInfo.uncompressed_size);
	if (ret < 0) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "unable to read file in archive");
		return -1;
	}
	unzCloseCurrentFile(archive);

	return 1;
}

/* ---------------------------------------------------------- */

static zipFile dukzip_require_zip(duk_context *ctx, int index) {
	zipFile result;

	duk_get_prop_string(ctx, index, ZIPHANDLE_PROP);
	result = duk_get_pointer(ctx, -1);
	duk_pop(ctx);
	if (!result) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "Expected dukzip archive at index %d", index);
		return NULL;
	}

	return result;
}

static zipFile dukzip_zip_from_this(duk_context *ctx) {
	zipFile f;
	duk_push_this(ctx);
	f = dukzip_require_unz(ctx, -1);
	duk_pop(ctx);
	return f;
}

static void dukzip_push_zipfile(duk_context *ctx, zipFile archive, const char *filename) {
	/* create object with readable Dukzip archive prototype */
	duk_push_object(ctx);
	duk_get_global_string(ctx, DUKZIP_ZIP_PROTOTYPE);
	duk_set_prototype(ctx, -2);

	/* set the archive pointer data */
	duk_push_pointer(ctx, archive);
	duk_put_prop_string(ctx, -2, ZIPHANDLE_PROP);

	/* set path property */
	duk_push_string(ctx, filename);
	duk_put_prop_string(ctx, -2, ZIPFILENAME_PROP);
}

static duk_ret_t dukzip_zip_finalizer(duk_context *ctx) {
	zipFile archive = dukzip_require_zip(ctx, 0);
	zipClose(archive, "");

	return 0;
}

/* ---------------------------------------------------------- */

/*
utility method to inspect an options object. 
*/

static void dukzip_zip_checkoptions(duk_context *ctx, duk_idx_t idx, const char **filename, duk_int_t *level, duk_int_t *method, const char **comment) 
{
	duk_get_prop_string(ctx, idx, "filename");
	if (duk_is_string(ctx, -1)) {
		*filename = duk_get_string(ctx, -1);
		duk_pop(ctx);
	} else {
		duk_pop(ctx);
	}

	duk_get_prop_string(ctx, idx, "level");
	if (duk_is_number(ctx, -1)) {
		*level = duk_get_int(ctx, -1);
		duk_pop(ctx);
	} else {
		duk_pop(ctx);
	}

	duk_get_prop_string(ctx, idx, "method");
	if (duk_is_string(ctx, -1)) {
		duk_push_string(ctx, "deflate");
		if (duk_equals(ctx, -1, -2)) {
			*method = Z_DEFLATED;
		} else {
			duk_pop(ctx);
			duk_push_string(ctx, "store");
			if (duk_equals(ctx, -1, -2)) {
				*method = 0;
			}
		}
		duk_pop_2(ctx);
	} else {
		duk_pop(ctx);
	}

	duk_get_prop_string(ctx, idx, "comment");
	if (duk_is_string(ctx, -1)) {
		*comment = duk_get_string(ctx, -1);
		duk_pop(ctx);
	} else {
		duk_pop(ctx);
	}
}

/*
Low-level file writing

USAGE: newFile to create file in zip, write to add data to file, close to finish writing
*/

static duk_ret_t dukzip_zip_newfile(duk_context *ctx) {
	zip_fileinfo zi = {0};
	int res = ZIP_OK;
	zipFile archive = dukzip_zip_from_this(ctx);
	
	const char *filename = "";
	duk_int_t level = Z_DEFAULT_COMPRESSION;
	duk_int_t method = Z_DEFLATED;
	const char *comment = "";


	if (duk_is_object(ctx, 0)) {
		dukzip_zip_checkoptions(ctx, 0, &filename, &level, &method, &comment);
	} else {
		filename = duk_require_string(ctx, 0);

		if (duk_is_number(ctx, 1)) {
			level = duk_get_int(ctx, 1);
		}
	}

	res = zipOpenNewFileInZip64(archive, filename, &zi, NULL, 0, NULL, 0, comment, method, level, 1);

	if (res == ZIP_OK) {
		duk_push_true(ctx);
	} else {
		duk_push_false(ctx);
	}
	return 1;
}

static duk_ret_t dukzip_zip_write(duk_context *ctx) {
	int res = ZIP_OK;
	zipFile archive = dukzip_zip_from_this(ctx);

	if (duk_is_string(ctx, 0)) {

		int outputl = 0;
		const char *output = duk_get_lstring(ctx, 0, &outputl);

		res = zipWriteInFileInZip(archive, output, outputl);

	} else if (duk_is_buffer(ctx, 0) || duk_is_object(ctx, 0)) {

		int outputl = 0;
		void *output = duk_require_buffer_data(ctx, 0, &outputl);

		res = zipWriteInFileInZip(archive, output, outputl);

	} else {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "unable to write argument to zip file (supported types: string, buffer)");
		return -1;
	}

	if (res == ZIP_OK) {
		duk_push_true(ctx);
	} else {
		duk_push_false(ctx);
	}
	return 1;
}

static duk_ret_t dukzip_zip_close(duk_context *ctx) {
	int res = ZIP_OK;
	zipFile archive = dukzip_zip_from_this(ctx);

	res = zipCloseFileInZip(archive);

	if (res == ZIP_OK) {
		duk_push_true(ctx);
	} else {
		duk_push_false(ctx);
	}
	return 1;
}

/*
High-level single-step file adding

If the first argument is an object, the data property should contain the data to
write to the zip.

Otherwise, first argument is filename, second argument is the data, optional third
argument is the compression level.
*/

static duk_ret_t dukzip_zip_add(duk_context *ctx) {
	zip_fileinfo zi = {0};
	int res = ZIP_OK;
	zipFile archive = dukzip_zip_from_this(ctx);
	
	const char *filename = "";
	duk_int_t level = Z_DEFAULT_COMPRESSION;
	duk_int_t method = Z_DEFLATED;
	const char *comment = "";

	int datalen = 0;
	void *data = NULL;

	if (duk_is_object(ctx, 0)) {
		dukzip_zip_checkoptions(ctx, 0, &filename, &level, &method, &comment);

		duk_get_prop_string(ctx, 0, "data");
		if (duk_is_string(ctx, -1)) {
			data = (void *)duk_get_lstring(ctx, -1, &datalen);
		} else if (duk_is_buffer(ctx, -1) || duk_is_object(ctx, -1)) {
			data = duk_require_buffer_data(ctx, -1, &datalen);
		} else {
			duk_error(ctx, DUK_ERR_TYPE_ERROR, "unable to write data to zip file (supported types: string, buffer)");
			return -1;
		}

	} else {
		filename = duk_require_string(ctx, 0);

		if (duk_is_string(ctx, 1)) {
			data = (void *)duk_get_lstring(ctx, 1, &datalen);
		} else if (duk_is_buffer(ctx, 1) || duk_is_object(ctx, 1)) {
			data = duk_require_buffer_data(ctx, 1, &datalen);
		} else {
			duk_error(ctx, DUK_ERR_TYPE_ERROR, "unable to write argument to zip file (supported types: string, buffer)");
			return -1;
		}

		if (duk_is_number(ctx, 2)) {
			level = duk_get_int(ctx, 2);
		}

		/* push dummy to normalize stack */
		duk_push_string(ctx, "dummy");
	}

	res = zipOpenNewFileInZip64(archive, filename, &zi, NULL, 0, NULL, 0, comment, method, level, 1);
	if (res != ZIP_OK) {
		goto error;
	}

	res = zipWriteInFileInZip(archive, data, datalen);
	if (res != ZIP_OK) {
		goto error;
	}

	res = zipCloseFileInZip(archive);
	duk_pop(ctx); /* pop buffer (or dummy) from stack */

	if (res == ZIP_OK) {
		duk_push_true(ctx);
	} else {
		duk_push_false(ctx);
	}
	return 1;

error:
	zipCloseFileInZip(archive);
	duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not write file '%s'", filename);
	return -1;

}


/* ---------------------------------------------------------- */

static duk_ret_t dukzip_open(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	const char *filemode;

	if (duk_is_string(ctx, 1)) {
		filemode = duk_require_string(ctx, 1);
	} else {
		filemode = "r";
	}

	if (filemode[0] == 'r') {

		unzFile archive;

		archive = unzOpen64(filename);
		if (archive == NULL) {
			duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file '%s'", filename);
		}
		
		dukzip_push_unzfile(ctx, archive, filename);
		return 1;

	} else if (filemode[0] == 'w') {

		zipFile archive;

		archive = zipOpen64(filename, APPEND_STATUS_CREATE);
		if (archive == NULL) {
			duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file '%s'", filename);
		}

		dukzip_push_zipfile(ctx, archive, filename);
		return 1;

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
	{ "readFile", dukzip_unz_readfile, 0 },
	{ NULL, NULL, 0 }
};

static const duk_function_list_entry dukzip_zip_prototype[] = {
	{ "open", dukzip_zip_newfile, 2 },
	{ "write", dukzip_zip_write, 1 },
	{ "close", dukzip_zip_close, 0 },
	{ "add", dukzip_zip_add, 3 },
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

	/* create writable archive prototype */
	duk_push_object(ctx);
	duk_push_c_function(ctx, dukzip_zip_finalizer, 1);
	duk_set_finalizer(ctx, -2);
	duk_put_function_list(ctx, -1, dukzip_zip_prototype);
	duk_put_global_string(ctx, DUKZIP_ZIP_PROTOTYPE);

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
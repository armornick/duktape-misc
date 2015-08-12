/*
* srduk.c
* Duktape interpreter for self-running programs
* Based on srlua by Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* This code is hereby placed in the public domain.
*/

#ifdef _WIN32
#include <windows.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glue.h"
#include "duktape.h"

#ifdef _WIN32
#define alert(progname,message)	MessageBox(NULL,message,progname,MB_ICONERROR | MB_OK)
#define getprogname()	char name[MAX_PATH]; argv[0]= GetModuleFileName(NULL,name,sizeof(name)) ? name : NULL;
#else
#define alert(progname,message)	fprintf(stderr,"%s: %s\n",progname,message)
#define getprogname()
#endif

static void srduk_push_argv (duk_context *ctx, int argc, char *argv[]) {
	duk_idx_t arg_array_index; int i;

	arg_array_index = duk_push_array(ctx);

	for (i = 0; i < argc; i++) {
		duk_push_string(ctx, argv[i]);
		duk_put_prop_index(ctx, arg_array_index, i);
	}

	duk_put_global_string(ctx, "argv");
}

#define cannot(x) duk_error(ctx,DUK_ERR_INTERNAL_ERROR,"cannot %s %s: %s",x,srduk_filename,strerror(errno))

static void srduk_load_script (duk_context *ctx, char *srduk_filename) {
	Glue t;
	FILE *f=fopen(srduk_filename,"rb");

	if (f==NULL) cannot("open");
	if (fseek(f,-sizeof(t),SEEK_END)!=0) cannot("seek");
	if (fread(&t,sizeof(t),1,f)!=1) cannot("read");
	if (memcmp(t.sig,GLUESIG,GLUELEN)!=0) duk_error(ctx,DUK_ERR_INTERNAL_ERROR,"no Duktape program found in %s",srduk_filename);
	if (fseek(f,t.size1,SEEK_SET)!=0) cannot("seek");

	char *filebuff = malloc(t.size2); int n;
	if (filebuff == NULL) duk_error(ctx,DUK_ERR_INTERNAL_ERROR,"could not allocate memory to hold file");
	n = fread(filebuff,1,t.size2,f);
	duk_eval_lstring_noresult(ctx, filebuff, n);

	free(filebuff);
	fclose(f);
}

static duk_ret_t pmain (duk_context *ctx) {
	int argc; char** argv;
	argc = duk_require_int(ctx, 0);
	argv = duk_require_pointer(ctx, 1);

	srduk_push_argv(ctx, argc, argv);
	// register any user libraries/functions here
	srduk_load_script(ctx, argv[0]);
}

static void fatal(const char* progname, const char* message)
{
 alert(progname,message);
 exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[]) {
	
	getprogname();
	if (argv[0]==NULL) fatal("srduk","cannot locate this executable");

	duk_context *ctx = duk_create_heap_default();
	if (ctx==NULL) fatal(argv[0],"failed to create duktape heap");

	duk_push_c_function(ctx, pmain, 2);
	duk_push_int(ctx, argc);
	duk_push_pointer(ctx, argv);
	if (duk_pcall(ctx, 2) != DUK_EXEC_SUCCESS) fatal(argv[0],duk_to_string(ctx, -1));

	duk_destroy_heap(ctx);

	return EXIT_SUCCESS;
}
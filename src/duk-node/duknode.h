#ifndef _DUKNODE_H_
#define _DUKNODE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <duktape.h>

#include <sys/types.h>
#include <sys/stat.h>

/*
Platform detection
Based on the Foundation Library (see: https://github.com/rampantpixels/foundation_lib/blob/master/foundation/platform.h)
*/

#if defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )

	#define DUKNODE_PLATFORM_WINDOWS 1
	#define DUKNODE_PLATFORM_POSIX 0
	#define DUKNODE_PLATFORM_NAME "win32"
	#define DUKNODE_PLATFORM_TYPE "Windows"

	#define DLL_EXPORT __declspec(dllexport)

#else

	#define DUKNODE_PLATFORM_POSIX 1
	#define DUKNODE_PLATFORM_WINDOWS 0
	#define DUKNODE_PLATFORM_NAME "posix"

	#define DLL_EXPORT

	#if ( defined( __linux__ ) || defined( __linux ) )
		#define DUKNODE_PLATFORM_TYPE "Linux"
		#define DUKNODE_PLATFORM_LINUX 1
	#elif ( defined( __BSD__ ) || defined( __FreeBSD__ ) )
		#define DUKNODE_PLATFORM_TYPE "BSD"
		#define DUKNODE_PLATFORM_BSD 1
	#else
		#error Unknown platform
	#endif

#endif

#  if defined( __x86_64__ ) || defined( _M_AMD64 ) || defined( _AMD64_ )
	#define DUKNODE_ARCH_X86_64 1
	#define DUKNODE_ARCH_NAME "x64"
#  elif defined( __x86__ ) || defined( _M_IX86 ) || defined( _X86_ )
	#define DUKNODE__ARCH_X86 1
	#define DUKNODE_ARCH_NAME "ia32"
#  elif defined( __ia64__ ) || defined( _M_IA64 ) || defined( _IA64_ )
	#define DUKNODE__ARCH_IA64 1
	#define DUKNODE_ARCH_NAME "x64"
#  endif

#if DUKNODE_PLATFORM_WINDOWS 

	#include <direct.h>

	#define chdir _chdir
	#define getcwd _getcwd
	#define stat _stat
	#define lstat _stat
	#define mkdir(path, mode) _mkdir(path)

	#define S_IFDIR _S_IFDIR
	#define S_IFREG _S_IFREG

	int setenv(const char *envname, const char *envval, int overwrite);

	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

#else

	#include <unistd.h>
	#include <limits.h>



#endif

/* max path string length */
#define DUKNODE_MAX_PATH 512

#define DUKNODE_BUFSIZ 4096

/* Functions to push/pull streams to Duktape */
void push_difstream(duk_context *ctx, FILE *inputf);
void push_dofstream(duk_context *ctx, FILE *outputf);
FILE* dfstream_require_file(duk_context *ctx, int index);
void register_dfstream(duk_context *ctx);

/* register modSearch */
void register_mod_search(duk_context *ctx);

/* register singleton objects */
void register_dconsole(duk_context *ctx);
void register_dprocess(duk_context *ctx);

/*
Every module has two functions to register them:

* register_* which sets the module as a global object
* preload_* which registers it in the package.preload table (which is the first place require looks)

*/

/* os module */
void register_dos(duk_context *ctx);
void preload_dos(duk_context *ctx);

/* fs module */
void register_dfs(duk_context *ctx);
void preload_dfs(duk_context *ctx);

#endif 
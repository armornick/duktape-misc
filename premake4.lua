-- Main project container
solution "DynamicDuktape"
	location "makefiles"
	targetdir "build"
	
	configurations {'release'}
	flags { 'StaticRuntime' }

	configuration 'release'
		flags {'Optimize'}

	-- constants
	local DUKTAPE_DIR = path.join('vendor','duktape-1.3.0')
	local DUKPLUS_DIR = path.join('src','dukplus')
	local DUKNODE_DIR = path.join('src','duk-node')

	local ZLIB_DIR = path.join('vendor','zlib-1.2.8')
	local MINIZP_DIR = path.join(ZLIB_DIR, 'contrib','minizip')

	-- http://duktape.org/
	project "duktape"
		kind "SharedLib"
		language "C"

		defines { 'DUK_OPT_DLL_BUILD' }

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKTAPE_DIR, 'src', 'duktape.c') }

		configuration { 'windows','gmake' }
			linkoptions { '-static' }

	-- hello world duktape example (http://duktape.org/)
	project "duktape-hello"
		kind "ConsoleApp"
		language "C"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKTAPE_DIR, 'examples', 'hello', 'hello.c') }
		links { "duktape" }

	-- Duktape commandline interpreter
	project "duk"
		kind "ConsoleApp"
		language "C"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKTAPE_DIR, 'examples', 'cmdline', 'duk_cmdline.c') }
		links { "duktape" }

	-- Duktape application with require and lua-esque libs
	project "dukplus"
		kind "ConsoleApp"
		language "C"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKPLUS_DIR, 'loadlib.c'), path.join(DUKPLUS_DIR, 'iolib.c'), path.join(DUKPLUS_DIR, 'oslib.c'),
			path.join(DUKPLUS_DIR, 'main.c') }
		links { "duktape" }

	-- C test module for Duktape
	project "test-module"
		kind "SharedLib"
		language "C"
		targetname "test"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKPLUS_DIR, 'test-module.c') }
		links { "duktape" }

	-- C test module for Duktape with embedded Javascript
	project "embed-js-module-test"
		kind "SharedLib"
		language "C"
		targetname "jstest"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKPLUS_DIR, 'embed-js-module.c') }
		links { "duktape" }

	-- utility application to embed scripts into srduk
	project "glue"
		kind "ConsoleApp"
		language "C"

		includedirs { path.join('vendor','srlua-5.3') }

		files { path.join('vendor','srlua-5.3','glue.c') }

	-- Duktape interpreter that executes embedded script
	project "srduk"
		kind "ConsoleApp"
		language "C"

		includedirs { path.join(DUKTAPE_DIR, 'src'), path.join('vendor','srlua-5.3') }

		files { path.join('src','srduk.c') }
		links { 'duktape' }

	-- Node API subset for Duktape
	project "duknode"
		kind "ConsoleApp"
		language "C"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKPLUS_DIR, 'loadlib.c'), path.join(DUKNODE_DIR, 'dfstream.c'), path.join(DUKNODE_DIR, 'dconsole.c'),
			path.join(DUKNODE_DIR, 'dprocess.c'), path.join(DUKNODE_DIR, 'dos.c'), path.join(DUKNODE_DIR, 'dfs.c'), 
			path.join(DUKNODE_DIR, 'dpath.c'), 
			path.join(DUKNODE_DIR, 'main.c') }
		links { "duktape" }

	-- filesystem module for Duktape
	project "dukfs"
		kind "SharedLib"
		language "C"
		targetname "dfs"

		includedirs { path.join(DUKTAPE_DIR, 'src'), ZLIB_DIR, MINIZP_DIR }

		files { path.join('src','dukfs','dfs.c') }
		defines { "BUILD_AS_DLL" }
		links { "duktape" }

	-- zlib compression library
	project "zlib"
		kind "StaticLib"
		language "C"

		files {
			path.join(ZLIB_DIR,'adler32.c'), path.join(ZLIB_DIR,'compress.c'), path.join(ZLIB_DIR,'crc32.c'), 
			path.join(ZLIB_DIR,'deflate.c'), path.join(ZLIB_DIR,'gzclose.c'), path.join(ZLIB_DIR,'gzlib.c'),
			path.join(ZLIB_DIR,'gzread.c'), path.join(ZLIB_DIR,'gzwrite.c'), path.join(ZLIB_DIR,'infback.c'),
			path.join(ZLIB_DIR,'inffast.c'), path.join(ZLIB_DIR,'inflate.c'), path.join(ZLIB_DIR,'inftrees.c'),
			path.join(ZLIB_DIR,'trees.c'), path.join(ZLIB_DIR,'uncompr.c'), path.join(ZLIB_DIR,'zutil.c')
		}

	-- minizip helper library for zlib
	project "minizip"
		kind "StaticLib"
		language "C"

		includedirs { ZLIB_DIR }

		files { path.join(MINIZP_DIR,'unzip.c'), path.join(MINIZP_DIR,'zip.c'), path.join(MINIZP_DIR,'ioapi.c') }

		configuration { 'windows' }
			files { path.join(MINIZP_DIR,'iowin32.c') }

	-- zip module for Duktape
	project "dukzip"
		kind "SharedLib"
		language "C"
		targetname "zip"

		includedirs { path.join(DUKTAPE_DIR, 'src'), ZLIB_DIR, MINIZP_DIR }

		files { path.join('src','dukzip','zip.c') }
		defines { "BUILD_AS_DLL" }
		links { "duktape", "minizip", "zlib" }

	
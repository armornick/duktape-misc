-- Main project container
solution "DynamicDuktape"
	targetdir "build"
	configurations {'release'}
	flags { 'StaticRuntime' }

	configuration 'release'
		flags {'Optimize'}

	-- constants
	local DUKTAPE_DIR = path.join('vendor','duktape-1.2.2')
	local DUKPLUS_DIR = path.join('src','dukplus')
	local DUKNODE_DIR = path.join('src','duk-node')

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
	project "loadlib-test"
		kind "ConsoleApp"
		language "C"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKPLUS_DIR, 'loadlib.c'), path.join(DUKPLUS_DIR, 'iolib.c'), path.join(DUKPLUS_DIR, 'oslib.c'),
			path.join(DUKPLUS_DIR, 'loadlib-test.c') }
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


	
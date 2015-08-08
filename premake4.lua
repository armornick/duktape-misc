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

	project "duk"
		kind "ConsoleApp"
		language "C"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKTAPE_DIR, 'examples', 'cmdline', 'duk_cmdline.c') }
		links { "duktape" }

	project "loadlib-test"
		kind "ConsoleApp"
		language "C"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKPLUS_DIR, 'loadlib.c'), path.join(DUKPLUS_DIR, 'loadlib-test.c') }
		links { "duktape" }

	project "test-module"
		kind "SharedLib"
		language "C"
		targetname "test"

		includedirs { path.join(DUKTAPE_DIR, 'src') }

		files { path.join(DUKPLUS_DIR, 'test-module.c') }
		links { "duktape" }
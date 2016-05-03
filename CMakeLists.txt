cmake_minimum_required(VERSION 3.0)
project(duktape-dynamic)

# remove 'lib' prefix for dlls 
if(WIN32 AND CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# -----------------------------------------------------

set(DUKTAPE_DIR vendor/duktape-1.4.0)
set(DUKTAPE_INCLUDE_DIR ${DUKTAPE_DIR}/src)

set(DUKMAKE_DIR src/dukmake)
set(MINIDUKWIN_DIR src/minidukwin)

# -----------------------------------------------------

set(DUKTAPE_SRCS
	${DUKTAPE_DIR}/src/duktape.c
)

add_library(duktape SHARED ${DUKTAPE_SRCS})
target_include_directories(duktape PUBLIC ${DUKTAPE_INCLUDE_DIR})
target_compile_definitions(duktape PUBLIC DUK_OPT_DLL_BUILD)

# -----------------------------------------------------

set(DUKTAPE_HELLO_SRCS
	${DUKTAPE_DIR}/examples/hello/hello.c
)

add_executable(duktape-hello ${DUKTAPE_HELLO_SRCS})
target_include_directories(duktape-hello PUBLIC ${DUKTAPE_INCLUDE_DIR})
target_link_libraries(duktape-hello duktape)

set(DUKTAPE_DUK_SRCS
	${DUKTAPE_DIR}/examples/cmdline/duk_cmdline.c
)

add_executable(duk ${DUKTAPE_DUK_SRCS})
target_include_directories(duk PUBLIC ${DUKTAPE_INCLUDE_DIR})
target_link_libraries(duk duktape)

# -----------------------------------------------------

set(DUKPLUS_DIR src/dukplus)

set(DUKPLUS_SRCS
	${DUKPLUS_DIR}/loadlib.c
	${DUKPLUS_DIR}/iolib.c
	${DUKPLUS_DIR}/oslib.c
	${DUKPLUS_DIR}/main.c
)

add_executable(dukplus ${DUKPLUS_SRCS})
target_include_directories(dukplus PUBLIC ${DUKTAPE_INCLUDE_DIR})
target_link_libraries(dukplus duktape)

# -----------------------------------------------------

set(DUKNODE_DIR src/duk-node)

set(DUKNODE_SRCS
	${DUKPLUS_DIR}/loadlib.c
	${DUKNODE_DIR}/dfstream.c
	${DUKNODE_DIR}/dconsole.c
	${DUKNODE_DIR}/dprocess.c
	${DUKNODE_DIR}/dos.c
	${DUKNODE_DIR}/dfs.c
	${DUKNODE_DIR}/dpath.c
	${DUKNODE_DIR}/main.c
)

add_executable(duknode ${DUKNODE_SRCS})
target_include_directories(duknode PUBLIC ${DUKTAPE_INCLUDE_DIR})
target_link_libraries(duknode duktape)

# -----------------------------------------------------

set(SRLUA_DIR vendor/srlua-5.3)

add_executable(glue ${SRLUA_DIR}/glue.c)
target_include_directories(glue PUBLIC ${SRLUA_DIR})

add_executable(srduk src/srduk.c)
target_include_directories(srduk PUBLIC ${DUKTAPE_INCLUDE_DIR} ${SRLUA_DIR})
target_link_libraries(srduk duktape)

# -----------------------------------------------------

add_library(test-module SHARED ${DUKPLUS_DIR}/test-module.c)
target_include_directories(test-module PUBLIC ${DUKTAPE_DIR})
target_link_libraries(test-module duktape)
set_target_properties (test-module PROPERTIES OUTPUT_NAME test)

add_library(embed-js-module-test SHARED ${DUKPLUS_DIR}/embed-js-module.c)
target_include_directories(embed-js-module-test PUBLIC ${DUKTAPE_DIR})
target_link_libraries(embed-js-module-test duktape)
set_target_properties (embed-js-module-test PROPERTIES OUTPUT_NAME jstest)

# -----------------------------------------------------

add_library(dukfs SHARED src/dukfs/dfs.c)
target_include_directories(dukfs PUBLIC ${DUKTAPE_DIR})
target_link_libraries(dukfs duktape)
target_compile_definitions(dukfs PUBLIC BUILD_AS_DLL)
set_target_properties (dukfs PROPERTIES OUTPUT_NAME dfs)

# -----------------------------------------------------

set(ZLIB_DIR vendor/zlib-1.2.8)
set(MINIZIP_DIR ${ZLIB_DIR}/contrib/minizip)

set(ZLIB_SRCS
	${ZLIB_DIR}/adler32.c
	${ZLIB_DIR}/compress.c
	${ZLIB_DIR}/crc32.c
	${ZLIB_DIR}/deflate.c
	${ZLIB_DIR}/gzclose.c
	${ZLIB_DIR}/gzlib.c
	${ZLIB_DIR}/gzread.c
	${ZLIB_DIR}/gzwrite.c
	${ZLIB_DIR}/infback.c
	${ZLIB_DIR}/inffast.c
	${ZLIB_DIR}/inflate.c
	${ZLIB_DIR}/inftrees.c
	${ZLIB_DIR}/trees.c
	${ZLIB_DIR}/uncompr.c
	${ZLIB_DIR}/zutil.c
)

set(MINIZIP_SRCS
	${MINIZIP_DIR}/unzip.c
	${MINIZIP_DIR}/zip.c
	${MINIZIP_DIR}/ioapi.c
)

if (WIN32)
	set(MINIZIP_SRCS ${MINIZIP_SRCS} ${MINIZIP_DIR}/iowin32.c)
endif()

add_library(zlib STATIC ${ZLIB_SRCS})

add_library(minizip STATIC ${MINIZIP_SRCS})
target_include_directories(minizip PUBLIC ${ZLIB_DIR})
target_link_libraries(minizip zlib)

add_library(dukzip SHARED src/dukzip/zip.c)
target_include_directories(dukzip PUBLIC ${DUKTAPE_DIR} ${ZLIB_DIR} ${MINIZIP_DIR})
target_link_libraries(dukzip duktape zlib minizip)
target_compile_definitions(dukzip PUBLIC BUILD_AS_DLL)
set_target_properties (dukzip PROPERTIES OUTPUT_NAME zip)

# -----------------------------------------------------

if (WIN32)

set(DUKSOCK_SRCS
	src/sock/sock.c
	src/sock/main.c
)

add_library(duksock SHARED ${DUKSOCK_SRCS})
target_include_directories(duksock PUBLIC ${DUKTAPE_DIR})
target_link_libraries(duksock duktape ws2_32)

endif()

# -----------------------------------------------------

set(DUKPP_DIR src/dukpp)
set(DUKPP_TEST_DIR src/dukpp-tests)

add_executable(dukpp-types ${DUKPP_TEST_DIR}/typetest.cpp)
target_include_directories(dukpp-types PUBLIC ${DUKTAPE_DIR} ${DUKPP_DIR})
target_link_libraries(dukpp-types duktape)

add_executable(dukpp-binding ${DUKPP_TEST_DIR}/bindertest.cpp ${DUKPP_TEST_DIR}/file.cpp)
target_include_directories(dukpp-binding PUBLIC ${DUKTAPE_DIR} ${DUKPP_DIR})
target_link_libraries(dukpp-binding duktape)

add_executable(dukpp-values ${DUKPP_TEST_DIR}/valuetest.cpp)
target_include_directories(dukpp-values PUBLIC ${DUKTAPE_DIR} ${DUKPP_DIR})
target_link_libraries(dukpp-values duktape)

# -----------------------------------------------------
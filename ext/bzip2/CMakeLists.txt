cmake_minimum_required(VERSION 3.20)

make_release_only()

if( ZD_CMAKE_COMPILER_IS_GNUC_COMPATIBLE )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fomit-frame-pointer" )
endif()

add_definitions( -DBZ_NO_STDIO )
add_library( bz2
    blocksort.c
    bzlib.c
    compress.c
    crctable.c
    decompress.c
    huffman.c
    randtable.c)

set_target_properties(bz2 PROPERTIES
	POSITION_INDEPENDENT_CODE ON
)

cmake_minimum_required(VERSION 3.20)

set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG" )

# Disable warnings for << operator precedence (4554) and
# unreferenced labels (4102) from VC
if( MSVC )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4554 /wd4102" )
endif()

if( ZD_CMAKE_COMPILER_IS_GNUC_COMPATIBLE )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra" )
endif()


if( NOT MSVC AND NOT APPLE )
	if( NOT CMAKE_CROSSCOMPILING )
		add_executable( arithchk arithchk.c )
	endif()
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/arith.h 
		COMMAND arithchk >${CMAKE_CURRENT_BINARY_DIR}/arith.h
		DEPENDS arithchk )

	if( NOT CMAKE_CROSSCOMPILING )
		add_executable( qnan qnan.c arith.h )
		target_include_directories(qnan
		PUBLIC
			${CMAKE_CURRENT_BINARY_DIR}
		)
		set( CROSS_EXPORTS ${CROSS_EXPORTS} arithchk qnan PARENT_SCOPE )
	endif()
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gd_qnan.h
		COMMAND qnan >${CMAKE_CURRENT_BINARY_DIR}/gd_qnan.h
		DEPENDS qnan )
		
	set( GEN_FP_FILES arith.h gd_qnan.h )
	set( GEN_FP_DEPS ${CMAKE_CURRENT_BINARY_DIR}/arith.h ${CMAKE_CURRENT_BINARY_DIR}/gd_qnan.h )
endif()

add_library( gdtoa
	${GEN_FP_FILES}
	dmisc.c
	dtoa.c
	misc.c
)

target_include_directories(gdtoa
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(gdtoa
PRIVATE
	-DINFNAN_CHECK
	-DMULTIPLE_THREADS
)

set_target_properties(gdtoa PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------
# SETUP
# ---------------------------------------------------------------
cmake_minimum_required (VERSION 2.8)
project (Congeal)

message(STATUS "BUILDING FOR [" ${CMAKE_SYSTEM_NAME} "]")

# ---------------------------------------------------------------
# get the GCC version
# ---------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX)
	EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
		ARGS --version
		OUTPUT_VARIABLE GNUCXX_COMPILER_VERSION
	)
	STRING(REGEX REPLACE ".* ([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\1.\\2.\\3"
		GNUCXX_COMPILER_VERSION ${GNUCXX_COMPILER_VERSION})
endif(CMAKE_COMPILER_IS_GNUCXX)



add_definitions(-DPTHREAD_BARRIER=1)

if(${CMAKE_SYSTEM_NAME} MATCHES "CYGWIN")
endif(${CMAKE_SYSTEM_NAME} MATCHES "CYGWIN")

# ---------------------------------------------------------------
# Mac settings
# ---------------------------------------------------------------
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message(STATUS "Using Darwin configutation")
	add_definitions(-DPTHREAD_BARRIER=1)

	if(CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-format")

		set(CMAKE_CXX_FLAGS_RELEASE )
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-format")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Winline")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fast")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --param max-inline-insns-single=10000")

		# these parameters only work for newer versions of gcc
		if(${GNUCXX_COMPILER_VERSION} VERSION_GREATER 4.2.0)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --param inline-unit-growth=10000")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --param large-function-growth=10000")
		endif(${GNUCXX_COMPILER_VERSION} VERSION_GREATER 4.2.0)
	endif(CMAKE_COMPILER_IS_GNUCXX)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


# ---------------------------------------------------------------
# Linux settings
# ---------------------------------------------------------------
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(STATUS "Using Linux configutation")

	add_definitions(-DPTHREAD_BARRIER=0)

	if(CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

		set(CMAKE_CXX_FLAGS_RELEASE )
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-format")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -finline-limit=2000")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --param inline-unit-growth=90000")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --param max-inline-insns-single=20000")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -falign-loops=16")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -falign-jumps=16")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -falign-functions=16")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstrict-aliasing")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funroll-loops")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ftree-loop-linear")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsched-interblock")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fgcse-sm")

		# these parameters only work for newer versions of gcc
		if(${GNUCXX_COMPILER_VERSION} VERSION_GREATER 4.2.0)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --param large-function-growth=90000")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --param large-stack-frame=90000")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --param large-stack-frame-growth=900")
		endif(${GNUCXX_COMPILER_VERSION} VERSION_GREATER 4.2.0)
	endif(CMAKE_COMPILER_IS_GNUCXX)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")



# ---------------------------------------------------------------
# Build it
# ---------------------------------------------------------------
file(GLOB All_CPP "*.c++")
add_executable(Congeal ${All_CPP})


if(CMAKE_COMPILER_IS_GNUCXX)
	target_link_libraries (Congeal pthread)
elseif(WIN32)
	target_link_libraries (Congeal ws2_32)
endif(CMAKE_COMPILER_IS_GNUCXX)






install(TARGETS Congeal
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

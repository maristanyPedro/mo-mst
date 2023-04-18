cmake_minimum_required(VERSION 3.10)
project(m_mst LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

set(Boost_INCLUDE_DIR /nfs/optimi/usr/sw/boost/include)
set(Boost_LIBRARY_DIR /nfs/optimi/usr/sw/boost/lib)

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(Boost 1.74.0)
find_package(Boost)
#find_package(Boost 1.74.0 COMPONENTS dynamic_bitset)
MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
set(execName "BN_AND_IGMDA_${CMAKE_BUILD_TYPE}.o")
#set(execName "IGMDA_4d_${CMAKE_BUILD_TYPE}.o")
#if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	#	add_compile_options(-std=c++14 -g -Winline -Wall -Wextra -pedantic)
	add_compile_options(-std=c++14 -g -Wall -Wextra -pedantic)
	add_executable(${execName}
			datastructures/includes/graph.h
			datastructures/src/graph.cpp
			datastructures/includes/GraphCompacter.h
			datastructures/src/GraphCompacter.cpp
			preprocessing/includes/Preprocessor.h
			preprocessing/src/Preprocessor.cpp
			preprocessing/includes/Dfs.h
			preprocessing/src/Dfs.cpp
			search/includes/Solution.h
			search/includes/MultiPrim.h
			search/src/MultiPrim.cpp
			search/includes/BN.h
			search/src/BN.cpp
			m_mst.cpp
			datastructures/includes/typedefs.h datastructures/includes/Label.h datastructures/includes/BinaryHeap.h search/includes/ImplicitNode.h search/src/ImplicitNode.cpp)

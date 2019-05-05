#############################
# Specify the source files  #
#############################

FILE(GLOB INSTRECLIB_SOURCES *.cpp)
FILE(GLOB INSTRECLIB_HEADERS *.h)
FILE(GLOB INSTRECLIB_UTILS_SOURCES Utils/*.cpp)
FILE(GLOB INSTRECLIB_UTILS_HEADERS Utils/*.h)

# TODO(andrei): This seems to error out on a fresh setup; we may need a better system.
# Once we build pangolin the first time, the problem goes away, but that's not a very
# healthy approach...
#find_package(Pangolin REQUIRED)
#include_directories(${Pangolin_INCLUDE_DIRS})

IF(WITH_CUDA)
  include_directories(${CUDA_INCLUDE_DIRS})
ELSE()
  # We could still run this without CUDA if using, e.g., pre-segmented frames.
  add_definitions(-DCOMPILE_WITHOUT_CUDA)
ENDIF()

##############################################################
# Specify the include directories, target and link libraries #
##############################################################

# TODO(andrei): In the future, this is where you would add dependencies on Caffe or whatever.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

#set("BOOST_ROOT" "/home/catphive/boost")
#set("BOOST_INCLUDEDIR" "/home/catphive/boost/include")
#set("BOOST_LIBRARYDIR" "/home/catphive/boost/lib")
#set("Boost_NO_BOOST_CMAKE" ON)

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)

#set( BOOST_ROOT "/home/catphive/boost" CACHE PATH "Boost library path" )
#set( Boost_NO_SYSTEM_PATHS on CACHE BOOL "Do not search system for Boost" )
#find_package(Boost COMPONENTS process)


#include_directories(${Boost_INCLUDE_DIRS})


set(Protobuf_PROTOC_EXECUTABLE "/usr/bin/protoc")
find_package(CURL REQUIRED)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/server_result.proto)
protobuf_generate_python(PROTO_PY proto/server_result.proto)

message("PROTOC_LIBRARY_LOCATIONS")
message(${Protobuf_PROTOC_EXECUTABLE})
message(${Protobuf_LIBRARY})
message(${Protobuf_PROTOC_LIBRARY})
message(${Protobuf_INCLUDE_DIR})


add_library(InstRecLib
${INSTRECLIB_SOURCES}
${INSTRECLIB_UTILS_SOURCES}
${INSTRECLIB_HEADERS}
${INSTRECLIB_UTILS_HEADERS}
${PROTO_SRCS}
${PROTO_HDRS})


include_directories(${CURL_INCLUDE_DIR})

target_link_libraries(InstRecLib ${CURL_LIBRARIES} ${PROTOBUF_LIBRARIES})

#target_link_libraries(InstRecLib${Boost_LIBRARIES})

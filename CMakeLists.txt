
cmake_minimum_required(VERSION 2.8)

project(plutracer)

set(DEPD_DIR ${PROJECT_SOURCE_DIR}/depd)

include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${DEPD_DIR}/glm")

file(GLOB SOURCE src/*.cpp src/surfaces/*.cpp inc/*.h inc/*/*.h)
add_executable(plutracer ${SOURCE})
set_property(TARGET plutracer PROPERTY CXX_STANDARD 14)
if(UNIX)
	add_definitions("--std=gnu++11") # old cmake is silly
	target_link_libraries(plutracer pthread)
endif()


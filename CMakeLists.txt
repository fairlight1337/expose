cmake_minimum_required(VERSION 2.6)
project(expose)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(${PROJECT_NAME}_VERSION_MAJOR_0)
set(${PROJECT_NAME}_VERSION_MINOR_1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(${PROJECT_NAME}
  src/main.cpp src/XHelper.cpp)

target_link_libraries(${PROJECT_NAME}
  X11 turbojpeg microhttpd)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin)

cmake_minimum_required(VERSION 3.4.3)
project(SimpleProject)
set(CMAKE_CXX_STANDARD 17)

option(LLVM_ENABLE_RTTI ON)
find_package(LLVM REQUIRED CONFIG)
option(LLVM_ENABLE_RTTI ON)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS} -DLLVM_ENABLE_RTTI=1)

# Now build our tools
add_executable(main main.cpp)
add_executable(odc odc.cpp)
add_executable(work work.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use

llvm_map_components_to_libnames(llvm_libs all)

# Link against LLVM libraries
target_link_libraries(main LLVM)
target_link_libraries(odc LLVM)
target_link_libraries(work LLVM)


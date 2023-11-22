cmake_minimum_required(VERSION 3.5)

project("tcp_client")

# Find python and Boost - both are required dependencies
find_package(PythonLibs REQUIRED)
find_package(Boost COMPONENTS python REQUIRED)

# Without this, any build libraries automatically have names "lib{x}.so"
set(CMAKE_SHARED_MODULE_PREFIX "")

# Add a shared module - modules are intended to be imported at runtime.
# - This is where you add the source files
# add_library(tcp_client MODULE tcp_client.cc)
add_executable(tcp_client test.cc)

# Set up the libraries and header search paths for this target
target_link_libraries(tcp_client
    ${Boost_LIBRARIES} ${PYTHON_LIBRARIES}
    ${PROJECT_SOURCE_DIR}/deps/sockpp/build/libsockpp.a
    )

target_include_directories(tcp_client PRIVATE ${PYTHON_INCLUDE_DIRS})

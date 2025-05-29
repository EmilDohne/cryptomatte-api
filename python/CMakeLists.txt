# Define the project
# --------------------------------------------------------------------------
project(cryptomatte_api_python)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

pybind11_add_module(cryptomatte_api_py src/main.cpp)
target_include_directories(cryptomatte_api_py PUBLIC src)
target_link_libraries(cryptomatte_api_py PUBLIC cryptomatte_api)
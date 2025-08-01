cmake_minimum_required(VERSION 3.15...3.26)
project(${SKBUILD_PROJECT_NAME} LANGUAGES CXX)

set(PYBIND11_FINDPYTHON ON)
pybind11_add_module(cryptomatte_api_py bindings/main.cpp)
# Ensure we build as compressed_image and not compressed_image_py
set_target_properties(cryptomatte_api_py PROPERTIES OUTPUT_NAME cryptomatte_api)
target_include_directories(cryptomatte_api_py PRIVATE cryptomatte_api bindings)
target_link_libraries(cryptomatte_api_py PUBLIC nlohmann_json pybind11_json py_image_util cryptomatte_api)

install(TARGETS cryptomatte_api_py
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
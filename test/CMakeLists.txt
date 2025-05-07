file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Enable ctest
enable_testing()
add_executable(cryptomatte_api_test ${MY_SOURCES} "main.cpp")

if(MSVC)
    target_compile_options(cryptomatte_api_test PRIVATE /MP /utf-8)
endif()
target_link_libraries(cryptomatte_api_test PRIVATE cryptomatte_api)
target_link_libraries(cryptomatte_api_test PRIVATE doctest)

include(CTest)
add_test(test_cryptomatte_api cryptomatte_api_test)

# Copy the images/ folder to the build dir to run the tests
add_custom_command(TARGET cryptomatte_api_test POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_CURRENT_SOURCE_DIR}/images/ $<TARGET_FILE_DIR:cryptomatte_api_test>/images)
add_custom_command(TARGET cryptomatte_api_test POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo 
                    "Finished copying test files to output directory $<TARGET_FILE_DIR:cryptomatte_api_test>/images")
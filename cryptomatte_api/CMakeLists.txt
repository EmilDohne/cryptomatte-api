project(CryptomatteAPI)
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(cryptomatte_api ${MY_SOURCES})
target_include_directories(cryptomatte_api PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(cryptomatte_api PRIVATE "${PROJECT_SOURCE_DIR}/include/cryptomatte")
target_link_libraries(cryptomatte_api PUBLIC
	compressed_image 
	spdlog::spdlog_header_only 
	OpenImageIO::OpenImageIO
)

if (MSVC)
	target_compile_options(cryptomatte_api PRIVATE /utf-8 /MP /DNOMINMAX)
endif()

# Crank up warning levels on both MSVC, Clang and GCC
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_compile_options(
		cryptomatte_api 
		PRIVATE 
		-Wall 
		-Werror 
		-Wextra
	)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(
		cryptomatte_api 
		PRIVATE 
		/W4 
		/WX 
		/w44062 
		/w44464 
		/w45264
		/wd4702  # Disable C4702: unreachable code, seems to trigger in release builds from fmt lib.
	)
endif()
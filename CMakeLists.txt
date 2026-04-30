cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Easygram VERSION 1.0 LANGUAGES CXX)

set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
add_link_options(-fsanitize=address)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Td REQUIRED)
find_package(logger REQUIRED)
find_package(ftxui REQUIRED)
find_package(CLI11 REQUIRED)

include_directories(src)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(main
	src/scenes/load.cpp
	src/scenes/phone.cpp
	src/scenes/code.cpp
	src/scenes/password.cpp
	src/scenes/main.cpp

	src/telegram/processor.cpp
	src/telegram/sender.cpp
	src/telegram/responses.cpp
	src/telegram/facade.cpp

	src/utils/chats.cpp
	src/utils/messages.cpp

	src/main.cpp
	src/renderer.cpp
)

target_include_directories(main PRIVATE src)

target_link_libraries(main
	PRIVATE ftxui::screen
	PRIVATE ftxui::dom
	PRIVATE ftxui::component
	logger
	PRIVATE Td::TdStatic
)

if (EMSCRIPTEN)
  string(APPEND CMAKE_CXX_FLAGS " -s USE_PTHREADS")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s ASYNCIFY")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s PROXY_TO_PTHREAD")
endif()

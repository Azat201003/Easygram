cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Easygram VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Td 1.8.50 REQUIRED
    # PATHS "${CMAKE_CURRENT_SOURCE_DIR}/libs/td/install/lib/cmake/Td"
    # NO_DEFAULT_PATH
)

include(FetchContent)

FetchContent_Declare(
  logger
  GIT_REPOSITORY https://github.com/Azat201003/logger.git
  GIT_TAG        v1.1.9
)
FetchContent_MakeAvailable(logger)

# FetchContent_Declare(
#   Td
#   GIT_REPOSITORY https://github.com/tdlib/td.git
#   GIT_TAG        v1.8.0
# )
# FetchContent_MakeAvailable(Td)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG        v6.1.0
)
FetchContent_MakeAvailable(ftxui)


# find_package(ftxui REQUIRED)

add_library(scenes src/scenes.cpp)
add_library(telegram src/telegram.cpp)

add_executable(main src/main.cpp)

target_link_libraries(main
    PRIVATE
    Td::TdStatic          
    ftxui::screen         
    ftxui::dom
    ftxui::component
    logger
)

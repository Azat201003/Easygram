cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Easygram VERSION 1.0 LANGUAGES CXX)

# Установка стандарта C++17 (обязательно для FTXUI)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключение TDLib
find_package(Td 1.8.50 REQUIRED
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}/libs/td/install/lib/cmake/Td"
    NO_DEFAULT_PATH
)

# Вместо find_package() добавляем поддиректорию с FTXUI
add_subdirectory(libs/FTXUI)

add_executable(main main.cpp utils/scenes.hpp)

# Линковка со всеми зависимостями
target_link_libraries(main
    PRIVATE
    Td::TdStatic          # Для TDLib
    ftxui::screen         # Для FTXUI
    ftxui::dom
    ftxui::component
)
# add_library(utils STATIC utils/scenes.hpp)
target_include_directories(main PRIVATE utils)
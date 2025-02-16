cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(
        CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} /JMC"
    )
else()
    set(
        CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough"
    )
endif()

set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/src/antlr4-4.13.2-complete.jar)
include(${CMAKE_CURRENT_SOURCE_DIR}/src/FindANTLR.cmake)

add_definitions(
    -DANTLR4CPP_STATIC
    -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
)

# Исправленный подход: Скачиваем ANTLR и подключаем C++ runtime вручную
include(FetchContent)
FetchContent_Declare(
    antlr4
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG 4.13.2
)
FetchContent_GetProperties(antlr4)
if(NOT antlr4_POPULATED)
    FetchContent_Populate(antlr4)
    add_subdirectory(${antlr4_SOURCE_DIR}/runtime/Cpp ${antlr4_BINARY_DIR})  # Добавляем C++ runtime
endif()

# Проверка, что цель antlr4_static существует
if (NOT TARGET antlr4_static)
    message(FATAL_ERROR "ANTLR C++ runtime не найден!")
else()
    message(STATUS "ANTLR C++ runtime подключен.")
endif()

# Генерация парсера
antlr_target(FormulaParser ${CMAKE_CURRENT_SOURCE_DIR}/src/Formula.g4 LEXER PARSER LISTENER)

include_directories(
    ${ANTLR4_INCLUDE_DIRS}
    ${ANTLR_FormulaParser_OUTPUT_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${antlr4_SOURCE_DIR}/runtime/Cpp/runtime/src  # Путь к заголовкам
)

file(GLOB sources
    src/*.cpp
    src/*.h
)

add_executable(
    spreadsheet
    ${ANTLR_FormulaParser_CXX_OUTPUTS}
    ${sources}
)

# Линкуем статическую библиотеку ANTLR
target_link_libraries(spreadsheet antlr4_static)

install(
    TARGETS spreadsheet
    DESTINATION bin
    EXPORT spreadsheet
)

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT spreadsheet)
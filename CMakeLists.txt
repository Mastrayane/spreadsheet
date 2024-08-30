# Устанавливаем минимальную версию CMake
cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

# Устанавливаем имя проекта
set(PROJECT_NAME spreadsheet)
project(${PROJECT_NAME})

# Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)

# Настройка флагов компиляции в зависимости от компилятора
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # Добавляем флаг /JMC для отладки в Visual Studio
    set(
        CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} /JMC"
    )
else()
    # Устанавливаем общие флаги для других компиляторов
    set(
        CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough"
    )
endif()

# Устанавливаем путь к исполняемому файлу ANTLR
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/antlr-4.13.2-complete.jar)

# Подключаем скрипт для поиска ANTLR
include(${CMAKE_CURRENT_SOURCE_DIR}/FindANTLR.cmake)

# Добавляем определения для компиляции
add_definitions(
    -DANTLR4CPP_STATIC
    -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
)

# Устанавливаем опцию для статической CRT в Visual C++
set(WITH_STATIC_CRT OFF CACHE BOOL "Visual C++ static CRT for ANTLR" FORCE)

# Добавляем поддиректорию с ANTLR4 runtime
add_subdirectory(antlr4_runtime)

# Создаем цель для генерации кода ANTLR
antlr_target(FormulaParser Formula.g4 LEXER PARSER LISTENER)

# Добавляем директории для включения заголовочных файлов
include_directories(
    ${ANTLR4_INCLUDE_DIRS}
    ${ANTLR_FormulaParser_OUTPUT_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/antlr4_runtime/runtime/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Собираем список исходных файлов
file(GLOB_RECURSE sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

# Создаем исполняемый файл проекта
add_executable(
    ${PROJECT_NAME}
    ${ANTLR_FormulaParser_CXX_OUTPUTS}
    ${sources}
)

# Линкуем библиотеку ANTLR4 с проектом
target_link_libraries(spreadsheet antlr4_static)

# Настройка опций компиляции для Visual Studio
if(MSVC)
    target_compile_options(antlr4_static PRIVATE /W0)
endif()

# Устанавливаем цель для инсталляции
install(
    TARGETS ${PROJECT_NAME}
    DESTINATION bin
    EXPORT ${PROJECT_NAME}
)

# Устанавливаем стартовый проект в Visual Studio
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${PROJECT_NAME})
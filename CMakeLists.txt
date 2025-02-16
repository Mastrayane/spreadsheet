cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(spreadsheet)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /JMC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-implicit-fallthrough")
endif()

# Получаем последнюю версию ANTLR с GitHub
include(FetchContent)
FetchContent_Declare(
    antlr4_latest
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG master
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(antlr4_latest)

# Чтение версии из runtime/Cpp/VERSION
set(ANTLR_VERSION_FILE "${antlr4_latest_SOURCE_DIR}/runtime/Cpp/VERSION")
if(EXISTS ${ANTLR_VERSION_FILE})
    file(READ ${ANTLR_VERSION_FILE} ANTLR_VERSION)
    string(STRIP "${ANTLR_VERSION}" ANTLR_VERSION)  # Исправление ошибки STRIP
    message(STATUS "Используется ANTLR версии: ${ANTLR_VERSION}")
else()
    message(FATAL_ERROR "Файл VERSION не найден в ${ANTLR_VERSION_FILE}")
endif()


# Загрузка ANTLR JAR (теперь имя файла зависит от версии)
set(ANTLR_JAR_URL "https://www.antlr.org/download/antlr-${ANTLR_VERSION}-complete.jar")
set(ANTLR_JAR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/antlr4-${ANTLR_VERSION}-complete.jar")

if (NOT EXISTS ${ANTLR_JAR_PATH})
    message(STATUS "Загрузка ANTLR JAR...")
    file(DOWNLOAD ${ANTLR_JAR_URL} ${ANTLR_JAR_PATH} SHOW_PROGRESS)
endif()

# Удаляем старые JAR-файлы
file(GLOB OLD_JAR_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/antlr4-*-complete.jar")
foreach(JAR_FILE IN LISTS OLD_JAR_FILES)
    if(NOT JAR_FILE STREQUAL ANTLR_JAR_PATH)
        message(STATUS "Удаление старого JAR-файла: ${JAR_FILE}")
        file(REMOVE ${JAR_FILE})
    endif()
endforeach()

include(${CMAKE_CURRENT_SOURCE_DIR}/src/FindANTLR.cmake)  # Передаем ANTLR_VERSION_FILE через область видимости

# Подключаем ANTLR C++ runtime
add_subdirectory(${antlr4_latest_SOURCE_DIR}/runtime/Cpp ${antlr4_latest_BINARY_DIR})

# Проверка цели antlr4_static
if (NOT TARGET antlr4_static)
    message(FATAL_ERROR "ANTLR C++ runtime не найден!")
else()
    message(STATUS "ANTLR C++ runtime подключен.")
endif()

# Генерация парсера через FindANTLR.cmake
antlr_target(FormulaParser ${CMAKE_CURRENT_SOURCE_DIR}/src/Formula.g4 LEXER PARSER LISTENER)

include_directories(
    ${ANTLR4_INCLUDE_DIRS}
    ${ANTLR_FormulaParser_OUTPUT_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${antlr4_latest_SOURCE_DIR}/runtime/Cpp/runtime/src
)

file(GLOB sources src/*.cpp src/*.h)
add_executable(spreadsheet ${ANTLR_FormulaParser_CXX_OUTPUTS} ${sources})
target_link_libraries(spreadsheet antlr4_static)

install(TARGETS spreadsheet DESTINATION bin EXPORT spreadsheet)
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT spreadsheet)
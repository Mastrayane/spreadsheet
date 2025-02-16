# FindANTLR.cmake (обновленный)
# --------------------------------------------------------
# Этот файл ищет ANTLR JAR и определяет макрос для генерации парсера.
# Версия ANTLR синхронизируется с файлом VERSION, который передается через переменную ANTLR_VERSION_FILE.

# Проверяем, что переменная ANTLR_VERSION_FILE определена
if(NOT DEFINED ANTLR_VERSION_FILE)
    message(FATAL_ERROR "ANTLR_VERSION_FILE не указан!")
endif()

# Читаем версию ANTLR из файла VERSION
if(EXISTS ${ANTLR_VERSION_FILE})
    file(READ ${ANTLR_VERSION_FILE} ANTLR_VERSION)
    string(STRIP "${ANTLR_VERSION}" ANTLR_VERSION)
    message(STATUS "Найдена версия ANTLR: ${ANTLR_VERSION}")
else()
    message(FATAL_ERROR "Файл VERSION не найден: ${ANTLR_VERSION_FILE}")
endif()

# Ищем JAR-файл с динамическим именем на основе версии
set(ANTLR_JAR_NAME "antlr4-${ANTLR_VERSION}-complete.jar")
find_file(ANTLR_EXECUTABLE
    NAMES ${ANTLR_JAR_NAME}
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/src
    REQUIRED
    DOC "ANTLR JAR-файл"
)

# Проверяем, что Java установлена
find_package(Java QUIET COMPONENTS Runtime)
if(NOT Java_JAVA_EXECUTABLE)
    message(FATAL_ERROR "Java не найдена! Убедитесь, что Java установлена и доступна в PATH.")
endif()

# Если ANTLR JAR найден, определяем макрос для генерации парсера
if(ANTLR_EXECUTABLE AND Java_JAVA_EXECUTABLE)
    # Проверяем, что ANTLR JAR работает
    execute_process(
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE}
        OUTPUT_VARIABLE ANTLR_COMMAND_OUTPUT
        ERROR_VARIABLE ANTLR_COMMAND_ERROR
        RESULT_VARIABLE ANTLR_COMMAND_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT ANTLR_COMMAND_RESULT EQUAL 0)
        message(FATAL_ERROR "Ошибка при запуске ANTLR: ${ANTLR_COMMAND_ERROR}")
    endif()

    # Макрос для генерации парсера
    macro(ANTLR_TARGET Name InputFile)
        set(ANTLR_OPTIONS LEXER PARSER LISTENER VISITOR)
        set(ANTLR_ONE_VALUE_ARGS PACKAGE OUTPUT_DIRECTORY DEPENDS_ANTLR)
        set(ANTLR_MULTI_VALUE_ARGS COMPILE_FLAGS DEPENDS)
        cmake_parse_arguments(ANTLR_TARGET
                              "${ANTLR_OPTIONS}"
                              "${ANTLR_ONE_VALUE_ARGS}"
                              "${ANTLR_MULTI_VALUE_ARGS}"
                              ${ARGN})

        set(ANTLR_${Name}_INPUT ${InputFile})
        get_filename_component(ANTLR_INPUT ${InputFile} NAME_WE)

        # Определяем выходной каталог
        if(ANTLR_TARGET_OUTPUT_DIRECTORY)
            set(ANTLR_${Name}_OUTPUT_DIR ${ANTLR_TARGET_OUTPUT_DIRECTORY})
        else()
            set(ANTLR_${Name}_OUTPUT_DIR
                ${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated_src/${ANTLR_INPUT})
        endif()

        unset(ANTLR_${Name}_CXX_OUTPUTS)

        # Определяем выходные файлы в зависимости от опций
        if((ANTLR_TARGET_LEXER AND NOT ANTLR_TARGET_PARSER) OR
           (ANTLR_TARGET_PARSER AND NOT ANTLR_TARGET_LEXER))
            list(APPEND ANTLR_${Name}_CXX_OUTPUTS
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}.h
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}.cpp)
            set(ANTLR_${Name}_OUTPUTS
                ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}.interp
                ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}.tokens)
        else()
            list(APPEND ANTLR_${Name}_CXX_OUTPUTS
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Lexer.h
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Lexer.cpp
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Parser.h
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Parser.cpp)
            list(APPEND ANTLR_${Name}_OUTPUTS
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Lexer.interp
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Lexer.tokens)
        endif()

        if(ANTLR_TARGET_LISTENER)
            list(APPEND ANTLR_${Name}_CXX_OUTPUTS
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}BaseListener.h
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}BaseListener.cpp
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Listener.h
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Listener.cpp)
            list(APPEND ANTLR_TARGET_COMPILE_FLAGS -listener)
        endif()

        if(ANTLR_TARGET_VISITOR)
            list(APPEND ANTLR_${Name}_CXX_OUTPUTS
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}BaseVisitor.h
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}BaseVisitor.cpp
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Visitor.h
                 ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Visitor.cpp)
            list(APPEND ANTLR_TARGET_COMPILE_FLAGS -visitor)
        endif()

        if(ANTLR_TARGET_PACKAGE)
            list(APPEND ANTLR_TARGET_COMPILE_FLAGS -package ${ANTLR_TARGET_PACKAGE})
        endif()

        list(APPEND ANTLR_${Name}_OUTPUTS ${ANTLR_${Name}_CXX_OUTPUTS})

        if(ANTLR_TARGET_DEPENDS_ANTLR)
            if(ANTLR_${ANTLR_TARGET_DEPENDS_ANTLR}_INPUT)
                list(APPEND ANTLR_TARGET_DEPENDS
                     ${ANTLR_${ANTLR_TARGET_DEPENDS_ANTLR}_INPUT})
                list(APPEND ANTLR_TARGET_DEPENDS
                     ${ANTLR_${ANTLR_TARGET_DEPENDS_ANTLR}_OUTPUTS})
            else()
                message(SEND_ERROR
                        "ANTLR target '${ANTLR_TARGET_DEPENDS_ANTLR}' не найден")
            endif()
        endif()

        # Команда для генерации парсера
        add_custom_command(
            OUTPUT ${ANTLR_${Name}_OUTPUTS}
            COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE}
                    ${InputFile}
                    -o ${ANTLR_${Name}_OUTPUT_DIR}
                    -no-listener
                    -Dlanguage=Cpp
                    ${ANTLR_TARGET_COMPILE_FLAGS}
            DEPENDS ${InputFile}
                    ${ANTLR_TARGET_DEPENDS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building ${Name} with ANTLR ${ANTLR_VERSION}"
        )
    endmacro(ANTLR_TARGET)
else()
    message(FATAL_ERROR "ANTLR JAR не найден или Java не установлена!")
endif()

# Подключаем стандартный механизм поиска пакетов
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    ANTLR
    REQUIRED_VARS ANTLR_EXECUTABLE Java_JAVA_EXECUTABLE
    VERSION_VAR ANTLR_VERSION
)
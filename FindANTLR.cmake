# Найти пакет Java с компонентом Runtime
find_package(Java QUIET COMPONENTS Runtime)

# Найти исполняемый файл ANTLR, если он еще не найден
if(NOT ANTLR_EXECUTABLE)
  find_program(ANTLR_EXECUTABLE
               NAMES antlr.jar antlr4.jar antlr-4.jar antlr-4.13.2-complete.jar)
endif()

# Если ANTLR и Java найдены, выполнить команду для получения версии ANTLR
if(ANTLR_EXECUTABLE AND Java_JAVA_EXECUTABLE)
  execute_process(
      COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE}
      OUTPUT_VARIABLE ANTLR_COMMAND_OUTPUT
      ERROR_VARIABLE ANTLR_COMMAND_ERROR
      RESULT_VARIABLE ANTLR_COMMAND_RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Проверить результат выполнения команды
  if(ANTLR_COMMAND_RESULT EQUAL 0)
    # Извлечь версию ANTLR из вывода команды
    string(REGEX MATCH "Version [0-9]+(\\.[0-9]+)*" ANTLR_VERSION ${ANTLR_COMMAND_OUTPUT})
    string(REPLACE "Version " "" ANTLR_VERSION ${ANTLR_VERSION})
  else()
    # Вывести сообщение об ошибке, если команда завершилась с ошибкой
    message(
        SEND_ERROR
        "Command '${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE}' "
        "failed with the output '${ANTLR_COMMAND_ERROR}'")
  endif()

  # Определить макрос для создания целей с использованием ANTLR
  macro(ANTLR_TARGET Name InputFile)
    # Определить опции и аргументы макроса
    set(ANTLR_OPTIONS LEXER PARSER LISTENER VISITOR)
    set(ANTLR_ONE_VALUE_ARGS PACKAGE OUTPUT_DIRECTORY DEPENDS_ANTLR)
    set(ANTLR_MULTI_VALUE_ARGS COMPILE_FLAGS DEPENDS)
    cmake_parse_arguments(ANTLR_TARGET
                          "${ANTLR_OPTIONS}"
                          "${ANTLR_ONE_VALUE_ARGS}"
                          "${ANTLR_MULTI_VALUE_ARGS}"
                          ${ARGN})

    # Установить входные данные для ANTLR
    set(ANTLR_${Name}_INPUT ${InputFile})

    # Получить базовое имя входного файла
    get_filename_component(ANTLR_INPUT ${InputFile} NAME_WE)

    # Установить директорию вывода
    if(ANTLR_TARGET_OUTPUT_DIRECTORY)
      set(ANTLR_${Name}_OUTPUT_DIR ${ANTLR_TARGET_OUTPUT_DIRECTORY})
    else()
      set(ANTLR_${Name}_OUTPUT_DIR
          ${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated_src/${ANTLR_INPUT})
    endif()

    # Очистить список выходных файлов C++
    unset(ANTLR_${Name}_CXX_OUTPUTS)

    # Определить выходные файлы в зависимости от типа (лексер, парсер)
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

    # Добавить выходные файлы для слушателя, если указан
    if(ANTLR_TARGET_LISTENER)
      list(APPEND ANTLR_${Name}_CXX_OUTPUTS
           ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}BaseListener.h
           ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}BaseListener.cpp
           ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Listener.h
           ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Listener.cpp)
      list(APPEND ANTLR_TARGET_COMPILE_FLAGS -listener)
    endif()

    # Добавить выходные файлы для посетителя, если указан
    if(ANTLR_TARGET_VISITOR)
      list(APPEND ANTLR_${Name}_CXX_OUTPUTS
           ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}BaseVisitor.h
           ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}BaseVisitor.cpp
           ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Visitor.h
           ${ANTLR_${Name}_OUTPUT_DIR}/${ANTLR_INPUT}Visitor.cpp)
      list(APPEND ANTLR_TARGET_COMPILE_FLAGS -visitor)
    endif()

    # Добавить флаг пакета, если указан
    if(ANTLR_TARGET_PACKAGE)
      list(APPEND ANTLR_TARGET_COMPILE_FLAGS -package ${ANTLR_TARGET_PACKAGE})
    endif()

    # Добавить выходные файлы C++ в общий список выходных файлов
    list(APPEND ANTLR_${Name}_OUTPUTS ${ANTLR_${Name}_CXX_OUTPUTS})

    # Добавить зависимости от других целей ANTLR, если указаны
    if(ANTLR_TARGET_DEPENDS_ANTLR)
      if(ANTLR_${ANTLR_TARGET_DEPENDS_ANTLR}_INPUT)
        list(APPEND ANTLR_TARGET_DEPENDS
             ${ANTLR_${ANTLR_TARGET_DEPENDS_ANTLR}_INPUT})
        list(APPEND ANTLR_TARGET_DEPENDS
             ${ANTLR_${ANTLR_TARGET_DEPENDS_ANTLR}_OUTPUTS})
      else()
        message(SEND_ERROR
                "ANTLR target '${ANTLR_TARGET_DEPENDS_ANTLR}' not found")
      endif()
    endif()

    # Добавить пользовательскую команду для генерации файлов с помощью ANTLR
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
        COMMENT "Building ${Name} with ANTLR ${ANTLR_VERSION}")
  endmacro(ANTLR_TARGET)

endif(ANTLR_EXECUTABLE AND Java_JAVA_EXECUTABLE)

# Включить стандартные аргументы для поиска пакета
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    ANTLR
    REQUIRED_VARS ANTLR_EXECUTABLE Java_JAVA_EXECUTABLE
    VERSION_VAR ANTLR_VERSION)
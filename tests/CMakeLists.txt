CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(Tests)

LOAD_SPDLOG()
LOAD_GFLAGS()
LOAD_GTEST()
LOAD_OPENCV()
LOAD_SNDFINE()
LOAD_KISSFFT()
LOAD_ONNXRUNTIME()

INCLUDE_DIRECTORIES(
    ${PROJECT_INCLUDE_DIR}
    ${CMAKE_INCLUDE_PATH}
)

LINK_LIBRARIES(
    infer
    fbank
    module_logger
    SndFile::sndfile
    kissfft::kissfft
    onnxruntime::onnxruntime
    opencv::opencv
    gflags::gflags
    gtest::gtest
    spdlog::spdlog
)

FILE(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR}/tests)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/tests)
FILE(GLOB_RECURSE APP_SOURCES *.cc)
MESSAGE(STATUS "APP SOURCES: ${APP_SOURCES}")
FOREACH(sourcefile ${APP_SOURCES})
    STRING(REGEX MATCH "[^/]+$" sourcefilewithoutpath ${sourcefile})
    STRING(REPLACE ".cc" "" testname ${sourcefilewithoutpath})
    ADD_EXECUTABLE( ${testname} ${sourcefile})
    
    INSTALL(TARGETS ${testname} DESTINATION tests)
ENDFOREACH(sourcefile ${APP_SOURCES})

FILE(GLOB DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/data/*")
IF(DATA_FILES)
    INSTALL(FILES ${DATA_FILES} DESTINATION data)
ENDIF()

FILE(GLOB MODEL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/models/*")
IF(MODEL_FILES)
    INSTALL(FILES ${MODEL_FILES} DESTINATION models)
ENDIF()

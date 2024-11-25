# Specialized libraries can be compiled separately, softinked to the 3RDPARTY_DIR, and then handled independently.
SET(3RDPARTY_ROOT ${PROJECT_SOURCE_DIR}/3rdparty)
SET(3RDPARTY_DIR ${PROJECT_SOURCE_DIR}/3rdparty/target/${TARGET_OS}_${TARGET_ARCH})
MESSAGE(STATUS "3RDPARTY_DIR: ${3RDPARTY_DIR}")

MACRO(LOAD_SPDLOG)
    ADD_DEFINITIONS(-DSPDLOG_USE_STD_FORMAT)
    FIND_PACKAGE(spdlog REQUIRED)
ENDMACRO()

MACRO(LOAD_GFLAGS)
    FIND_PACKAGE(gflags REQUIRED)
ENDMACRO()

MACRO(LOAD_GTEST)
    FIND_PACKAGE(GTest REQUIRED)
ENDMACRO()

MACRO(LOAD_SNDFINE)
    FIND_PACKAGE(SndFile REQUIRED)
ENDMACRO()

MACRO(LOAD_KISSFFT)
    FIND_PACKAGE(kissfft REQUIRED)
ENDMACRO()

MACRO(LOAD_OPENCV)
    FIND_PACKAGE(OpenCV REQUIRED)
    IF(OpenCV_INCLUDE_DIRS)
        MESSAGE(STATUS "Opencv library status:")
        MESSAGE(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
        MESSAGE(STATUS "    opencv version: ${OpenCV_VERSION}")
        MESSAGE(STATUS "    libraries: ${OpenCV_LIBS}")
        ELSE()
        MESSAGE(FATAL_ERROR "OpenCV not found!")
    ENDIF()
ENDMACRO()

MACRO(LOAD_ONNXRUNTIME)
    FIND_PACKAGE(onnxruntime REQUIRED)
ENDMACRO()

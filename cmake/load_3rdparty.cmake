# Specialized libraries can be compiled separately, softinked to the 3RDPARTY_DIR, and then handled independently.
SET(3RDPARTY_ROOT ${PROJECT_SOURCE_DIR}/3rdparty)
SET(3RDPARTY_DIR ${PROJECT_SOURCE_DIR}/3rdparty/target/${TARGET_OS}_${TARGET_ARCH})
MESSAGE(STATUS "3RDPARTY_DIR: ${3RDPARTY_DIR}")

MACRO(LOAD_SNDFINE)
    SET(SNDFINE_HOME ${3RDPARTY_DIR}/sndfile)
    MESSAGE(STATUS "SNDFINE_HOME: ${SNDFINE_HOME}")

    SET(SndFile_INCLUDE_DIR "${SNDFINE_HOME}/include")
    SET(SndFile_LIBRARY_DIR "${SNDFINE_HOME}/lib")

    SET(SndFile_LIBS
        sndfile
    )

    IF(SndFile_INCLUDE_DIR)
        MESSAGE(STATUS "sndfile libraries path: ${SndFile_INCLUDE_DIR}")
        MESSAGE(STATUS "sndfile libraries path: ${SndFile_LIBRARY_DIR}")
        MESSAGE(STATUS "sndfile libraries : ${SndFile_LIBS}")
    ELSE()
        MESSAGE(FATAL_ERROR "SndFile_INCLUDE_DIR not found!")
    ENDIF()

    LINK_DIRECTORIES(
        ${SndFile_LIBRARY_DIR}
    )
ENDMACRO()

MACRO(LOAD_KISSFFT)
    SET(KISSFFT_HOME ${3RDPARTY_DIR}/kissfft)
    SET(kissfft_INCLUDE_DIR "${KISSFFT_HOME}/include")
    SET(kissfft_LIBRARY_DIR "${KISSFFT_HOME}/lib")

    SET(kissfft_LIBS
        kissfft-float
    )

    IF(kissfft_INCLUDE_DIR)
        MESSAGE(STATUS "kissfft libraries path: ${kissfft_INCLUDE_DIR}")
        MESSAGE(STATUS "kissfft libraries path: ${kissfft_LIBRARY_DIR}")
        MESSAGE(STATUS "kissfft libraries : ${kissfft_LIBS}")
    ELSE()
        MESSAGE(FATAL_ERROR "kissfft_INCLUDE_DIR not found!")
    ENDIF()

    LINK_DIRECTORIES(
        ${kissfft_LIBRARY_DIR}
    )
ENDMACRO()

MACRO(LOAD_OPENCV)
    SET(OPENCV_HOME ${3RDPARTY_DIR}/opencv)
    SET(OpenCV_INCLUDE_DIRS ${OPENCV_HOME}/jni/include)
    SET(OpenCV_LIBRARY_DIRS ${OPENCV_HOME}/libs/${ANDROID_ABI})

    SET(OpenCV_LIBS
        ${OpenCV_LIBRARY_DIRS}/libopencv_core.so
        ${OpenCV_LIBRARY_DIRS}/libopencv_imgproc.so
        ${OpenCV_LIBRARY_DIRS}/libopencv_imgcodecs.so
        ${OpenCV_LIBRARY_DIRS}/libopencv_highgui.so
        ${OpenCV_LIBRARY_DIRS}/libopencv_video.so
        ${OpenCV_LIBRARY_DIRS}/libopencv_videoio.so
    )

ENDMACRO()

MACRO(LOAD_ONNXRUNTIME)
    SET(ONNXRUNTIME_HOME ${3RDPARTY_DIR}/onnxruntime)
    SET(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_HOME}/include")
    SET(ONNXRUNTIME_LIBRARY_DIR "${ONNXRUNTIME_HOME}/lib")
    SET(ONNXRUNTIME_LIBS
        onnxruntime
    )

    IF(ONNXRUNTIME_INCLUDE_DIR)
        MESSAGE(STATUS "ONNXRUNTIME_INCLUDE_DIR : ${ONNXRUNTIME_INCLUDE_DIR}")
        MESSAGE(STATUS "ONNXRUNTIME_LIBRARY_DIR : ${ONNXRUNTIME_LIBRARY_DIR}")
        MESSAGE(STATUS "ONNXRUNTIME_LIBS : ${ONNXRUNTIME_LIBS}")
    ELSE()
        MESSAGE(FATAL_ERROR "ONNXRUNTIME_LIBS not found!")
    ENDIF()

    LINK_DIRECTORIES(
        ${ONNXRUNTIME_LIBRARY_DIR}
    )

ENDMACRO()
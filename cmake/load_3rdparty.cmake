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
    
    IF (TARGET_OS STREQUAL "Android" )
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
    ELSE()
        SET(OpenCV_LIBRARY_DIR ${OPENCV_HOME}/lib)
        LIST(APPEND CMAKE_PREFIX_PATH ${OpenCV_LIBRARY_DIR}/cmake)
        FIND_PACKAGE(OpenCV CONFIG REQUIRED COMPONENTS core imgproc highgui videoio imgcodecs calib3d)
        
        IF(OpenCV_INCLUDE_DIRS)
            MESSAGE(STATUS "Opencv library status:")
            MESSAGE(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
            MESSAGE(STATUS "    libraries dir: ${OpenCV_LIBRARY_DIR}")
            MESSAGE(STATUS "    libraries: ${OpenCV_LIBS}")
        ELSE()
            MESSAGE(FATAL_ERROR "OpenCV not found!")
        ENDIF()
    
        LINK_DIRECTORIES(
            ${OpenCV_LIBRARY_DIR}
        )
    ENDIF()

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

MACRO(LOAD_FFMPEG)
    FIND_FILE(FFMPEG_INCLUDE_DIR include ${3RDPARTY_DIR}/ffmpeg NO_DEFAULT_PATH)
    FIND_FILE(FFMPEG_LIBRARY_DIR lib ${3RDPARTY_DIR}/ffmpeg NO_DEFAULT_PATH)
    SET(FFMPEG_LIBS
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        postproc
        lzma
        va
        va-drm
        va-x11
        X11
        vdpau
        bz2
    )

    IF(FFMPEG_INCLUDE_DIR)
        MESSAGE(STATUS "ffmpeg include path: ${FFMPEG_INCLUDE_DIR}")
        MESSAGE(STATUS "ffmpeg libraries path: ${FFMPEG_LIBRARY_DIR}")
        MESSAGE(STATUS "ffmpeg libraries : ${FFMPEG_LIBS}")
    ELSE()
        MESSAGE(FATAL_ERROR "FFMPEG_INCLUDE_DIR not found!")
    ENDIF()
    LINK_DIRECTORIES(
        ${FFMPEG_LIBRARY_DIR}
    )
ENDMACRO()

MACRO(LOAD_ANDROID_ENV)
    SET(ANDROID_JIN_INCLUDE_DIR "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include")
    SET(ANDROID_JIN_LIBS_DIR "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/${TARGET_ARCH}-linux-android/24")
    SET(ANDROID_JIN_LIBS 
        android
        log
    )

    LINK_DIRECTORIES(${ANDROID_JIN_LIBS_DIR})
ENDMACRO()

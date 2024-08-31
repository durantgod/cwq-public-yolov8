cmake_minimum_required(VERSION 3.16.0)
project(Analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread ")


set(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# system library
set(INCLUDE_DIR /usr/local/include)
set(LIB_DIR /usr/local/lib)
include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR})

# onnxruntime
include_directories(/E:/new/BXC_VideoAnalyzer_v3.41/Analyzer_linux_lib/onnxruntime-win-x64-1.17.3/include)
link_directories(/E:/new/BXC_VideoAnalyzer_v3.41/Analyzer_linux_lib/onnxruntime-win-x64-1.17.3/lib)

find_package(OpenCV REQUIRED)

set(source
        Analyzer/Core/Utils/CalcuIOU.cpp
        Analyzer/Core/Utils/Request.cpp
        Analyzer/Core/Algorithm.cpp
        Analyzer/Core/AlgorithmOnnxRuntime.cpp
        Analyzer/Core/Analyzer.cpp
        Analyzer/Core/AvPullStream.cpp
        Analyzer/Core/AvPushStream.cpp
        Analyzer/Core/Config.cpp
        Analyzer/Core/Frame.cpp
        Analyzer/Core/GenerateAlarmVideo.cpp
        Analyzer/Core/Scheduler.cpp
        Analyzer/Core/Server.cpp
        Analyzer/Core/Worker.cpp
        Analyzer/main.cpp)

add_executable(Analyzer ${source})
target_link_libraries(Analyzer ${OpenCV_LIBS})
target_link_libraries(Analyzer event curl jsoncpp avformat avcodec avutil swscale swresample pthread)
target_link_libraries(Analyzer onnxruntime )





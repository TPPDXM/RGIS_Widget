# GravMagnFrontend —— 重磁数据处理前端（Qt Widgets 界面）
#
# 工程说明：
#   * 对应原 MFC 工程 RGISGravMagnDataProcessing（VC6/MFC）的 Qt 前端迁移；
#   * 前端只通过 backend/RgisBackend.h 中定义的接口 IRgisBackend 与后端（算法核心）交互，
#     后端实现该接口后调用 CBackendService::setRgisBackend() 注入即可对接；
#   * 未注入后端时自动使用 backend/NullBackend.cpp 中的占位实现（界面可运行）。
#
# 环境：
#   * Windows
#   * Qt 5.15.2（D:\Qt\5.15.2\msvc2019_64 或 D:\Qt\5.15.2\mingw81_64）
#
# 编译方式（以 Qt Creator 打开本文件即可，或命令行）：
#   MSVC2019 x64 ：
#     cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=D:/Qt/5.15.2/msvc2019_64
#     cmake --build build --config Release
#   MinGW 8.1 x64 ：
#     cmake -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=D:/Qt/5.15.2/mingw81_64 \
#           -DCMAKE_MAKE_PROGRAM=D:/Qt/Tools/mingw810_64/bin/mingw32-make.exe
#     cmake --build build

cmake_minimum_required(VERSION 3.16)

project(GravMagnFrontend VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.h
    src/FrontendUtils.h
    src/MainWindow.cpp
    src/backend/RgisBackend.h
    src/backend/NullBackend.h
    src/backend/NullBackend.cpp
    src/dlg/FreqDomainCmpsFilterDlg.h
    src/dlg/FreqDomainCmpsFilterDlg.cpp
    src/dlg/FreqDomainDownwardDlg.h
    src/dlg/FreqDomainDownwardDlg.cpp
    src/dlg/FreqDomainGradientDlg.h
    src/dlg/FreqDomainGradientDlg.cpp
    src/dlg/FreqDomainGradwardDlg.h
    src/dlg/FreqDomainGradwardDlg.cpp
    src/dlg/FreqDomainHorzGradDlg.h
    src/dlg/FreqDomainHorzGradDlg.cpp
    src/dlg/FreqDomainIterDrapeDlg.h
    src/dlg/FreqDomainIterDrapeDlg.cpp
    src/dlg/FreqDomainIterwardDlg.h
    src/dlg/FreqDomainIterwardDlg.cpp
    src/dlg/FreqDomainMaxiGradDlg.h
    src/dlg/FreqDomainMaxiGradDlg.cpp
    src/dlg/FreqDomainMultiCpnDlg.h
    src/dlg/FreqDomainMultiCpnDlg.cpp
    src/dlg/FreqDomainNormFilterDlg.h
    src/dlg/FreqDomainNormFilterDlg.cpp
    src/dlg/FreqDomainOneDerivDlg.h
    src/dlg/FreqDomainOneDerivDlg.cpp
    src/dlg/FreqDomainPsudoGraDlg.h
    src/dlg/FreqDomainPsudoGraDlg.cpp
    src/dlg/FreqDomainReToPoleDlg.h
    src/dlg/FreqDomainReToPoleDlg.cpp
    src/dlg/FreqDomainRmToPoleDlg.h
    src/dlg/FreqDomainRmToPoleDlg.cpp
    src/dlg/FreqDomainStructureDlg.h
    src/dlg/FreqDomainStructureDlg.cpp
    src/dlg/FreqDomainThreeCpnDlg.h
    src/dlg/FreqDomainThreeCpnDlg.cpp
    src/dlg/FreqDomainTiltGradDlg.h
    src/dlg/FreqDomainTiltGradDlg.cpp
    src/dlg/FreqDomainTotlGradDlg.h
    src/dlg/FreqDomainTotlGradDlg.cpp
    src/dlg/FreqDomainTwoDerivDlg.h
    src/dlg/FreqDomainTwoDerivDlg.cpp
    src/dlg/FreqDomainUpwardDlg.h
    src/dlg/FreqDomainUpwardDlg.cpp
)

add_executable(GravMagnFrontend WIN32 ${PROJECT_SOURCES})

# 源代码目录加入包含路径（头文件按 "backend/xxx.h"、"core/xxx.h"、"dlg/xxx.h" 引用）
target_include_directories(GravMagnFrontend PRIVATE src)

target_link_libraries(GravMagnFrontend PRIVATE Qt5::Widgets)

if(MSVC)
    # 源码统一按 UTF-8 解析，避免中文注释/字符串在 MSVC 下出现乱码
    target_compile_options(GravMagnFrontend PRIVATE /utf-8)
endif()

if(MINGW)
    target_compile_options(GravMagnFrontend PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

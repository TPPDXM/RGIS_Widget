// FrontendUtils.h : 前端公共工具（合并头文件，全 header-only）
//
// 本文件把原先分散的 4 个文件合并为 1 个（原 BackendService.h/.cpp、
// BackendConvert.h、ExtendGridSize.h），全部为内联实现，无 .cpp。
// 包含三部分：
//   1. CBackendService        —— 后端实例注册与获取（原 BackendService）
//   2. toBackendString / fromBackendString —— 前端字符串与后端接口字符串转换（原 BackendConvert）
//   3. suggestExtendSize      —— 默认扩边尺寸（2 的幂）计算（原 ExtendGridSize）
//
// 说明：使用函数内静态局部变量保存后端指针/占位实例（C++11 保证 inline 函数内的
//       static 跨编译单元只有一份，且线程安全的初始化），故无需 .cpp 文件。

#pragma once

#include <string>

#include <QString>

#include "backend/RgisBackend.h"
#include "backend/NullBackend.h"

//---------------------------------------------------------------------------
// 1. 后端服务（后端实例的全局注册与获取）
//---------------------------------------------------------------------------

// 后端服务类（全局单例风格，仅静态方法，全部内联实现）
class CBackendService
{
public:
    // 功能：注入真实后端实现（由宿主程序在启动时调用；传入 NULL 时恢复为占位实现）
    static void setRgisBackend(IRgisBackend* pBackend)
    {
        backendStorage() = pBackend;
    }

    // 功能：获取当前后端接口指针（从未注入时返回内置占位实现，永不为 NULL）
    static IRgisBackend* rgisBackend()
    {
        IRgisBackend* pBackend = backendStorage();
        if (pBackend == NULL)
        {
            return nullBackendInstance();
        }
        return pBackend;
    }

private:
    // 功能：后端指针存储（函数内静态变量，C++11 保证跨编译单元唯一）
    // 返回：当前注册的后端实现指针（NULL 表示使用占位实现）
    static IRgisBackend*& backendStorage()
    {
        static IRgisBackend* sBackend = NULL;
        return sBackend;
    }

    // 功能：获取内置占位后端实例（函数内静态对象，保证线程安全初始化）
    static IRgisBackend* nullBackendInstance()
    {
        static CNullRgisBackend sNullBackend;
        return &sNullBackend;
    }
};

//---------------------------------------------------------------------------
// 2. 前端字符串与后端接口字符串（UTF-8）之间的转换
//---------------------------------------------------------------------------

// 功能：将前端字符串转换为后端接口使用的 UTF-8 字符串
inline std::string toBackendString(const QString& text)
{
    return text.toUtf8().constData();
}

// 功能：将后端接口返回的 UTF-8 字符串转换为前端字符串
inline QString fromBackendString(const std::string& text)
{
    return QString::fromUtf8(text.c_str());
}

//---------------------------------------------------------------------------
// 3. 网格扩边尺寸计算工具
//---------------------------------------------------------------------------

// 功能：计算默认扩边尺寸（最小的满足条件的 2 的幂）
// 参数：nPointCount —— 原始数据点数（行数或列数）
// 返回：默认扩边尺寸（>= 2，为 2 的幂）
// 规则（与原 MFC 工程 FreqDomain*Dlg::ReadData 一致）：
//   1. 取 2 的幂 P，使 P 大于等于原始点数 n；
//   2. 若 (P - n) < P / 2（扩边区太小，影响 FFT 结果），则将 P 翻倍。
inline int suggestExtendSize(int nPointCount)
{
    if (nPointCount <= 0)
    {
        return 2;
    }

    int nSize = 1;
    while (nSize < nPointCount)
    {
        nSize *= 2;
    }

    if ((nSize - nPointCount) < nSize / 2)
    {
        nSize *= 2;
    }

    return nSize;
}

// BackendConvert.h : 前端字符串与后端接口字符串之间的转换工具
//
// 功能：后端接口（RgisBackend.h）为保持无 Qt 依赖，路径使用 UTF-8 编码的
//       std::string 传递；前端(QString 内部为 UTF-16)在调用时统一经本工具转换。

#pragma once

#include <string>

#include <QString>

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

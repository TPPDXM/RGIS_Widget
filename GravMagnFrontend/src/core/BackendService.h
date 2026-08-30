// BackendService.h : 后端服务（后端实例的全局注册与获取）
//
// 功能：
//   前端界面只通过 CBackendService::rgisBackend() 获取后端接口指针，
//   宿主程序（后端对接方）在启动时调用 CBackendService::setRgisBackend()
//   注入真实的后端实现；未注入时自动使用内置占位实现（CNullRgisBackend），
//   保证界面在无后端的情况下依然可以运行（文件头读取可用，算法返回未接入错误）。
//
// 对应原 MFC 工程中用静态成员封装的算法对象（如 CFreqDomainCmpsFilterDlg::m_FFT），
// 此处以全局服务的形式统一管理。

#pragma once

#include "../backend/RgisBackend.h"

// 后端服务类（全局单例风格，仅静态方法）
class CBackendService
{
public:
    // 功能：注入真实后端实现（由宿主程序在启动时调用；传入 NULL 时恢复为占位实现）
    static void setRgisBackend(IRgisBackend* pBackend);

    // 功能：获取当前后端接口指针（从未注入时返回内置占位实现，永不为 NULL）
    static IRgisBackend* rgisBackend();

private:
    // 功能：获取内置占位后端实例（函数内静态对象，保证线程安全初始化）
    static IRgisBackend* nullBackendInstance();

    static IRgisBackend* msBackend;    // 当前注册的后端实现指针（NULL 表示使用占位实现）
};

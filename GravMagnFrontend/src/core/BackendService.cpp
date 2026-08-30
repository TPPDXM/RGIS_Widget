// BackendService.cpp : 后端服务实现

#include "BackendService.h"

#include "../backend/NullBackend.h"

// 当前注册的后端实现指针（NULL 表示使用内置占位实现）
IRgisBackend* CBackendService::msBackend = NULL;

// 功能：注入真实后端实现（由宿主程序在启动时调用；传入 NULL 时恢复为占位实现）
void CBackendService::setRgisBackend(IRgisBackend* pBackend)
{
    msBackend = pBackend;
}

// 功能：获取内置占位后端实例（函数内静态对象，保证线程安全初始化）
IRgisBackend* CBackendService::nullBackendInstance()
{
    static CNullRgisBackend sNullBackend;
    return &sNullBackend;
}

// 功能：获取当前后端接口指针（从未注入时返回内置占位实现，永不为 NULL）
IRgisBackend* CBackendService::rgisBackend()
{
    if (msBackend == NULL)
    {
        return nullBackendInstance();
    }
    return msBackend;
}

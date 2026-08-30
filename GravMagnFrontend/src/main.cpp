// main.cpp : 前端程序入口
//
// 功能：
//   1. 创建 Qt 应用（Qt 5.15.2 / Qt Widgets）；
//   2. 注入后端实现（真实后端实现 IRgisBackend 后在此调用
//      CBackendService::setRgisBackend(new ...) 完成对接；
//      当前未注入，使用内置占位实现）；
//   3. 打开宿主主窗口。
//
// 命令行参数：可将候选数据文件（分号分隔）作为参数传入，
//             例如：GravMagnFrontend.exe "C:\data\a.grd;C:\data\b.grd"

#include <QApplication>
#include <QStringList>

#include "MainWindow.h"
#include "core/BackendService.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("GravMagnFrontend"));
    app.setOrganizationName(QStringLiteral("RGIS"));

    // ===== 后端对接入口 =====
    // 接入正式后端时，取消下面注释并替换为真实后端实现：
    //   CBackendService::setRgisBackend(new CYourBackend());
    // 未注入时 CBackendService::rgisBackend() 返回内置占位实现（CNullRgisBackend）。
    // =========================

    // 候选数据文件（分号分隔，来自命令行参数）
    QString strFileNames = app.arguments().size() > 1 ? app.arguments().at(1) : QString();

    CGravMagnMainWindow mainWindow(strFileNames);
    mainWindow.show();

    return app.exec();
}

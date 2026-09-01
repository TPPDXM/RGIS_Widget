// GravMagnVolumeInvDlg.h : 重磁三维体反演处理流程类（Qt 前端）
//
// 对应原 MFC 工程：CMyGravMagnVolumeInvProc（GravMagnVolumeInvDlg.cpp）
// 原工程特点：无独立对话框界面，流程类配合"CGridFileNameDlg（网格文件名对话框，
// IDD_DLG_INVERSION_PARAMS，标题'三维重磁异常自动反演参数设置'）"完成
// 反演参数收集与反演计算；本前端工程按原工程移植方式实现：
//   - CGravMagnVolumeInvProc::onDoInvsProcess()：设置默认参数并打开 CGridFileNameDlg；
//   - CGravMagnVolumeInvProc::doInvsProcess(CGridFileNameDlg*)：读取对话框收集的参数，
//     调用后端完成反演（后端接口待后端提供，当前为占位提示）。
//
// 说明：对话框界面（网格文件名对话框）见 GridFileNameDlg.h/.cpp。

#pragma once

#include <QString>

class QWidget;
class CGridFileNameDlg;

// 重磁三维体反演处理流程类（前端侧；算法/数据读写在后端）
class CGravMagnVolumeInvProc
{
public:
    // 构造函数：初始化默认反演参数
    CGravMagnVolumeInvProc();

    // 析构函数
    ~CGravMagnVolumeInvProc();

    // ===== 流程入口 =====
    // 功能：设置默认参数并打开"三维重磁异常自动反演参数设置"对话框（模态）
    //       （对应原工程 CMyGravMagnVolumeInvProc::OnDoInvsProcess）
    // 参数：strFileNames —— 候选数据文件列表（分号分隔，可为空）
    // 参数：pParent —— 父窗口（可为 NULL）
    void onDoInvsProcess(const QString& strFileNames, QWidget* pParent);

    // ===== 对话框回调 =====
    // 功能：读取对话框收集的全部参数并调用后端完成重磁三维体反演
    //       （对应原工程 CMyGravMagnVolumeInvProc::DoInvsProcess）
    // 参数：pDlg —— 收集参数的网格文件名对话框（不允许为 NULL）
    void doInvsProcess(CGridFileNameDlg* pDlg);

private:
    // ===== 默认反演参数（对应原工程 OnDoInvsProcess 中的默认值）=====
    bool    mGravityData;       // 是否为重力数据（默认 true）
    bool    mKm;                // 长度单位是否为公里（默认 false）
    double  mInclination;       // 磁化倾角（度，默认 90）
    double  mDeclination;       // 磁化偏角（度，默认 0）
    double  mBaselineDec;       // 基线方位角（度，默认 0）
    double  mProfileDec;        // 测线方位角（度，默认 90）
    bool    mPrevModelsAsInit;  // 是否有先前模型物性分布作为初始模型（默认 false）
};

// GravMagnVolumeInvDlg.cpp : 重磁三维体反演处理流程类实现
//
// 说明：
//   * 对应原 MFC 工程 CMyGravMagnVolumeInvProc（无独立界面，配合网格文件名对话框工作）；
//   * 后端接口（processGravMagnVolumeInv）由后端提供后接入，当前"开始反演"为占位提示；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）。

#include "GravMagnVolumeInvDlg.h"

#include <QMessageBox>

#include "GridFileNameDlg.h"

// 构造函数：初始化默认反演参数（与原工程 OnDoInvsProcess 中默认值一致）
CGravMagnVolumeInvProc::CGravMagnVolumeInvProc()
    : mGravityData(true)
    , mKm(false)
    , mInclination(90.0)
    , mDeclination(0.0)
    , mBaselineDec(0.0)
    , mProfileDec(90.0)
    , mPrevModelsAsInit(false)
{
}

// 析构函数
CGravMagnVolumeInvProc::~CGravMagnVolumeInvProc()
{
}

// 功能：设置默认参数并打开"三维重磁异常自动反演参数设置"对话框（模态）
// 参数：strFileNames —— 候选数据文件列表（分号分隔，可为空）
// 参数：pParent —— 父窗口（可为 NULL）
void CGravMagnVolumeInvProc::onDoInvsProcess(const QString& strFileNames, QWidget* pParent)
{
    // 创建网格文件名对话框（对应原工程 CGridFileNameDlg），设置默认参数后模态打开
    CGridFileNameDlg dlg(strFileNames, pParent);
    dlg.setGravMagnVolumeInvProc(this);
    dlg.setGravityData(mGravityData);           // 是否重力数据
    dlg.setKm(mKm);                             // 长度单位是否为公里
    dlg.setInclination(mInclination);           // 磁化倾角
    dlg.setDeclination(mDeclination);           // 磁化偏角
    dlg.setProfileDec(mProfileDec);             // 测线方位角
    dlg.setBaselineDec(mBaselineDec);           // 基线方位角
    dlg.setPrevModelsAsInit(mPrevModelsAsInit); // 是否有先前模型作为初始模型
    dlg.exec();
}

// 功能：读取对话框收集的全部参数并调用后端完成重磁三维体反演
// 参数：pDlg —— 收集参数的网格文件名对话框（不允许为 NULL）
void CGravMagnVolumeInvProc::doInvsProcess(CGridFileNameDlg* pDlg)
{
    if (pDlg == NULL)
    {
        return;
    }

    // 从对话框取回全部参数（对应原工程 DoInvsProcess 开头的 pdlg->m_xxx 取值）
    mGravityData = pDlg->gravityData();
    mKm = pDlg->km();
    mDeclination = pDlg->declination();
    mInclination = pDlg->inclination();
    mBaselineDec = pDlg->baselineDec();
    mProfileDec = pDlg->profileDec();

    // TODO(后端对接)：后端接口 processGravMagnVolumeInv 提供后在此接入：
    //   组装 GravMagnVolumeInvParams（场值数据文件/地形高程文件/控制参数文件/
    //   正演场值文件/模型数据文件/反演信息文件/初始模型文件、
    //   模型范围（X/Y/Z 最小、最大与网格距）、重力数据标志、公里标志、
    //   倾角/偏角/测线方位角/基线方位角、初始模型标志等），调用后端完成反演，
    //   完成后提示"反演结束!"（与原工程一致）。
    // 当前：前端界面已完成，后端未接入，给出占位提示。
    QMessageBox::warning(pDlg, QStringLiteral("处理失败"),
        QStringLiteral("算法后端尚未接入：重磁三维体反演功能等待后端实现。"));
}

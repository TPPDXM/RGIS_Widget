// MainWindow.cpp : 前端宿主主窗口实现
//
// 按钮/分组布局对照原 MFC 工程 TESTGravMagnDataProcessing.rc 中
// IDD_TESTGravMagnDataProcessing（见 example_img/MainWindow.png）：
//   分组 1：重磁数据预处理
//   分组 2：频率域重磁数据处理
//   分组 3：空间域重磁数据处理
//   分组 4：重磁数据预处理
//   分组 5：重磁数据预处理
// 每组 8 列按钮，退出按钮位于最后一组右下角。

#include "MainWindow.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "dlg/FreqDomainCmpsFilterDlg.h"
#include "dlg/FreqDomainDownwardDlg.h"
#include "dlg/FreqDomainGradwardDlg.h"
#include "dlg/FreqDomainHorzGradDlg.h"
#include "dlg/FreqDomainIterDrapeDlg.h"
#include "dlg/FreqDomainIterwardDlg.h"
#include "dlg/FreqDomainMaxiGradDlg.h"
#include "dlg/FreqDomainMultiCpnDlg.h"
#include "dlg/FreqDomainNormFilterDlg.h"
#include "dlg/FreqDomainTwoDerivDlg.h"
#include "dlg/FreqDomainUpwardDlg.h"
#include "dlg/GravGradCoImagingDlg.h"
#include "dlg/GravMagnVolumeInvDlg.h"
#include "dlg/FreqDomainOneDerivDlg.h"
#include "dlg/FreqDomainPsudoGraDlg.h"
#include "dlg/FreqDomainReToPoleDlg.h"
#include "dlg/FreqDomainStructureDlg.h"
#include "dlg/FreqDomainThreeCpnDlg.h"
#include "dlg/FreqDomainTotlGradDlg.h"
#include "dlg/GravMidTerrainCorrectionDlg.h"
#include "dlg/GravUnionTerrainCorrectionDlg.h"
#include "dlg/MagnGradCoImagingDlg.h"
#include "dlg/MagnIntensityCalculationDlg.h"
#include "dlg/GridDataRecoveryDlg.h"

// 功能按钮的固定尺寸（与原工程 71*20 对话框单位按钮比例协调，
// 宽度取值保证“归一化标准差垂向导数”等长名称完整显示）
static const int sFunctionButtonWidth = 158;
static const int sFunctionButtonHeight = 28;

// 构造函数：保存候选文件列表并创建界面
// 参数：strFileNames —— 候选数据文件列表（分号分隔，可为空）
CGravMagnMainWindow::CGravMagnMainWindow(const QString& strFileNames)
    : QMainWindow(NULL)
    , mStrFileNames(strFileNames)
{
    initUi();
}

// 析构函数：无动态资源需要释放
CGravMagnMainWindow::~CGravMagnMainWindow()
{
}

// 功能：按参考布局创建全部功能按钮与分组
void CGravMagnMainWindow::initUi()
{
    setWindowTitle(QStringLiteral("重磁数据预处理和处理"));
    setMinimumSize(1330, 780);

    // 中央部件与纵向主布局
    QWidget* pCentralWidget = new QWidget(this);
    QVBoxLayout* pMainLayout = new QVBoxLayout(pCentralWidget);
    pMainLayout->setContentsMargins(8, 8, 8, 8);
    pMainLayout->setSpacing(6);
    setCentralWidget(pCentralWidget);

    // ================= 分组 1：重磁数据预处理 =================
    QGridLayout* pLayout1 = createGroupLayout(pMainLayout, QStringLiteral("重磁数据预处理"));
    addFunctionButton(pLayout1, 0, 0, QStringLiteral("网格数据差分扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 1, QStringLiteral("网格数据差分补空"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 2, QStringLiteral("空区还原"), FunctionGridDataRecovery);   // 已实现：网格数据空白区还原
    addFunctionButton(pLayout1, 0, 3, QStringLiteral("五万中区地形改正"), FunctionGravMidTerrain);   // 已实现：重力中区地形改正
    addFunctionButton(pLayout1, 0, 4, QStringLiteral("磁化强度计算"), FunctionMagnIntensity);   // 已实现：磁化强度计算
    addFunctionButton(pLayout1, 0, 5, QStringLiteral("单点地磁要素计算"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 6, QStringLiteral("多点地磁要素计算"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 7, QStringLiteral("海陆连片重力地形改正"), FunctionGravUnionTerrain); // 已实现：重力联合(平面带)地形改正
    addFunctionButton(pLayout1, 1, 0, QStringLiteral("观测系统误差计算"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 1, QStringLiteral("延时改正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 2, QStringLiteral("一致性试验"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 3, QStringLiteral("噪声试验"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 4, QStringLiteral("不含基点正常场改正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 5, QStringLiteral("含基点正常场改正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 6, QStringLiteral("日变校正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 7, QStringLiteral("三维重磁自动反演"), FunctionGravMagnVolumeInv); // 已实现：重磁三维体反演(网格文件名对话框)

    // ================= 分组 2：频率域重磁数据处理 =================
    QGridLayout* pLayout2 = createGroupLayout(pMainLayout, QStringLiteral("频率域重磁数据处理"));
    addFunctionButton(pLayout2, 0, 0, QStringLiteral("向上延拓"), FunctionUpward);     // 已实现：频率域向上延拓
    addFunctionButton(pLayout2, 0, 1, QStringLiteral("向下延拓"), FunctionDownward);        // 已实现：频率域向下延拓
    addFunctionButton(pLayout2, 0, 2, QStringLiteral("逐次向下延拓"), FunctionGradward);    // 已实现：频率域逐次向下延拓(正则化滤波)
    addFunctionButton(pLayout2, 0, 3, QStringLiteral("迭代向下延拓"), FunctionIterward);    // 已实现：频率域迭代向下延拓
    addFunctionButton(pLayout2, 0, 4, QStringLiteral("迭代曲化平"), FunctionIterDrape);     // 已实现：频率域迭代曲化平
    addFunctionButton(pLayout2, 0, 5, QStringLiteral("一阶导数"), FunctionOneDeriv);    // 已实现：频率域一阶导数
    addFunctionButton(pLayout2, 0, 6, QStringLiteral("变磁倾角化极"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 0, 7, QStringLiteral("二阶导数"), FunctionTwoDeriv);  // 已实现：频率域二阶导数
    addFunctionButton(pLayout2, 1, 0, QStringLiteral("总水平方向导数"), FunctionHorzGrad);   // 已实现：频率域总水平方向导数
    addFunctionButton(pLayout2, 1, 1, QStringLiteral("解析信号"), FunctionTotlGrad);   // 已实现：频率域总梯度(解析信号)
    addFunctionButton(pLayout2, 1, 2, QStringLiteral("三分量转换"), FunctionThreeCpn); // 已实现：频率域三分量转换
    addFunctionButton(pLayout2, 1, 3, QStringLiteral("任意方向分量转换"), FunctionMultiCpn); // 已实现：任意磁化方向分量转换
    addFunctionButton(pLayout2, 1, 4, QStringLiteral("正则化滤波"), FunctionNormFilter);     // 已实现：频率域正则化滤波
    addFunctionButton(pLayout2, 1, 5, QStringLiteral("Dz 化极"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 1, 6, QStringLiteral("磁源重力异常"), FunctionPsudoGra);  // 已实现：频率域伪重力(磁源重力异常)
    addFunctionButton(pLayout2, 1, 7, QStringLiteral("线性构造增强"), FunctionStructure); // 已实现：频率域构造(小子域滤波/线性构造增强)
    addFunctionButton(pLayout2, 2, 0, QStringLiteral("补偿圆滑滤波"), FunctionCmpsFilter);   // 已实现：频率域组合滤波
    addFunctionButton(pLayout2, 2, 1, QStringLiteral("化极"), FunctionReToPole);      // 已实现：频率域剩余化极(化极)
    addFunctionButton(pLayout2, 2, 2, QStringLiteral("低磁纬度化极"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 3, QStringLiteral("分带变磁倾角化极"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 4, QStringLiteral("三维重力相关成像"), FunctionGravGradCoImaging); // 已实现：三维重力异常和梯度相关成像
    addFunctionButton(pLayout2, 2, 5, QStringLiteral("三维磁力相关成像"), FunctionMagnGradCoImaging); // 已实现：三维磁异常和梯度相关成像
    addFunctionButton(pLayout2, 2, 6, QStringLiteral("三维密度界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 7, QStringLiteral("三维磁性界面反演"), FunctionNotImplemented);

    // ================= 分组 3：空间域重磁数据处理 =================
    QGridLayout* pLayout3 = createGroupLayout(pMainLayout, QStringLiteral("空间域重磁数据处理"));
    addFunctionButton(pLayout3, 0, 0, QStringLiteral("向上延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 1, QStringLiteral("向下延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 2, QStringLiteral("曲化平"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 3, QStringLiteral("水平一阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 4, QStringLiteral("水平二阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 5, QStringLiteral("垂向一阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 6, QStringLiteral("线性回归分析"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 7, QStringLiteral("滑动平均滤波"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 1, 0, QStringLiteral("垂向二阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 1, 1, QStringLiteral("趋势分析"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 1, 2, QStringLiteral("相关分析"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 1, 3, QStringLiteral("回归分析"), FunctionNotImplemented);

    // ================= 分组 4：重磁数据预处理 =================
    QGridLayout* pLayout4 = createGroupLayout(pMainLayout, QStringLiteral("重磁数据预处理"));
    addFunctionButton(pLayout4, 0, 0, QStringLiteral("二维密度界面正演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 1, QStringLiteral("二维密度界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 2, QStringLiteral("二维磁性界面正演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 3, QStringLiteral("二维磁性界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 4, QStringLiteral("三维密度界面正演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 5, QStringLiteral("三维密度界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 6, QStringLiteral("三维磁性界面正演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 7, QStringLiteral("三维磁性界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 0, QStringLiteral("倾斜角"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 1, QStringLiteral("倾斜角总水平方向导数"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 2, QStringLiteral("最大水平方向导数"), FunctionMaxiGrad);  // 已实现：频率域最大水平方向导数
    addFunctionButton(pLayout4, 1, 3, QStringLiteral("Theta 图"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 4, QStringLiteral("归一化标准差"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 5, QStringLiteral("归一化总水平导数垂向导数"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 6, QStringLiteral("归一化标准差垂向导数"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 7, QStringLiteral("归一化 Theta 图垂向导数"), FunctionNotImplemented);

    // ================= 分组 5：重磁数据预处理（含退出按钮）=================
    QGridLayout* pLayout5 = createGroupLayout(pMainLayout, QStringLiteral("重磁数据预处理"));
    addFunctionButton(pLayout5, 0, 0, QStringLiteral("剖面数据插值"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 1, QStringLiteral("剖面数据余弦函数扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 2, QStringLiteral("剖面数据曲化平"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 3, QStringLiteral("剖面数据最小曲率扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 4, QStringLiteral("剖面数据最小曲率补空"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 5, QStringLiteral("剖面数据圆滑"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 6, QStringLiteral("剖面数据空间域向上延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 7, QStringLiteral("剖面数据空间域向下延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 1, 0, QStringLiteral("网格数据最小曲率扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 1, 1, QStringLiteral("网格数据最小曲率补空"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 1, 2, QStringLiteral("网格数据差分扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 1, 3, QStringLiteral("网格数据差分补空"), FunctionNotImplemented);

    // 退出按钮（原工程 IDOK，位于最后一组右下角）
    QPushButton* pBtnExit = new QPushButton(QStringLiteral("退出"), this);
    pBtnExit->setFixedSize(sFunctionButtonWidth, sFunctionButtonHeight);
    pBtnExit->setDefault(true);
    connect(pBtnExit, &QPushButton::clicked, this, &CGravMagnMainWindow::onExitClicked);
    pLayout5->addWidget(pBtnExit, 1, 7);
}

// 功能：创建一个功能分组（标题居中，返回按钮网格布局）
// 参数：pMainLayout —— 主纵向布局（分组加入其中）
// 参数：strTitle —— 分组标题（原工程 GROUPBOX 的 BS_CENTER 居中标题）
QGridLayout* CGravMagnMainWindow::createGroupLayout(QVBoxLayout* pMainLayout, const QString& strTitle)
{
    QGroupBox* pGroup = new QGroupBox(strTitle, this);
    pGroup->setAlignment(Qt::AlignCenter);

    QGridLayout* pLayout = new QGridLayout(pGroup);
    pLayout->setContentsMargins(10, 8, 10, 8);
    pLayout->setHorizontalSpacing(6);
    pLayout->setVerticalSpacing(6);

    pMainLayout->addWidget(pGroup);
    return pLayout;
}

// 功能：在分组网格的指定位置添加功能按钮并连接回调
// 参数：pLayout —— 分组按钮网格布局
// 参数：nRow/nCol —— 按钮所在行列（0 起）
// 参数：strText —— 按钮文本（与原工程按钮文本一致）
// 参数：eType —— 功能类型（已实现的连接真实对话框，其余连接“未实现”提示）
void CGravMagnMainWindow::addFunctionButton(QGridLayout* pLayout, int nRow, int nCol,
                                            const QString& strText, EFunctionType eType)
{
    if (pLayout == NULL)
    {
        return;
    }

    QPushButton* pButton = new QPushButton(strText, this);
    pButton->setFixedSize(sFunctionButtonWidth, sFunctionButtonHeight);

    if (eType == FunctionCmpsFilter)
    {
        // 已实现：打开“补偿圆滑滤波”（频率域组合滤波）对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenCmpsFilterClicked);
    }
    else if (eType == FunctionDownward)
    {
        // 已实现：打开“频率域向下延拓”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenDownwardClicked);
    }
    else if (eType == FunctionGradward)
    {
        // 已实现：打开“频率域逐次向下延拓(正则化滤波)”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenGradwardClicked);
    }
    else if (eType == FunctionHorzGrad)
    {
        // 已实现：打开“频率域总水平方向导数”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenHorzGradClicked);
    }
    else if (eType == FunctionIterDrape)
    {
        // 已实现：打开“频率域迭代曲化平”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenIterDrapeClicked);
    }
    else if (eType == FunctionIterward)
    {
        // 已实现：打开“频率域迭代向下延拓”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenIterwardClicked);
    }
    else if (eType == FunctionMaxiGrad)
    {
        // 已实现：打开“频率域最大水平方向导数”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenMaxiGradClicked);
    }
    else if (eType == FunctionMultiCpn)
    {
        // 已实现：打开“频率域任意方向分量转换(多分量)”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenMultiCpnClicked);
    }
    else if (eType == FunctionNormFilter)
    {
        // 已实现：打开“频率域正则化滤波”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenNormFilterClicked);
    }
    else if (eType == FunctionOneDeriv)
    {
        // 已实现：打开“频率域一阶导数”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenOneDerivClicked);
    }
    else if (eType == FunctionPsudoGra)
    {
        // 已实现：打开“频率域伪重力(磁源重力异常)”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenPsudoGraClicked);
    }
    else if (eType == FunctionReToPole)
    {
        // 已实现：打开“频率域剩余化极(化极)”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenReToPoleClicked);
    }
    else if (eType == FunctionStructure)
    {
        // 已实现：打开“频率域构造(小子域滤波/线性构造增强)”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenStructureClicked);
    }
    else if (eType == FunctionThreeCpn)
    {
        // 已实现：打开“频率域三分量转换”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenThreeCpnClicked);
    }
    else if (eType == FunctionTotlGrad)
    {
        // 已实现：打开“频率域总梯度(解析信号)”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenTotlGradClicked);
    }
    else if (eType == FunctionTwoDeriv)
    {
        // 已实现：打开“频率域二阶导数”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenTwoDerivClicked);
    }
    else if (eType == FunctionUpward)
    {
        // 已实现：打开“频率域向上延拓”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenUpwardClicked);
    }
    else if (eType == FunctionGravGradCoImaging)
    {
        // 已实现：打开“三维重力异常和梯度相关成像”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenGravGradCoImagingClicked);
    }
    else if (eType == FunctionGravMagnVolumeInv)
    {
        // 已实现：打开“重磁三维体反演(网格文件名对话框)”流程
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenGravMagnVolumeInvClicked);
    }
    else if (eType == FunctionGravMidTerrain)
    {
        // 已实现：打开“重力中区地形改正”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenGravMidTerrainClicked);
    }
    else if (eType == FunctionGravUnionTerrain)
    {
        // 已实现：打开“重力联合(平面带)地形改正”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenGravUnionTerrainClicked);
    }
    else if (eType == FunctionMagnGradCoImaging)
    {
        // 已实现：打开“三维磁异常和梯度相关成像”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenMagnGradCoImagingClicked);
    }
    else if (eType == FunctionMagnIntensity)
    {
        // 已实现：打开“磁化强度计算”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenMagnIntensityClicked);
    }
    else if (eType == FunctionGridDataRecovery)
    {
        // 已实现：打开“网格数据空白区还原”对话框
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onOpenGridDataRecoveryClicked);
    }
    else
    {
        // 未实现：统一提示“该功能前端尚未开发”
        connect(pButton, &QPushButton::clicked, this, &CGravMagnMainWindow::onNotImplementedClicked);
    }

    pLayout->addWidget(pButton, nRow, nCol);
}

// 功能：打开补偿圆滑滤波对话框（模态）
void CGravMagnMainWindow::openCmpsFilterDlg()
{
    CFreqDomainCmpsFilterDlg dlg(mStrFileNames, this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainCmpsFilterDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域向下延拓对话框（模态）
void CGravMagnMainWindow::openDownwardDlg()
{
    CFreqDomainDownwardDlg dlg(mStrFileNames, this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainDownwardDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域逐次向下延拓(正则化滤波)对话框（模态）
void CGravMagnMainWindow::openGradwardDlg()
{
    CFreqDomainGradwardDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainGradwardDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域总水平方向导数对话框（模态）
void CGravMagnMainWindow::openHorzGradDlg()
{
    CFreqDomainHorzGradDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainHorzGradDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域迭代曲化平对话框（模态）
void CGravMagnMainWindow::openIterDrapeDlg()
{
    CFreqDomainIterDrapeDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainIterDrapeDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域迭代向下延拓对话框（模态）
void CGravMagnMainWindow::openIterwardDlg()
{
    CFreqDomainIterwardDlg dlg(mStrFileNames, this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainIterwardDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域最大水平方向导数对话框（模态）
void CGravMagnMainWindow::openMaxiGradDlg()
{
    CFreqDomainMaxiGradDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainMaxiGradDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域任意方向分量转换(多分量)对话框（模态）
void CGravMagnMainWindow::openMultiCpnDlg()
{
    CFreqDomainMultiCpnDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainMultiCpnDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域正则化滤波对话框（模态）
void CGravMagnMainWindow::openNormFilterDlg()
{
    CFreqDomainNormFilterDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainNormFilterDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域一阶导数对话框（模态）
void CGravMagnMainWindow::openOneDerivDlg()
{
    CFreqDomainOneDerivDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainOneDerivDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域伪重力(磁源重力异常)对话框（模态）
void CGravMagnMainWindow::openPsudoGraDlg()
{
    CFreqDomainPsudoGraDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainPsudoGraDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域剩余化极(化极)对话框（模态）
void CGravMagnMainWindow::openReToPoleDlg()
{
    CFreqDomainReToPoleDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainReToPoleDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域构造(小子域滤波/线性构造增强)对话框（模态）
void CGravMagnMainWindow::openStructureDlg()
{
    CFreqDomainStructureDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainStructureDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域三分量转换对话框（模态）
void CGravMagnMainWindow::openThreeCpnDlg()
{
    CFreqDomainThreeCpnDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainThreeCpnDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域总梯度(解析信号)对话框（模态）
void CGravMagnMainWindow::openTotlGradDlg()
{
    CFreqDomainTotlGradDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainTotlGradDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域二阶导数对话框（模态）
void CGravMagnMainWindow::openTwoDerivDlg()
{
    CFreqDomainTwoDerivDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainTwoDerivDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开频率域向上延拓对话框（模态）
void CGravMagnMainWindow::openUpwardDlg()
{
    CFreqDomainUpwardDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CFreqDomainUpwardDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开三维重力异常和梯度相关成像对话框（模态）
void CGravMagnMainWindow::openGravGradCoImagingDlg()
{
    CGravGradCoImagingDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线/体数据显示由前端后续版本提供）
    connect(&dlg, &CGravGradCoImagingDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    connect(&dlg, &CGravGradCoImagingDlg::viewVolumeFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开重磁三维体反演参数设置对话框（模态，网格文件名对话框）
void CGravMagnMainWindow::openGravMagnVolumeInvDlg()
{
    // 创建处理流程类并打开参数设置对话框（对应原工程 CMyGravMagnVolumeInvProc::OnDoInvsProcess）
    CGravMagnVolumeInvProc proc;
    proc.onDoInvsProcess(mStrFileNames, this);
}

// 功能：打开重力中区地形改正对话框（模态）
void CGravMagnMainWindow::openGravMidTerrainDlg()
{
    CGravMidTerrainCorrectionDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CGravMidTerrainCorrectionDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开重力联合(平面带)地形改正对话框（模态）
void CGravMagnMainWindow::openGravUnionTerrainDlg()
{
    CGravUnionTerrainCorrectionDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CGravUnionTerrainCorrectionDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开三维磁异常和梯度相关成像对话框（模态）
void CGravMagnMainWindow::openMagnGradCoImagingDlg()
{
    CMagnGradCoImagingDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线/体数据由前端后续版本提供）
    connect(&dlg, &CMagnGradCoImagingDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    connect(&dlg, &CMagnGradCoImagingDlg::viewVolumeFileRequested,
        this, &CGravMagnMainWindow::onViewVolumeFileRequested);
    dlg.exec();
}

// 功能：打开磁化强度计算对话框（模态）
void CGravMagnMainWindow::openMagnIntensityDlg()
{
    CMagnIntensityCalculationDlg dlg(this);
    dlg.exec();
}

// 功能：打开网格数据空白区还原对话框（模态）
void CGravMagnMainWindow::openGridDataRecoveryDlg()
{
    CGridDataRecoveryDlg dlg(this);
    // 连接对话框的“显示”请求信号（等值线显示由前端后续版本提供）
    connect(&dlg, &CGridDataRecoveryDlg::viewGridFileRequested,
        this, &CGravMagnMainWindow::onViewGridFileRequested);
    dlg.exec();
}

// 功能：打开“补偿圆滑滤波”（频率域组合滤波）对话框
void CGravMagnMainWindow::onOpenCmpsFilterClicked()
{
    openCmpsFilterDlg();
}

// 功能：打开“频率域向下延拓”对话框
void CGravMagnMainWindow::onOpenDownwardClicked()
{
    openDownwardDlg();
}

// 功能：打开“频率域逐次向下延拓(正则化滤波)”对话框
void CGravMagnMainWindow::onOpenGradwardClicked()
{
    openGradwardDlg();
}

// 功能：打开“频率域总水平方向导数”对话框
void CGravMagnMainWindow::onOpenHorzGradClicked()
{
    openHorzGradDlg();
}

// 功能：打开“频率域迭代曲化平”对话框
void CGravMagnMainWindow::onOpenIterDrapeClicked()
{
    openIterDrapeDlg();
}

// 功能：打开“频率域迭代向下延拓”对话框
void CGravMagnMainWindow::onOpenIterwardClicked()
{
    openIterwardDlg();
}

// 功能：打开“频率域最大水平方向导数”对话框
void CGravMagnMainWindow::onOpenMaxiGradClicked()
{
    openMaxiGradDlg();
}

// 功能：打开“频率域任意方向分量转换(多分量)”对话框
void CGravMagnMainWindow::onOpenMultiCpnClicked()
{
    openMultiCpnDlg();
}

// 功能：打开“频率域正则化滤波”对话框
void CGravMagnMainWindow::onOpenNormFilterClicked()
{
    openNormFilterDlg();
}

// 功能：打开“频率域一阶导数”对话框
void CGravMagnMainWindow::onOpenOneDerivClicked()
{
    openOneDerivDlg();
}

// 功能：打开“频率域伪重力(磁源重力异常)”对话框
void CGravMagnMainWindow::onOpenPsudoGraClicked()
{
    openPsudoGraDlg();
}

// 功能：打开“频率域剩余化极(化极)”对话框
void CGravMagnMainWindow::onOpenReToPoleClicked()
{
    openReToPoleDlg();
}

// 功能：打开“频率域构造(小子域滤波/线性构造增强)”对话框
void CGravMagnMainWindow::onOpenStructureClicked()
{
    openStructureDlg();
}

// 功能：打开“频率域三分量转换”对话框
void CGravMagnMainWindow::onOpenThreeCpnClicked()
{
    openThreeCpnDlg();
}

// 功能：打开“频率域总梯度(解析信号)”对话框
void CGravMagnMainWindow::onOpenTotlGradClicked()
{
    openTotlGradDlg();
}

// 功能：打开“频率域二阶导数”对话框
void CGravMagnMainWindow::onOpenTwoDerivClicked()
{
    openTwoDerivDlg();
}

// 功能：打开“频率域向上延拓”对话框
void CGravMagnMainWindow::onOpenUpwardClicked()
{
    openUpwardDlg();
}

// 功能：打开“三维重力异常和梯度相关成像”对话框
void CGravMagnMainWindow::onOpenGravGradCoImagingClicked()
{
    openGravGradCoImagingDlg();
}

// 功能：打开“重磁三维体反演(网格文件名对话框)”流程
void CGravMagnMainWindow::onOpenGravMagnVolumeInvClicked()
{
    openGravMagnVolumeInvDlg();
}

// 功能：打开“重力中区地形改正”对话框
void CGravMagnMainWindow::onOpenGravMidTerrainClicked()
{
    openGravMidTerrainDlg();
}

// 功能：打开“重力联合(平面带)地形改正”对话框
void CGravMagnMainWindow::onOpenGravUnionTerrainClicked()
{
    openGravUnionTerrainDlg();
}

// 功能：打开“三维磁异常和梯度相关成像”对话框
void CGravMagnMainWindow::onOpenMagnGradCoImagingClicked()
{
    openMagnGradCoImagingDlg();
}

// 功能：打开“磁化强度计算”对话框
void CGravMagnMainWindow::onOpenMagnIntensityClicked()
{
    openMagnIntensityDlg();
}

// 功能：打开“网格数据空白区还原”对话框
void CGravMagnMainWindow::onOpenGridDataRecoveryClicked()
{
    openGridDataRecoveryDlg();
}

// 功能：提示该功能前端尚未开发（未实现功能统一回调）
void CGravMagnMainWindow::onNotImplementedClicked()
{
    // 通过 sender() 取得被点击的按钮文本，向用户说明该功能尚未开发
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());
    if (pButton == NULL)
    {
        return;
    }
    QMessageBox::information(this, QStringLiteral("功能尚未开发"),
        QStringLiteral("“%1”功能前端尚未开发。\n\n已实现功能：补偿圆滑滤波、频率域向下延拓、\n"
            "频率域逐次向下延拓、频率域总水平方向导数、频率域迭代曲化平、\n"
            "频率域迭代向下延拓、频率域最大水平方向导数、\n"
            "频率域任意方向分量转换、频率域正则化滤波、\n"
            "频率域一阶导数、频率域伪重力（磁源重力异常）、\n"
            "频率域剩余化极（化极）、频率域构造（小子域滤波/线性构造增强）、\n"
            "频率域三分量转换、频率域总梯度（解析信号）、\n"
            "频率域二阶导数、频率域向上延拓、\n"
            "三维重力异常和梯度相关成像、重磁三维体反演、\n"
            "重力中区地形改正、重力联合（平面带）地形改正、\n"
            "三维磁异常和梯度相关成像、磁化强度计算、\n"
            "网格数据空白区还原。")
            .arg(pButton->text()));
}

// 功能：“退出”按钮——关闭主窗口退出程序
void CGravMagnMainWindow::onExitClicked()
{
    close();
}

// 功能：处理对话框的“显示”请求（等值线显示窗口将在前端后续版本实现）
void CGravMagnMainWindow::onViewGridFileRequested(const QString& strFilePath)
{
    QMessageBox::information(this, QStringLiteral("等值线显示"),
        QStringLiteral("等值线显示功能将在前端后续版本提供。\n文件：") + strFilePath);
}

// 功能：处理对话框的体数据“显示”请求（三维体数据视图将在前端后续版本实现）
void CGravMagnMainWindow::onViewVolumeFileRequested(const QString& strFilePath)
{
    QMessageBox::information(this, QStringLiteral("体数据视图"),
        QStringLiteral("三维体数据视图功能将在前端后续版本提供。\n文件：") + strFilePath);
}

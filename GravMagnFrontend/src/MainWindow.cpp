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
    setWindowTitle(QString::fromUtf8("重磁数据预处理和处理"));
    setMinimumSize(1330, 780);

    // 中央部件与纵向主布局
    QWidget* pCentralWidget = new QWidget(this);
    QVBoxLayout* pMainLayout = new QVBoxLayout(pCentralWidget);
    pMainLayout->setContentsMargins(8, 8, 8, 8);
    pMainLayout->setSpacing(6);
    setCentralWidget(pCentralWidget);

    // ================= 分组 1：重磁数据预处理 =================
    QGridLayout* pLayout1 = createGroupLayout(pMainLayout, QString::fromUtf8("重磁数据预处理"));
    addFunctionButton(pLayout1, 0, 0, QString::fromUtf8("网格数据差分扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 1, QString::fromUtf8("网格数据差分补空"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 2, QString::fromUtf8("空区还原"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 3, QString::fromUtf8("五万中区地形改正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 4, QString::fromUtf8("磁化强度计算"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 5, QString::fromUtf8("单点地磁要素计算"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 6, QString::fromUtf8("多点地磁要素计算"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 0, 7, QString::fromUtf8("海陆连片重力地形改正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 0, QString::fromUtf8("观测系统误差计算"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 1, QString::fromUtf8("延时改正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 2, QString::fromUtf8("一致性试验"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 3, QString::fromUtf8("噪声试验"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 4, QString::fromUtf8("不含基点正常场改正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 5, QString::fromUtf8("含基点正常场改正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 6, QString::fromUtf8("日变校正"), FunctionNotImplemented);
    addFunctionButton(pLayout1, 1, 7, QString::fromUtf8("三维重磁自动反演"), FunctionNotImplemented);

    // ================= 分组 2：频率域重磁数据处理 =================
    QGridLayout* pLayout2 = createGroupLayout(pMainLayout, QString::fromUtf8("频率域重磁数据处理"));
    addFunctionButton(pLayout2, 0, 0, QString::fromUtf8("向上延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 0, 1, QString::fromUtf8("向下延拓"), FunctionDownward);        // 已实现：频率域向下延拓
    addFunctionButton(pLayout2, 0, 2, QString::fromUtf8("逐次向下延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 0, 3, QString::fromUtf8("迭代向下延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 0, 4, QString::fromUtf8("迭代曲化平"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 0, 5, QString::fromUtf8("一阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 0, 6, QString::fromUtf8("变磁倾角化极"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 0, 7, QString::fromUtf8("二阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 1, 0, QString::fromUtf8("总水平方向导数"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 1, 1, QString::fromUtf8("解析信号"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 1, 2, QString::fromUtf8("三分量转换"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 1, 3, QString::fromUtf8("任意方向分量转换"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 1, 4, QString::fromUtf8("正则化滤波"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 1, 5, QString::fromUtf8("Dz 化极"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 1, 6, QString::fromUtf8("磁源重力异常"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 1, 7, QString::fromUtf8("线性构造增强"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 0, QString::fromUtf8("补偿圆滑滤波"), FunctionCmpsFilter);   // 已实现：频率域组合滤波
    addFunctionButton(pLayout2, 2, 1, QString::fromUtf8("化极"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 2, QString::fromUtf8("低磁纬度化极"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 3, QString::fromUtf8("分带变磁倾角化极"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 4, QString::fromUtf8("三维重力相关成像"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 5, QString::fromUtf8("三维磁力相关成像"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 6, QString::fromUtf8("三维密度界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout2, 2, 7, QString::fromUtf8("三维磁性界面反演"), FunctionNotImplemented);

    // ================= 分组 3：空间域重磁数据处理 =================
    QGridLayout* pLayout3 = createGroupLayout(pMainLayout, QString::fromUtf8("空间域重磁数据处理"));
    addFunctionButton(pLayout3, 0, 0, QString::fromUtf8("向上延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 1, QString::fromUtf8("向下延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 2, QString::fromUtf8("曲化平"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 3, QString::fromUtf8("水平一阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 4, QString::fromUtf8("水平二阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 5, QString::fromUtf8("垂向一阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 6, QString::fromUtf8("线性回归分析"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 0, 7, QString::fromUtf8("滑动平均滤波"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 1, 0, QString::fromUtf8("垂向二阶导数"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 1, 1, QString::fromUtf8("趋势分析"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 1, 2, QString::fromUtf8("相关分析"), FunctionNotImplemented);
    addFunctionButton(pLayout3, 1, 3, QString::fromUtf8("回归分析"), FunctionNotImplemented);

    // ================= 分组 4：重磁数据预处理 =================
    QGridLayout* pLayout4 = createGroupLayout(pMainLayout, QString::fromUtf8("重磁数据预处理"));
    addFunctionButton(pLayout4, 0, 0, QString::fromUtf8("二维密度界面正演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 1, QString::fromUtf8("二维密度界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 2, QString::fromUtf8("二维磁性界面正演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 3, QString::fromUtf8("二维磁性界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 4, QString::fromUtf8("三维密度界面正演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 5, QString::fromUtf8("三维密度界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 6, QString::fromUtf8("三维磁性界面正演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 0, 7, QString::fromUtf8("三维磁性界面反演"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 0, QString::fromUtf8("倾斜角"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 1, QString::fromUtf8("倾斜角总水平方向导数"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 2, QString::fromUtf8("最大水平方向导数"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 3, QString::fromUtf8("Theta 图"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 4, QString::fromUtf8("归一化标准差"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 5, QString::fromUtf8("归一化总水平导数垂向导数"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 6, QString::fromUtf8("归一化标准差垂向导数"), FunctionNotImplemented);
    addFunctionButton(pLayout4, 1, 7, QString::fromUtf8("归一化 Theta 图垂向导数"), FunctionNotImplemented);

    // ================= 分组 5：重磁数据预处理（含退出按钮）=================
    QGridLayout* pLayout5 = createGroupLayout(pMainLayout, QString::fromUtf8("重磁数据预处理"));
    addFunctionButton(pLayout5, 0, 0, QString::fromUtf8("剖面数据插值"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 1, QString::fromUtf8("剖面数据余弦函数扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 2, QString::fromUtf8("剖面数据曲化平"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 3, QString::fromUtf8("剖面数据最小曲率扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 4, QString::fromUtf8("剖面数据最小曲率补空"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 5, QString::fromUtf8("剖面数据圆滑"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 6, QString::fromUtf8("剖面数据空间域向上延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 0, 7, QString::fromUtf8("剖面数据空间域向下延拓"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 1, 0, QString::fromUtf8("网格数据最小曲率扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 1, 1, QString::fromUtf8("网格数据最小曲率补空"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 1, 2, QString::fromUtf8("网格数据差分扩边"), FunctionNotImplemented);
    addFunctionButton(pLayout5, 1, 3, QString::fromUtf8("网格数据差分补空"), FunctionNotImplemented);

    // 退出按钮（原工程 IDOK，位于最后一组右下角）
    QPushButton* pBtnExit = new QPushButton(QString::fromUtf8("退出"), this);
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

// 功能：提示该功能前端尚未开发（未实现功能统一回调）
void CGravMagnMainWindow::onNotImplementedClicked()
{
    // 通过 sender() 取得被点击的按钮文本，向用户说明该功能尚未开发
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());
    if (pButton == NULL)
    {
        return;
    }
    QMessageBox::information(this, QString::fromUtf8("功能尚未开发"),
        QString::fromUtf8("“%1”功能前端尚未开发。\n\n已实现功能：补偿圆滑滤波、频率域向下延拓。")
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
    QMessageBox::information(this, QString::fromUtf8("等值线显示"),
        QString::fromUtf8("等值线显示功能将在前端后续版本提供。\n文件：") + strFilePath);
}

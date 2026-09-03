// VolumeDataViewDlg.cpp : 三维体数据视图对话框实现
//
// 说明：
//   * 对照原 MFC 工程 CVolumeDataViewDlg（IDD_VolumeDataViewDlg）的菜单/动作/状态栏与可缩放窗口；
//   * 所有回调均为成员函数引用（connect 第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误通过返回值 + 消息框/状态栏反馈，做足空指针检查；
//   * 中文字符串一律 QStringLiteral；体数据读取经后端接口 readVolumeData。

#include "VolumeDataViewDlg.h"

#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

#include "FrontendUtils.h"
#include "VolumeDataGLView.h"

// 构造函数：保存文件名并创建界面
// 参数：strFileName —— 初始体数据文件路径（原工程构造参数 CString Str）
// 参数：pParent —— 父窗口（可为 NULL）
CVolumeDataViewDlg::CVolumeDataViewDlg(const QString& strFileName, QWidget* pParent)
    : QDialog(pParent)
    , mMenuBar(NULL)
    , mView(NULL)
    , mStatusBar(NULL)
    , mStatusLabel(NULL)
    , mFileName(strFileName)
    , mShowAxisOn(true)
{
    initUi();

    // 加载体数据文件（若提供了）
    if (!mFileName.isEmpty())
    {
        loadVolume(mFileName);
    }
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CVolumeDataViewDlg::~CVolumeDataViewDlg()
{
}

// 功能：创建菜单栏/中央视图/状态栏并连接信号槽
void CVolumeDataViewDlg::initUi()
{
    setWindowTitle(QStringLiteral("三维体数据视图"));
    resize(900, 640);
    setMinimumSize(640, 480);

    // ================= 中央三维体数据视图 =================
    QVBoxLayout* pMainLayout = new QVBoxLayout(this);
    pMainLayout->setContentsMargins(6, 6, 6, 6);
    mView = new CVolumeDataGLView(this);
    pMainLayout->addWidget(mView);

    // ================= 菜单栏 =================
    // QDialog 没有 QMainWindow 的 menuBar()，用 QVBoxLayout::setMenuBar 挂接
    mMenuBar = new QMenuBar(this);
    pMainLayout->setMenuBar(mMenuBar);

    // 文件
    QMenu* pFileMenu = mMenuBar->addMenu(QStringLiteral("文件"));
    pFileMenu->addAction(QStringLiteral("打开..."), this, &CVolumeDataViewDlg::onFileOpen);
    pFileMenu->addSeparator();
    pFileMenu->addAction(QStringLiteral("退出"), this, &CVolumeDataViewDlg::onExit);

    // 显示
    QMenu* pShowMenu = mMenuBar->addMenu(QStringLiteral("显示"));
    pShowMenu->addAction(QStringLiteral("线框模式"), this, &CVolumeDataViewDlg::onLineMode);
    pShowMenu->addAction(QStringLiteral("填充模式"), this, &CVolumeDataViewDlg::onFillMode);
    pShowMenu->addSeparator();
    pShowMenu->addAction(QStringLiteral("显示坐标轴"), this, &CVolumeDataViewDlg::onShowAxis);
    pShowMenu->addAction(QStringLiteral("光照"), this, &CVolumeDataViewDlg::onLighting);
    pShowMenu->addAction(QStringLiteral("切片/全部方向"), this, &CVolumeDataViewDlg::onViewSliceOrAll);
    pShowMenu->addSeparator();
    pShowMenu->addAction(QStringLiteral("模型旋转"), this, &CVolumeDataViewDlg::onModelRotate);
    pShowMenu->addAction(QStringLiteral("恢复视角"), this, &CVolumeDataViewDlg::onRestore);
    pShowMenu->addAction(QStringLiteral("视图参数..."), this, &CVolumeDataViewDlg::onViewParaSetting);

    // 切片
    QMenu* pSliceMenu = mMenuBar->addMenu(QStringLiteral("切片"));
    pSliceMenu->addAction(QStringLiteral("正交切片"), this, &CVolumeDataViewDlg::onSliceNormal);
    pSliceMenu->addAction(QStringLiteral("随机方向切片"), this, &CVolumeDataViewDlg::onSliceRandom);
    pSliceMenu->addAction(QStringLiteral("仅显示立方体"), this, &CVolumeDataViewDlg::onViewCube);

    // 显示面
    QMenu* pFaceMenu = mMenuBar->addMenu(QStringLiteral("显示面"));
    pFaceMenu->addAction(QStringLiteral("顶面"), this, &CVolumeDataViewDlg::onShowTop);
    pFaceMenu->addAction(QStringLiteral("底面"), this, &CVolumeDataViewDlg::onShowBottom);
    pFaceMenu->addAction(QStringLiteral("左面"), this, &CVolumeDataViewDlg::onShowLeft);
    pFaceMenu->addAction(QStringLiteral("右面"), this, &CVolumeDataViewDlg::onShowRight);
    pFaceMenu->addAction(QStringLiteral("前面"), this, &CVolumeDataViewDlg::onShowFront);
    pFaceMenu->addAction(QStringLiteral("后面"), this, &CVolumeDataViewDlg::onShowBack);

    // 缩放
    QMenu* pZoomMenu = mMenuBar->addMenu(QStringLiteral("缩放"));
    pZoomMenu->addAction(QStringLiteral("放大"), this, &CVolumeDataViewDlg::onZoomIn);
    pZoomMenu->addAction(QStringLiteral("缩小"), this, &CVolumeDataViewDlg::onZoomOut);
    pZoomMenu->addSeparator();
    pZoomMenu->addAction(QStringLiteral("Z 轴放大"), this, &CVolumeDataViewDlg::onUpZScale);
    pZoomMenu->addAction(QStringLiteral("Z 轴缩小"), this, &CVolumeDataViewDlg::onDownZScale);

    // 帮助
    QMenu* pHelpMenu = mMenuBar->addMenu(QStringLiteral("帮助"));
    pHelpMenu->addAction(QStringLiteral("关于"), this, &CVolumeDataViewDlg::onAbout);

    // ================= 状态栏 =================
    mStatusBar = new QStatusBar(this);
    mStatusLabel = new QLabel(this);
    mStatusBar->addWidget(mStatusLabel, 1);
    pMainLayout->addWidget(mStatusBar);

    mStatusLabel->setText(QStringLiteral("就绪（可通过“文件→打开...”加载三维体数据文件）"));
}

// 功能：调用后端读取体数据并刷新视图/标题/状态栏
// 参数：strFilePath —— 体数据文件路径
void CVolumeDataViewDlg::loadVolume(const QString& strFilePath)
{
    if (strFilePath.isEmpty())
    {
        return;
    }

    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        if (mStatusLabel != NULL)
        {
            mStatusLabel->setText(QStringLiteral("后端接口未初始化，无法加载体数据。"));
        }
        return;
    }

    VolumeData vol;
    BackendError error;
    if (!pBackend->readVolumeData(toBackendString(strFilePath), vol, error))
    {
        if (mStatusLabel != NULL)
        {
            mStatusLabel->setText(QStringLiteral("加载失败：") + fromBackendString(error.message));
        }
        return;
    }

    // 成功加载：刷新视图与标题
    mFileName = strFilePath;
    if (mView != NULL)
    {
        mView->setVolumeData(vol);
    }
    setWindowTitle(QStringLiteral("模型数据显示: ") + strFilePath);
    if (mStatusLabel != NULL)
    {
        mStatusLabel->setText(QStringLiteral("已加载体数据：")
            + QString::number(vol.layers) + QStringLiteral(" × ")
            + QString::number(vol.rows) + QStringLiteral(" × ")
            + QString::number(vol.cols));
    }
}

// 功能："打开"体数据文件
void CVolumeDataViewDlg::onFileOpen()
{
    QString strFilePath = QFileDialog::getOpenFileName(this,
        QStringLiteral("请选择三维体数据文件"), mFileName,
        QStringLiteral("三维体数据文件 (*.vol);;所有数据文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadVolume(strFilePath);
}

// 功能："退出"关闭对话框
void CVolumeDataViewDlg::onExit()
{
    reject();
}

// 功能："关于"提示
void CVolumeDataViewDlg::onAbout()
{
    QMessageBox::information(this, QStringLiteral("关于"),
        QStringLiteral("三维体数据视图（Qt 前端）\n\n"
            "对应原 MFC 工程“三维体数据视图”对话框。\n"
            "三维体数据由后端接口读取并渲染为三方向正交切片。"));
}

// 功能：线框模式
void CVolumeDataViewDlg::onLineMode()
{
    if (mView != NULL)
    {
        mView->setLineMode(true);
    }
}

// 功能：填充模式
void CVolumeDataViewDlg::onFillMode()
{
    if (mView != NULL)
    {
        mView->setFillMode(true);
    }
}

// 功能：显示/隐藏坐标轴
void CVolumeDataViewDlg::onShowAxis()
{
    mShowAxisOn = !mShowAxisOn;
    if (mView != NULL)
    {
        mView->setShowAxis(mShowAxisOn);
    }
}

// 功能：光照/透明开关
void CVolumeDataViewDlg::onLighting()
{
    if (mView != NULL)
    {
        mView->setLighting(!mView->lightingEnabled());
    }
}

// 功能：切片/全部方向切换
void CVolumeDataViewDlg::onViewSliceOrAll()
{
    if (mView != NULL)
    {
        mView->setViewSliceOrAll(!mView->sliceOrAllEnabled());
    }
}

// 功能：模型自动旋转
void CVolumeDataViewDlg::onModelRotate()
{
    if (mView != NULL)
    {
        mView->setModelRotate(!mView->rotateEnabled());
    }
}

// 功能：恢复默认视角
void CVolumeDataViewDlg::onRestore()
{
    if (mView != NULL)
    {
        mView->restoreView();
    }
}

// 功能：视图参数设置（简化：提示）
void CVolumeDataViewDlg::onViewParaSetting()
{
    QMessageBox::information(this, QStringLiteral("视图参数设置"),
        QStringLiteral("当前为简化切片示意模式；视图参数（切片数、透明度等）暂不开放。"));
}

// 功能：正交切片
void CVolumeDataViewDlg::onSliceNormal()
{
    if (mView != NULL)
    {
        mView->setSliceMode(0);
    }
}

// 功能：随机方向切片
void CVolumeDataViewDlg::onSliceRandom()
{
    if (mView != NULL)
    {
        mView->setSliceMode(1);
    }
}

// 功能：仅显示立方体
void CVolumeDataViewDlg::onViewCube()
{
    if (mView != NULL)
    {
        mView->setSliceMode(2);
    }
}

// 功能：显示/隐藏顶面（Z 方向切片组）
void CVolumeDataViewDlg::onShowTop()
{
    if (mView != NULL)
    {
        mView->setShowTop(!mView->topEnabled());
    }
}

// 功能：显示/隐藏底面（Z 方向切片组）
void CVolumeDataViewDlg::onShowBottom()
{
    if (mView != NULL)
    {
        mView->setShowBottom(!mView->bottomEnabled());
    }
}

// 功能：显示/隐藏左面（X 方向切片组）
void CVolumeDataViewDlg::onShowLeft()
{
    if (mView != NULL)
    {
        mView->setShowLeft(!mView->leftEnabled());
    }
}

// 功能：显示/隐藏右面（X 方向切片组）
void CVolumeDataViewDlg::onShowRight()
{
    if (mView != NULL)
    {
        mView->setShowRight(!mView->rightEnabled());
    }
}

// 功能：显示/隐藏前面（Y 方向切片组）
void CVolumeDataViewDlg::onShowFront()
{
    if (mView != NULL)
    {
        mView->setShowFront(!mView->frontEnabled());
    }
}

// 功能：显示/隐藏后面（Y 方向切片组）
void CVolumeDataViewDlg::onShowBack()
{
    if (mView != NULL)
    {
        mView->setShowBack(!mView->backEnabled());
    }
}

// 功能：放大
void CVolumeDataViewDlg::onZoomIn()
{
    if (mView != NULL)
    {
        mView->zoomIn();
    }
}

// 功能：缩小
void CVolumeDataViewDlg::onZoomOut()
{
    if (mView != NULL)
    {
        mView->zoomOut();
    }
}

// 功能：Z 轴放大
void CVolumeDataViewDlg::onUpZScale()
{
    if (mView != NULL)
    {
        mView->upZScale();
    }
}

// 功能：Z 轴缩小
void CVolumeDataViewDlg::onDownZScale()
{
    if (mView != NULL)
    {
        mView->downZScale();
    }
}

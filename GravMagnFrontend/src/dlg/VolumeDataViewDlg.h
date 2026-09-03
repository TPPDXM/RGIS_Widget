// VolumeDataViewDlg.h : 三维体数据视图对话框（Qt 前端）
//
// 对应原 MFC 工程：CVolumeDataViewDlg（IDD_VolumeDataViewDlg，内嵌 ActiveX 体渲染控件
//   VolumeDataViewCtl 的宿主）的 Qt 重写。原工程用菜单 + 工具栏 + 状态栏 + 大视图控件，
//   Qt 端改为"菜单栏 + 中央 OpenGL 体数据视图 + 状态栏"，交互（旋转/缩放/切片/面向/显隐等）
//   全部保留，3D 渲染用简化三方向切片示意（见 CVolumeDataGLView）。
//
// 前端职责（界面与交互，体数据读取在后端）：
//   1. 通过后端接口 readVolumeData 加载体数据文件（. 三维体数据二进制格式）；
//   2. 三个方向切片显示、线框/填充、六个面显隐、坐标轴、光照、切片/全部、旋转/恢复、
//      缩放/Z 轴缩放等操作；
//   3. 窗口可缩放（对应原工程 WS_THICKFRAME），打开其它体数据文件（"打开"）。

#pragma once

#include <QDialog>
#include <QString>

class CVolumeDataGLView;
class QMenuBar;
class QStatusBar;
class QLabel;

// 三维体数据视图对话框
class CVolumeDataViewDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：strFileName —— 初始体数据文件路径（原工程构造参数 CString Str）
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CVolumeDataViewDlg(const QString& strFileName, QWidget* pParent = nullptr);

    // 析构函数
    ~CVolumeDataViewDlg() override;

private slots:
    // ===== 文件回调 =====
    void onFileOpen();                      // 功能："打开"体数据文件
    void onExit();                          // 功能："退出"关闭对话框
    void onAbout();                         // 功能："关于"提示

    // ===== 显示模式回调 =====
    void onLineMode();                      // 功能：线框模式
    void onFillMode();                      // 功能：填充模式
    void onShowAxis();                      // 功能：显示/隐藏坐标轴
    void onLighting();                      // 功能：光照/透明开关
    void onViewSliceOrAll();                // 功能：切片/全部方向切换
    void onModelRotate();                   // 功能：模型自动旋转
    void onRestore();                       // 功能：恢复默认视角
    void onViewParaSetting();               // 功能：视图参数设置（简化：提示）

    // ===== 切片模式回调 =====
    void onSliceNormal();                   // 功能：正交切片（对应原 OnCutParaSet）
    void onSliceRandom();                   // 功能：随机方向切片（对应原 OnRandomSlicept）
    void onViewCube();                      // 功能：仅显示立方体（对应原 OnViewCube）

    // ===== 六个面显隐回调 =====
    void onShowTop();                       // 功能：显示/隐藏顶（Z 方向切片组）
    void onShowBottom();                    // 功能：显示/隐藏底（Z 方向切片组）
    void onShowLeft();                      // 功能：显示/隐藏左（X 方向切片组）
    void onShowRight();                     // 功能：显示/隐藏右（X 方向切片组）
    void onShowFront();                     // 功能：显示/隐藏前（Y 方向切片组）
    void onShowBack();                      // 功能：显示/隐藏后（Y 方向切片组）

    // ===== 缩放回调 =====
    void onZoomIn();                        // 功能：放大
    void onZoomOut();                       // 功能：缩小
    void onUpZScale();                      // 功能：Z 轴放大
    void onDownZScale();                    // 功能：Z 轴缩小

private:
    // ===== 界面初始化 =====
    void initUi();                          // 功能：创建菜单栏/中央视图/状态栏并连接信号槽

    // ===== 数据加载 =====
    void loadVolume(const QString& strFilePath);    // 功能：调用后端读取体数据并刷新视图/标题/状态栏

    // ===== 界面控件 =====
    QMenuBar*       mMenuBar;       // 菜单栏（文件/显示/切片/显示面/缩放/帮助）
    CVolumeDataGLView* mView;       // 中央三维体数据 OpenGL 视图
    QStatusBar*     mStatusBar;     // 状态栏（显示加载状态/提示/时间）
    QLabel*         mStatusLabel;   // 状态栏主信息标签

    // ===== 状态数据 =====
    QString mFileName;      // 当前体数据文件路径（原 m_strFileName）
    bool    mShowAxisOn;    // 坐标轴当前是否显示（用于菜单复选状态）
};

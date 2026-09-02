// DrawOpenGLContourDlg.h : OpenGL 等值线显示窗口（Qt 前端）
//
// 对应原 MFC 工程：CDrawOpenGLContourDlg（IDD_DrawOpenGLContour）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_DrawOpenGLContour 绘制：
//   顶部工具按钮行（打开/参数/显示/放大/缩小/移动/还原/取消）+ 中部 OpenGL 等值线绘图区 + 底部状态栏
//
// 前端职责（本窗口为纯源码可独立重写的等值线显示功能）：
//   1. 接收一个 .grd 网格文件路径（各处理对话框“显示”按钮经 viewGridFileRequested 传入）；
//   2. 读取网格数据，绘制等值线（Marching Squares 生成，OpenGL 渲染）；
//   3. 提供缩放/平移/还原/选择文件等交互（对应原工程菜单/工具条命令）。
//   说明：原工程等值线绘制封装在 ActiveX 控件（无源码），此处为前端从零实现的等价显示窗口。
//   工具栏为简洁起见以“按钮行”呈现（对应原 IDR_Contour 工具条），状态栏显示文件名与缩放信息。

#pragma once

#include <QDialog>

// 前置声明（减少头文件之间的相互包含）
class QPushButton;
class QLabel;
class CContourGLWidget;

// OpenGL 等值线显示窗口
class CDrawOpenGLContourDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：strFilePath —— 待显示的 .grd 网格文件路径（可为空，稍后经“打开”选择）
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CDrawOpenGLContourDlg(const QString& strFilePath, QWidget* pParent = nullptr);

    // 析构函数
    ~CDrawOpenGLContourDlg() override;

private slots:
    // ===== 工具按钮回调（对应原工程工具条/菜单命令）=====
    void onOpenFileClicked();       // 功能：选择一个新的 .grd 文件显示
    void onZoomInClicked();         // 功能：放大视图
    void onZoomOutClicked();        // 功能：缩小视图
    void onResetClicked();          // 功能：还原视图（全图）
    void onInfoTextChanged(const QString& strInfo);  // 功能：刷新状态栏信息文本

private:
    // ===== 界面初始化 =====
    void initUi();                          // 功能：按 .rc 布局创建窗口并连接信号槽

    // ===== 工具 =====
    // 功能：缩放指定倍数（放大/缩小共用一个实现）
    void applyZoom(double factor);
    // 功能：弹文件对话框选择 .grd 并加载
    QString askOpenFilePath(const QString& strTitle);

    // ===== 界面控件 =====
    CContourGLWidget* mWidget;      // OpenGL 等值线绘图区（对应原 IDC_CONTOURGLCTLCTRL）
    QLabel*     mInfoLabel;         // 底部状态栏信息（对应原状态栏）

    QString mFilePath;              // 当前显示的 .grd 文件路径（对应原 m_strFileName）
};

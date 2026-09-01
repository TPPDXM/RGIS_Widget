// GravMidTerrainCorrectionDlg.h : 重力中区地形改正对话框（Qt 前端）
//
// 对应原 MFC 工程：CGravMidTerrainCorrectionDlg（IDD_GravMidTerrainCorrection）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_GravMidTerrainCorrection 绘制：
//   重力测点数据文件输入 / 高程网格数据文件输入 / 高程网格数据文件信息 /
//   外接口形状选择 / 内接口形状选择 / 地形改正参数输入 / 地形改正结果数据文件输出
//
// 前端职责（本对话框只做界面与交互，算法/数据读写在后端）：
//   1. 测点数据文件(.txt)与地形高程网格数据文件(.grd)的选择、信息显示；
//   2. 外/内接口形状单选、起始/终止半径与地壳密度输入；
//   3. “确定”时校验输入，并调用后端接口 processMidTerrain 完成处理；
//   4. “显示”按钮请求查看地形网格文件等值线（viewGridFileRequested 交由宿主处理）。

#pragma once

#include <QDialog>

#include "backend/RgisBackend.h"

// 前置声明（减少头文件之间的相互包含）
class QLineEdit;
class QPushButton;
class QRadioButton;
class QDoubleSpinBox;
class QLabel;
class QButtonGroup;

// 重力中区地形改正对话框
class CGravMidTerrainCorrectionDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CGravMidTerrainCorrectionDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CGravMidTerrainCorrectionDlg() override;

signals:
    // 功能：请求宿主显示指定网格文件的等值线图（对应原 MFC 工程“显示”按钮
    //       弹出的 CDrawOpenGLContourDlg，等值线显示窗口由前端后续版本实现）
    void viewGridFileRequested(const QString& strFilePath);

private slots:
    // ===== 输入/输出文件回调 =====
    void onOpenTxtFileClicked();                // 功能：“...”选择重力测点数据文件
    void onOpenGrdFileClicked();                // 功能：“...”选择地形高程网格数据文件
    void onOpenGrdFileViewClicked();            // 功能：显示地形高程网格文件等值线
    void onOpenResFileClicked();                // 功能：“...”选择地形改正结果数据文件
    void onTxtFileTextEdited(const QString& strText);   // 功能：手动输入测点文件名（同步成员变量）
    void onGrdFileTextEdited(const QString& strText);   // 功能：手动输入高程网格文件名（同步成员变量）
    void onResFileTextEdited(const QString& strText);   // 功能：手动输入结果文件名（同步成员变量）

    // ===== 接口形状单选回调 =====
    void onOuterShapeClicked(int nShapeId);     // 功能：外接口形状切换（0=方形 1=圆形）
    void onInnerShapeClicked(int nShapeId);     // 功能：内接口形状切换（0=方形 1=圆形）

    // ===== 处理参数回调 =====
    void onStartRadiusChanged(double dValue);   // 功能：起始半径变化（同步成员变量）
    void onEndRadiusChanged(double dValue);     // 功能：终止半径变化（同步成员变量）
    void onRockDensityChanged(double dValue);   // 功能：地壳密度变化（同步成员变量）

    // ===== 确定 / 取消 =====
    void onOkClicked();                         // 功能：“确定”——校验参数并调用后端处理
    void onCancelClicked();                     // 功能：“取消”——关闭对话框

private:
    // ===== 界面初始化 =====
    void initUi();                              // 功能：按 .rc 布局创建全部控件并连接信号槽

    // ===== 数据加载 =====
    void loadGrdFile(const QString& strFilePath);   // 功能：读取高程网格文件头并刷新网格信息
    void loadTxtFile(const QString& strFilePath);   // 功能：统计测点数据行数并刷新“重力测点数”
    void updateGridInfoDisplay();                   // 功能：把网格信息成员变量刷到界面显示

    // ===== 文件选择 / 显示 =====
    QString askOpenFilePath(const QString& strTitle, const QString& strFilter);  // 功能：弹出打开文件对话框
    void viewGridFile(const QString& strFilePath);  // 功能：检查文件存在并发出发送显示请求

    // ===== 处理 =====
    bool validateInputs(QString& strError);     // 功能：确定前校验输入项，返回 false 表示校验失败
    void runProcess();                          // 功能：组装参数并调用后端 processMidTerrain

    // ===== 界面控件（重力测点数据文件输入）=====
    QLineEdit*      mEditTxtFile;       // 重力测点数据文件路径（对应原 IDC_FileNameTxt）
    QPushButton*    mBtnOpenTxtFile;    // 测点文件“...”按钮

    // ===== 界面控件（高程网格数据文件输入）=====
    QLineEdit*      mEditGrdFile;       // 地形高程网格数据文件路径（对应原 IDC_FileNameGrd）
    QPushButton*    mBtnOpenGrdFile;    // 高程网格文件“...”按钮
    QPushButton*    mBtnOpenGrdFileView;// 高程网格文件“显示”按钮

    // ===== 界面控件（高程网格数据文件信息，只读显示）=====
    QLineEdit*  mEditGravityNum;    // 重力测点数（只读，对应原 IDC_GravityNum）
    QLineEdit*  mEditRows;          // 网格行数（只读，对应原 IDC_Rows）
    QLineEdit*  mEditCols;          // 网格列数（只读，对应原 IDC_Cols）
    QLineEdit*  mEditHStep;         // 高程网格距（只读，对应原 IDC_HStep）
    QLineEdit*  mEditZMin;          // 高程最小值（只读，对应原 IDC_ZMin）
    QLineEdit*  mEditZMax;          // 高程最大值（只读，对应原 IDC_ZMax）

    // ===== 界面控件（外/内接口形状选择，单选）=====
    QRadioButton*   mRadioOutRect;  // 外接方形（默认选中）
    QRadioButton*   mRadioOutCircle;// 外接圆形
    QRadioButton*   mRadioInRect;   // 内接方形（默认选中）
    QRadioButton*   mRadioInCircle; // 内接圆形
    QButtonGroup*   mGroupOuter;    // 外接口形状单选组（id 0=方形 1=圆形）
    QButtonGroup*   mGroupInner;    // 内接口形状单选组（id 0=方形 1=圆形）

    // ===== 界面控件（地形改正参数输入）=====
    QDoubleSpinBox* mSpinStartRadius;   // 起始半径（默认 20）
    QDoubleSpinBox* mSpinEndRadius;     // 终止半径（默认 500）
    QDoubleSpinBox* mSpinRockDensity;   // 地壳密度（默认 2.67）

    // ===== 界面控件（地形改正结果数据文件输出）=====
    QLineEdit*  mEditResFile;       // 地形改正结果数据文件路径（对应原 IDC_FileNameRes）
    QPushButton* mBtnOpenResFile;   // 结果文件“...”按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // “确定”按钮（校验并调用后端处理）
    QPushButton* mBtnCancel;         // “取消”按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mTxtFilePath;       // 重力测点数据文件路径（原 m_filenametxt）
    QString mGrdFilePath;       // 地形高程网格数据文件路径（原 m_filenamegrd）
    QString mResFilePath;       // 地形改正结果数据文件路径（原 m_filenameres）
    QString mLoadedGrdPath;     // 最近一次成功读取文件头的高程网格文件路径

    int     mRowsNum;           // 网格行数（原 m_Rows）
    int     mColsNum;           // 网格列数（原 m_Cols）
    double  mHstep;             // 高程网格距（原 m_hstep，回填为整数）
    double  mZMin;              // 高程最小值（原 m_zMin）
    double  mZMax;              // 高程最大值（原 m_zMax）
    int     mGravityNum;        // 重力测点数（原 m_gravitynum）

    double  mStartRadius;       // 起始半径（原 m_startraid，默认 20）
    double  mEndRadius;         // 终止半径（原 m_EndRaid，默认 500）
    double  mRockDensity;       // 地壳密度（原 m_rockdentisy，默认 2.67）
    int     mOuterShape;        // 外接口形状（0=方形 1=圆形）
    int     mInnerShape;        // 内接口形状（0=方形 1=圆形）
};

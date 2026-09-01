// GravUnionTerrainCorrectionDlg.h : 重力联合（平面带）地形改正对话框（Qt 前端）
//
// 对应原 MFC 工程：CGravUnionTerrainCorrectionDlg（IDD_GravUnionTerrainCorrection）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_GravUnionTerrainCorrection 绘制：
//   重力测点数据文件输入 / 高程网格数据文件输入 / 网格数据信息 /
//   地形改正方法选择 / 地形改正形状选择 / 地形改正类型选择 /
//   地形改正输入参数 / 地形改正数据文件输出
//
// 前端职责（本对话框只做界面与交互，算法/数据读写在后端）：
//   1. 测点数据文件(.dat)与地形高程网格数据文件(.grd)的选择、网格信息显示；
//   2. 地形改正方法/形状/类型三组单选，内/外环半径、密度、三个方位列数输入；
//   3. “确定”时校验输入，并调用后端接口 processUnionTerrain 完成处理；
//   4. “显示”按钮请求查看地形网格文件等值线（viewGridFileRequested 交由宿主处理）。

#pragma once

#include <QDialog>

#include "backend/RgisBackend.h"

// 前置声明（减少头文件之间的相互包含）
class QLineEdit;
class QPushButton;
class QRadioButton;
class QDoubleSpinBox;
class QSpinBox;
class QButtonGroup;

// 重力联合（平面带）地形改正对话框
class CGravUnionTerrainCorrectionDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CGravUnionTerrainCorrectionDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CGravUnionTerrainCorrectionDlg() override;

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

    // ===== 三组单选回调 =====
    void onKindsClicked(int nId);               // 功能：地形改正方法切换（0=常规计算 1=三观测列方差分解）
    void onShapeClicked(int nId);               // 功能：地形改正形状切换（0=环形 1=回形 2=钱形 3=枷形）
    void onTypesClicked(int nId);               // 功能：地形改正类型切换（0=常规 1=陆岛 2=广义）

    // ===== 处理参数回调 =====
    void onMinRadiusChanged(double dValue);     // 功能：地改内环半径变化（同步成员变量）
    void onMaxRadiusChanged(double dValue);     // 功能：地改外环半径变化（同步成员变量）
    void onDensityChanged(double dValue);       // 功能：地形改正密度变化（同步成员变量）
    void onAziNum1Changed(int nValue);          // 功能：第一列方位数变化（同步成员变量）
    void onAziNum2Changed(int nValue);          // 功能：第二列方位数变化（同步成员变量）
    void onAziNum3Changed(int nValue);          // 功能：第三列方位数变化（同步成员变量）

    // ===== 确定 / 取消 =====
    void onOkClicked();                         // 功能：“确定”——校验参数并调用后端处理
    void onCancelClicked();                     // 功能：“取消”——关闭对话框

private:
    // ===== 界面初始化 =====
    void initUi();                              // 功能：按 .rc 布局创建全部控件并连接信号槽

    // ===== 数据加载 =====
    void loadGrdFile(const QString& strFilePath);   // 功能：读取高程网格文件头并刷新网格信息
    void updateDefaultSavePath(const QString& strFilePath); // 功能：根据测点文件生成默认结果名（_Out+扩展名）
    void updateGridInfoDisplay();                   // 功能：把网格信息成员变量刷到界面显示

    // ===== 文件选择 / 显示 =====
    QString askOpenFilePath(const QString& strTitle, const QString& strFilter);  // 功能：弹出打开文件对话框
    void viewGridFile(const QString& strFilePath);  // 功能：检查文件存在并发出发送显示请求

    // ===== 处理 =====
    bool validateInputs(QString& strError);     // 功能：确定前校验输入项，返回 false 表示校验失败
    void runProcess();                          // 功能：组装参数并调用后端 processUnionTerrain

    // ===== 界面控件（重力测点数据文件输入）=====
    QLineEdit*      mEditTxtFile;       // 重力测点数据文件路径（对应原 IDC_OpenFile）
    QPushButton*    mBtnOpenTxtFile;    // 测点文件“...”按钮

    // ===== 界面控件（高程网格数据文件输入）=====
    QLineEdit*      mEditGrdFile;       // 地形高程网格数据文件路径（对应原 IDC_OpenGridFile）
    QPushButton*    mBtnOpenGrdFile;    // 高程网格文件“...”按钮
    QPushButton*    mBtnOpenGrdFileView;// 高程网格文件“显示”按钮

    // ===== 界面控件（网格数据信息，只读显示）=====
    QLineEdit*  mEditRowsNum;       // 网格行数（只读，对应原 IDC_RowsNum）
    QLineEdit*  mEditColsNum;       // 网格列数（只读，对应原 IDC_ColsNum）
    QLineEdit*  mEditRowStep;       // 网格行距（只读，对应原 IDC_RowStep）
    QLineEdit*  mEditColStep;       // 网格列距（只读，对应原 IDC_ColStep）

    // ===== 界面控件（地形改正方法选择，2 项）=====
    QRadioButton*   mRadioKinds1;   // 常规计算（默认选中）
    QRadioButton*   mRadioKinds2;   // 三观测列方差分解
    QButtonGroup*   mGroupKinds;    // 方法单选组（id 0/1）

    // ===== 界面控件（地形改正形状选择，4 项）=====
    QRadioButton*   mRadioShape1;   // 环形（默认选中）
    QRadioButton*   mRadioShape2;   // 回形
    QRadioButton*   mRadioShape3;   // 钱形
    QRadioButton*   mRadioShape4;   // 枷形
    QButtonGroup*   mGroupShape;    // 形状单选组（id 0/1/2/3）

    // ===== 界面控件（地形改正类型选择，3 项）=====
    QRadioButton*   mRadioTypes1;   // 常规地形改正（默认选中）
    QRadioButton*   mRadioTypes2;   // 陆岛地形改正
    QRadioButton*   mRadioTypes3;   // 广义地形改正
    QButtonGroup*   mGroupTypes;    // 类型单选组（id 0/1/2）

    // ===== 界面控件（地形改正输入参数）=====
    QDoubleSpinBox* mSpinMinRadius; // 地改内环半径（默认 50）
    QDoubleSpinBox* mSpinMaxRadius; // 地改外环半径（默认 2000）
    QDoubleSpinBox* mSpinDensity;   // 地形改正密度（默认 2.67）
    QSpinBox*       mSpinAziNum1;   // 第一列方位数（默认 72）
    QSpinBox*       mSpinAziNum2;   // 第二列方位数（默认 36）
    QSpinBox*       mSpinAziNum3;   // 第三列方位数（默认 24）

    // ===== 界面控件（地形改正数据文件输出）=====
    QLineEdit*  mEditResFile;       // 地形改正结果数据文件路径（对应原 IDC_SaveFile）
    QPushButton* mBtnOpenResFile;   // 结果文件“...”按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // “确定”按钮（校验并调用后端处理）
    QPushButton* mBtnCancel;         // “取消”按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mTxtFilePath;       // 重力测点数据文件路径（原 m_strOpenFile）
    QString mGrdFilePath;       // 地形高程网格数据文件路径（原 m_strOpenGridFile）
    QString mResFilePath;       // 地形改正结果数据文件路径（原 m_strSaveFile）
    QString mLoadedGrdPath;     // 最近一次成功读取文件头的高程网格文件路径

    int     mRowsNum;       // 网格行数（原 m_nRowsNum）
    int     mColsNum;       // 网格列数（原 m_nColsNum）
    double  mRowStep;       // 网格行距（原 m_fRowStep）
    double  mColStep;       // 网格列距（原 m_fColStep）

    int     mKindsType;     // 地形改正方法（0=常规计算 1=三观测列方差分解）
    int     mShapeType;     // 地形改正形状（0=环形 1=回形 2=钱形 3=枷形）
    int     mTypesType;     // 地形改正类型（0=常规 1=陆岛 2=广义）

    double  mMinRadius;     // 地改内环半径（原 m_fMinRads，默认 50）
    double  mMaxRadius;     // 地改外环半径（原 m_fMaxRads，默认 2000）
    double  mDensity;       // 地形改正密度（原 m_fDensity，默认 2.67）
    int     mAziNum1;       // 第一列方位数（原 m_AziNum1，默认 72）
    int     mAziNum2;       // 第二列方位数（原 m_AziNum2，默认 36）
    int     mAziNum3;       // 第三列方位数（原 m_AziNum3，默认 24）
};

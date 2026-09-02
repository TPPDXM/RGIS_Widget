// ProfDataInterpolationDlg.h : 剖面数据插值对话框（Qt 前端）
//
// 对应原 MFC 工程：CProfDataInterpolationDlg（IDD_ProfDataInterpolation）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_ProfDataInterpolation 绘制：
//   剖面数据文件输入 / 剖面数据信息 / 处理参数 / 剖面数据文件输出
//
// 前端职责（本对话框为纯源码可独立重写功能，不做后端算法接口，仅前端实现）：
//   1. 剖面数据文件（文本，每行 3 列：坐标/值1/值2）的读取与信息显示；
//   2. 处理参数（坐标最小值/最大值/测点点距）输入；
//   3. “确定”时按参数对非等间距剖面做线性插值，写出结果文件；
//   4. 文件为三列、且要求非等间距（等间距报“无需插值”）。
// 说明：原工程算法即对话框内实现（无 PfProcesses.dll 依赖），故前端直接实现。

#pragma once

#include <QDialog>
#include <QVector>

// 前置声明（减少头文件之间的相互包含）
class QLineEdit;
class QPushButton;

// 剖面数据插值对话框
class CProfDataInterpolationDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CProfDataInterpolationDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CProfDataInterpolationDlg() override;

private slots:
    // ===== 输入/输出文件回调 =====
    void onOpenFileClicked();               // 功能：“...”选择剖面数据文件
    void onOpenFileTextEdited(const QString& strText);  // 功能：手动输入输入文件名（同步成员变量）
    void onSaveFileClicked();               // 功能：“...”选择结果数据文件
    void onSaveFileTextEdited(const QString& strText);  // 功能：手动输入输出文件名（同步成员变量）

    // ===== 处理参数回调 =====
    void onZMinEdited(const QString& strText);  // 功能：坐标最小值变化（同步成员变量）
    void onZMaxEdited(const QString& strText);  // 功能：坐标最大值变化（同步成员变量）
    void onStepEdited(const QString& strText);  // 功能：测点点距变化（同步成员变量）

    // ===== 确定 / 取消 =====
    void onOkClicked();                         // 功能：“确定”——校验参数并做插值处理
    void onCancelClicked();                     // 功能：“取消”——关闭对话框

private:
    // ===== 界面初始化 =====
    void initUi();                              // 功能：按 .rc 布局创建全部控件并连接信号槽

    // ===== 数据加载 =====
    bool loadFile(const QString& strFilePath, QString& strError);  // 功能：读取剖面文件并刷新信息、默认值

    // ===== 文件选择 =====
    QString askOpenFilePath(const QString& strTitle);       // 功能：弹出打开文件对话框
    QString askSaveFilePath(const QString& strTitle, const QString& strDefaultPath);  // 功能：弹出保存文件对话框

    // ===== 处理 =====
    bool validateInputs(QString& strError);     // 功能：确定前校验输入项，返回 false 表示校验失败
    void runInterpolate();                      // 功能：按参数对剖面数据插值并写出结果文件

    // ===== 界面控件（剖面数据文件输入）=====
    QLineEdit*  mEditOpenFile;      // 剖面数据文件路径（对应原 IDC_OpenFile）
    QPushButton* mBtnOpenFile;      // 输入文件“...”按钮

    // ===== 界面控件（剖面数据信息，只读）=====
    QLineEdit*  mEditXMin;          // 坐标最小值（只读，对应原 IDC_XMin）
    QLineEdit*  mEditXMax;          // 坐标最大值（只读，对应原 IDC_XMax）
    QLineEdit*  mEditPnts;          // 测点点数（只读，对应原 IDC_Pnts）

    // ===== 界面控件（处理参数，可编辑）=====
    QLineEdit*  mEditZMin;          // 坐标最小值（对应原 IDC_ZMin）
    QLineEdit*  mEditZMax;          // 坐标最大值（对应原 IDC_ZMax）
    QLineEdit*  mEditStep;          // 测点点距（对应原 IDC_Step）

    // ===== 界面控件（剖面数据文件输出）=====
    QLineEdit*  mEditSaveFile;      // 结果数据文件路径（对应原 IDC_SaveFile）
    QPushButton* mBtnSaveFile;      // 输出文件“...”按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // “确定”按钮（校验并做插值）
    QPushButton* mBtnCancel;         // “取消”按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mOpenFilePath;           // 当前输入剖面文件路径（对应原 m_strOpenProfFile）
    QString mSaveFilePath;           // 结果输出文件路径（对应原 m_strSaveProfFile）
    QString mLoadedFilePath;         // 最近一次成功读取文件的路径（防止重复加载）

    bool    mDataValid;              // 剖面数据是否已成功读取（对应原 m_bIsvalidOfOpenFile）
    int     mPnts;                   // 测点点数（对应原 m_nPnts）
    double  mXMin;                   // 坐标最小值（对应原 m_fXMin）
    double  mXMax;                   // 坐标最大值（对应原 m_fXMax）
    double  mZMin;                   // 处理参数：坐标最小值（对应原 m_fZMin）
    double  mZMax;                   // 处理参数：坐标最大值（对应原 m_fZMax）
    double  mStep;                   // 处理参数：测点点距（对应原 m_fStep）

    // ===== 剖面数据（3 列：列0 坐标、列1 值1、列2 值2）=====
    QVector<double> mCoord;          // 列0：测点坐标
    QVector<double> mVal1;           // 列1：值1
    QVector<double> mVal2;           // 列2：值2
};

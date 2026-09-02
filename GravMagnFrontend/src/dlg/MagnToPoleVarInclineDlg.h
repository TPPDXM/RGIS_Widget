// MagnToPoleVarInclineDlg.h : 分带变磁倾角化极对话框（Qt 前端）
//
// 对应原 MFC 工程：CMagnToPoleVarInclineDlg（IDD_MagnToPoleVarIncline）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_MagnToPoleVarIncline 绘制：
//   数据文件输入 / 网格数据信息 / 数据扩边方法 / 分带参数输入（纬度最大值/最小值/间隔/分块数）/
//   测区地磁场参数输入（分带表格：分块序号/中心纬度/地磁倾角/地磁偏角）/ 数据文件输出
//
// 前端职责（本对话框只做界面与交互，算法/数据读写在后端）：
//   1. 输入数据文件选择、网格信息显示、默认输出文件名生成（基准名 + "_RTP.grd"）；
//   2. 扩充行/列数、行/列方位角、扩边方法（默认余弦函数衰减）、分带纬度参数
//      （默认最大值 44、最小值 40、间隔 1，分块数随参数自动计算并只读显示）、
//      分带表格（分块序号/中心纬度/地磁倾角/地磁偏角，纬度参数变化时重建）；
//   3. “确定”时校验输入，调用后端接口 processMagnToPoleVarIncline 完成处理
//      （接口尚未由后端提供，当前为参数校验 + 占位提示，接入方式见 runProcess 注释）；
//   4. “显示”按钮请求查看等值线（viewGridFileRequested 信号交由宿主处理）。

#pragma once

#include <QDialog>

#include "backend/RgisBackend.h"

// 前置声明（减少头文件之间的相互包含）
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTableWidget;
class QButtonGroup;

// 分带变磁倾角化极对话框
class CMagnToPoleVarInclineDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CMagnToPoleVarInclineDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CMagnToPoleVarInclineDlg() override;

signals:
    // 功能：请求宿主显示指定网格文件的等值线图（对应原 MFC 工程“显示”按钮，
    //       等值线显示窗口由前端后续版本实现）
    void viewGridFileRequested(const QString& strFilePath);

private slots:
    // ===== 输入/输出文件回调 =====
    void onOpenFileClicked();                   // 功能：“...”选择输入数据文件
    void onOpenFileViewClicked();               // 功能：显示输入数据文件等值线
    void onOpenFileTextEdited(const QString& strText);      // 功能：手动输入文件名（同步成员变量）
    void onSaveFileClicked();                   // 功能：“...”选择处理结果数据文件
    void onSaveFileViewClicked();               // 功能：显示处理结果文件等值线
    void onSaveFileTextEdited(const QString& strText);      // 功能：手动输入输出文件名（同步成员变量）

    // ===== 扩充尺寸回调 =====
    void onExRowsValueChanged(int nValue);      // 功能：扩充行数变化（同步成员变量）
    void onExColsValueChanged(int nValue);      // 功能：扩充列数变化（同步成员变量）

    // ===== 扩边方法单选回调 =====
    void onExpandMethodClicked(int nMethodId);  // 功能：数据扩边方法切换（同步成员变量）

    // ===== 处理参数回调 =====
    void onProfileValueChanged(double dValue);  // 功能：行方位角变化（同步成员变量）
    void onBaseLineValueChanged(double dValue); // 功能：列方位角变化（同步成员变量）

    // ===== 分带参数回调 =====
    void onLatiMaxValueChanged(double dValue);  // 功能：纬度最大值变化（重建分带表格）
    void onLatiMinValueChanged(double dValue);  // 功能：纬度最小值变化（重建分带表格）
    void onLatiStpValueChanged(double dValue);  // 功能：纬度间隔变化（重建分带表格）

    // ===== 确定 / 取消 =====
    void onOkClicked();                         // 功能：“确定”——校验参数并调用后端处理
    void onCancelClicked();                     // 功能：“取消”——关闭对话框

private:
    // ===== 界面初始化 =====
    void initUi();                              // 功能：按 .rc 布局创建全部控件并连接信号槽

    // ===== 数据加载 =====
    void loadFile(const QString& strFilePath);              // 功能：读取文件头并刷新网格信息、默认值
    void updateDefaultSavePaths(const QString& strFilePath);// 功能：生成默认输出文件名
    void updateGridInfoDisplay();               // 功能：把网格信息成员变量刷到界面显示

    // ===== 文件选择 / 显示 =====
    QString askOpenFilePath(const QString& strTitle);       // 功能：弹出打开文件对话框
    QString askSaveFilePath(const QString& strTitle, const QString& strDefaultPath);  // 功能：弹出保存文件对话框
    void viewGridFile(const QString& strFilePath);          // 功能：检查文件存在并发出发送显示请求

    // ===== 处理 =====
    bool validateInputs(QString& strError);     // 功能：确定前校验输入项，返回 false 表示校验失败
    void runProcess();                          // 功能：调用后端 processMagnToPoleVarIncline（接口待后端提供，当前占位）

    // ===== 工具 =====
    void setSpinExtendSize(QSpinBox* pSpin, int nSize);     // 功能：同步设置扩充微调框下限与数值
    void rebuildBlockTable();                   // 功能：按纬度最大值/最小值/间隔重建分带表格（对应原工程表格初始化）
    int  calcBlockCount() const;                // 功能：按纬度参数计算纬度分块数（原工程 OnOK 中的分块数规则）

    // ===== 界面控件（数据文件输入）=====
    QLineEdit*      mEditOpenFile;      // 输入数据文件路径（可编辑文本，对应原 IDC_OpenFile）
    QPushButton*    mBtnOpenFile;       // 输入文件“...”按钮
    QPushButton*    mBtnOpenFileView;   // 输入文件“显示”按钮

    // ===== 界面控件（网格数据信息，只读显示）=====
    QLineEdit*  mEditRowsNum;       // 网格行数（只读，原 IDC_EDIT_RowsNum）
    QLineEdit*  mEditColsNum;       // 网格列数（只读，原 IDC_EDIT_ColsNum）
    QLineEdit*  mEditRowStep;       // 网格行距（只读，原 IDC_EDIT_RowStep）
    QLineEdit*  mEditColStep;       // 网格列距（只读，原 IDC_EDIT_ColStep）

    // ===== 界面控件（处理参数：方位角 + 扩充行/列数）=====
    QDoubleSpinBox* mSpinProfile;      // 行方位角（原 IDC_EDIT_ProfileDirection，默认 90）
    QDoubleSpinBox* mSpinBaseLine;     // 列方位角（原 IDC_EDIT_BaseLineDirection，默认 0）
    QSpinBox*       mSpinExRows;       // 扩充行数（原 IDC_EDIT_RowsExNum）
    QSpinBox*       mSpinExCols;       // 扩充列数（原 IDC_EDIT_ColsExNum）

    // ===== 界面控件（数据扩边方法，单选，4 种）=====
    QRadioButton*   mRadioCosFun;    // 余弦函数衰减（默认选中，原 IDC_CosFun）
    QRadioButton*   mRadioAvgDif;    // 平均值差分（原 IDC_AvgDif）
    QRadioButton*   mRadioInvPow;    // 反距离加权（原 IDC_InvPow）
    QRadioButton*   mRadioMinCrv;    // 最小曲率（原 IDC_MinCrv）
    QButtonGroup*   mButtonGroupExpand;  // 扩边方法单选按钮组（id 对应 ExpandMethod 枚举）

    // ===== 界面控件（分带参数输入）=====
    QDoubleSpinBox* mSpinLatiMax;    // 纬度最大值（原 IDC_LATIMAX，默认 44，0~90）
    QDoubleSpinBox* mSpinLatiMin;    // 纬度最小值（原 IDC_LATIMIN，默认 40，0~90）
    QDoubleSpinBox* mSpinLatiStp;    // 纬度间隔（原 IDC_LATISTP，默认 1，0~4）
    QLineEdit*      mEditRegRows;    // 纬度分块数（只读，原 IDC_REGROWS）

    // ===== 界面控件（测区地磁场参数输入：分带表格）=====
    QTableWidget*   mTableBlocks;    // 分带表格（4 列：分块序号/中心纬度/地磁倾角/地磁偏角，对应原 MSFlexGrid）

    // ===== 界面控件（数据文件输出）=====
    QLineEdit*  mEditSaveFile;       // 处理结果数据文件名（默认 基准名 + "_RTP.grd"）
    QPushButton* mBtnSaveFile;       // 输出文件“...”按钮
    QPushButton* mBtnSaveFileView;   // 输出文件“显示”按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // “确定”按钮（校验并调用后端处理）
    QPushButton* mBtnCancel;         // “取消”按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mInputFilePath;              // 当前输入数据文件路径（对应原 m_strOpenFile）
    QString mOutputFilePath;             // 处理结果输出文件路径（对应原 m_strSaveFile，默认 基准名+"_RTP.grd"）
    QString mLoadedFilePath;             // 最近一次成功读取文件头的文件路径（防止重复加载）

    int     mRowsNum;       // 网格行数（原 m_nRowsNum）
    int     mColsNum;       // 网格列数（原 m_nColsNum）
    double  mRowStep;       // 网格行距（原 m_fRowStep）
    double  mColStep;       // 网格列距（原 m_fColStep）

    int     mExRows;        // 当前扩充行数（原 m_nRowsExNum）
    int     mExCols;        // 当前扩充列数（原 m_nColsExNum）
    int     mMinExRows;     // 最小扩充行数（读文件时计算，原 m_nMinRows）
    int     mMinExCols;     // 最小扩充列数（读文件时计算，原 m_nMinCols）
    int     mExpandMethod;  // 数据扩边方法（对应 ExpandMethod 枚举，原单选按钮组）
    double  mProfile;       // 行方位角（原 m_fProfileDirection，默认 90.0）
    double  mBaseLine;      // 列方位角（原 m_fBaseLineDirection，默认 0.0）

    double  mLatiMax;       // 纬度最大值（原 m_fLatiMax，默认 44.0）
    double  mLatiMin;       // 纬度最小值（原 m_fLatiMin，默认 40.0）
    double  mLatiStp;       // 纬度间隔（原 m_fLatiStp，默认 1.0）
    int     mRegRows;       // 纬度分块数（原 m_nRegRows，由纬度参数计算，只读显示）
};

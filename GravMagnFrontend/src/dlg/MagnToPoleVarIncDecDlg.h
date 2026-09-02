// MagnToPoleVarIncDecDlg.h : 变磁化倾角化极对话框（Qt 前端）
//
// 对应原 MFC 工程：CMagnToPoleVarIncDecDlg（IDD_MagnToPoleVarIncDec）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_MagnToPoleVarIncDec 绘制：
//   数据文件输入 / 网格数据信息 / 数据扩边方法 / 测区地磁场参数（文件）输入 / 数据文件输出
//
// 前端职责（本对话框只做界面与交互，算法/数据读写在后端）：
//   1. 输入数据文件选择、网格信息显示、默认输出文件名生成（基准名 + "Pro.grd"）；
//   2. 扩充行/列数、行/列方位角、扩边方法（默认余弦函数衰减）、
//      四角点磁化倾角/偏角（默认 10/11/14/15、30/30/30/30，可勾选“近似线性变化”）与倾角/偏角数据文件；
//   3. “确定”时校验输入，调用后端接口 processMagnToPoleVarIncDec 完成处理
//      （接口尚未由后端提供，当前为参数校验 + 占位提示，接入方式见 runProcess 注释）；
//   4. “显示”按钮请求查看等值线（viewGridFileRequested 信号交由宿主处理）。

#pragma once

#include <QDialog>

#include "backend/RgisBackend.h"

// 前置声明（减少头文件之间的相互包含）
class QCheckBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QButtonGroup;

// 变磁化倾角化极对话框
class CMagnToPoleVarIncDecDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CMagnToPoleVarIncDecDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CMagnToPoleVarIncDecDlg() override;

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

    // ===== 倾角/偏角数据文件回调 =====
    void onInclinationFileClicked();            // 功能：“...”选择地磁倾角数据文件
    void onInclinationFileViewClicked();        // 功能：显示地磁倾角数据文件等值线
    void onInclinationFileTextEdited(const QString& strText);  // 功能：手动输入倾角文件名（同步成员变量）
    void onDeclinationFileClicked();            // 功能：“...”选择地磁偏角数据文件
    void onDeclinationFileViewClicked();        // 功能：显示地磁偏角数据文件等值线
    void onDeclinationFileTextEdited(const QString& strText); // 功能：手动输入偏角文件名（同步成员变量）

    // ===== 扩充尺寸回调 =====
    void onExRowsValueChanged(int nValue);      // 功能：扩充行数变化（同步成员变量）
    void onExColsValueChanged(int nValue);      // 功能：扩充列数变化（同步成员变量）

    // ===== 扩边方法单选回调 =====
    void onExpandMethodClicked(int nMethodId);  // 功能：数据扩边方法切换（同步成员变量）

    // ===== 处理参数回调 =====
    void onProfileValueChanged(double dValue);  // 功能：行方位角变化（同步成员变量）
    void onBaseLineValueChanged(double dValue); // 功能：列方位角变化（同步成员变量）

    // ===== 测区地磁场参数回调 =====
    void onCheckLinearToggled(bool bChecked);   // 功能：“四角点近似线性变化”勾选状态变化
    void onInc1ValueChanged(double dValue);     // 功能：角点1磁化倾角变化（同步成员变量）
    void onInc2ValueChanged(double dValue);     // 功能：角点2磁化倾角变化（同步成员变量）
    void onInc3ValueChanged(double dValue);     // 功能：角点3磁化倾角变化（同步成员变量）
    void onInc4ValueChanged(double dValue);     // 功能：角点4磁化倾角变化（同步成员变量）
    void onDec1ValueChanged(double dValue);     // 功能：角点1磁化偏角变化（同步成员变量）
    void onDec2ValueChanged(double dValue);     // 功能：角点2磁化偏角变化（同步成员变量）
    void onDec3ValueChanged(double dValue);     // 功能：角点3磁化偏角变化（同步成员变量）
    void onDec4ValueChanged(double dValue);     // 功能：角点4磁化偏角变化（同步成员变量）

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
    void runProcess();                          // 功能：调用后端 processMagnToPoleVarIncDec（接口待后端提供，当前占位）

    // ===== 工具 =====
    void setSpinExtendSize(QSpinBox* pSpin, int nSize);     // 功能：同步设置扩充微调框下限与数值

    // ===== 界面控件（数据文件输入）=====
    QLineEdit*      mEditOpenFile;      // 输入数据文件路径（可编辑文本，对应原 IDC_OpenFile）
    QPushButton*    mBtnOpenFile;       // 输入文件“...”按钮
    QPushButton*    mBtnOpenFileView;   // 输入文件“显示”按钮

    // ===== 界面控件（网格数据信息，只读显示）=====
    QLineEdit*  mEditRowsNum;       // 网格行数（只读，原 IDC_EDIT_RowsNum）
    QLineEdit*  mEditColsNum;       // 网格列数（只读，原 IDC_EDIT_ColsNum）
    QLineEdit*  mEditRowStep;       // 网格行距（只读，原 IDC_yStep）
    QLineEdit*  mEditColStep;       // 网格列距（只读，原 IDC_xStep）

    // ===== 界面控件（处理参数：方位角 + 扩充行/列数）=====
    QDoubleSpinBox* mSpinProfile;      // 行方位角（原 IDC_EDIT_ProfileDirection，默认 90，-360~360）
    QDoubleSpinBox* mSpinBaseLine;     // 列方位角（原 IDC_EDIT_BaseLineDirection，默认 0，-360~360）
    QSpinBox*       mSpinExRows;       // 扩充行数（原 IDC_EDIT_RowsExNum）
    QSpinBox*       mSpinExCols;       // 扩充列数（原 IDC_EDIT_ColsExNum）

    // ===== 界面控件（数据扩边方法，单选，4 种）=====
    QRadioButton*   mRadioCosFun;    // 余弦函数衰减（默认选中，原 IDC_CosFun）
    QRadioButton*   mRadioAvgDif;    // 平均值差分（原 IDC_AvgDif）
    QRadioButton*   mRadioInvPow;    // 反距离加权（原 IDC_InvPow）
    QRadioButton*   mRadioMinCrv;    // 最小曲率（原 IDC_MinCrv）
    QButtonGroup*   mButtonGroupExpand;  // 扩边方法单选按钮组（id 对应 ExpandMethod 枚举）

    // ===== 界面控件（测区地磁场参数输入）=====
    QCheckBox*      mCheckLinear;        // “测区四个角点的磁化倾角和磁化偏角近似线性变化”（原 IDC_CHECK_LINEAR）
    QDoubleSpinBox* mSpinInc1;           // 角点1磁化倾角（原 IDC_EDIT_I1，默认 10，-90~90）
    QDoubleSpinBox* mSpinInc2;           // 角点2磁化倾角（原 IDC_EDIT_I2，默认 11，-90~90）
    QDoubleSpinBox* mSpinInc3;           // 角点3磁化倾角（原 IDC_EDIT_I3，默认 14，-90~90）
    QDoubleSpinBox* mSpinInc4;           // 角点4磁化倾角（原 IDC_EDIT_I4，默认 15，-90~90）
    QDoubleSpinBox* mSpinDec1;           // 角点1磁化偏角（原 IDC_EDIT_D1，默认 30，-360~360）
    QDoubleSpinBox* mSpinDec2;           // 角点2磁化偏角（原 IDC_EDIT_D2，默认 30，-360~360）
    QDoubleSpinBox* mSpinDec3;           // 角点3磁化偏角（原 IDC_EDIT_D3，默认 30，-360~360）
    QDoubleSpinBox* mSpinDec4;           // 角点4磁化偏角（原 IDC_EDIT_D4，默认 30，-360~360）
    QLineEdit*      mEditInclinationFile;   // 磁化倾角数据文件路径（原 IDC_EDIT_FILE_INCLINATION）
    QLineEdit*      mEditDeclinationFile;   // 磁化偏角数据文件路径（原 IDC_EDIT_FILE_DECLINATION）
    QPushButton*    mBtnInclinationFile;    // 倾角文件“...”按钮
    QPushButton*    mBtnInclinationFileView;// 倾角文件“显示”按钮
    QPushButton*    mBtnDeclinationFile;    // 偏角文件“...”按钮
    QPushButton*    mBtnDeclinationFileView;// 偏角文件“显示”按钮

    // ===== 界面控件（数据文件输出）=====
    QLineEdit*  mEditSaveFile;       // 处理结果数据文件名（默认 基准名 + "Pro.grd"）
    QPushButton* mBtnSaveFile;       // 输出文件“...”按钮
    QPushButton* mBtnSaveFileView;   // 输出文件“显示”按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // “确定”按钮（校验并调用后端处理）
    QPushButton* mBtnCancel;         // “取消”按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mInputFilePath;              // 当前输入数据文件路径（对应原 m_strOpenFile）
    QString mOutputFilePath;             // 处理结果输出文件路径（对应原 m_strSaveFile，默认 基准名+"Pro.grd"）
    QString mInclinationFilePath;        // 磁化倾角数据文件路径（对应原 m_strFileNameOfInclination）
    QString mDeclinationFilePath;        // 磁化偏角数据文件路径（对应原 m_strFileNameOfDeclination）
    QString mLoadedFilePath;             // 最近一次成功读取文件头的文件路径（防止重复加载）

    int     mRowsNum;       // 网格行数（原 m_nRowsNum）
    int     mColsNum;       // 网格列数（原 m_nColsNum）
    double  mRowStep;       // 网格行距（原 m_fRowStep）
    double  mColStep;       // 网格列距（原 m_fColStep）

    int     mExRows;        // 当前扩充行数（原 m_nExRows）
    int     mExCols;        // 当前扩充列数（原 m_nExCols）
    int     mMinExRows;     // 最小扩充行数（读文件时计算，原 m_nMinRows）
    int     mMinExCols;     // 最小扩充列数（读文件时计算，原 m_nMinCols）
    int     mExpandMethod;  // 数据扩边方法（对应 ExpandMethod 枚举，原单选按钮组）
    double  mProfile;       // 行方位角（原 m_fProfileDirection，默认 90.0）
    double  mBaseLine;      // 列方位角（原 m_fBaseLineDirection，默认 0.0）
    bool    mCheckLinearState;   // “四个角点近似线性变化”勾选状态（默认 false）
    double  mInc1;          // 角点1磁化倾角（原 m_fInclinationOf1st，默认 10.0）
    double  mInc2;          // 角点2磁化倾角（原 m_fInclinationOf2nd，默认 11.0）
    double  mInc3;          // 角点3磁化倾角（原 m_fInclinationOf3rd，默认 14.0）
    double  mInc4;          // 角点4磁化倾角（原 m_fInclinationOf4th，默认 15.0）
    double  mDec1;          // 角点1磁化偏角（原 m_fDeclinationOf1st，默认 30.0）
    double  mDec2;          // 角点2磁化偏角（原 m_fDeclinationOf2nd，默认 30.0）
    double  mDec3;          // 角点3磁化偏角（原 m_fDeclinationOf3rd，默认 30.0）
    double  mDec4;          // 角点4磁化偏角（原 m_fDeclinationOf4th，默认 30.0）
};

// FreqDomainHorzGradDlg.h : 频率域总水平方向导数对话框（Qt 前端）
//
// 对应原 MFC 工程：CFreqDomainHorzGradDlg（IDD_FreqDomainHorzGrad）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_FreqDomainHorzGrad 绘制：
//   数据文件输入 / 网格数据信息 / 数据扩边信息 / 数据扩边方法 / 数据文件输出
//
// 前端职责（本对话框只做界面与交互，算法/数据读写在后端）：
//   1. 输入数据文件的选择、网格信息显示、默认输出文件名生成（_THDR.grd）；
//   2. 扩边行数/列数（2 的幂）微调、扩边方法单选（4 种）；
//   3. “确定”时校验输入，并调用后端接口 processHorzGrad 完成处理；
//   4. “显示”按钮请求查看等值线（通过 viewGridFileRequested 信号交由宿主处理）。

#pragma once

#include <QDialog>


// 前置声明（减少头文件之间的相互包含）
class QLineEdit;
class QPushButton;
class QRadioButton;
class QButtonGroup;
class QSpinBox;

// 频率域总水平方向导数对话框
class CFreqDomainHorzGradDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CFreqDomainHorzGradDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CFreqDomainHorzGradDlg() override;

signals:
    // 功能：请求宿主显示指定网格文件的等值线图（对应原 MFC 工程“显示”按钮
    //       弹出的 CDrawOpenGLContourDlg，等值线显示窗口由前端后续版本实现）
    void viewGridFileRequested(const QString& strFilePath);

private slots:
    // ===== 输入/输出文件回调 =====
    void onOpenFileClicked();               // 功能：“...”选择输入数据文件
    void onOpenFileViewClicked();           // 功能：显示输入数据文件等值线
    void onOpenFileTextEdited(const QString& strText);  // 功能：手动输入文件名（同步成员变量）
    void onSaveFileClicked();               // 功能：“...”选择处理结果数据文件
    void onSaveFileViewClicked();           // 功能：显示处理结果文件等值线
    void onSaveFileTextEdited(const QString& strText);  // 功能：手动输入输出文件名（同步成员变量）

    // ===== 扩边尺寸回调 =====
    void onExRowsValueChanged(int nValue);  // 功能：扩边行数变化（同步成员变量）
    void onExColsValueChanged(int nValue);  // 功能：扩边列数变化（同步成员变量）

    // ===== 扩边方法单选回调 =====
    void onExpandMethodClicked(int nMethodId);  // 功能：数据扩边方法切换（同步成员变量）

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
    void runProcess();                          // 功能：组装参数并调用后端 processHorzGrad

    // ===== 工具 =====
    void setSpinExtendSize(QSpinBox* pSpin, int nSize); // 功能：同步设置扩边微调框下限与数值

    // ===== 界面控件（数据文件输入）=====
    QLineEdit*      mEditOpenFile;      // 输入数据文件路径（可编辑文本，对应原 IDC_OpenFile）
    QPushButton*    mBtnOpenFile;       // 输入文件“...”按钮
    QPushButton*    mBtnOpenFileView;   // 输入文件“显示”按钮

    // ===== 界面控件（网格数据信息，只读显示）=====
    QLineEdit*  mEditRowsNum;       // 网格行数（只读）
    QLineEdit*  mEditColsNum;       // 网格列数（只读）
    QLineEdit*  mEditRowStep;       // 网格行距（只读）
    QLineEdit*  mEditColStep;       // 网格列距（只读）

    // ===== 界面控件（数据扩边信息）=====
    QSpinBox*   mSpinExRows;     // 扩边行数微调框（默认值为 2 的幂）
    QSpinBox*   mSpinExCols;     // 扩边列数微调框（默认值为 2 的幂）

    // ===== 界面控件（数据扩边方法，单选，4 种）=====
    QRadioButton*   mRadioCosFun;    // 余弦函数衰减（默认选中）
    QRadioButton*   mRadioAvgDif;    // 平均值差分
    QRadioButton*   mRadioInvPow;    // 反距离加权
    QRadioButton*   mRadioMinCrv;    // 最小曲率
    QButtonGroup*   mButtonGroupExpand;  // 扩边方法单选按钮组（id 对应 ExpandMethod 枚举）

    // ===== 界面控件（数据文件输出）=====
    QLineEdit*  mEditSaveFile;       // 处理结果数据文件名（默认 基准名 + "_THDR.grd"）
    QPushButton* mBtnSaveFile;       // 输出文件“...”按钮
    QPushButton* mBtnSaveFileView;   // 输出文件“显示”按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // “确定”按钮（校验并调用后端处理）
    QPushButton* mBtnCancel;         // “取消”按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mInputFilePath;          // 当前输入数据文件路径（对应原 m_strOpenFile）
    QString mOutputFilePath;         // 处理结果输出文件路径（对应原 m_strSaveFile）
    QString mLoadedFilePath;         // 最近一次成功读取文件头的文件路径（防止重复加载）

    int     mRowsNum;       // 网格行数（原 m_nRowsNum）
    int     mColsNum;       // 网格列数（原 m_nColsNum）
    double  mColStep;       // 网格列距（原 m_fColStep）
    double  mRowStep;       // 网格行距（原 m_fRowStep）

    int     mExRows;        // 当前扩边行数（原 m_nExRows）
    int     mExCols;        // 当前扩边列数（原 m_nExCols）
    int     mMinExRows;     // 最小扩边行数（读文件时计算，原 m_nMinExRows）
    int     mMinExCols;     // 最小扩边列数（读文件时计算，原 m_nMinExCols）
    int     mExpandMethod;  // 数据扩边方法（对应 ExpandMethod 枚举，原单选按钮组）

};

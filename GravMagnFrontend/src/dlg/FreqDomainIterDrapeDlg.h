// FreqDomainIterDrapeDlg.h : 频率域迭代曲化平（迭代下延计算）对话框（Qt 前端）
//
// 对应原 MFC 工程：CFreqDomainIterDrapeDlg（IDD_FreqDomainIterDrape）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_FreqDomainIterDrape 绘制：
//   重磁场值数据文件输入 / 地形高程数据文件输入 / 网格数据信息（含高程最小、最大值）/
//   数据扩边信息 / 数据扩边方法 / 网格数据信息（延拓高度、切割平面数、迭代因子、迭代均方差）
//   及切割平面高程表格（平面序号 / 平面高程，对应原工程 MSFlexGrid）/
//   曲化平结果数据文件输出
//
// 前端职责（本对话框只做界面与交互，算法/数据读写在后端）：
//   1. 重磁场值、地形高程两个输入文件的选择与一致性校验、网格信息显示；
//   2. 扩边行数/列数（2 的幂）微调、扩边方法单选（3 种）、延拓与迭代参数输入；
//   3. 切割平面高程表格（平面序号 / 平面高程，序号 0 ~ 切割平面数）；
//   4. “确定”时校验输入，并调用后端接口 processIterDrape 完成处理；
//   5. “显示”按钮请求查看等值线（通过 viewGridFileRequested 信号交由宿主处理）。

#pragma once

#include <QDialog>

#include "backend/RgisBackend.h"

// 前置声明（减少头文件之间的相互包含）
class QLineEdit;
class QPushButton;
class QRadioButton;
class QButtonGroup;
class QSpinBox;
class QDoubleSpinBox;
class QTableWidget;
class QSpinBox;

// 频率域迭代曲化平（迭代下延计算）对话框
class CFreqDomainIterDrapeDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CFreqDomainIterDrapeDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CFreqDomainIterDrapeDlg() override;

signals:
    // 功能：请求宿主显示指定网格文件的等值线图（对应原 MFC 工程“显示”按钮
    //       弹出的 CDrawOpenGLContourDlg，等值线显示窗口由前端后续版本实现）
    void viewGridFileRequested(const QString& strFilePath);

private slots:
    // ===== 重磁场值数据文件回调 =====
    void onOpenFileClicked();               // 功能：“...”选择重磁场值输入数据文件
    void onOpenFileViewClicked();           // 功能：显示重磁场值输入文件等值线
    void onOpenFileTextEdited(const QString& strText);  // 功能：手动输入文件名（同步成员变量）

    // ===== 地形高程数据文件回调 =====
    void onOpenDemFileClicked();            // 功能：“...”选择地形高程输入数据文件
    void onOpenDemFileViewClicked();        // 功能：显示地形高程输入文件等值线
    void onOpenDemFileTextEdited(const QString& strText);  // 功能：手动输入文件名（同步成员变量）

    // ===== 输出文件回调 =====
    void onSaveFileClicked();               // 功能：“...”选择曲化平结果数据文件
    void onSaveFileViewClicked();           // 功能：显示曲化平结果文件等值线
    void onSaveFileTextEdited(const QString& strText);  // 功能：手动输入输出文件名（同步成员变量）

    // ===== 扩边尺寸回调 =====
    void onExRowsValueChanged(int nValue);  // 功能：扩边行数变化（同步成员变量）
    void onExColsValueChanged(int nValue);  // 功能：扩边列数变化（同步成员变量）

    // ===== 扩边方法单选回调 =====
    void onExpandMethodClicked(int nMethodId);  // 功能：数据扩边方法切换（同步成员变量）

    // ===== 处理参数回调 =====
    void onHeightValueChanged(double dValue);   // 功能：延拓高度变化（同步成员变量）
    void onPlaneNumsValueChanged(int nValue);   // 功能：切割平面数变化（重建切割平面高程表格）
    void onIterErrorValueChanged(double dValue);    // 功能：迭代均方差变化（同步成员变量）
    void onIterFactorValueChanged(double dValue);   // 功能：迭代因子变化（同步成员变量）
    void onHMinEdited();                    // 功能：高程最小值输入完成（重建切割平面高程表格）
    void onHMaxEdited();                    // 功能：高程最大值输入完成（重建切割平面高程表格）
    void onPlaneTableClicked();             // 功能：点击切割平面高程表格（空行填入默认高程）

    // ===== 确定 / 取消 =====
    void onOkClicked();                         // 功能：“确定”——校验参数并调用后端处理
    void onCancelClicked();                     // 功能：“取消”——关闭对话框

private:
    // ===== 界面初始化 =====
    void initUi();                              // 功能：按 .rc 布局创建全部控件并连接信号槽

    // ===== 数据加载 =====
    void loadFile(const QString& strFilePath);              // 功能：读取重磁场值文件头并刷新网格信息、默认值
    void loadDemFile(const QString& strFilePath);           // 功能：读取地形高程文件头并校验一致性、刷新高程范围
    void updateDefaultSavePaths(const QString& strFilePath);// 功能：生成默认输出文件名
    void updateGridInfoDisplay();               // 功能：把网格信息成员变量刷到界面显示

    // ===== 切割平面高程表格 =====
    void rebuildPlaneTable();                   // 功能：按切割平面数与高程范围重建表格行并填入默认高程
    bool readPlaneTable(QString& strError);     // 功能：读取表格各平面高程并校验（对应原工程 OnOK 前的表格读取）

    // ===== 文件选择 / 显示 =====
    QString askOpenFilePath(const QString& strTitle);           // 功能：弹出打开文件对话框
    QString askSaveFilePath(const QString& strTitle, const QString& strDefaultPath);  // 功能：弹出保存文件对话框
    void viewGridFile(const QString& strFilePath);              // 功能：检查文件存在并发出发送显示请求

    // ===== 处理 =====
    bool validateInputs(QString& strError);     // 功能：确定前校验输入项，返回 false 表示校验失败
    void runProcess();                          // 功能：组装参数并调用后端 processIterDrape

    // ===== 工具 =====
    void setSpinExtendSize(QSpinBox* pSpin, int nSize);     // 功能：同步设置扩边微调框下限与数值

    // ===== 界面控件（重磁场值数据文件输入）=====
    QLineEdit*      mEditOpenFile;      // 重磁场值输入数据文件路径（可编辑文本，对应原 IDC_OpenFile）
    QPushButton*    mBtnOpenFile;       // 重磁场值输入文件“...”按钮
    QPushButton*    mBtnOpenFileView;   // 重磁场值输入文件“显示”按钮

    // ===== 界面控件（地形高程数据文件输入）=====
    QLineEdit*      mEditOpenDemFile;   // 地形高程输入数据文件路径（可编辑文本，对应原 IDC_OpenDEMFile）
    QPushButton*    mBtnOpenDemFile;    // 地形高程输入文件“...”按钮
    QPushButton*    mBtnOpenDemFileView;// 地形高程输入文件“显示”按钮

    // ===== 界面控件（网格数据信息，只读显示 + 高程范围）=====
    QLineEdit*  mEditRowsNum;       // 网格行数（只读，对应原 IDC_RowsNum）
    QLineEdit*  mEditColsNum;       // 网格列数（只读，对应原 IDC_ColsNum）
    QLineEdit*  mEditRowStep;       // 网格行距（只读，对应原 IDC_RowStep）
    QLineEdit*  mEditColStep;       // 网格列距（只读，对应原 IDC_ColStep）
    QLineEdit*  mEditHMin;          // 高程最小值（可编辑，对应原 IDC_hMin，读地形文件后默认地形 z 最小值）
    QLineEdit*  mEditHMax;          // 高程最大值（可编辑，对应原 IDC_hMax，读地形文件后默认地形 z 最大值）

    // ===== 界面控件（数据扩边信息）=====
    QSpinBox*   mSpinExRows;     // 扩边行数微调框（默认值为 2 的幂）
    QSpinBox*   mSpinExCols;     // 扩边列数微调框（默认值为 2 的幂）

    // ===== 界面控件（数据扩边方法，单选，仅 3 种）=====
    QRadioButton*   mRadioCosFun;    // 余弦函数衰减（默认选中）
    QRadioButton*   mRadioAvgDif;    // 平均值差分
    QRadioButton*   mRadioInvPow;    // 反距离加权
    QButtonGroup*   mButtonGroupExpand;  // 扩边方法单选按钮组（id 对应 ExpandMethod 枚举）

    // ===== 界面控件（网格数据信息：延拓与迭代参数）=====
    QDoubleSpinBox* mSpinHeight;     // 延拓高度（范围 0~999999，读文件后默认 1 倍列距；原工程该字段不参与计算，保留界面）
    QSpinBox*       mSpinPlaneNums;  // 切割平面数（范围 1~99，默认 5，对应原 IDC_PlaneNums）
    QDoubleSpinBox* mSpinIterError;  // 迭代均方差（范围 0~9999，默认 0.5，对应原 IDC_IterError）
    QDoubleSpinBox* mSpinIterFactor; // 迭代因子（范围 0~10，默认 1.5，对应原 IDC_IterFactor）

    // ===== 界面控件（切割平面高程表格，对应原 MSFlexGrid）=====
    QTableWidget*   mTablePlanes;    // 切割平面高程表格（2 列：平面序号 / 平面高程，行数 = 切割平面数 + 1）

    // ===== 界面控件（曲化平结果数据文件输出）=====
    QLineEdit*  mEditSaveFile;       // 曲化平结果数据文件名（默认 基准名 + "_Pro.grd"）
    QPushButton* mBtnSaveFile;       // 输出文件“...”按钮
    QPushButton* mBtnSaveFileView;   // 输出文件“显示”按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // “确定”按钮（校验并调用后端处理）
    QPushButton* mBtnCancel;         // “取消”按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mInputFilePath;          // 当前重磁场值输入数据文件路径（对应原 m_strOpenFile）
    QString mDemFilePath;            // 当前地形高程输入数据文件路径（对应原 m_strOpenDEMFile）
    QString mOutputFilePath;         // 曲化平结果输出文件路径（对应原 m_strSaveFile）
    QString mLoadedFilePath;         // 最近一次成功读取文件头的重磁场文件路径（防止重复加载）
    QString mLoadedDemFilePath;      // 最近一次成功读取文件头的地形文件路径（防止重复加载）

    GridFileHead mGridHead;          // 重磁场值网格文件头缓存（与地形文件一致性校验用）
    GridFileHead mDemHead;           // 地形高程网格文件头缓存（与重磁场文件一致性校验用）

    int     mRowsNum;       // 网格行数（重磁场，原 m_nRowsNum）
    int     mColsNum;       // 网格列数（重磁场，原 m_nColsNum）
    double  mColStep;       // 网格列距（重磁场，原 m_fColStep）
    double  mRowStep;       // 网格行距（重磁场，原 m_fRowStep）
    double  mHMin;          // 高程最小值（原 m_fHMin）
    double  mHMax;          // 高程最大值（原 m_fHMax）

    int     mExRows;        // 当前扩边行数（原 m_nExRows）
    int     mExCols;        // 当前扩边列数（原 m_nExCols）
    int     mMinExRows;     // 最小扩边行数（读文件时计算，原 m_nMinExRows）
    int     mMinExCols;     // 最小扩边列数（读文件时计算，原 m_nMinExCols）

    double  mHeight;        // 延拓高度（原 m_fHeight，读文件后默认 1 倍列距）
    int     mPlaneNums;     // 切割平面数（原 m_nPlaneNums）
    double  mIterError;     // 迭代均方差（原 m_fIterError）
    double  mIterFactor;    // 迭代因子（原 m_fIterFactor）
    int     mExpandMethod;  // 数据扩边方法（对应 ExpandMethod 枚举，原单选按钮组）

};

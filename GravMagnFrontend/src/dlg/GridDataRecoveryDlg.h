// GridDataRecoveryDlg.h : 网格数据空白区还原对话框（Qt 前端）
//
// 对应原 MFC 工程：CGridDataRecoveryDlg（IDD_GridDataRecovery）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_GridDataRecovery 绘制：
//   还原数据输入 / 带空区的网格数据输入 / 网格数据信息 / 还原后的网格数据输出
//
// 前端职责（本对话框只做界面与交互，算法/数据读写在后端）：
//   1. 还原数据文件、带空区网格数据文件的选择与网格信息显示；
//   2. 加载时校验两文件行列数/坐标范围一致性（与原工程 ReadMissingData 一致，
//      “输入数据没有空白区”需数据体统计，由后端处理时校验）；
//   3. “确定”时校验输入，并调用后端接口 processGridDataRecovery 完成还原；
//   4. “显示”按钮请求查看等值线（通过 viewGridFileRequested 信号交由宿主处理）。
//
// 说明：后端接口（processGridDataRecovery）尚未由后端提供，本对话框当前仅实现
//       界面与参数校验，处理调用点以 TODO(后端对接) 预留（与三维重力/磁异常相关
//       成像对话框的处理方式一致）。

#pragma once

#include <QDialog>

// 前置声明（减少头文件之间的相互包含）
class QLineEdit;
class QPushButton;

// 网格数据空白区还原对话框
class CGridDataRecoveryDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CGridDataRecoveryDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CGridDataRecoveryDlg() override;

signals:
    // 功能：请求宿主显示指定网格文件的等值线图（对应原 MFC 工程“显示”按钮
    //       弹出的 CDrawOpenGLContourDlg，等值线显示窗口由前端后续版本实现）
    void viewGridFileRequested(const QString& strFilePath);

private slots:
    // ===== 输入文件回调（还原数据）=====
    void onOpenFileClicked();               // 功能：“...”选择还原数据文件
    void onOpenFileViewClicked();           // 功能：显示还原数据文件等值线
    void onOpenFileTextEdited(const QString& strText);  // 功能：手动输入还原数据文件名（同步成员变量）

    // ===== 输入文件回调（带空区的网格数据）=====
    void onOpenMissingFileClicked();        // 功能：“...”选择带空区网格数据文件
    void onOpenMissingFileViewClicked();    // 功能：显示带空区网格数据文件等值线
    void onOpenMissingFileTextEdited(const QString& strText);  // 功能：手动输入带空区文件名（同步成员变量）

    // ===== 输出文件回调 =====
    void onSaveFileClicked();               // 功能：“...”选择还原结果数据文件
    void onSaveFileViewClicked();           // 功能：显示还原结果文件等值线
    void onSaveFileTextEdited(const QString& strText);  // 功能：手动输入输出文件名（同步成员变量）

    // ===== 确定 / 取消 =====
    void onOkClicked();                         // 功能：“确定”——校验参数并调用后端处理
    void onCancelClicked();                     // 功能：“取消”——关闭对话框

private:
    // ===== 界面初始化 =====
    void initUi();                              // 功能：按 .rc 布局创建全部控件并连接信号槽

    // ===== 数据加载 =====
    void loadFile(const QString& strFilePath);              // 功能：读取还原数据文件头并刷新网格信息、默认输出名
    void loadMissingFile(const QString& strFilePath);       // 功能：读取带空区文件头并做一致性校验
    void checkHeadConsistency();                // 功能：两个输入文件头一致性校验（行列数/坐标范围）
    void updateGridInfoDisplay();               // 功能：把网格信息成员变量刷到界面显示

    // ===== 文件选择 / 显示 =====
    QString askOpenFilePath(const QString& strTitle);       // 功能：弹出打开文件对话框
    QString askSaveFilePath(const QString& strTitle, const QString& strDefaultPath);  // 功能：弹出保存文件对话框
    void viewGridFile(const QString& strFilePath);          // 功能：检查文件存在并发出发送显示请求

    // ===== 处理 =====
    bool validateInputs(QString& strError);     // 功能：确定前校验输入项，返回 false 表示校验失败
    void runProcess();                          // 功能：组装参数并调用后端 processGridDataRecovery

    // ===== 界面控件（还原数据输入）=====
    QLineEdit*      mEditOpenFile;      // 还原数据文件路径（可编辑文本，对应原 IDC_OpenFile）
    QPushButton*    mBtnOpenFile;       // 还原数据“...”按钮
    QPushButton*    mBtnOpenFileView;   // 还原数据“显示”按钮

    // ===== 界面控件（带空区的网格数据输入）=====
    QLineEdit*      mEditOpenMissingFile;   // 带空区网格数据文件路径（对应原 IDC_OpenMissingFile）
    QPushButton*    mBtnOpenMissingFile;    // 带空区文件“...”按钮
    QPushButton*    mBtnOpenMissingFileView;// 带空区文件“显示”按钮

    // ===== 界面控件（网格数据信息，只读显示）=====
    QLineEdit*  mEditRowsNum;       // 网格行数（只读）
    QLineEdit*  mEditColsNum;       // 网格列数（只读）
    QLineEdit*  mEditRowStep;       // 网格行距（只读）
    QLineEdit*  mEditColStep;       // 网格列距（只读）

    // ===== 界面控件（还原后的网格数据输出）=====
    QLineEdit*  mEditSaveFile;       // 还原结果数据文件名（默认 基准名 + "_Pro.grd"）
    QPushButton* mBtnSaveFile;       // 输出文件“...”按钮
    QPushButton* mBtnSaveFileView;   // 输出文件“显示”按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // “确定”按钮（校验并调用后端处理）
    QPushButton* mBtnCancel;         // “取消”按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mInputFilePath;          // 还原数据文件路径（对应原 m_strOpenFile）
    QString mMissingFilePath;        // 带空区网格数据文件路径（对应原 m_strOpenMissingFile）
    QString mOutputFilePath;         // 还原结果输出文件路径（对应原 m_strSaveFile）
    QString mLoadedFilePath;         // 最近一次成功读取文件头的还原数据文件路径（防止重复加载）
    QString mLoadedMissingFilePath;  // 最近一次成功读取文件头的空区数据文件路径（防止重复加载）

    int     mRowsNum;       // 网格行数（原 m_nRowsNum）
    int     mColsNum;       // 网格列数（原 m_nColsNum）
    double  mColStep;       // 网格列距（原 m_fColStep）
    double  mRowStep;       // 网格行距（原 m_fRowStep）

    // 两个输入文件头的缓存（用于行列数/坐标范围一致性校验，对应原工程 m_obFile1/m_obFile2 文件头）
    bool    mHasDataHead;       // 还原数据文件头是否已读取
    bool    mHasMissingHead;    // 带空区文件头是否已读取
    int     mDataRows;          // 还原数据行数缓存
    int     mDataCols;          // 还原数据列数缓存
    double  mDataXMin;          // 还原数据 X 最小值缓存
    double  mDataXMax;          // 还原数据 X 最大值缓存
    double  mDataYMin;          // 还原数据 Y 最小值缓存
    double  mDataYMax;          // 还原数据 Y 最大值缓存
    int     mMissingRows;       // 带空区数据行数缓存
    int     mMissingCols;       // 带空区数据列数缓存
    double  mMissingXMin;       // 带空区数据 X 最小值缓存
    double  mMissingXMax;       // 带空区数据 X 最大值缓存
    double  mMissingYMin;       // 带空区数据 Y 最小值缓存
    double  mMissingYMax;       // 带空区数据 Y 最大值缓存
};

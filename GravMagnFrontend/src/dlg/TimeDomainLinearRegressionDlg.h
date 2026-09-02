// TimeDomainLinearRegressionDlg.h : 一元线性回归分析对话框（Qt 前端）
//
// 对应原 MFC 工程：CTimeDomainLinearRegressionDlg（IDD_TimeDomainLinearRegression）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_TimeDomainLinearRegression 绘制：
//   打开数据文件 / 文件数据内容 / 回归分析数据选择 / 图形显示(画布) / 一元线性回归方程 / 回归计算/退出
//
// 前端职责（本对话框为纯源码可独立重写功能，无后端算法接口，仅前端实现）：
//   1. 打开表格数据文件（首行列名、后续数值，分隔符空格/逗号/|/Tab）；
//   2. 文件数据内容以表格展示，X 轴/Y 轴下拉选择参与回归的列；
//   3. 点击“回归计算”用最小二乘求斜率/截距，显示回归方程并绘制散点与回归线；
//   4. 无输出文件，纯图形/结果展示（对应原工程）。

#pragma once

#include <QDialog>
#include <QVector>

// 前置声明（减少头文件之间的相互包含）
class QLineEdit;
class QPushButton;
class QComboBox;
class QTableWidget;

// 散点/回归线画布控件（对应原工程“图形显示”分组内的绘图区）
class CRegressionPlotWidget : public QWidget
{
    Q_OBJECT

public:
    // 构造函数
    explicit CRegressionPlotWidget(QWidget* pParent = nullptr);

    // 功能：设置要绘制的散点数据与回归线（x/y 为点值，k/b 为回归线系数）
    // 参数：xValues/yValues —— 参与回归的点列；k/b —— 回归斜率/截距
    void setPlotData(const QVector<double>& xValues, const QVector<double>& yValues, double k, double b);

protected:
    // 功能：绘制散点与回归线（QPainter 绘制，数据坐标映射到控件坐标）
    void paintEvent(QPaintEvent* pEvent) override;

private:
    QVector<double> mxValues;   // 参与回归的 X 值
    QVector<double> myValues;   // 参与回归的 Y 值
    double  mk;                 // 回归斜率
    double  mb;                 // 回归截距
    bool    mHasData;           // 是否已设置绘图数据
};

// 一元线性回归分析对话框
class CTimeDomainLinearRegressionDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CTimeDomainLinearRegressionDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CTimeDomainLinearRegressionDlg() override;

private slots:
    // ===== 文件 / 计算回调 =====
    void onOpenFileClicked();               // 功能：“...”选择数据文件并读入表格
    void onCalcClicked();                   // 功能：点击“回归计算”——最小二乘求 k/b 并显示
    void onExitClicked();                   // 功能：点击“退出”按钮关闭对话框

private:
    // ===== 界面初始化 =====
    void initUi();                          // 功能：按 .rc 布局创建全部控件并连接信号槽

    // ===== 数据加载 =====
    bool readTableFile(const QString& strFilePath, QString& strError);  // 功能：读取表格数据文件（列名+数值）

    // ===== 工具 =====
    QString askOpenFilePath(const QString& strTitle);   // 功能：弹出打开文件对话框

    // ===== 界面控件（打开数据文件）=====
    QLineEdit*  mEditFileName;      // 数据文件路径（对应原 IDC_FILENAME_EDIT）
    QPushButton* mBtnOpenFile;      // “...”选择文件按钮（对应原 IDC_OPENFILE_BUTTON）

    // ===== 界面控件（文件数据内容）=====
    QTableWidget* mTableData;       // 数据表格（对应原 IDC_DATALIST_LIST）

    // ===== 界面控件（回归分析数据选择）=====
    QComboBox*  mComboX;            // X 轴数据列下拉（对应原 IDC_XAXIS_COMBO）
    QComboBox*  mComboY;            // Y 轴数据列下拉（对应原 IDC_YAXIS_COMBO）

    // ===== 界面控件（图形显示）=====
    CRegressionPlotWidget* mPlot;   // 散点/回归线画布（对应原“图形显示”分组）

    // ===== 界面控件（一元线性回归方程）=====
    QLineEdit*  mEditExpression;    // 回归方程显示（对应原 IDC_EXPRESSION_EDIT）

    // ===== 界面控件（回归计算 / 退出）=====
    QPushButton* mBtnCalc;          // “回归计算”默认按钮（对应原 IDC_CALA_BUTTON）
    QPushButton* mBtnExit;          // “退出”按钮（对应原 IDCANCEL）

    // ===== 状态数据 =====
    QStringList mColNames;              // 文件各列名称（表头）
    QVector<QVector<double> > mDataRows;// 文件数据行（每行各列数值）
};

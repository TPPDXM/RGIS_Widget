// TimeDomainLinearRegressionDlg.cpp : 一元线性回归分析对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_TimeDomainLinearRegression；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）；
//   * 本对话框为纯源码可独立重写功能，数据读取/回归算法均在前端实现，不涉及后端。

#include "TimeDomainLinearRegressionDlg.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>

// ================= 散点/回归线画布控件 =================

// 构造函数
CRegressionPlotWidget::CRegressionPlotWidget(QWidget* pParent)
    : QWidget(pParent)
    , mk(0.0)
    , mb(0.0)
    , mHasData(false)
{
    // 画布背景色，与原工程图形显示区一致
    setAutoFillBackground(true);
}

// 功能：设置要绘制的散点数据与回归线
void CRegressionPlotWidget::setPlotData(const QVector<double>& xValues, const QVector<double>& yValues,
                                        double k, double b)
{
    mxValues = xValues;
    myValues = yValues;
    mk = k;
    mb = b;
    mHasData = (xValues.size() > 0);
    update();
}

// 功能：绘制散点与回归线
void CRegressionPlotWidget::paintEvent(QPaintEvent* pEvent)
{
    (void)pEvent;   // 参数暂不使用（绘制范围固定为控件矩形）
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 背景
    painter.fillRect(rect(), QColor(255, 255, 255));

    // 边框
    painter.setPen(QColor(160, 160, 160));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    if (!mHasData || mxValues.size() <= 0)
    {
        painter.setPen(QColor(90, 90, 90));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("请先读取数据并选择 X/Y 轴后再回归计算"));
        return;
    }

    // 计算数据范围（含回归线端点）
    double xMin = mxValues.at(0), xMax = mxValues.at(0);
    double yMin = myValues.at(0), yMax = myValues.at(0);
    for (int i = 1; i < mxValues.size(); i++)
    {
        if (mxValues.at(i) < xMin) { xMin = mxValues.at(i); }
        if (mxValues.at(i) > xMax) { xMax = mxValues.at(i); }
        if (myValues.at(i) < yMin) { yMin = myValues.at(i); }
        if (myValues.at(i) > yMax) { yMax = myValues.at(i); }
    }
    // 把回归线端点纳入范围，避免超出绘图区
    double regX0 = xMin, regX1 = xMax;
    double regY0 = mk * regX0 + mb;
    double regY1 = mk * regX1 + mb;
    if (regY0 < yMin) { yMin = regY0; }
    if (regY0 > yMax) { yMax = regY0; }
    if (regY1 < yMin) { yMin = regY1; }
    if (regY1 > yMax) { yMax = regY1; }

    double spanX = (xMax - xMin);
    double spanY = (yMax - yMin);
    if (spanX <= 0.0) { spanX = 1.0; }
    if (spanY <= 0.0) { spanY = 1.0; }

    // 留边距
    int w = width();
    int h = height();
    double margin = 24.0;
    double plotW = w - margin * 2.0;
    double plotH = h - margin * 2.0;

    // 数据坐标 -> 像素（Y 轴翻转：数据大在上）
    auto mapX = [&](double x) -> double { return margin + (x - xMin) / spanX * plotW; };
    auto mapY = [&](double y) -> double { return margin + (yMax - y) / spanY * plotH; };

    // 散点
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(60, 120, 200));
    for (int i = 0; i < mxValues.size(); i++)
    {
        painter.drawEllipse(QPointF(mapX(mxValues.at(i)), mapY(myValues.at(i))), 3.0, 3.0);
    }

    // 回归线
    painter.setPen(QPen(QColor(220, 60, 60), 2));
    painter.drawLine(QPointF(mapX(regX0), mapY(regY0)), QPointF(mapX(regX1), mapY(regY1)));

    // 坐标范围标注
    painter.setPen(QColor(90, 90, 90));
    painter.drawText(QRectF(0, h - 22, w, 20), Qt::AlignRight,
        QStringLiteral("x:[%1, %2] y:[%3, %4]")
            .arg(xMin, 0, 'f', 3).arg(xMax, 0, 'f', 3).arg(yMin, 0, 'f', 3).arg(yMax, 0, 'f', 3));
}

// ================= 对话框 =================

// 构造函数：初始化全部成员并创建界面
CTimeDomainLinearRegressionDlg::CTimeDomainLinearRegressionDlg(QWidget* pParent)
    : QDialog(pParent)
    , mEditFileName(NULL)
    , mBtnOpenFile(NULL)
    , mTableData(NULL)
    , mComboX(NULL)
    , mComboY(NULL)
    , mPlot(NULL)
    , mEditExpression(NULL)
    , mBtnCalc(NULL)
    , mBtnExit(NULL)
{
    initUi();
}

// 析构函数
CTimeDomainLinearRegressionDlg::~CTimeDomainLinearRegressionDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CTimeDomainLinearRegressionDlg::initUi()
{
    setWindowTitle(QStringLiteral("一元线性回归分析"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 打开数据文件 =================
    QGroupBox* pGroupOpen = new QGroupBox(QStringLiteral("打开数据文件"), this);
    QGridLayout* pOpenLayout = new QGridLayout(pGroupOpen);
    mEditFileName = new QLineEdit(pGroupOpen);
    mBtnOpenFile = new QPushButton(QStringLiteral("..."), pGroupOpen);
    mBtnOpenFile->setFixedWidth(32);
    pOpenLayout->addWidget(mEditFileName, 0, 0);
    pOpenLayout->addWidget(mBtnOpenFile, 0, 1);
    pOpenLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupOpen);

    // ================= 文件数据内容（表格）=================
    QGroupBox* pGroupTable = new QGroupBox(QStringLiteral("文件数据内容"), this);
    QHBoxLayout* pTableLayout = new QHBoxLayout(pGroupTable);
    mTableData = new QTableWidget(pGroupTable);
    mTableData->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTableData->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableData->setMinimumHeight(120);
    pTableLayout->addWidget(mTableData);
    pMainLayout->addWidget(pGroupTable);

    // ================= 回归分析数据选择 =================
    QGroupBox* pGroupSel = new QGroupBox(QStringLiteral("回归分析数据选择"), this);
    QGridLayout* pSelLayout = new QGridLayout(pGroupSel);
    pSelLayout->addWidget(new QLabel(QStringLiteral("X 轴数据"), pGroupSel), 0, 0);
    mComboX = new QComboBox(pGroupSel);
    pSelLayout->addWidget(mComboX, 0, 1);
    pSelLayout->addWidget(new QLabel(QStringLiteral("Y 轴数据"), pGroupSel), 0, 2);
    mComboY = new QComboBox(pGroupSel);
    pSelLayout->addWidget(mComboY, 0, 3);
    pSelLayout->setColumnStretch(1, 1);
    pSelLayout->setColumnStretch(3, 1);
    pMainLayout->addWidget(pGroupSel);

    // ================= 图形显示（画布）=================
    QGroupBox* pGroupPlot = new QGroupBox(QStringLiteral("图形显示"), this);
    QHBoxLayout* pPlotLayout = new QHBoxLayout(pGroupPlot);
    mPlot = new CRegressionPlotWidget(pGroupPlot);
    mPlot->setMinimumSize(240, 180);
    pPlotLayout->addWidget(mPlot);
    pMainLayout->addWidget(pGroupPlot);

    // ================= 一元线性回归方程 =================
    QGroupBox* pGroupExpr = new QGroupBox(QStringLiteral("一元线性回归方程"), this);
    QHBoxLayout* pExprLayout = new QHBoxLayout(pGroupExpr);
    mEditExpression = new QLineEdit(pGroupExpr);
    mEditExpression->setReadOnly(true);
    mEditExpression->setStyleSheet(QStringLiteral("border:none;"));
    pExprLayout->addWidget(mEditExpression);
    pMainLayout->addWidget(pGroupExpr);

    // ================= 回归计算 / 退出 =================
    QHBoxLayout* pButtonLayout = new QHBoxLayout();
    mBtnCalc = new QPushButton(QStringLiteral("回归计算"), this);
    mBtnExit = new QPushButton(QStringLiteral("退出"), this);
    mBtnCalc->setDefault(true);
    pButtonLayout->addWidget(mBtnCalc);
    pButtonLayout->addStretch(1);
    pButtonLayout->addWidget(mBtnExit);
    pMainLayout->addLayout(pButtonLayout);

    // ================= 信号槽连接（全部使用成员函数引用，不使用 lambda）=================
    for (QGroupBox* pGroup : findChildren<QGroupBox*>())
    {
        pGroup->setAlignment(Qt::AlignCenter);
    }

    connect(mBtnOpenFile, &QPushButton::clicked, this, &CTimeDomainLinearRegressionDlg::onOpenFileClicked);
    connect(mBtnCalc, &QPushButton::clicked, this, &CTimeDomainLinearRegressionDlg::onCalcClicked);
    connect(mBtnExit, &QPushButton::clicked, this, &CTimeDomainLinearRegressionDlg::onExitClicked);

    setMinimumWidth(470);
    adjustSize();
}

// 功能：选择数据文件并读入表格（对应原工程 wLoadDataFile）
void CTimeDomainLinearRegressionDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }

    QString strError;
    if (!readTableFile(strFilePath, strError))
    {
        QMessageBox::warning(this, QStringLiteral("读取数据文件错误"), strError);
        return;
    }

    if (mEditFileName != NULL)
    {
        mEditFileName->setText(strFilePath);
    }
}

// 功能：读取表格数据文件（首行列名，后续数值；分隔符空格/逗号/|/Tab）
bool CTimeDomainLinearRegressionDlg::readTableFile(const QString& strFilePath, QString& strError)
{
    if (strFilePath.isEmpty())
    {
        strError = QStringLiteral("读取数据文件路径错误。");
        return false;
    }

    QFile file(strFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        strError = QStringLiteral("读取数据文件错误。");
        return false;
    }

    QTextStream in(&file);
    // 读取表头列名（第一个非空行）
    QStringList colNames;
    QVector<QVector<double> > dataRows;
    bool bHeaderFound = false;
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
        {
            continue;
        }
        // 统一把逗号/|/Tab/空格折叠为分隔
        QString normalized = line;
        normalized.replace(QChar(','), QChar(' '));
        normalized.replace(QChar('|'), QChar(' '));
        normalized.replace(QChar('\t'), QChar(' '));
        QStringList parts = normalized.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (!bHeaderFound)
        {
            colNames = parts;              // 首行为列名
            bHeaderFound = true;
            continue;
        }
        QVector<double> row;
        for (int k = 0; k < parts.size(); k++)
        {
            row.append(parts.at(k).toDouble());
        }
        dataRows.append(row);
    }
    file.close();

    if (!bHeaderFound || colNames.size() <= 0)
    {
        strError = QStringLiteral("读取数据文件错误：未找到表头列名。");
        return false;
    }

    mColNames = colNames;
    mDataRows = dataRows;

    // 填充表格（列 = 列名，行 = 数据行）
    mTableData->clear();
    mTableData->setColumnCount(colNames.size());
    mTableData->setHorizontalHeaderLabels(colNames);
    mTableData->setRowCount(dataRows.size());
    for (int r = 0; r < dataRows.size(); r++)
    {
        for (int c = 0; c < colNames.size(); c++)
        {
            QString strText;
            if (c < dataRows.at(r).size())
            {
                strText = QString::number(dataRows.at(r).at(c), 'f', 6);
            }
            QTableWidgetItem* pItem = new QTableWidgetItem(strText);
            pItem->setTextAlignment(Qt::AlignCenter);
            mTableData->setItem(r, c, pItem);
        }
    }
    mTableData->resizeColumnsToContents();

    // 填充 X/Y 轴下拉（文字 = 列名，userData = 列索引，避免排序后序号错位）
    mComboX->clear();
    mComboY->clear();
    for (int c = 0; c < colNames.size(); c++)
    {
        mComboX->addItem(colNames.at(c), c);
        mComboY->addItem(colNames.at(c), c);
    }
    if (mComboX->count() > 1) { mComboX->setCurrentIndex(0); }
    if (mComboY->count() > 1) { mComboY->setCurrentIndex(1); }

    // 清空回归方程与画布
    if (mEditExpression != NULL)
    {
        mEditExpression->clear();
    }
    if (mPlot != NULL)
    {
        QVector<double> empty;
        mPlot->setPlotData(empty, empty, 0.0, 0.0);
    }
    return true;
}

// 功能：点击“回归计算”——最小二乘求斜率/截距并显示（对应原工程 OnCalaButton）
void CTimeDomainLinearRegressionDlg::onCalcClicked()
{
    // 取 X/Y 列索引（userData）
    int xIndex = mComboX->currentData().toInt();
    int yIndex = mComboY->currentData().toInt();
    if (xIndex == yIndex)
    {
        QMessageBox::warning(this, QStringLiteral("参数错误"), QStringLiteral("X 轴与 Y 轴数据不能相同。"));
        return;
    }
    if (mDataRows.size() < 2)
    {
        QMessageBox::warning(this, QStringLiteral("参数错误"), QStringLiteral("参与回归的数据点数不足。"));
        return;
    }

    // 提取 X/Y 列有效数据
    QVector<double> xValues;
    QVector<double> yValues;
    for (int r = 0; r < mDataRows.size(); r++)
    {
        const QVector<double>& row = mDataRows.at(r);
        if (xIndex < row.size() && yIndex < row.size())
        {
            xValues.append(row.at(xIndex));
            yValues.append(row.at(yIndex));
        }
    }

    int n = xValues.size();
    if (n < 2)
    {
        QMessageBox::warning(this, QStringLiteral("参数错误"), QStringLiteral("参与回归的数据点数不足。"));
        return;
    }

    // 最小二乘：k=(n·Σxy−Σx·Σy)/(n·Σx²−Σx·Σx) ，b=(Σy−k·Σx)/n
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    for (int i = 0; i < n; i++)
    {
        double x = xValues.at(i);
        double y = yValues.at(i);
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    double denom = n * sumX2 - sumX * sumX;
    if (denom == 0.0)
    {
        QMessageBox::warning(this, QStringLiteral("参数错误"), QStringLiteral("X 轴数据方差为 0，无法回归。"));
        return;
    }
    double k = (n * sumXY - sumX * sumY) / denom;
    double b = (sumY - k * sumX) / n;

    // 显示方程（对应原工程：b>=0 用 +，否则用 -，系数 6 位小数）
    QString strEquation;
    if (b >= 0.0)
    {
        strEquation = QStringLiteral("Y = ") + QString::number(k, 'f', 6)
            + QStringLiteral(" X + ") + QString::number(b, 'f', 6);
    }
    else
    {
        strEquation = QStringLiteral("Y = ") + QString::number(k, 'f', 6)
            + QStringLiteral(" X - ") + QString::number(-b, 'f', 6);
    }
    if (mEditExpression != NULL)
    {
        mEditExpression->setText(strEquation);
    }
    // 更新画布（散点 + 回归线）
    if (mPlot != NULL)
    {
        mPlot->setPlotData(xValues, yValues, k, b);
    }
}

// 功能：点击“退出”按钮关闭对话框（对应原工程 OnCancel）
void CTimeDomainLinearRegressionDlg::onExitClicked()
{
    reject();
}

// 功能：弹出打开文件对话框（文本/表格数据文件）
QString CTimeDomainLinearRegressionDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("数据文件 (*.dat);;文本文件 (*.txt);;所有数据文件 (*.*)"));
}

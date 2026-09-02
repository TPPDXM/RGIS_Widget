// ProfDataInterpolationDlg.cpp : 剖面数据插值对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_ProfDataInterpolation；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）；
//   * 本对话框为纯源码可独立重写功能，剖面文件读取/插值均在前端实现，不涉及后端。

#include "ProfDataInterpolationDlg.h"

#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextStream>
#include <QVBoxLayout>

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CProfDataInterpolationDlg::CProfDataInterpolationDlg(QWidget* pParent)
    : QDialog(pParent)
    , mEditOpenFile(NULL)
    , mBtnOpenFile(NULL)
    , mEditXMin(NULL)
    , mEditXMax(NULL)
    , mEditPnts(NULL)
    , mEditZMin(NULL)
    , mEditZMax(NULL)
    , mEditStep(NULL)
    , mEditSaveFile(NULL)
    , mBtnSaveFile(NULL)
    , mBtnOk(NULL)
    , mBtnCancel(NULL)
    , mDataValid(false)
    , mPnts(0)
    , mXMin(0.0)
    , mXMax(0.0)
    , mZMin(0.0)
    , mZMax(0.0)
    , mStep(0.0)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CProfDataInterpolationDlg::~CProfDataInterpolationDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CProfDataInterpolationDlg::initUi()
{
    setWindowTitle(QStringLiteral("剖面数据插值"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 剖面数据文件输入 =================
    QGroupBox* pGroupInput = new QGroupBox(QStringLiteral("剖面数据文件输入"), this);
    QGridLayout* pInputLayout = new QGridLayout(pGroupInput);
    mEditOpenFile = new QLineEdit(pGroupInput);
    mBtnOpenFile = new QPushButton(QStringLiteral("..."), pGroupInput);
    mBtnOpenFile->setFixedWidth(32);
    pInputLayout->addWidget(mEditOpenFile, 0, 0);
    pInputLayout->addWidget(mBtnOpenFile, 0, 1);
    pInputLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupInput);

    // ================= 剖面数据信息（只读显示）=================
    // 对应 .rc 中的 IDC_XMin / IDC_XMax / IDC_Pnts
    QGroupBox* pGroupInfo = new QGroupBox(QStringLiteral("剖面数据信息"), this);
    QGridLayout* pInfoLayout = new QGridLayout(pGroupInfo);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("坐标最小值"), pGroupInfo), 0, 0);
    mEditXMin = new QLineEdit(pGroupInfo);
    mEditXMin->setReadOnly(true);
    mEditXMin->setAlignment(Qt::AlignCenter);
    mEditXMin->setFixedWidth(110);
    pInfoLayout->addWidget(mEditXMin, 0, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("坐标最大值"), pGroupInfo), 0, 2);
    mEditXMax = new QLineEdit(pGroupInfo);
    mEditXMax->setReadOnly(true);
    mEditXMax->setAlignment(Qt::AlignCenter);
    mEditXMax->setFixedWidth(110);
    pInfoLayout->addWidget(mEditXMax, 0, 3);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("测点点数"), pGroupInfo), 0, 4);
    mEditPnts = new QLineEdit(pGroupInfo);
    mEditPnts->setReadOnly(true);
    mEditPnts->setAlignment(Qt::AlignCenter);
    mEditPnts->setFixedWidth(110);
    pInfoLayout->addWidget(mEditPnts, 0, 5);
    pMainLayout->addWidget(pGroupInfo);

    // ================= 处理参数（可编辑）=================
    // 对应 .rc 中的 IDC_ZMin / IDC_ZMax / IDC_Step
    QGroupBox* pGroupParams = new QGroupBox(QStringLiteral("处理参数"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("坐标最小值"), pGroupParams), 0, 0);
    mEditZMin = new QLineEdit(pGroupParams);
    mEditZMin->setAlignment(Qt::AlignCenter);
    mEditZMin->setFixedWidth(110);
    pParamsLayout->addWidget(mEditZMin, 0, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("坐标最大值"), pGroupParams), 0, 2);
    mEditZMax = new QLineEdit(pGroupParams);
    mEditZMax->setAlignment(Qt::AlignCenter);
    mEditZMax->setFixedWidth(110);
    pParamsLayout->addWidget(mEditZMax, 0, 3);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("测点点距"), pGroupParams), 0, 4);
    mEditStep = new QLineEdit(pGroupParams);
    mEditStep->setAlignment(Qt::AlignCenter);
    mEditStep->setFixedWidth(110);
    pParamsLayout->addWidget(mEditStep, 0, 5);
    pMainLayout->addWidget(pGroupParams);

    // ================= 剖面数据文件输出 =================
    // 对应 .rc 中的 IDC_SaveFile / ID_SaveProfFile
    QGroupBox* pGroupOutput = new QGroupBox(QStringLiteral("剖面数据文件输出"), this);
    QGridLayout* pOutputLayout = new QGridLayout(pGroupOutput);
    mEditSaveFile = new QLineEdit(pGroupOutput);
    mBtnSaveFile = new QPushButton(QStringLiteral("..."), pGroupOutput);
    mBtnSaveFile->setFixedWidth(32);
    pOutputLayout->addWidget(mEditSaveFile, 0, 0);
    pOutputLayout->addWidget(mBtnSaveFile, 0, 1);
    pOutputLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupOutput);

    // ================= 确定 / 取消 =================
    QHBoxLayout* pButtonLayout = new QHBoxLayout();
    mBtnOk = new QPushButton(QStringLiteral("确  定"), this);
    mBtnCancel = new QPushButton(QStringLiteral("取  消"), this);
    mBtnOk->setDefault(true);
    pButtonLayout->addWidget(mBtnOk);
    pButtonLayout->addStretch(1);
    pButtonLayout->addWidget(mBtnCancel);
    pMainLayout->addLayout(pButtonLayout);

    // ================= 信号槽连接（全部使用成员函数引用，不使用 lambda）=================
    // 分组框标题居中（对应原工程 GROUPBOX 的 BS_CENTER 样式）
    for (QGroupBox* pGroup : findChildren<QGroupBox*>())
    {
        pGroup->setAlignment(Qt::AlignCenter);
    }

    // 输入 / 输出文件
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CProfDataInterpolationDlg::onOpenFileClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CProfDataInterpolationDlg::onOpenFileTextEdited);
    connect(mBtnSaveFile, &QPushButton::clicked, this, &CProfDataInterpolationDlg::onSaveFileClicked);
    connect(mEditSaveFile, &QLineEdit::textEdited, this, &CProfDataInterpolationDlg::onSaveFileTextEdited);

    // 处理参数
    connect(mEditZMin, &QLineEdit::textEdited, this, &CProfDataInterpolationDlg::onZMinEdited);
    connect(mEditZMax, &QLineEdit::textEdited, this, &CProfDataInterpolationDlg::onZMaxEdited);
    connect(mEditStep, &QLineEdit::textEdited, this, &CProfDataInterpolationDlg::onStepEdited);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CProfDataInterpolationDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CProfDataInterpolationDlg::onCancelClicked);

    // 对话框按内容固定尺寸
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：选择剖面数据文件（对应原工程 OnOpenProfFile）
void CProfDataInterpolationDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择剖面数据文件"));
    if (strFilePath.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("您还没有剖面数据文件!"));
        return;
    }

    QString strError;
    if (!loadFile(strFilePath, strError))
    {
        QMessageBox::warning(this, QStringLiteral("读取失败"), strError);
        return;
    }
}

// 功能：手动输入输入文件名（同步成员变量）
void CProfDataInterpolationDlg::onOpenFileTextEdited(const QString& strText)
{
    mOpenFilePath = strText;
    mDataValid = false;
}

// 功能：选择结果数据文件（对应原工程 OnSaveProfFile）
void CProfDataInterpolationDlg::onSaveFileClicked()
{
    QString strFilePath = askSaveFilePath(QStringLiteral("请输入处理结果数据文件名"), mSaveFilePath);
    if (strFilePath.isEmpty())
    {
        return;
    }
    mSaveFilePath = strFilePath;
    if (mEditSaveFile != NULL)
    {
        mEditSaveFile->setText(strFilePath);
    }
}

// 功能：手动输入输出文件名（同步成员变量）
void CProfDataInterpolationDlg::onSaveFileTextEdited(const QString& strText)
{
    mSaveFilePath = strText;
}

// 功能：坐标最小值变化（同步成员变量）
void CProfDataInterpolationDlg::onZMinEdited(const QString& strText)
{
    mZMin = strText.toDouble();
}

// 功能：坐标最大值变化（同步成员变量）
void CProfDataInterpolationDlg::onZMaxEdited(const QString& strText)
{
    mZMax = strText.toDouble();
}

// 功能：测点点距变化（同步成员变量）
void CProfDataInterpolationDlg::onStepEdited(const QString& strText)
{
    mStep = strText.toDouble();
}

// 功能：读取剖面文件并刷新信息、默认值（对应原工程 ReadData）
// 参数：strFilePath —— 剖面数据文件路径
// 参数：strError —— 失败时输出的错误描述
// 返回：true 成功；false 失败
bool CProfDataInterpolationDlg::loadFile(const QString& strFilePath, QString& strError)
{
    if (strFilePath.isEmpty())
    {
        strError = QStringLiteral("剖面数据文件路径为空。");
        return false;
    }
    // 与最近一次成功加载的文件相同则不再重复读取
    if (strFilePath == mLoadedFilePath && mDataValid)
    {
        return true;
    }

    QFile file(strFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        strError = QStringLiteral("打开") + strFilePath + QStringLiteral("文件错误!");
        return false;
    }

    // 逐行读取并用 [ ,\t-] 分隔（对应原工程 strtok 分隔符 ", -"）
    QTextStream in(&file);
    QVector<QVector<double> > rows;
    QRegularExpression re(QStringLiteral("[,\\s-]+"));
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
        {
            continue;
        }
        QStringList parts = line.split(re, Qt::SkipEmptyParts);
        if (parts.size() < 3)
        {
            continue;
        }
        QVector<double> row(3, 0.0);
        for (int k = 0; k < 3; k++)
        {
            row[k] = parts.at(k).toDouble();
        }
        rows.append(row);
    }
    file.close();

    int nRows = rows.size();
    if (nRows < 3)
    {
        strError = QStringLiteral("剖面数据 ") + strFilePath + QStringLiteral(" 不是三列数据（或数据行数不足）!");
        return false;
    }

    // 计算各列范围
    double xMin = rows.at(0).at(0);
    double xMax = rows.at(0).at(0);
    double zMin = rows.at(0).at(1);
    double zMax = rows.at(0).at(1);
    double vMin = rows.at(0).at(2);
    double vMax = rows.at(0).at(2);
    for (int i = 1; i < nRows; i++)
    {
        double x = rows.at(i).at(0);
        double z = rows.at(i).at(1);
        double v = rows.at(i).at(2);
        if (x < xMin) { xMin = x; }
        if (x > xMax) { xMax = x; }
        if (z < zMin) { zMin = z; }
        if (z > zMax) { zMax = z; }
        if (v < vMin) { vMin = v; }
        if (v > vMax) { vMax = v; }
    }

    // 等间距检查（对应原工程 Index2=0：相邻坐标差恒定则拒绝）
    double firstStep = rows.at(1).at(0) - rows.at(0).at(0);
    bool bEquidistant = true;
    for (int i = 2; i < nRows; i++)
    {
        double curStep = rows.at(i).at(0) - rows.at(i - 1).at(0);
        if (curStep < firstStep - 1e-6 || curStep > firstStep + 1e-6)
        {
            bEquidistant = false;
            break;
        }
    }
    if (bEquidistant)
    {
        strError = QStringLiteral("数据 ") + strFilePath + QStringLiteral(" 是等间距，无需插值!");
        return false;
    }

    // 保存数据到成员
    mCoord.clear();
    mVal1.clear();
    mVal2.clear();
    for (int i = 0; i < nRows; i++)
    {
        mCoord.append(rows.at(i).at(0));
        mVal1.append(rows.at(i).at(1));
        mVal2.append(rows.at(i).at(2));
    }

    mLoadedFilePath = strFilePath;
    mOpenFilePath = strFilePath;
    mDataValid = true;

    mPnts = nRows;
    mXMin = xMin;
    mXMax = xMax;
    mZMin = xMin;
    mZMax = xMax;
    mStep = (xMax - xMin) / (nRows - 1);

    // 刷新界面显示
    if (mEditOpenFile != NULL)
    {
        mEditOpenFile->setText(strFilePath);
    }
    if (mEditXMin != NULL) { mEditXMin->setText(QString::number(mXMin, 'f', 6)); }
    if (mEditXMax != NULL) { mEditXMax->setText(QString::number(mXMax, 'f', 6)); }
    if (mEditPnts != NULL) { mEditPnts->setText(QString::number(mPnts)); }
    if (mEditZMin != NULL) { mEditZMin->setText(QString::number(mZMin, 'f', 6)); }
    if (mEditZMax != NULL) { mEditZMax->setText(QString::number(mZMax, 'f', 6)); }
    if (mEditStep != NULL) { mEditStep->setText(QString::number(mStep, 'f', 6)); }

    // 默认输出文件名：<打开文件去扩展名>_Out.<原扩展名>
    int nPos = strFilePath.lastIndexOf('.');
    if (nPos > 0)
    {
        QString strBase = strFilePath.left(nPos);
        QString strExt = strFilePath.mid(nPos + 1);
        mSaveFilePath = strBase + QStringLiteral("_Out.") + strExt;
        if (mEditSaveFile != NULL)
        {
            mEditSaveFile->setText(mSaveFilePath);
        }
    }

    return true;
}

// 功能：弹出打开文件对话框（文本/剖面数据文件）
QString CProfDataInterpolationDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("文本文件 (*.dat);;文本文件 (*.txt);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框（文本/剖面数据文件）
QString CProfDataInterpolationDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath,
        QStringLiteral("文本文件 (*.dat);;文本文件 (*.txt);;所有数据文件 (*.*)"));
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的手工校验，并给出更明确的提示）
bool CProfDataInterpolationDlg::validateInputs(QString& strError)
{
    // 输入数据文件检查
    if (mOpenFilePath.isEmpty())
    {
        strError = QStringLiteral("请先选择剖面数据文件。");
        return false;
    }
    if (!QFile::exists(mOpenFilePath))
    {
        strError = QStringLiteral("剖面数据文件不存在：") + mOpenFilePath;
        return false;
    }
    // 数据有效性检查
    if (!mDataValid)
    {
        strError = QStringLiteral("剖面数据未成功读取，请重新选择文件。");
        return false;
    }
    // 输出文件检查
    if (mSaveFilePath.isEmpty())
    {
        strError = QStringLiteral("请输入剖面数据文件输出路径。");
        return false;
    }
    // 参数检查（对应原工程 OnOK）
    if (mStep <= 0.0)
    {
        strError = QStringLiteral("测点点距必须大于零！");
        return false;
    }
    if (mZMax < mZMin)
    {
        strError = QStringLiteral("测点坐标最小值和坐标最大值参数输入错误！");
        return false;
    }
    return true;
}

// 功能：按参数对剖面数据插值并写出结果文件（对应原工程 OnOK 主体逻辑）
void CProfDataInterpolationDlg::runInterpolate()
{
    // 输出点数
    double dRange = mZMax - mZMin;
    int rows = (int)(dRange / mStep) + 1;
    if (rows < 1)
    {
        rows = 1;
    }

    // 结果点序列（3 列）
    QVector<double> outCoord(rows, 0.0);
    QVector<double> outVal1(rows, 0.0);
    QVector<double> outVal2(rows, 0.0);

    int nPnts = mCoord.size();
    for (int i = 0; i < rows; i++)
    {
        double x = mZMin + i * mStep;
        outCoord[i] = x;

        // 越界外推取端点；否则线性插值（用正确的相邻点斜率，规避原工程两处分母/分子笔误）
        if (x <= mCoord.at(0))
        {
            outVal1[i] = mVal1.at(0);
            outVal2[i] = mVal2.at(0);
        }
        else if (x >= mCoord.at(nPnts - 1))
        {
            outVal1[i] = mVal1.at(nPnts - 1);
            outVal2[i] = mVal2.at(nPnts - 1);
        }
        else
        {
            int j = 0;
            // 找到插值区间（相邻两输入点）
            while (j < nPnts - 2 && x >= mCoord.at(j + 1))
            {
                j++;
            }
            double x0 = mCoord.at(j);
            double x1 = mCoord.at(j + 1);
            double dx = x1 - x0;
            if (dx != 0.0)
            {
                double t = (x - x0) / dx;
                outVal1[i] = mVal1.at(j) + (mVal1.at(j + 1) - mVal1.at(j)) * t;
                outVal2[i] = mVal2.at(j) + (mVal2.at(j + 1) - mVal2.at(j)) * t;
            }
            else
            {
                outVal1[i] = mVal1.at(j);
                outVal2[i] = mVal2.at(j);
            }
        }
    }

    // 写出结果（每行 3 值，%.6f 空格分隔，无表头，对应原工程 SaveProfData）
    QFile file(mSaveFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QStringLiteral("文件错误"),
            QStringLiteral("打开") + mSaveFilePath + QStringLiteral("文件错误!"));
        return;
    }
    QTextStream out(&file);
    for (int i = 0; i < rows; i++)
    {
        out << QString::number(outCoord[i], 'f', 6) << " ";
        out << QString::number(outVal1[i], 'f', 6) << " ";
        out << QString::number(outVal2[i], 'f', 6) << " \n";
    }
    file.close();

    QMessageBox::information(this, QStringLiteral("处理完成"), QStringLiteral("剖面数据插值计算结束!"));
}

// 功能：“确定”——校验参数并做插值处理（对应原工程 OnOK）
void CProfDataInterpolationDlg::onOkClicked()
{
    QString strError;
    if (!validateInputs(strError))
    {
        QMessageBox::warning(this, QStringLiteral("参数错误"), strError);
        return;
    }
    runInterpolate();
}

// 功能：“取消”——关闭对话框（对应原工程 OnCancel）
void CProfDataInterpolationDlg::onCancelClicked()
{
    reject();
}

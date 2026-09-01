// GravMidTerrainCorrectionDlg.cpp : 重力中区地形改正对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_GravMidTerrainCorrection；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）。

#include "GravMidTerrainCorrectionDlg.h"

#include <cmath>
#include <QApplication>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include "backend/RgisBackend.h"
#include "FrontendUtils.h"

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CGravMidTerrainCorrectionDlg::CGravMidTerrainCorrectionDlg(QWidget* pParent)
    : QDialog(pParent)
    , mEditTxtFile(NULL)
    , mBtnOpenTxtFile(NULL)
    , mEditGrdFile(NULL)
    , mBtnOpenGrdFile(NULL)
    , mBtnOpenGrdFileView(NULL)
    , mEditGravityNum(NULL)
    , mEditRows(NULL)
    , mEditCols(NULL)
    , mEditHStep(NULL)
    , mEditZMin(NULL)
    , mEditZMax(NULL)
    , mRadioOutRect(NULL)
    , mRadioOutCircle(NULL)
    , mRadioInRect(NULL)
    , mRadioInCircle(NULL)
    , mGroupOuter(NULL)
    , mGroupInner(NULL)
    , mSpinStartRadius(NULL)
    , mSpinEndRadius(NULL)
    , mSpinRockDensity(NULL)
    , mEditResFile(NULL)
    , mBtnOpenResFile(NULL)
    , mBtnOk(NULL)
    , mBtnCancel(NULL)
    , mRowsNum(0)
    , mColsNum(0)
    , mHstep(0.0)
    , mZMin(0.0)
    , mZMax(0.0)
    , mGravityNum(0)
    , mStartRadius(20.0)
    , mEndRadius(500.0)
    , mRockDensity(2.67)
    , mOuterShape(0)
    , mInnerShape(0)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CGravMidTerrainCorrectionDlg::~CGravMidTerrainCorrectionDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CGravMidTerrainCorrectionDlg::initUi()
{
    setWindowTitle(QStringLiteral("1:5万重力中区地形改正"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 重力测点数据文件输入 =================
    QGroupBox* pGroupTxt = new QGroupBox(QStringLiteral("重力测点数据文件输入"), this);
    QGridLayout* pTxtLayout = new QGridLayout(pGroupTxt);
    mEditTxtFile = new QLineEdit(pGroupTxt);
    mBtnOpenTxtFile = new QPushButton(QStringLiteral("..."), pGroupTxt);
    mBtnOpenTxtFile->setFixedWidth(32);
    pTxtLayout->addWidget(mEditTxtFile, 0, 0);
    pTxtLayout->addWidget(mBtnOpenTxtFile, 0, 1);
    pTxtLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupTxt);

    // ================= 高程网格数据文件输入 =================
    QGroupBox* pGroupGrd = new QGroupBox(QStringLiteral("高程网格数据文件输入"), this);
    QGridLayout* pGrdLayout = new QGridLayout(pGroupGrd);
    mEditGrdFile = new QLineEdit(pGroupGrd);
    mBtnOpenGrdFile = new QPushButton(QStringLiteral("..."), pGroupGrd);
    mBtnOpenGrdFile->setFixedWidth(32);
    mBtnOpenGrdFileView = new QPushButton(QStringLiteral("显示"), pGroupGrd);
    mBtnOpenGrdFileView->setFixedWidth(48);
    pGrdLayout->addWidget(mEditGrdFile, 0, 0);
    pGrdLayout->addWidget(mBtnOpenGrdFile, 0, 1);
    pGrdLayout->addWidget(mBtnOpenGrdFileView, 0, 2);
    pGrdLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupGrd);

    // ================= 高程网格数据文件信息（只读，2 行 3 列）=================
    // 对应 .rc 中的 IDC_GravityNum / IDC_Rows / IDC_Cols / IDC_HStep / IDC_ZMin / IDC_ZMax
    QGroupBox* pGroupInfo = new QGroupBox(QStringLiteral("高程网格数据文件信息"), this);
    QGridLayout* pInfoLayout = new QGridLayout(pGroupInfo);
    // 第一行：重力测点数 / 网格行数 / 网格列数
    pInfoLayout->addWidget(new QLabel(QStringLiteral("重力测点数"), pGroupInfo), 0, 0);
    mEditGravityNum = new QLineEdit(pGroupInfo);
    mEditGravityNum->setReadOnly(true);
    mEditGravityNum->setAlignment(Qt::AlignCenter);
    mEditGravityNum->setFixedWidth(90);
    pInfoLayout->addWidget(mEditGravityNum, 0, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格行数"), pGroupInfo), 0, 2);
    mEditRows = new QLineEdit(pGroupInfo);
    mEditRows->setReadOnly(true);
    mEditRows->setAlignment(Qt::AlignCenter);
    mEditRows->setFixedWidth(90);
    pInfoLayout->addWidget(mEditRows, 0, 3);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格列数"), pGroupInfo), 0, 4);
    mEditCols = new QLineEdit(pGroupInfo);
    mEditCols->setReadOnly(true);
    mEditCols->setAlignment(Qt::AlignCenter);
    mEditCols->setFixedWidth(90);
    pInfoLayout->addWidget(mEditCols, 0, 5);
    // 第二行：高程网格距 / 高程最小值 / 高程最大值
    pInfoLayout->addWidget(new QLabel(QStringLiteral("高程网格距"), pGroupInfo), 1, 0);
    mEditHStep = new QLineEdit(pGroupInfo);
    mEditHStep->setReadOnly(true);
    mEditHStep->setAlignment(Qt::AlignCenter);
    mEditHStep->setFixedWidth(90);
    pInfoLayout->addWidget(mEditHStep, 1, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("高程最小值"), pGroupInfo), 1, 2);
    mEditZMin = new QLineEdit(pGroupInfo);
    mEditZMin->setReadOnly(true);
    mEditZMin->setAlignment(Qt::AlignCenter);
    mEditZMin->setFixedWidth(90);
    pInfoLayout->addWidget(mEditZMin, 1, 3);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("高程最大值"), pGroupInfo), 1, 4);
    mEditZMax = new QLineEdit(pGroupInfo);
    mEditZMax->setReadOnly(true);
    mEditZMax->setAlignment(Qt::AlignCenter);
    mEditZMax->setFixedWidth(90);
    pInfoLayout->addWidget(mEditZMax, 1, 5);
    pMainLayout->addWidget(pGroupInfo);

    // ================= 外接口形状选择（单选）=================
    // 对应 .rc 中的 IDC_RADIO_Rect / IDC_RADIO_Circle（默认方形，与原工程 OnInitDialog 一致）
    QGroupBox* pGroupOuter = new QGroupBox(QStringLiteral("外接口形状选择"), this);
    QHBoxLayout* pOuterLayout = new QHBoxLayout(pGroupOuter);
    mGroupOuter = new QButtonGroup(this);
    mRadioOutRect = new QRadioButton(QStringLiteral("方形"), pGroupOuter);
    mRadioOutCircle = new QRadioButton(QStringLiteral("圆形"), pGroupOuter);
    mGroupOuter->addButton(mRadioOutRect, 0);
    mGroupOuter->addButton(mRadioOutCircle, 1);
    mRadioOutRect->setChecked(true);
    mOuterShape = 0;
    pOuterLayout->addWidget(mRadioOutRect);
    pOuterLayout->addWidget(mRadioOutCircle);
    pOuterLayout->addStretch(1);
    pMainLayout->addWidget(pGroupOuter);

    // ================= 内接口形状选择（单选）=================
    // 对应 .rc 中的 IDC_RADIO_InRect / IDC_RADIO_InCircle（默认方形，与原工程 OnInitDialog 一致）
    QGroupBox* pGroupInner = new QGroupBox(QStringLiteral("内接口形状选择"), this);
    QHBoxLayout* pInnerLayout = new QHBoxLayout(pGroupInner);
    mGroupInner = new QButtonGroup(this);
    mRadioInRect = new QRadioButton(QStringLiteral("方形"), pGroupInner);
    mRadioInCircle = new QRadioButton(QStringLiteral("圆形"), pGroupInner);
    mGroupInner->addButton(mRadioInRect, 0);
    mGroupInner->addButton(mRadioInCircle, 1);
    mRadioInRect->setChecked(true);
    mInnerShape = 0;
    pInnerLayout->addWidget(mRadioInRect);
    pInnerLayout->addWidget(mRadioInCircle);
    pInnerLayout->addStretch(1);
    pMainLayout->addWidget(pGroupInner);

    // ================= 地形改正参数输入 =================
    // 对应 .rc 中的 IDC_StartRaid / IDC_EndRaid / IDC_RockDentisy
    QGroupBox* pGroupParams = new QGroupBox(QStringLiteral("地形改正参数输入"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("起始半径"), pGroupParams), 0, 0);
    mSpinStartRadius = new QDoubleSpinBox(pGroupParams);
    mSpinStartRadius->setRange(0.0, 9999999.0);
    mSpinStartRadius->setDecimals(3);
    mSpinStartRadius->setSingleStep(1.0);
    mSpinStartRadius->setValue(mStartRadius);
    mSpinStartRadius->setAlignment(Qt::AlignCenter);
    mSpinStartRadius->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinStartRadius, 0, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("终止半径"), pGroupParams), 0, 2);
    mSpinEndRadius = new QDoubleSpinBox(pGroupParams);
    mSpinEndRadius->setRange(0.0, 9999999.0);
    mSpinEndRadius->setDecimals(3);
    mSpinEndRadius->setSingleStep(1.0);
    mSpinEndRadius->setValue(mEndRadius);
    mSpinEndRadius->setAlignment(Qt::AlignCenter);
    mSpinEndRadius->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinEndRadius, 0, 3);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("地壳密度"), pGroupParams), 0, 4);
    mSpinRockDensity = new QDoubleSpinBox(pGroupParams);
    mSpinRockDensity->setRange(0.0, 999.0);
    mSpinRockDensity->setDecimals(3);
    mSpinRockDensity->setSingleStep(0.1);
    mSpinRockDensity->setValue(mRockDensity);
    mSpinRockDensity->setAlignment(Qt::AlignCenter);
    mSpinRockDensity->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinRockDensity, 0, 5);
    pMainLayout->addWidget(pGroupParams);

    // ================= 地形改正结果数据文件输出 =================
    // 对应 .rc 中的 IDC_FileNameRes / IDC_OpenFileRes
    QGroupBox* pGroupRes = new QGroupBox(QStringLiteral("地形改正结果数据文件输出"), this);
    QGridLayout* pResLayout = new QGridLayout(pGroupRes);
    mEditResFile = new QLineEdit(pGroupRes);
    mBtnOpenResFile = new QPushButton(QStringLiteral("..."), pGroupRes);
    mBtnOpenResFile->setFixedWidth(32);
    pResLayout->addWidget(mEditResFile, 0, 0);
    pResLayout->addWidget(mBtnOpenResFile, 0, 1);
    pResLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupRes);

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
    // 分组框标题居中（对应原工程 GROUPBOX 的 BS_CENTER 样式，见示例图片 SubWindow.png）
    for (QGroupBox* pGroup : findChildren<QGroupBox*>())
    {
        pGroup->setAlignment(Qt::AlignCenter);
    }

    // 输入文件
    connect(mBtnOpenTxtFile, &QPushButton::clicked, this, &CGravMidTerrainCorrectionDlg::onOpenTxtFileClicked);
    connect(mEditTxtFile, &QLineEdit::textEdited, this, &CGravMidTerrainCorrectionDlg::onTxtFileTextEdited);
    connect(mBtnOpenGrdFile, &QPushButton::clicked, this, &CGravMidTerrainCorrectionDlg::onOpenGrdFileClicked);
    connect(mBtnOpenGrdFileView, &QPushButton::clicked, this, &CGravMidTerrainCorrectionDlg::onOpenGrdFileViewClicked);
    connect(mEditGrdFile, &QLineEdit::textEdited, this, &CGravMidTerrainCorrectionDlg::onGrdFileTextEdited);
    // 结果输出文件
    connect(mBtnOpenResFile, &QPushButton::clicked, this, &CGravMidTerrainCorrectionDlg::onOpenResFileClicked);
    connect(mEditResFile, &QLineEdit::textEdited, this, &CGravMidTerrainCorrectionDlg::onResFileTextEdited);

    // 外/内接口形状单选组
    connect(mGroupOuter, &QButtonGroup::idClicked, this, &CGravMidTerrainCorrectionDlg::onOuterShapeClicked);
    connect(mGroupInner, &QButtonGroup::idClicked, this, &CGravMidTerrainCorrectionDlg::onInnerShapeClicked);

    // 处理参数
    connect(mSpinStartRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravMidTerrainCorrectionDlg::onStartRadiusChanged);
    connect(mSpinEndRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravMidTerrainCorrectionDlg::onEndRadiusChanged);
    connect(mSpinRockDensity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravMidTerrainCorrectionDlg::onRockDensityChanged);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CGravMidTerrainCorrectionDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CGravMidTerrainCorrectionDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：选择重力测点数据文件（对应原工程 OnOpenFileTxt）
void CGravMidTerrainCorrectionDlg::onOpenTxtFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择测点数据文件"),
        QStringLiteral("文本文件 (*.txt);;数据文件 (*.dat);;所有文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mTxtFilePath = strFilePath;
    if (mEditTxtFile != NULL)
    {
        mEditTxtFile->setText(strFilePath);
    }
    loadTxtFile(strFilePath);
}

// 功能：手动输入测点文件名（同步成员变量）
void CGravMidTerrainCorrectionDlg::onTxtFileTextEdited(const QString& strText)
{
    mTxtFilePath = strText;
}

// 功能：选择地形高程网格数据文件（对应原工程 OnOpenFileGrd）
void CGravMidTerrainCorrectionDlg::onOpenGrdFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择地形网格数据文件"),
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadGrdFile(strFilePath);
}

// 功能：手动输入高程网格文件名（同步成员变量）
void CGravMidTerrainCorrectionDlg::onGrdFileTextEdited(const QString& strText)
{
    mGrdFilePath = strText;
}

// 功能：选择地形改正结果数据文件（对应原工程 OnOpenFileRes）
void CGravMidTerrainCorrectionDlg::onOpenResFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择地形改正结果数据文件"),
        QStringLiteral("文本文件 (*.txt);;数据文件 (*.dat);;所有文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mResFilePath = strFilePath;
    if (mEditResFile != NULL)
    {
        mEditResFile->setText(strFilePath);
    }
}

// 功能：手动输入结果文件名（同步成员变量）
void CGravMidTerrainCorrectionDlg::onResFileTextEdited(const QString& strText)
{
    mResFilePath = strText;
}

// 功能：读取高程网格文件头并刷新网格信息（对应原工程 ReadData 的网格信息部分）
void CGravMidTerrainCorrectionDlg::loadGrdFile(const QString& strFilePath)
{
    if (strFilePath.isEmpty())
    {
        return;
    }
    if (strFilePath == mLoadedGrdPath)
    {
        return;
    }

    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        return;
    }
    GridFileHead head;
    BackendError error;
    if (!pBackend->readGridFileHead(toBackendString(strFilePath), head, error))
    {
        QMessageBox::warning(this, QStringLiteral("读取文件失败"), fromBackendString(error.message));
        return;
    }

    mLoadedGrdPath = strFilePath;
    mGrdFilePath = strFilePath;
    if (mEditGrdFile != NULL)
    {
        mEditGrdFile->setText(strFilePath);
    }

    // 网格信息（与原工程 ReadData 一致：行/列、网格距、高程范围）
    mRowsNum = head.rows;
    mColsNum = head.cols;
    mZMin = head.zMin;
    mZMax = head.zMax;
    double dHstep = (head.cols > 1) ? (head.xMax - head.xMin) / (head.cols - 1) : 0.0;
    // 高程网格距规整为整数（与原工程 ReadData 中 fmod 取整规则一致：非整数则向上取整）
    double dTemp = fmod(dHstep, 1.0);
    mHstep = (dTemp != 0.0) ? (double)((int)dHstep + 1) : (double)((int)dHstep);

    updateGridInfoDisplay();
}

// 功能：统计测点数据行数并刷新“重力测点数”（对应原工程 OnOpenFileTxt 解析点数）
void CGravMidTerrainCorrectionDlg::loadTxtFile(const QString& strFilePath)
{
    if (strFilePath.isEmpty())
    {
        return;
    }
    QFile file(strFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QStringLiteral("文件打开失败"),
            QStringLiteral("无法打开测点数据文件：") + strFilePath);
        return;
    }

    // 统计非空数据行数作为“重力测点数”（原工程逐行读取重力测点）
    int nCount = 0;
    while (!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString strLine = QString::fromUtf8(line.trimmed());
        if (!strLine.isEmpty())
        {
            nCount++;
        }
    }
    file.close();

    mGravityNum = nCount;
    if (mEditGravityNum != NULL)
    {
        mEditGravityNum->setText(QString::number(mGravityNum));
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CGravMidTerrainCorrectionDlg::updateGridInfoDisplay()
{
    if (mEditRows != NULL)
    {
        mEditRows->setText(QString::number(mRowsNum));
    }
    if (mEditCols != NULL)
    {
        mEditCols->setText(QString::number(mColsNum));
    }
    if (mEditHStep != NULL)
    {
        mEditHStep->setText(QString::number(mHstep, 'f', 3));
    }
    if (mEditZMin != NULL)
    {
        mEditZMin->setText(QString::number(mZMin, 'f', 3));
    }
    if (mEditZMax != NULL)
    {
        mEditZMax->setText(QString::number(mZMax, 'f', 3));
    }
}

// 功能：弹出打开文件对话框（按过滤器）
QString CGravMidTerrainCorrectionDlg::askOpenFilePath(const QString& strTitle, const QString& strFilter)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(), strFilter);
}

// 功能：检查文件存在并发出发送显示请求（对应原工程 OnOpenFileGrdView）
void CGravMidTerrainCorrectionDlg::viewGridFile(const QString& strFilePath)
{
    if (strFilePath.isEmpty())
    {
        return;
    }
    if (!QFile::exists(strFilePath))
    {
        QMessageBox::warning(this, QStringLiteral("文件不存在"),
            QStringLiteral("文件") + strFilePath + QStringLiteral("不存在!"));
        return;
    }
    emit viewGridFileRequested(strFilePath);
}

// 功能：显示地形高程网格文件等值线
void CGravMidTerrainCorrectionDlg::onOpenGrdFileViewClicked()
{
    viewGridFile(mGrdFilePath);
}

// 功能：外接口形状切换（0=方形 1=圆形）
void CGravMidTerrainCorrectionDlg::onOuterShapeClicked(int nShapeId)
{
    mOuterShape = nShapeId;
}

// 功能：内接口形状切换（0=方形 1=圆形）
void CGravMidTerrainCorrectionDlg::onInnerShapeClicked(int nShapeId)
{
    mInnerShape = nShapeId;
}

// 功能：起始半径变化（同步成员变量）
void CGravMidTerrainCorrectionDlg::onStartRadiusChanged(double dValue)
{
    mStartRadius = dValue;
}

// 功能：终止半径变化（同步成员变量）
void CGravMidTerrainCorrectionDlg::onEndRadiusChanged(double dValue)
{
    mEndRadius = dValue;
}

// 功能：地壳密度变化（同步成员变量）
void CGravMidTerrainCorrectionDlg::onRockDensityChanged(double dValue)
{
    mRockDensity = dValue;
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的校验，并给出更明确的提示）
bool CGravMidTerrainCorrectionDlg::validateInputs(QString& strError)
{
    // 输入文件检查（原工程 OnOK 中三段空值校验）
    if (mGrdFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择地形网格数据文件。");
        return false;
    }
    if (mTxtFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择重力测点数据文件。");
        return false;
    }
    if (mResFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择地形改正结果数据文件。");
        return false;
    }
    // 参数检查（原工程 OnOK 中 > 0 校验）
    if (mStartRadius <= 0.0)
    {
        strError = QStringLiteral("起始半径必须大于 0。");
        return false;
    }
    if (mEndRadius <= 0.0)
    {
        strError = QStringLiteral("终止半径必须大于 0。");
        return false;
    }
    if (mRockDensity <= 0.0)
    {
        strError = QStringLiteral("地壳密度必须大于 0。");
        return false;
    }
    // 半径应能被高程网格距整除（原工程 fmod 校验）
    if (mHstep > 0.0)
    {
        if (fmod(mStartRadius, mHstep) > 0.0001)
        {
            strError = QStringLiteral("起始半径不能被高程网格距整除。");
            return false;
        }
        if (fmod(mEndRadius, mHstep) > 0.0001)
        {
            strError = QStringLiteral("终止半径不能被高程网格距整除。");
            return false;
        }
    }
    return true;
}

// 功能：组装参数并调用后端 processMidTerrain（对应原工程 OnOK 主体逻辑）
void CGravMidTerrainCorrectionDlg::runProcess()
{
    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), QStringLiteral("后端接口未初始化。"));
        return;
    }

    // 组装处理参数（对应原工程界面上的全部输入项）
    MidTerrainParams params;
    params.txtFilePath = toBackendString(mTxtFilePath);
    params.grdFilePath = toBackendString(mGrdFilePath);
    params.resFilePath = toBackendString(mResFilePath);
    params.startRadius = (float)mStartRadius;
    params.endRadius = (float)mEndRadius;
    params.rockDensity = (float)mRockDensity;
    params.outerShape = mOuterShape;
    params.innerShape = mInnerShape;

    // 同步调用后端处理（处理期间显示等待光标，与原工程 BeginWaitCursor 一致）
    BackendError error;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bOk = pBackend->processMidTerrain(params, error);
    QApplication::restoreOverrideCursor();

    if (bOk)
    {
        // 处理完成提示（与原工程一致，处理完成后对话框不关闭，由用户点“取消”退出）
        QMessageBox::information(this, QStringLiteral("处理完成"),
            QStringLiteral("重力中区地形改正处理结束!"));
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), fromBackendString(error.message));
    }
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CGravMidTerrainCorrectionDlg::onOkClicked()
{
    QString strError;
    if (!validateInputs(strError))
    {
        QMessageBox::warning(this, QStringLiteral("参数错误"), strError);
        return;
    }
    runProcess();
}

// 功能：“取消”——关闭对话框（对应原工程 OnCancel）
void CGravMidTerrainCorrectionDlg::onCancelClicked()
{
    reject();
}

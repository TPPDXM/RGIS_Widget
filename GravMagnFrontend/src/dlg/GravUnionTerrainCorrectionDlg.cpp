// GravUnionTerrainCorrectionDlg.cpp : 重力联合（平面带）地形改正对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_GravUnionTerrainCorrection；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）。

#include "GravUnionTerrainCorrectionDlg.h"

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
#include <QSpinBox>
#include <QVBoxLayout>

#include "backend/RgisBackend.h"
#include "FrontendUtils.h"

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CGravUnionTerrainCorrectionDlg::CGravUnionTerrainCorrectionDlg(QWidget* pParent)
    : QDialog(pParent)
    , mEditTxtFile(NULL)
    , mBtnOpenTxtFile(NULL)
    , mEditGrdFile(NULL)
    , mBtnOpenGrdFile(NULL)
    , mBtnOpenGrdFileView(NULL)
    , mEditRowsNum(NULL)
    , mEditColsNum(NULL)
    , mEditRowStep(NULL)
    , mEditColStep(NULL)
    , mRadioKinds1(NULL)
    , mRadioKinds2(NULL)
    , mGroupKinds(NULL)
    , mRadioShape1(NULL)
    , mRadioShape2(NULL)
    , mRadioShape3(NULL)
    , mRadioShape4(NULL)
    , mGroupShape(NULL)
    , mRadioTypes1(NULL)
    , mRadioTypes2(NULL)
    , mRadioTypes3(NULL)
    , mGroupTypes(NULL)
    , mSpinMinRadius(NULL)
    , mSpinMaxRadius(NULL)
    , mSpinDensity(NULL)
    , mSpinAziNum1(NULL)
    , mSpinAziNum2(NULL)
    , mSpinAziNum3(NULL)
    , mEditResFile(NULL)
    , mBtnOpenResFile(NULL)
    , mBtnOk(NULL)
    , mBtnCancel(NULL)
    , mRowsNum(0)
    , mColsNum(0)
    , mRowStep(0.0)
    , mColStep(0.0)
    , mKindsType(0)
    , mShapeType(0)
    , mTypesType(0)
    , mMinRadius(50.0)
    , mMaxRadius(2000.0)
    , mDensity(2.67)
    , mAziNum1(72)
    , mAziNum2(36)
    , mAziNum3(24)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CGravUnionTerrainCorrectionDlg::~CGravUnionTerrainCorrectionDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CGravUnionTerrainCorrectionDlg::initUi()
{
    setWindowTitle(QStringLiteral("平面带重力地形改正"));
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

    // ================= 网格数据信息（只读，1 行 4 列）=================
    // 对应 .rc 中的 IDC_RowsNum / IDC_ColsNum / IDC_RowStep / IDC_ColStep
    QGroupBox* pGroupInfo = new QGroupBox(QStringLiteral("网格数据信息"), this);
    QGridLayout* pInfoLayout = new QGridLayout(pGroupInfo);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格行数"), pGroupInfo), 0, 0);
    mEditRowsNum = new QLineEdit(pGroupInfo);
    mEditRowsNum->setReadOnly(true);
    mEditRowsNum->setAlignment(Qt::AlignCenter);
    mEditRowsNum->setFixedWidth(90);
    pInfoLayout->addWidget(mEditRowsNum, 0, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格列数"), pGroupInfo), 0, 2);
    mEditColsNum = new QLineEdit(pGroupInfo);
    mEditColsNum->setReadOnly(true);
    mEditColsNum->setAlignment(Qt::AlignCenter);
    mEditColsNum->setFixedWidth(90);
    pInfoLayout->addWidget(mEditColsNum, 0, 3);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格行距"), pGroupInfo), 0, 4);
    mEditRowStep = new QLineEdit(pGroupInfo);
    mEditRowStep->setReadOnly(true);
    mEditRowStep->setAlignment(Qt::AlignCenter);
    mEditRowStep->setFixedWidth(90);
    pInfoLayout->addWidget(mEditRowStep, 0, 5);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格列距"), pGroupInfo), 0, 6);
    mEditColStep = new QLineEdit(pGroupInfo);
    mEditColStep->setReadOnly(true);
    mEditColStep->setAlignment(Qt::AlignCenter);
    mEditColStep->setFixedWidth(90);
    pInfoLayout->addWidget(mEditColStep, 0, 7);
    pMainLayout->addWidget(pGroupInfo);

    // ================= 地形改正方法选择（单选，2 项）=================
    // 对应 .rc 中的 IDC_KindsRadio1 / IDC_KindsRadio2（默认常规计算，与原工程 OnInitDialog 一致）
    QGroupBox* pGroupKinds = new QGroupBox(QStringLiteral("地形改正方法选择"), this);
    QHBoxLayout* pKindsLayout = new QHBoxLayout(pGroupKinds);
    mGroupKinds = new QButtonGroup(this);
    mRadioKinds1 = new QRadioButton(QStringLiteral("常规计算"), pGroupKinds);
    mRadioKinds2 = new QRadioButton(QStringLiteral("三观测列方差分解"), pGroupKinds);
    mGroupKinds->addButton(mRadioKinds1, 0);
    mGroupKinds->addButton(mRadioKinds2, 1);
    mRadioKinds1->setChecked(true);
    mKindsType = 0;
    pKindsLayout->addWidget(mRadioKinds1);
    pKindsLayout->addWidget(mRadioKinds2);
    pKindsLayout->addStretch(1);
    pMainLayout->addWidget(pGroupKinds);

    // ================= 地形改正形状选择（单选，4 项）=================
    // 对应 .rc 中的 IDC_ShapeRadio1/2/3/4（默认环形，与原工程 OnInitDialog 一致）
    QGroupBox* pGroupShape = new QGroupBox(QStringLiteral("地形改正形状选择"), this);
    QHBoxLayout* pShapeLayout = new QHBoxLayout(pGroupShape);
    mGroupShape = new QButtonGroup(this);
    mRadioShape1 = new QRadioButton(QStringLiteral("环形"), pGroupShape);
    mRadioShape2 = new QRadioButton(QStringLiteral("回形"), pGroupShape);
    mRadioShape3 = new QRadioButton(QStringLiteral("钱形"), pGroupShape);
    mRadioShape4 = new QRadioButton(QStringLiteral("枷形"), pGroupShape);
    mGroupShape->addButton(mRadioShape1, 0);
    mGroupShape->addButton(mRadioShape2, 1);
    mGroupShape->addButton(mRadioShape3, 2);
    mGroupShape->addButton(mRadioShape4, 3);
    mRadioShape1->setChecked(true);
    mShapeType = 0;
    pShapeLayout->addWidget(mRadioShape1);
    pShapeLayout->addWidget(mRadioShape2);
    pShapeLayout->addWidget(mRadioShape3);
    pShapeLayout->addWidget(mRadioShape4);
    pShapeLayout->addStretch(1);
    pMainLayout->addWidget(pGroupShape);

    // ================= 地形改正类型选择（单选，3 项）=================
    // 对应 .rc 中的 IDC_TypesRadio1/2/3（默认常规地形改正，与原工程 OnInitDialog 一致）
    QGroupBox* pGroupTypes = new QGroupBox(QStringLiteral("地形改正类型选择"), this);
    QHBoxLayout* pTypesLayout = new QHBoxLayout(pGroupTypes);
    mGroupTypes = new QButtonGroup(this);
    mRadioTypes1 = new QRadioButton(QStringLiteral("常规地形改正"), pGroupTypes);
    mRadioTypes2 = new QRadioButton(QStringLiteral("陆岛地形改正"), pGroupTypes);
    mRadioTypes3 = new QRadioButton(QStringLiteral("广义地形改正"), pGroupTypes);
    mGroupTypes->addButton(mRadioTypes1, 0);
    mGroupTypes->addButton(mRadioTypes2, 1);
    mGroupTypes->addButton(mRadioTypes3, 2);
    mRadioTypes1->setChecked(true);
    mTypesType = 0;
    pTypesLayout->addWidget(mRadioTypes1);
    pTypesLayout->addWidget(mRadioTypes2);
    pTypesLayout->addWidget(mRadioTypes3);
    pTypesLayout->addStretch(1);
    pMainLayout->addWidget(pGroupTypes);

    // ================= 地形改正输入参数（2 行）=================
    // 对应 .rc 中的 IDC_MinRads / IDC_MaxRads / IDC_RockDensity / IDC_AziNum1/2/3
    QGroupBox* pGroupParams = new QGroupBox(QStringLiteral("地形改正输入参数"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    // 第一行：地改内环半径 / 地改外环半径 / 地形改正密度
    pParamsLayout->addWidget(new QLabel(QStringLiteral("地改内环半径"), pGroupParams), 0, 0);
    mSpinMinRadius = new QDoubleSpinBox(pGroupParams);
    mSpinMinRadius->setRange(0.0, 9999999.0);
    mSpinMinRadius->setDecimals(3);
    mSpinMinRadius->setSingleStep(1.0);
    mSpinMinRadius->setValue(mMinRadius);
    mSpinMinRadius->setAlignment(Qt::AlignCenter);
    mSpinMinRadius->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinMinRadius, 0, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("地改外环半径"), pGroupParams), 0, 2);
    mSpinMaxRadius = new QDoubleSpinBox(pGroupParams);
    mSpinMaxRadius->setRange(0.0, 9999999.0);
    mSpinMaxRadius->setDecimals(3);
    mSpinMaxRadius->setSingleStep(1.0);
    mSpinMaxRadius->setValue(mMaxRadius);
    mSpinMaxRadius->setAlignment(Qt::AlignCenter);
    mSpinMaxRadius->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinMaxRadius, 0, 3);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("地形改正密度"), pGroupParams), 0, 4);
    mSpinDensity = new QDoubleSpinBox(pGroupParams);
    mSpinDensity->setRange(0.0, 999.0);
    mSpinDensity->setDecimals(3);
    mSpinDensity->setSingleStep(0.1);
    mSpinDensity->setValue(mDensity);
    mSpinDensity->setAlignment(Qt::AlignCenter);
    mSpinDensity->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinDensity, 0, 5);
    // 第二行：第一列方位数 / 第二列方位数 / 第三列方位数
    pParamsLayout->addWidget(new QLabel(QStringLiteral("第一列方位数"), pGroupParams), 1, 0);
    mSpinAziNum1 = new QSpinBox(pGroupParams);
    mSpinAziNum1->setRange(1, 999999);
    mSpinAziNum1->setValue(mAziNum1);
    mSpinAziNum1->setAlignment(Qt::AlignCenter);
    mSpinAziNum1->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinAziNum1, 1, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("第二列方位数"), pGroupParams), 1, 2);
    mSpinAziNum2 = new QSpinBox(pGroupParams);
    mSpinAziNum2->setRange(1, 999999);
    mSpinAziNum2->setValue(mAziNum2);
    mSpinAziNum2->setAlignment(Qt::AlignCenter);
    mSpinAziNum2->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinAziNum2, 1, 3);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("第三列方位数"), pGroupParams), 1, 4);
    mSpinAziNum3 = new QSpinBox(pGroupParams);
    mSpinAziNum3->setRange(1, 999999);
    mSpinAziNum3->setValue(mAziNum3);
    mSpinAziNum3->setAlignment(Qt::AlignCenter);
    mSpinAziNum3->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinAziNum3, 1, 5);
    pMainLayout->addWidget(pGroupParams);

    // ================= 地形改正数据文件输出 =================
    // 对应 .rc 中的 IDC_SaveFile / ID_SaveTextFile
    QGroupBox* pGroupRes = new QGroupBox(QStringLiteral("地形改正数据文件输出"), this);
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
    connect(mBtnOpenTxtFile, &QPushButton::clicked, this, &CGravUnionTerrainCorrectionDlg::onOpenTxtFileClicked);
    connect(mEditTxtFile, &QLineEdit::textEdited, this, &CGravUnionTerrainCorrectionDlg::onTxtFileTextEdited);
    connect(mBtnOpenGrdFile, &QPushButton::clicked, this, &CGravUnionTerrainCorrectionDlg::onOpenGrdFileClicked);
    connect(mBtnOpenGrdFileView, &QPushButton::clicked, this, &CGravUnionTerrainCorrectionDlg::onOpenGrdFileViewClicked);
    connect(mEditGrdFile, &QLineEdit::textEdited, this, &CGravUnionTerrainCorrectionDlg::onGrdFileTextEdited);
    // 结果输出文件
    connect(mBtnOpenResFile, &QPushButton::clicked, this, &CGravUnionTerrainCorrectionDlg::onOpenResFileClicked);
    connect(mEditResFile, &QLineEdit::textEdited, this, &CGravUnionTerrainCorrectionDlg::onResFileTextEdited);

    // 三组单选
    connect(mGroupKinds, &QButtonGroup::idClicked, this, &CGravUnionTerrainCorrectionDlg::onKindsClicked);
    connect(mGroupShape, &QButtonGroup::idClicked, this, &CGravUnionTerrainCorrectionDlg::onShapeClicked);
    connect(mGroupTypes, &QButtonGroup::idClicked, this, &CGravUnionTerrainCorrectionDlg::onTypesClicked);

    // 处理参数
    connect(mSpinMinRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravUnionTerrainCorrectionDlg::onMinRadiusChanged);
    connect(mSpinMaxRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravUnionTerrainCorrectionDlg::onMaxRadiusChanged);
    connect(mSpinDensity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravUnionTerrainCorrectionDlg::onDensityChanged);
    connect(mSpinAziNum1, QOverload<int>::of(&QSpinBox::valueChanged), this, &CGravUnionTerrainCorrectionDlg::onAziNum1Changed);
    connect(mSpinAziNum2, QOverload<int>::of(&QSpinBox::valueChanged), this, &CGravUnionTerrainCorrectionDlg::onAziNum2Changed);
    connect(mSpinAziNum3, QOverload<int>::of(&QSpinBox::valueChanged), this, &CGravUnionTerrainCorrectionDlg::onAziNum3Changed);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CGravUnionTerrainCorrectionDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CGravUnionTerrainCorrectionDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：选择重力测点数据文件（对应原工程 OnOpenFile）
void CGravUnionTerrainCorrectionDlg::onOpenTxtFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择测点数据文件"),
        QStringLiteral("文本文件 (*.dat);;所有数据文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mTxtFilePath = strFilePath;
    if (mEditTxtFile != NULL)
    {
        mEditTxtFile->setText(strFilePath);
    }
    updateDefaultSavePath(strFilePath);
}

// 功能：手动输入测点文件名（同步成员变量）
void CGravUnionTerrainCorrectionDlg::onTxtFileTextEdited(const QString& strText)
{
    mTxtFilePath = strText;
}

// 功能：选择地形高程网格数据文件（对应原工程 OnOpenGridFile）
void CGravUnionTerrainCorrectionDlg::onOpenGrdFileClicked()
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
void CGravUnionTerrainCorrectionDlg::onGrdFileTextEdited(const QString& strText)
{
    mGrdFilePath = strText;
}

// 功能：选择地形改正结果数据文件（对应原工程 OnSaveFile）
void CGravUnionTerrainCorrectionDlg::onOpenResFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请输入地形改正结果数据文件名"),
        QStringLiteral("文本文件 (*.dat);;所有数据文件 (*.*)"));
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
void CGravUnionTerrainCorrectionDlg::onResFileTextEdited(const QString& strText)
{
    mResFilePath = strText;
}

// 功能：根据测点数据文件生成默认结果文件名（对应原工程 OnOpenFile 中 "_Out"+扩展名 规则）
void CGravUnionTerrainCorrectionDlg::updateDefaultSavePath(const QString& strFilePath)
{
    // 取最后一个点号之前的内容作为基准名，并保留原扩展名（原工程 基准名 + "_Out" + 原扩展名）
    int nPos = strFilePath.lastIndexOf('.');
    QString strBase = strFilePath;
    QString strExt = QStringLiteral(".dat");
    if (nPos > 0)
    {
        strBase = strFilePath.left(nPos);
        strExt = strFilePath.mid(nPos);    // 含点，如 ".dat"
    }

    mResFilePath = strBase + QStringLiteral("_Out") + strExt;
    if (mEditResFile != NULL)
    {
        mEditResFile->setText(mResFilePath);
    }
}

// 功能：读取高程网格文件头并刷新网格信息（对应原工程 ReadData）
void CGravUnionTerrainCorrectionDlg::loadGrdFile(const QString& strFilePath)
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

    // 网格信息（与原工程 ReadData 中的 xStep/yStep 计算一致）
    mRowsNum = head.rows;
    mColsNum = head.cols;
    mRowStep = (head.rows > 1) ? (head.yMax - head.yMin) / (head.rows - 1) : 0.0;
    mColStep = (head.cols > 1) ? (head.xMax - head.xMin) / (head.cols - 1) : 0.0;

    updateGridInfoDisplay();
}

// 功能：把网格信息成员变量刷到界面显示
void CGravUnionTerrainCorrectionDlg::updateGridInfoDisplay()
{
    if (mEditRowsNum != NULL)
    {
        mEditRowsNum->setText(QString::number(mRowsNum));
    }
    if (mEditColsNum != NULL)
    {
        mEditColsNum->setText(QString::number(mColsNum));
    }
    if (mEditRowStep != NULL)
    {
        mEditRowStep->setText(QString::number(mRowStep, 'f', 3));
    }
    if (mEditColStep != NULL)
    {
        mEditColStep->setText(QString::number(mColStep, 'f', 3));
    }
}

// 功能：弹出打开文件对话框（按过滤器）
QString CGravUnionTerrainCorrectionDlg::askOpenFilePath(const QString& strTitle, const QString& strFilter)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(), strFilter);
}

// 功能：检查文件存在并发出发送显示请求（对应原工程 OnOpenGridFileView）
void CGravUnionTerrainCorrectionDlg::viewGridFile(const QString& strFilePath)
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
void CGravUnionTerrainCorrectionDlg::onOpenGrdFileViewClicked()
{
    viewGridFile(mGrdFilePath);
}

// 功能：地形改正方法切换（0=常规计算 1=三观测列方差分解）
void CGravUnionTerrainCorrectionDlg::onKindsClicked(int nId)
{
    mKindsType = nId;
}

// 功能：地形改正形状切换（0=环形 1=回形 2=钱形 3=枷形）
void CGravUnionTerrainCorrectionDlg::onShapeClicked(int nId)
{
    mShapeType = nId;
}

// 功能：地形改正类型切换（0=常规 1=陆岛 2=广义）
void CGravUnionTerrainCorrectionDlg::onTypesClicked(int nId)
{
    mTypesType = nId;
}

// 功能：地改内环半径变化（同步成员变量）
void CGravUnionTerrainCorrectionDlg::onMinRadiusChanged(double dValue)
{
    mMinRadius = dValue;
}

// 功能：地改外环半径变化（同步成员变量）
void CGravUnionTerrainCorrectionDlg::onMaxRadiusChanged(double dValue)
{
    mMaxRadius = dValue;
}

// 功能：地形改正密度变化（同步成员变量）
void CGravUnionTerrainCorrectionDlg::onDensityChanged(double dValue)
{
    mDensity = dValue;
}

// 功能：第一列方位数变化（同步成员变量）
void CGravUnionTerrainCorrectionDlg::onAziNum1Changed(int nValue)
{
    mAziNum1 = nValue;
}

// 功能：第二列方位数变化（同步成员变量）
void CGravUnionTerrainCorrectionDlg::onAziNum2Changed(int nValue)
{
    mAziNum2 = nValue;
}

// 功能：第三列方位数变化（同步成员变量）
void CGravUnionTerrainCorrectionDlg::onAziNum3Changed(int nValue)
{
    mAziNum3 = nValue;
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的校验，并给出更明确的提示）
bool CGravUnionTerrainCorrectionDlg::validateInputs(QString& strError)
{
    // 输入文件检查
    if (mTxtFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择重力测点数据文件。");
        return false;
    }
    if (mGrdFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择地形高程网格数据文件。");
        return false;
    }
    if (mResFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择地形改正结果数据文件。");
        return false;
    }
    // 参数检查（半径、密度必须大于 0）
    if (mMinRadius <= 0.0)
    {
        strError = QStringLiteral("地改内环半径必须大于 0。");
        return false;
    }
    if (mMaxRadius <= 0.0)
    {
        strError = QStringLiteral("地改外环半径必须大于 0。");
        return false;
    }
    if (mDensity <= 0.0)
    {
        strError = QStringLiteral("地形改正密度必须大于 0。");
        return false;
    }
    return true;
}

// 功能：组装参数并调用后端 processUnionTerrain（对应原工程 OnOK 主体逻辑）
void CGravUnionTerrainCorrectionDlg::runProcess()
{
    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), QStringLiteral("后端接口未初始化。"));
        return;
    }

    // 组装处理参数（对应原工程界面上的全部输入项）
    UnionTerrainParams params;
    params.txtFilePath = toBackendString(mTxtFilePath);
    params.grdFilePath = toBackendString(mGrdFilePath);
    params.resFilePath = toBackendString(mResFilePath);
    params.kindsType = mKindsType;
    params.shapeType = mShapeType;
    params.typesType = mTypesType;
    params.minRadius = (float)mMinRadius;
    params.maxRadius = (float)mMaxRadius;
    params.rockDensity = (float)mDensity;
    params.aziNum1 = mAziNum1;
    params.aziNum2 = mAziNum2;
    params.aziNum3 = mAziNum3;

    // 同步调用后端处理（处理期间显示等待光标，与原工程 BeginWaitCursor 一致）
    BackendError error;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bOk = pBackend->processUnionTerrain(params, error);
    QApplication::restoreOverrideCursor();

    if (bOk)
    {
        // 处理完成提示（与原工程一致，处理完成后对话框不关闭，由用户点“取消”退出）
        QMessageBox::information(this, QStringLiteral("处理完成"),
            QStringLiteral("重力联合地形改正处理结束!"));
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), fromBackendString(error.message));
    }
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CGravUnionTerrainCorrectionDlg::onOkClicked()
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
void CGravUnionTerrainCorrectionDlg::onCancelClicked()
{
    reject();
}

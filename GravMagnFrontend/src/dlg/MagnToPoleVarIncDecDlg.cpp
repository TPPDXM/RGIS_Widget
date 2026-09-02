// MagnToPoleVarIncDecDlg.cpp : 变磁化倾角化极对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_MagnToPoleVarIncDec；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）；
//   * 后端接口（processMagnToPoleVarIncDec）未由后端提供，本对话框当前仅实现界面与参数校验，
//     接入方式见 runProcess 注释。

#include "MagnToPoleVarIncDecDlg.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
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

#include "FrontendUtils.h"

// 最大扩充尺寸（与原 MFC 工程 OnDeltaposSpinExRows/ExCols 中 <= 65536 的约束一致）
static const int sMaxExtendSize = 65536;

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CMagnToPoleVarIncDecDlg::CMagnToPoleVarIncDecDlg(QWidget* pParent)
    : QDialog(pParent)
    , mEditOpenFile(NULL)
    , mBtnOpenFile(NULL)
    , mBtnOpenFileView(NULL)
    , mEditRowsNum(NULL)
    , mEditColsNum(NULL)
    , mEditRowStep(NULL)
    , mEditColStep(NULL)
    , mSpinProfile(NULL)
    , mSpinBaseLine(NULL)
    , mSpinExRows(NULL)
    , mSpinExCols(NULL)
    , mRadioCosFun(NULL)
    , mRadioAvgDif(NULL)
    , mRadioInvPow(NULL)
    , mRadioMinCrv(NULL)
    , mButtonGroupExpand(NULL)
    , mCheckLinear(NULL)
    , mSpinInc1(NULL)
    , mSpinInc2(NULL)
    , mSpinInc3(NULL)
    , mSpinInc4(NULL)
    , mSpinDec1(NULL)
    , mSpinDec2(NULL)
    , mSpinDec3(NULL)
    , mSpinDec4(NULL)
    , mEditInclinationFile(NULL)
    , mEditDeclinationFile(NULL)
    , mBtnInclinationFile(NULL)
    , mBtnInclinationFileView(NULL)
    , mBtnDeclinationFile(NULL)
    , mBtnDeclinationFileView(NULL)
    , mEditSaveFile(NULL)
    , mBtnSaveFile(NULL)
    , mBtnSaveFileView(NULL)
    , mBtnOk(NULL)
    , mBtnCancel(NULL)
    , mRowsNum(0)
    , mColsNum(0)
    , mRowStep(0.0)
    , mColStep(0.0)
    , mExRows(0)
    , mExCols(0)
    , mMinExRows(1)
    , mMinExCols(1)
    , mExpandMethod(ExpandCosFun)
    , mProfile(90.0)
    , mBaseLine(0.0)
    , mCheckLinearState(false)
    , mInc1(10.0)
    , mInc2(11.0)
    , mInc3(14.0)
    , mInc4(15.0)
    , mDec1(30.0)
    , mDec2(30.0)
    , mDec3(30.0)
    , mDec4(30.0)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CMagnToPoleVarIncDecDlg::~CMagnToPoleVarIncDecDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CMagnToPoleVarIncDecDlg::initUi()
{
    setWindowTitle(QStringLiteral("变磁化倾角化极"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 数据文件输入 =================
    QGroupBox* pGroupInput = new QGroupBox(QStringLiteral("数据文件输入"), this);
    QGridLayout* pInputLayout = new QGridLayout(pGroupInput);
    mEditOpenFile = new QLineEdit(pGroupInput);
    mBtnOpenFile = new QPushButton(QStringLiteral("..."), pGroupInput);
    mBtnOpenFile->setFixedWidth(32);
    mBtnOpenFileView = new QPushButton(QStringLiteral("显示"), pGroupInput);
    mBtnOpenFileView->setFixedWidth(48);
    pInputLayout->addWidget(mEditOpenFile, 0, 0);
    pInputLayout->addWidget(mBtnOpenFile, 0, 1);
    pInputLayout->addWidget(mBtnOpenFileView, 0, 2);
    pInputLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupInput);

    // ================= 网格数据信息（只读显示，2 行 2 列）=================
    // 对应 .rc 中的 IDC_EDIT_RowsNum / IDC_EDIT_ColsNum / IDC_yStep / IDC_xStep
    QGroupBox* pGroupInfo = new QGroupBox(QStringLiteral("网格数据信息"), this);
    QGridLayout* pInfoLayout = new QGridLayout(pGroupInfo);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格行数"), pGroupInfo), 0, 0);
    mEditRowsNum = new QLineEdit(pGroupInfo);
    mEditRowsNum->setReadOnly(true);
    mEditRowsNum->setAlignment(Qt::AlignCenter);
    mEditRowsNum->setFixedWidth(110);
    pInfoLayout->addWidget(mEditRowsNum, 0, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格列数"), pGroupInfo), 0, 2);
    mEditColsNum = new QLineEdit(pGroupInfo);
    mEditColsNum->setReadOnly(true);
    mEditColsNum->setAlignment(Qt::AlignCenter);
    mEditColsNum->setFixedWidth(110);
    pInfoLayout->addWidget(mEditColsNum, 0, 3);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格行距"), pGroupInfo), 1, 0);
    mEditRowStep = new QLineEdit(pGroupInfo);
    mEditRowStep->setReadOnly(true);
    mEditRowStep->setAlignment(Qt::AlignCenter);
    mEditRowStep->setFixedWidth(110);
    pInfoLayout->addWidget(mEditRowStep, 1, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格列距"), pGroupInfo), 1, 2);
    mEditColStep = new QLineEdit(pGroupInfo);
    mEditColStep->setReadOnly(true);
    mEditColStep->setAlignment(Qt::AlignCenter);
    mEditColStep->setFixedWidth(110);
    pInfoLayout->addWidget(mEditColStep, 1, 3);
    pMainLayout->addWidget(pGroupInfo);

    // ================= 处理参数（行/列方位角 + 扩充行/列数）=================
    // 对应 .rc 中的 IDC_EDIT_ProfileDirection / IDC_EDIT_BaseLineDirection /
    //           IDC_EDIT_RowsExNum / IDC_EDIT_ColsExNum
    QGroupBox* pGroupParams = new QGroupBox(QStringLiteral("处理参数"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("扩充行数"), pGroupParams), 0, 0);
    mSpinExRows = new QSpinBox(pGroupParams);
    mSpinExRows->setRange(1, sMaxExtendSize);
    mSpinExRows->setValue(1);
    mSpinExRows->setAlignment(Qt::AlignCenter);
    mSpinExRows->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinExRows, 0, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("扩充列数"), pGroupParams), 0, 2);
    mSpinExCols = new QSpinBox(pGroupParams);
    mSpinExCols->setRange(1, sMaxExtendSize);
    mSpinExCols->setValue(1);
    mSpinExCols->setAlignment(Qt::AlignCenter);
    mSpinExCols->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinExCols, 0, 3);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("行方位角"), pGroupParams), 0, 4);
    mSpinProfile = new QDoubleSpinBox(pGroupParams);
    mSpinProfile->setRange(-360.0, 360.0);
    mSpinProfile->setDecimals(3);
    mSpinProfile->setSingleStep(1.0);
    mSpinProfile->setValue(mProfile);
    mSpinProfile->setAlignment(Qt::AlignCenter);
    mSpinProfile->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinProfile, 0, 5);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("列方位角"), pGroupParams), 0, 6);
    mSpinBaseLine = new QDoubleSpinBox(pGroupParams);
    mSpinBaseLine->setRange(-360.0, 360.0);
    mSpinBaseLine->setDecimals(3);
    mSpinBaseLine->setSingleStep(1.0);
    mSpinBaseLine->setValue(mBaseLine);
    mSpinBaseLine->setAlignment(Qt::AlignCenter);
    mSpinBaseLine->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinBaseLine, 0, 7);
    pMainLayout->addWidget(pGroupParams);

    // ================= 数据扩边方法（单选，4 种）=================
    // 对应 .rc 中的 IDC_CosFun / IDC_AvgDif / IDC_InvPow / IDC_MinCrv
    QGroupBox* pGroupMethod = new QGroupBox(QStringLiteral("数据扩边方法"), this);
    QHBoxLayout* pMethodLayout = new QHBoxLayout(pGroupMethod);
    mButtonGroupExpand = new QButtonGroup(this);
    mRadioCosFun = new QRadioButton(QStringLiteral("余弦函数衰减"), pGroupMethod);
    mRadioAvgDif = new QRadioButton(QStringLiteral("平均值差分"), pGroupMethod);
    mRadioInvPow = new QRadioButton(QStringLiteral("反距离加权"), pGroupMethod);
    mRadioMinCrv = new QRadioButton(QStringLiteral("最小曲率"), pGroupMethod);
    mButtonGroupExpand->addButton(mRadioCosFun, ExpandCosFun);
    mButtonGroupExpand->addButton(mRadioAvgDif, ExpandAvgDif);
    mButtonGroupExpand->addButton(mRadioInvPow, ExpandInvPow);
    mButtonGroupExpand->addButton(mRadioMinCrv, ExpandMinCrv);
    mRadioCosFun->setChecked(true);     // 默认选中“余弦函数衰减”（与原工程 OnInitDialog 一致）
    mExpandMethod = ExpandCosFun;
    pMethodLayout->addWidget(mRadioCosFun);
    pMethodLayout->addWidget(mRadioAvgDif);
    pMethodLayout->addWidget(mRadioInvPow);
    pMethodLayout->addWidget(mRadioMinCrv);
    pMethodLayout->addStretch(1);
    pMainLayout->addWidget(pGroupMethod);

    // ================= 测区地磁场参数（文件）输入（单位：度）=================
    // 对应 .rc 中的复选框、四个角点、倾角/偏角数据文件行
    QGroupBox* pGroupGeo = new QGroupBox(QStringLiteral("测区地磁场参数（文件）输入（单位：度）"), this);
    QGridLayout* pGeoLayout = new QGridLayout(pGroupGeo);
    mCheckLinear = new QCheckBox(QStringLiteral("测区四个角点的磁化倾角和磁化偏角近似线性变化"), pGroupGeo);
    mCheckLinear->setChecked(false);
    pGeoLayout->addWidget(mCheckLinear, 0, 0, 1, 10);
    pGeoLayout->addWidget(new QLabel(QStringLiteral("第一行输入角点的磁化倾角，第二行输入磁化偏角"), pGroupGeo), 1, 0, 1, 10);

    pGeoLayout->addWidget(new QLabel(QStringLiteral("角点1倾角"), pGroupGeo), 2, 0);
    mSpinInc1 = new QDoubleSpinBox(pGroupGeo);
    mSpinInc1->setRange(-90.0, 90.0);
    mSpinInc1->setDecimals(3);
    mSpinInc1->setSingleStep(1.0);
    mSpinInc1->setValue(mInc1);
    mSpinInc1->setAlignment(Qt::AlignCenter);
    mSpinInc1->setFixedWidth(80);
    pGeoLayout->addWidget(mSpinInc1, 2, 1);
    pGeoLayout->addWidget(new QLabel(QStringLiteral("角点2倾角"), pGroupGeo), 2, 2);
    mSpinInc2 = new QDoubleSpinBox(pGroupGeo);
    mSpinInc2->setRange(-90.0, 90.0);
    mSpinInc2->setDecimals(3);
    mSpinInc2->setSingleStep(1.0);
    mSpinInc2->setValue(mInc2);
    mSpinInc2->setAlignment(Qt::AlignCenter);
    mSpinInc2->setFixedWidth(80);
    pGeoLayout->addWidget(mSpinInc2, 2, 3);
    pGeoLayout->addWidget(new QLabel(QStringLiteral("角点3倾角"), pGroupGeo), 2, 4);
    mSpinInc3 = new QDoubleSpinBox(pGroupGeo);
    mSpinInc3->setRange(-90.0, 90.0);
    mSpinInc3->setDecimals(3);
    mSpinInc3->setSingleStep(1.0);
    mSpinInc3->setValue(mInc3);
    mSpinInc3->setAlignment(Qt::AlignCenter);
    mSpinInc3->setFixedWidth(80);
    pGeoLayout->addWidget(mSpinInc3, 2, 5);
    pGeoLayout->addWidget(new QLabel(QStringLiteral("角点4倾角"), pGroupGeo), 2, 6);
    mSpinInc4 = new QDoubleSpinBox(pGroupGeo);
    mSpinInc4->setRange(-90.0, 90.0);
    mSpinInc4->setDecimals(3);
    mSpinInc4->setSingleStep(1.0);
    mSpinInc4->setValue(mInc4);
    mSpinInc4->setAlignment(Qt::AlignCenter);
    mSpinInc4->setFixedWidth(80);
    pGeoLayout->addWidget(mSpinInc4, 2, 7);

    pGeoLayout->addWidget(new QLabel(QStringLiteral("角点1偏角"), pGroupGeo), 3, 0);
    mSpinDec1 = new QDoubleSpinBox(pGroupGeo);
    mSpinDec1->setRange(-360.0, 360.0);
    mSpinDec1->setDecimals(3);
    mSpinDec1->setSingleStep(1.0);
    mSpinDec1->setValue(mDec1);
    mSpinDec1->setAlignment(Qt::AlignCenter);
    mSpinDec1->setFixedWidth(80);
    pGeoLayout->addWidget(mSpinDec1, 3, 1);
    pGeoLayout->addWidget(new QLabel(QStringLiteral("角点2偏角"), pGroupGeo), 3, 2);
    mSpinDec2 = new QDoubleSpinBox(pGroupGeo);
    mSpinDec2->setRange(-360.0, 360.0);
    mSpinDec2->setDecimals(3);
    mSpinDec2->setSingleStep(1.0);
    mSpinDec2->setValue(mDec2);
    mSpinDec2->setAlignment(Qt::AlignCenter);
    mSpinDec2->setFixedWidth(80);
    pGeoLayout->addWidget(mSpinDec2, 3, 3);
    pGeoLayout->addWidget(new QLabel(QStringLiteral("角点3偏角"), pGroupGeo), 3, 4);
    mSpinDec3 = new QDoubleSpinBox(pGroupGeo);
    mSpinDec3->setRange(-360.0, 360.0);
    mSpinDec3->setDecimals(3);
    mSpinDec3->setSingleStep(1.0);
    mSpinDec3->setValue(mDec3);
    mSpinDec3->setAlignment(Qt::AlignCenter);
    mSpinDec3->setFixedWidth(80);
    pGeoLayout->addWidget(mSpinDec3, 3, 5);
    pGeoLayout->addWidget(new QLabel(QStringLiteral("角点4偏角"), pGroupGeo), 3, 6);
    mSpinDec4 = new QDoubleSpinBox(pGroupGeo);
    mSpinDec4->setRange(-360.0, 360.0);
    mSpinDec4->setDecimals(3);
    mSpinDec4->setSingleStep(1.0);
    mSpinDec4->setValue(mDec4);
    mSpinDec4->setAlignment(Qt::AlignCenter);
    mSpinDec4->setFixedWidth(80);
    pGeoLayout->addWidget(mSpinDec4, 3, 7);

    // 倾角/偏角数据文件行（对应 .rc 中的 IDC_EDIT_FILE_INCLINATION / IDC_EDIT_FILE_DECLINATION）
    mEditInclinationFile = new QLineEdit(pGroupGeo);
    mBtnInclinationFile = new QPushButton(QStringLiteral("..."), pGroupGeo);
    mBtnInclinationFile->setFixedWidth(32);
    mBtnInclinationFileView = new QPushButton(QStringLiteral("显示"), pGroupGeo);
    mBtnInclinationFileView->setFixedWidth(48);
    pGeoLayout->addWidget(new QLabel(QStringLiteral("倾角文件"), pGroupGeo), 4, 0);
    pGeoLayout->addWidget(mEditInclinationFile, 4, 1, 1, 4);
    pGeoLayout->addWidget(mBtnInclinationFile, 4, 5);
    pGeoLayout->addWidget(mBtnInclinationFileView, 4, 6);
    mEditDeclinationFile = new QLineEdit(pGroupGeo);
    mBtnDeclinationFile = new QPushButton(QStringLiteral("..."), pGroupGeo);
    mBtnDeclinationFile->setFixedWidth(32);
    mBtnDeclinationFileView = new QPushButton(QStringLiteral("显示"), pGroupGeo);
    mBtnDeclinationFileView->setFixedWidth(48);
    pGeoLayout->addWidget(new QLabel(QStringLiteral("偏角文件"), pGroupGeo), 5, 0);
    pGeoLayout->addWidget(mEditDeclinationFile, 5, 1, 1, 4);
    pGeoLayout->addWidget(mBtnDeclinationFile, 5, 5);
    pGeoLayout->addWidget(mBtnDeclinationFileView, 5, 6);
    pGeoLayout->setColumnStretch(1, 1);
    pGeoLayout->setColumnStretch(2, 1);
    pGeoLayout->setColumnStretch(3, 1);
    pGeoLayout->setColumnStretch(4, 1);
    pMainLayout->addWidget(pGroupGeo);

    // ================= 数据文件输出 =================
    // 对应 .rc 中的 IDC_SaveFile / ID_SaveFile / ID_SaveFileView
    QGroupBox* pGroupOutput = new QGroupBox(QStringLiteral("数据文件输出"), this);
    QGridLayout* pOutputLayout = new QGridLayout(pGroupOutput);
    mEditSaveFile = new QLineEdit(pGroupOutput);
    mBtnSaveFile = new QPushButton(QStringLiteral("..."), pGroupOutput);
    mBtnSaveFile->setFixedWidth(32);
    mBtnSaveFileView = new QPushButton(QStringLiteral("显示"), pGroupOutput);
    mBtnSaveFileView->setFixedWidth(48);
    pOutputLayout->addWidget(mEditSaveFile, 0, 0);
    pOutputLayout->addWidget(mBtnSaveFile, 0, 1);
    pOutputLayout->addWidget(mBtnSaveFileView, 0, 2);
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
    // 分组框标题居中（对应原工程 GROUPBOX 的 BS_CENTER 样式，见示例图片 SubWindow.png）
    for (QGroupBox* pGroup : findChildren<QGroupBox*>())
    {
        pGroup->setAlignment(Qt::AlignCenter);
    }

    // 输入/输出文件
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onOpenFileViewClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CMagnToPoleVarIncDecDlg::onOpenFileTextEdited);
    connect(mBtnSaveFile, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onSaveFileClicked);
    connect(mBtnSaveFileView, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onSaveFileViewClicked);
    connect(mEditSaveFile, &QLineEdit::textEdited, this, &CMagnToPoleVarIncDecDlg::onSaveFileTextEdited);

    // 倾角/偏角数据文件
    connect(mBtnInclinationFile, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onInclinationFileClicked);
    connect(mBtnInclinationFileView, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onInclinationFileViewClicked);
    connect(mEditInclinationFile, &QLineEdit::textEdited, this, &CMagnToPoleVarIncDecDlg::onInclinationFileTextEdited);
    connect(mBtnDeclinationFile, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onDeclinationFileClicked);
    connect(mBtnDeclinationFileView, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onDeclinationFileViewClicked);
    connect(mEditDeclinationFile, &QLineEdit::textEdited, this, &CMagnToPoleVarIncDecDlg::onDeclinationFileTextEdited);

    // 扩充行/列数微调
    connect(mSpinExRows, QOverload<int>::of(&QSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onExRowsValueChanged);
    connect(mSpinExCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onExColsValueChanged);

    // 扩边方法单选组（id 与 ExpandMethod 枚举一致）
    connect(mButtonGroupExpand, &QButtonGroup::idClicked, this, &CMagnToPoleVarIncDecDlg::onExpandMethodClicked);

    // 处理参数（方位角）
    connect(mSpinProfile, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onProfileValueChanged);
    connect(mSpinBaseLine, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onBaseLineValueChanged);

    // 复选框
    connect(mCheckLinear, &QCheckBox::toggled, this, &CMagnToPoleVarIncDecDlg::onCheckLinearToggled);

    // 四角点磁化倾角/偏角（角度值微调框）
    connect(mSpinInc1, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onInc1ValueChanged);
    connect(mSpinInc2, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onInc2ValueChanged);
    connect(mSpinInc3, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onInc3ValueChanged);
    connect(mSpinInc4, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onInc4ValueChanged);
    connect(mSpinDec1, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onDec1ValueChanged);
    connect(mSpinDec2, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onDec2ValueChanged);
    connect(mSpinDec3, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onDec3ValueChanged);
    connect(mSpinDec4, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarIncDecDlg::onDec4ValueChanged);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CMagnToPoleVarIncDecDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：选择输入数据文件（对应原工程 OnOpenFile）
void CMagnToPoleVarIncDecDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeOpenFile）
void CMagnToPoleVarIncDecDlg::onOpenFileTextEdited(const QString& strText)
{
    mInputFilePath = strText;
}

// 功能：读取文件头并刷新网格信息、默认值（对应原工程 ReadData）
void CMagnToPoleVarIncDecDlg::loadFile(const QString& strFilePath)
{
    if (strFilePath.isEmpty())
    {
        return;
    }
    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        return;
    }
    // 与最近一次成功加载的文件相同则不再重复读取
    if (strFilePath == mLoadedFilePath)
    {
        return;
    }

    // 调用后端读取网格文件头（仅文件头，不读取数据体）
    GridFileHead head;
    BackendError error;
    if (!pBackend->readGridFileHead(toBackendString(strFilePath), head, error))
    {
        QMessageBox::warning(this, QStringLiteral("读取文件失败"), fromBackendString(error.message));
        return;
    }

    // 记录加载状态并同步输入框显示
    mLoadedFilePath = strFilePath;
    mInputFilePath = strFilePath;
    if (mEditOpenFile != NULL)
    {
        mEditOpenFile->setText(strFilePath);
    }

    // 网格信息（与原工程 xStep/yStep 计算一致）
    mColsNum = head.cols;
    mRowsNum = head.rows;
    mColStep = (head.cols > 1) ? (head.xMax - head.xMin) / (head.cols - 1) : 0.0;
    mRowStep = (head.rows > 1) ? (head.yMax - head.yMin) / (head.rows - 1) : 0.0;

    updateDefaultSavePaths(strFilePath);
    updateGridInfoDisplay();

    // 默认扩充尺寸（2 的幂，规则见 FrontendUtils.h，与原工程一致）
    mMinExCols = suggestExtendSize(mColsNum);
    mMinExRows = suggestExtendSize(mRowsNum);
    setSpinExtendSize(mSpinExCols, mMinExCols);
    setSpinExtendSize(mSpinExRows, mMinExRows);
}

// 功能：生成默认输出文件名（对应原工程 ReadData 中 基准名+"Pro.grd" 命名规则）
void CMagnToPoleVarIncDecDlg::updateDefaultSavePaths(const QString& strFilePath)
{
    // 取最后一个点号之前的内容作为基准名（与原工程 ReverseFind('.') 一致）
    int nPos = strFilePath.lastIndexOf('.');
    QString strBase = strFilePath;
    if (nPos > 0)
    {
        strBase = strFilePath.left(nPos);
    }

    mOutputFilePath = strBase + QStringLiteral("Pro.grd");

    if (mEditSaveFile != NULL)
    {
        mEditSaveFile->setText(mOutputFilePath);
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CMagnToPoleVarIncDecDlg::updateGridInfoDisplay()
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

// 功能：同步设置扩充微调框下限与数值（下限即当前文件要求的最小扩充尺寸）
void CMagnToPoleVarIncDecDlg::setSpinExtendSize(QSpinBox* pSpin, int nSize)
{
    if (pSpin == NULL)
    {
        return;
    }
    if (nSize < 1)
    {
        nSize = 1;
    }
    pSpin->setRange(nSize, sMaxExtendSize);
    pSpin->setValue(nSize);
}

// 功能：选择处理结果数据文件（对应原工程 OnSaveFile）
void CMagnToPoleVarIncDecDlg::onSaveFileClicked()
{
    QString strFilePath = askSaveFilePath(QStringLiteral("请输入处理结果数据文件名"), mOutputFilePath);
    if (strFilePath.isEmpty())
    {
        return;
    }
    mOutputFilePath = strFilePath;
    if (mEditSaveFile != NULL)
    {
        mEditSaveFile->setText(strFilePath);
    }
}

// 功能：手动输入输出文件名（同步成员变量，对应原工程 OnChangeSaveFile）
void CMagnToPoleVarIncDecDlg::onSaveFileTextEdited(const QString& strText)
{
    mOutputFilePath = strText;
}

// 功能：选择磁化倾角数据文件（对应原工程“...”按钮）
void CMagnToPoleVarIncDecDlg::onInclinationFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择磁化倾角数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mInclinationFilePath = strFilePath;
    if (mEditInclinationFile != NULL)
    {
        mEditInclinationFile->setText(strFilePath);
    }
}

// 功能：手动输入倾角文件名（同步成员变量）
void CMagnToPoleVarIncDecDlg::onInclinationFileTextEdited(const QString& strText)
{
    mInclinationFilePath = strText;
}

// 功能：选择磁化偏角数据文件（对应原工程“...”按钮）
void CMagnToPoleVarIncDecDlg::onDeclinationFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择磁化偏角数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mDeclinationFilePath = strFilePath;
    if (mEditDeclinationFile != NULL)
    {
        mEditDeclinationFile->setText(strFilePath);
    }
}

// 功能：手动输入偏角文件名（同步成员变量）
void CMagnToPoleVarIncDecDlg::onDeclinationFileTextEdited(const QString& strText)
{
    mDeclinationFilePath = strText;
}

// 功能：弹出打开文件对话框（.grd 网格数据文件）
QString CMagnToPoleVarIncDecDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框（.grd 网格数据文件）
QString CMagnToPoleVarIncDecDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath,
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：检查文件存在并发出发送显示请求（对应原工程 OnXxxFileView）
void CMagnToPoleVarIncDecDlg::viewGridFile(const QString& strFilePath)
{
    if (strFilePath.isEmpty())
    {
        return;
    }
    // 文件不存在时提示并返回（与原工程 GetFileAttributes == -1 判断一致）
    if (!QFile::exists(strFilePath))
    {
        QMessageBox::warning(this, QStringLiteral("文件不存在"),
            QStringLiteral("文件") + strFilePath + QStringLiteral("不存在!"));
        return;
    }
    // 等值线显示窗口由前端后续版本实现，此处交由宿主处理
    emit viewGridFileRequested(strFilePath);
}

// 功能：显示输入数据文件等值线
void CMagnToPoleVarIncDecDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示处理结果文件等值线
void CMagnToPoleVarIncDecDlg::onSaveFileViewClicked()
{
    viewGridFile(mOutputFilePath);
}

// 功能：显示磁化倾角数据文件等值线
void CMagnToPoleVarIncDecDlg::onInclinationFileViewClicked()
{
    viewGridFile(mInclinationFilePath);
}

// 功能：显示磁化偏角数据文件等值线
void CMagnToPoleVarIncDecDlg::onDeclinationFileViewClicked()
{
    viewGridFile(mDeclinationFilePath);
}

// 功能：扩充行数变化（同步成员变量）
void CMagnToPoleVarIncDecDlg::onExRowsValueChanged(int nValue)
{
    mExRows = nValue;
}

// 功能：扩充列数变化（同步成员变量）
void CMagnToPoleVarIncDecDlg::onExColsValueChanged(int nValue)
{
    mExCols = nValue;
}

// 功能：数据扩边方法切换（同步成员变量）
void CMagnToPoleVarIncDecDlg::onExpandMethodClicked(int nMethodId)
{
    mExpandMethod = nMethodId;
}

// 功能：行方位角变化（同步成员变量）
void CMagnToPoleVarIncDecDlg::onProfileValueChanged(double dValue)
{
    mProfile = dValue;
}

// 功能：列方位角变化（同步成员变量）
void CMagnToPoleVarIncDecDlg::onBaseLineValueChanged(double dValue)
{
    mBaseLine = dValue;
}

// 功能：“四角点近似线性变化”勾选状态变化
void CMagnToPoleVarIncDecDlg::onCheckLinearToggled(bool bChecked)
{
    mCheckLinearState = bChecked;
}

// 功能：角点1~4 磁化倾角变化（同步成员变量）
void CMagnToPoleVarIncDecDlg::onInc1ValueChanged(double dValue) { mInc1 = dValue; }
void CMagnToPoleVarIncDecDlg::onInc2ValueChanged(double dValue) { mInc2 = dValue; }
void CMagnToPoleVarIncDecDlg::onInc3ValueChanged(double dValue) { mInc3 = dValue; }
void CMagnToPoleVarIncDecDlg::onInc4ValueChanged(double dValue) { mInc4 = dValue; }

// 功能：角点1~4 磁化偏角变化（同步成员变量）
void CMagnToPoleVarIncDecDlg::onDec1ValueChanged(double dValue) { mDec1 = dValue; }
void CMagnToPoleVarIncDecDlg::onDec2ValueChanged(double dValue) { mDec2 = dValue; }
void CMagnToPoleVarIncDecDlg::onDec3ValueChanged(double dValue) { mDec3 = dValue; }
void CMagnToPoleVarIncDecDlg::onDec4ValueChanged(double dValue) { mDec4 = dValue; }

// 功能：确定前校验输入项（对应原工程 OnOK 中的空值检查，并给出更明确的提示）
bool CMagnToPoleVarIncDecDlg::validateInputs(QString& strError)
{
    // 输入数据文件检查
    if (mInputFilePath.isEmpty())
    {
        strError = QStringLiteral("请先选择输入数据文件。");
        return false;
    }
    if (!QFile::exists(mInputFilePath))
    {
        strError = QStringLiteral("输入数据文件不存在：") + mInputFilePath;
        return false;
    }
    // 输出文件检查
    if (mOutputFilePath.isEmpty())
    {
        strError = QStringLiteral("请输入处理结果数据文件路径。");
        return false;
    }
    // 扩充尺寸检查（必须不小于读文件时计算的最小尺寸）
    if (mExCols < mMinExCols || mExRows < mMinExRows)
    {
        strError = QStringLiteral("扩充行数/列数不得小于网格数据要求的最小尺寸。");
        return false;
    }
    return true;
}

// 功能：调用后端 processMagnToPoleVarIncDec（对应原工程 OnOK 主体逻辑）
// 说明：后端接口（processMagnToPoleVarIncDec）尚未由后端提供，本函数当前仅做参数校验与占位提示；
//       后端提供接口后，在下方注释位置接入：
//       IRgisBackend* pBackend = CBackendService::rgisBackend();
//       组装 MagnToPoleVarIncDecParams（输入文件、输出文件、扩充行列数、扩边方法、行/列方位角、
//       四角点磁化倾角/偏角、线性变化开关、倾角/偏角数据文件路径），
//       调用 pBackend->processMagnToPoleVarIncDec(params, error) 完成处理，
//       完成后提示“低磁纬度化极计算结束!”（与原工程一致，处理完成后对话框不关闭）。
//       后端处理约定（与原工程 OnOK 一致）：
//         1. 校验倾角/偏角数据文件与输入文件行列数、坐标范围一致；
//         2. 四角点线性变化插值 → 缺失数据插值、扩边、FFT、变磁化倾角化极算子、
//            FFT 求逆、截取原网格、缺失数据还原，按输出文件写出（DSBB）。
void CMagnToPoleVarIncDecDlg::runProcess()
{
    // TODO(后端对接)：后端接口 processMagnToPoleVarIncDec 提供后在此接入（见上方说明）。
    // 当前：前端界面已完成，后端未接入，给出占位提示。
    QMessageBox::warning(this, QStringLiteral("处理失败"),
        QStringLiteral("算法后端尚未接入：变磁化倾角化极功能等待后端实现。"));
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CMagnToPoleVarIncDecDlg::onOkClicked()
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
void CMagnToPoleVarIncDecDlg::onCancelClicked()
{
    reject();
}

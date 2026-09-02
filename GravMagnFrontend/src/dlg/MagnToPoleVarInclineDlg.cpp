// MagnToPoleVarInclineDlg.cpp : 分带变磁倾角化极对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_MagnToPoleVarIncline；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）；
//   * 后端接口（processMagnToPoleVarIncline）未由后端提供，本对话框当前仅实现界面与参数校验，
//     接入方式见 runProcess 注释。

#include "MagnToPoleVarInclineDlg.h"

#include <QApplication>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include "FrontendUtils.h"

// 最大扩充尺寸（与原 MFC 工程 OnDeltaposSpinExRows/ExCols 中 <= 65536 的约束一致）
static const int sMaxExtendSize = 65536;

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CMagnToPoleVarInclineDlg::CMagnToPoleVarInclineDlg(QWidget* pParent)
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
    , mSpinLatiMax(NULL)
    , mSpinLatiMin(NULL)
    , mSpinLatiStp(NULL)
    , mEditRegRows(NULL)
    , mTableBlocks(NULL)
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
    , mLatiMax(44.0)
    , mLatiMin(40.0)
    , mLatiStp(1.0)
    , mRegRows(0)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CMagnToPoleVarInclineDlg::~CMagnToPoleVarInclineDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CMagnToPoleVarInclineDlg::initUi()
{
    setWindowTitle(QStringLiteral("分带变磁倾角化极"));
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
    // 对应 .rc 中的 IDC_EDIT_RowsNum / IDC_EDIT_ColsNum / IDC_EDIT_RowStep / IDC_EDIT_ColStep
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

    // ================= 分带参数输入 =================
    // 对应 .rc 中的 IDC_LATIMIN / IDC_LATIMAX / IDC_LATISTP / IDC_REGROWS
    QGroupBox* pGroupLati = new QGroupBox(QStringLiteral("分带参数输入"), this);
    QGridLayout* pLatiLayout = new QGridLayout(pGroupLati);
    pLatiLayout->addWidget(new QLabel(QStringLiteral("纬度最大值"), pGroupLati), 0, 0);
    mSpinLatiMax = new QDoubleSpinBox(pGroupLati);
    mSpinLatiMax->setRange(0.0, 90.0);
    mSpinLatiMax->setDecimals(3);
    mSpinLatiMax->setSingleStep(1.0);
    mSpinLatiMax->setValue(mLatiMax);
    mSpinLatiMax->setAlignment(Qt::AlignCenter);
    mSpinLatiMax->setFixedWidth(100);
    pLatiLayout->addWidget(mSpinLatiMax, 0, 1);
    pLatiLayout->addWidget(new QLabel(QStringLiteral("纬度最小值"), pGroupLati), 0, 2);
    mSpinLatiMin = new QDoubleSpinBox(pGroupLati);
    mSpinLatiMin->setRange(0.0, 90.0);
    mSpinLatiMin->setDecimals(3);
    mSpinLatiMin->setSingleStep(1.0);
    mSpinLatiMin->setValue(mLatiMin);
    mSpinLatiMin->setAlignment(Qt::AlignCenter);
    mSpinLatiMin->setFixedWidth(100);
    pLatiLayout->addWidget(mSpinLatiMin, 0, 3);
    pLatiLayout->addWidget(new QLabel(QStringLiteral("纬度间隔"), pGroupLati), 0, 4);
    mSpinLatiStp = new QDoubleSpinBox(pGroupLati);
    mSpinLatiStp->setRange(0.0, 4.0);
    mSpinLatiStp->setDecimals(3);
    mSpinLatiStp->setSingleStep(1.0);
    mSpinLatiStp->setValue(mLatiStp);
    mSpinLatiStp->setAlignment(Qt::AlignCenter);
    mSpinLatiStp->setFixedWidth(100);
    pLatiLayout->addWidget(mSpinLatiStp, 0, 5);
    pLatiLayout->addWidget(new QLabel(QStringLiteral("纬度分块数"), pGroupLati), 0, 6);
    mEditRegRows = new QLineEdit(pGroupLati);
    mEditRegRows->setReadOnly(true);
    mEditRegRows->setAlignment(Qt::AlignCenter);
    mEditRegRows->setFixedWidth(80);
    pLatiLayout->addWidget(mEditRegRows, 0, 7);
    pLatiLayout->setColumnStretch(1, 1);
    pLatiLayout->setColumnStretch(3, 1);
    pLatiLayout->setColumnStretch(5, 1);
    pMainLayout->addWidget(pGroupLati);

    // ================= 测区地磁场参数输入（分带表格）=================
    // 对应 .rc 中的 MSFlexGrid（4 列：分块序号 / 中心纬度 / 地磁倾角 / 地磁偏角）
    QGroupBox* pGroupGeo = new QGroupBox(QStringLiteral("测区地磁场参数输入（单位：度）"), this);
    QGridLayout* pGeoLayout = new QGridLayout(pGroupGeo);
    mTableBlocks = new QTableWidget(pGroupGeo);
    mTableBlocks->setColumnCount(4);
    mTableBlocks->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("分块序号")
        << QStringLiteral("中心纬度")
        << QStringLiteral("地磁倾角")
        << QStringLiteral("地磁偏角"));
    mTableBlocks->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mTableBlocks->verticalHeader()->setVisible(false);
    mTableBlocks->setFixedHeight(200);
    pGeoLayout->addWidget(mTableBlocks, 0, 0);
    pGeoLayout->setRowStretch(0, 1);
    pMainLayout->addWidget(pGroupGeo, 1);

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
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CMagnToPoleVarInclineDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CMagnToPoleVarInclineDlg::onOpenFileViewClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CMagnToPoleVarInclineDlg::onOpenFileTextEdited);
    connect(mBtnSaveFile, &QPushButton::clicked, this, &CMagnToPoleVarInclineDlg::onSaveFileClicked);
    connect(mBtnSaveFileView, &QPushButton::clicked, this, &CMagnToPoleVarInclineDlg::onSaveFileViewClicked);
    connect(mEditSaveFile, &QLineEdit::textEdited, this, &CMagnToPoleVarInclineDlg::onSaveFileTextEdited);

    // 扩充行/列数微调
    connect(mSpinExRows, QOverload<int>::of(&QSpinBox::valueChanged), this, &CMagnToPoleVarInclineDlg::onExRowsValueChanged);
    connect(mSpinExCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &CMagnToPoleVarInclineDlg::onExColsValueChanged);

    // 扩边方法单选组（id 与 ExpandMethod 枚举一致）
    connect(mButtonGroupExpand, &QButtonGroup::idClicked, this, &CMagnToPoleVarInclineDlg::onExpandMethodClicked);

    // 处理参数（方位角）
    connect(mSpinProfile, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarInclineDlg::onProfileValueChanged);
    connect(mSpinBaseLine, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarInclineDlg::onBaseLineValueChanged);

    // 分带纬度参数（变化后重建分带表格，对应原工程纬度参数编辑后的表格重算）
    connect(mSpinLatiMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarInclineDlg::onLatiMaxValueChanged);
    connect(mSpinLatiMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarInclineDlg::onLatiMinValueChanged);
    connect(mSpinLatiStp, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CMagnToPoleVarInclineDlg::onLatiStpValueChanged);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CMagnToPoleVarInclineDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CMagnToPoleVarInclineDlg::onCancelClicked);

    // 初始重建分带表格（对应原工程 OnInitDialog 的表格初始化）
    rebuildBlockTable();

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：按纬度参数计算纬度分块数（原工程 OnOK 中的分块数规则）
// 说明：=(max-min)/stp；<=0 视为 1；<1 为 1；恰整除取商；否则取商+1
int CMagnToPoleVarInclineDlg::calcBlockCount() const
{
    if (mLatiStp <= 0.0)
    {
        return 1;
    }
    double dSpan = mLatiMax - mLatiMin;
    double dTemp = dSpan / mLatiStp;
    if (dTemp <= 0.0)
    {
        return 1;
    }
    if (dTemp < 1.0)
    {
        return 1;
    }
    int nTemp = (int)dTemp;
    if (nTemp * mLatiStp == dSpan)
    {
        return nTemp;
    }
    return nTemp + 1;
}

// 功能：按纬度最大值/最小值/间隔重建分带表格（对应原工程表格初始化）
// 说明：行数 = 纬度分块数；预填“分块序号”（1.0 ~ 分块数）与“中心纬度”（最小值 + (序号-0.5)*间隔），
//       地磁倾角/地磁偏角留空，由用户输入（原工程 MSFlexGrid 同样留空待用户填写）
void CMagnToPoleVarInclineDlg::rebuildBlockTable()
{
    if (mTableBlocks == NULL)
    {
        return;
    }

    mRegRows = calcBlockCount();
    if (mEditRegRows != NULL)
    {
        mEditRegRows->setText(QString::number(mRegRows));
    }

    mTableBlocks->setRowCount(mRegRows);
    for (int i = 0; i < mRegRows; i++)
    {
        // 分块序号（1 起）
        QTableWidgetItem* pSeqItem = new QTableWidgetItem(QString::number(i + 1));
        pSeqItem->setTextAlignment(Qt::AlignCenter);
        pSeqItem->setFlags(pSeqItem->flags() & ~Qt::ItemIsEditable);
        mTableBlocks->setItem(i, 0, pSeqItem);

        // 中心纬度 = 最小值 + (序号 - 0.5) * 间隔（与原工程纬度分带中心规则一致）
        double dCenter = mLatiMin + (i + 0.5) * mLatiStp;
        QTableWidgetItem* pCenterItem = new QTableWidgetItem(QString::number(dCenter, 'f', 3));
        pCenterItem->setTextAlignment(Qt::AlignCenter);
        pCenterItem->setFlags(pCenterItem->flags() & ~Qt::ItemIsEditable);
        mTableBlocks->setItem(i, 1, pCenterItem);

        // 地磁倾角/偏角：留空，由用户输入
        mTableBlocks->setItem(i, 2, new QTableWidgetItem(QString()));
        mTableBlocks->setItem(i, 3, new QTableWidgetItem(QString()));
    }
}

// 功能：选择输入数据文件（对应原工程 OnOpenFile）
void CMagnToPoleVarInclineDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeOpenFile）
void CMagnToPoleVarInclineDlg::onOpenFileTextEdited(const QString& strText)
{
    mInputFilePath = strText;
}

// 功能：读取文件头并刷新网格信息、默认值（对应原工程 ReadData）
void CMagnToPoleVarInclineDlg::loadFile(const QString& strFilePath)
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

// 功能：生成默认输出文件名（对应原工程 ReadData 中 基准名+"_RTP.grd" 命名规则）
void CMagnToPoleVarInclineDlg::updateDefaultSavePaths(const QString& strFilePath)
{
    // 取最后一个点号之前的内容作为基准名（与原工程 ReverseFind('.') 一致）
    int nPos = strFilePath.lastIndexOf('.');
    QString strBase = strFilePath;
    if (nPos > 0)
    {
        strBase = strFilePath.left(nPos);
    }

    mOutputFilePath = strBase + QStringLiteral("_RTP.grd");

    if (mEditSaveFile != NULL)
    {
        mEditSaveFile->setText(mOutputFilePath);
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CMagnToPoleVarInclineDlg::updateGridInfoDisplay()
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
void CMagnToPoleVarInclineDlg::setSpinExtendSize(QSpinBox* pSpin, int nSize)
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
void CMagnToPoleVarInclineDlg::onSaveFileClicked()
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
void CMagnToPoleVarInclineDlg::onSaveFileTextEdited(const QString& strText)
{
    mOutputFilePath = strText;
}

// 功能：弹出打开文件对话框（.grd 网格数据文件）
QString CMagnToPoleVarInclineDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框（.grd 网格数据文件）
QString CMagnToPoleVarInclineDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath,
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：检查文件存在并发出发送显示请求（对应原工程 OnOpenFileView / OnSaveFileView）
void CMagnToPoleVarInclineDlg::viewGridFile(const QString& strFilePath)
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
void CMagnToPoleVarInclineDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示处理结果文件等值线
void CMagnToPoleVarInclineDlg::onSaveFileViewClicked()
{
    viewGridFile(mOutputFilePath);
}

// 功能：扩充行数变化（同步成员变量）
void CMagnToPoleVarInclineDlg::onExRowsValueChanged(int nValue)
{
    mExRows = nValue;
}

// 功能：扩充列数变化（同步成员变量）
void CMagnToPoleVarInclineDlg::onExColsValueChanged(int nValue)
{
    mExCols = nValue;
}

// 功能：数据扩边方法切换（同步成员变量）
void CMagnToPoleVarInclineDlg::onExpandMethodClicked(int nMethodId)
{
    mExpandMethod = nMethodId;
}

// 功能：行方位角变化（同步成员变量）
void CMagnToPoleVarInclineDlg::onProfileValueChanged(double dValue)
{
    mProfile = dValue;
}

// 功能：列方位角变化（同步成员变量）
void CMagnToPoleVarInclineDlg::onBaseLineValueChanged(double dValue)
{
    mBaseLine = dValue;
}

// 功能：纬度最大值变化（重建分带表格）
void CMagnToPoleVarInclineDlg::onLatiMaxValueChanged(double dValue)
{
    mLatiMax = dValue;
    rebuildBlockTable();
}

// 功能：纬度最小值变化（重建分带表格）
void CMagnToPoleVarInclineDlg::onLatiMinValueChanged(double dValue)
{
    mLatiMin = dValue;
    rebuildBlockTable();
}

// 功能：纬度间隔变化（重建分带表格）
void CMagnToPoleVarInclineDlg::onLatiStpValueChanged(double dValue)
{
    mLatiStp = dValue;
    rebuildBlockTable();
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的空值检查，并给出更明确的提示）
bool CMagnToPoleVarInclineDlg::validateInputs(QString& strError)
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
    // 纬度参数检查（与原工程“测区纬度参数输入错误!”一致）
    if (mLatiStp <= 0.0 || mLatiMax <= mLatiMin)
    {
        strError = QStringLiteral("测区纬度参数输入错误!");
        return false;
    }
    // 分带表格检查（对应原工程 OnOK 读取 MSFlexGrid 的校验）
    if (mTableBlocks == NULL)
    {
        strError = QStringLiteral("测区地磁场参数表格未初始化。");
        return false;
    }
    for (int i = 0; i < mRegRows; i++)
    {
        QTableWidgetItem* pIncItem = mTableBlocks->item(i, 2);
        QTableWidgetItem* pDecItem = mTableBlocks->item(i, 3);
        if (pIncItem == NULL || pIncItem->text().isEmpty())
        {
            strError = QStringLiteral("第 %1 分带地磁倾角参数输入没有完成！").arg(i + 1);
            return false;
        }
        if (pDecItem == NULL || pDecItem->text().isEmpty())
        {
            strError = QStringLiteral("第 %1 分带地磁偏角参数输入没有完成！").arg(i + 1);
            return false;
        }
        // 范围校验（与原工程 -90~90 一致）
        bool bOk = false;
        double dInc = pIncItem->text().toDouble(&bOk);
        if (!bOk || dInc < -90.0 || dInc > 90.0)
        {
            strError = QStringLiteral("第 %1 分带地磁倾角参数输入错误！").arg(i + 1);
            return false;
        }
        double dDec = pDecItem->text().toDouble(&bOk);
        if (!bOk || dDec < -360.0 || dDec > 360.0)
        {
            strError = QStringLiteral("第 %1 分带地磁偏角参数输入错误！").arg(i + 1);
            return false;
        }
    }
    // 扩充尺寸检查（必须不小于读文件时计算的最小尺寸）
    if (mExCols < mMinExCols || mExRows < mMinExRows)
    {
        strError = QStringLiteral("扩充行数/列数不得小于网格数据要求的最小尺寸。");
        return false;
    }
    return true;
}

// 功能：调用后端 processMagnToPoleVarIncline（对应原工程 OnOK 主体逻辑）
// 说明：后端接口（processMagnToPoleVarIncline）尚未由后端提供，本函数当前仅做参数校验与占位提示；
//       后端提供接口后，在下方注释位置接入：
//       IRgisBackend* pBackend = CBackendService::rgisBackend();
//       组装 MagnToPoleVarInclineParams（输入文件、输出文件、扩充行列数、扩边方法、行/列方位角、
//       纬度最大值/最小值/间隔、各分带中心纬度/地磁倾角/地磁偏角（从分带表格读取）），
//       调用 pBackend->processMagnToPoleVarIncline(params, error) 完成处理，
//       完成后提示“磁异常分区化极计算结束 !”（与原工程一致，处理完成后对话框不关闭）。
//       后端处理约定（与原工程 OnOK 一致）：
//         1. 按纬度分块逐带做分区化极（每带取表内中心纬度、倾角、偏角，FFT + FreqDomainReToPoleProcess）；
//         2. 缺失数据插值、扩边、FFT 求逆、截取原网格、缺失数据还原，按输出文件写出（DSBB）。
void CMagnToPoleVarInclineDlg::runProcess()
{
    // TODO(后端对接)：后端接口 processMagnToPoleVarIncline 提供后在此接入（见上方说明）。
    // 当前：前端界面已完成，后端未接入，给出占位提示。
    QMessageBox::warning(this, QStringLiteral("处理失败"),
        QStringLiteral("算法后端尚未接入：分带变磁倾角化极功能等待后端实现。"));
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CMagnToPoleVarInclineDlg::onOkClicked()
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
void CMagnToPoleVarInclineDlg::onCancelClicked()
{
    reject();
}

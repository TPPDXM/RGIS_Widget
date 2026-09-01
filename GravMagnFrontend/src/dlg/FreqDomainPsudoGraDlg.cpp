// FreqDomainPsudoGraDlg.cpp : 频率域伪重力（磁源重力异常）对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_FreqDomainPsudoGra；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查。

#include "FreqDomainPsudoGraDlg.h"

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

// 最大扩边尺寸（与原 MFC 工程 OnDeltaposSpinExCols 中 <= 65536 的约束一致）
static const int sMaxExtendSize = 65536;

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CFreqDomainPsudoGraDlg::CFreqDomainPsudoGraDlg(QWidget* pParent)
    : QDialog(pParent)
    , mEditOpenFile(NULL)
    , mBtnOpenFile(NULL)
    , mBtnOpenFileView(NULL)
    , mEditRowsNum(NULL)
    , mEditColsNum(NULL)
    , mEditRowStep(NULL)
    , mEditColStep(NULL)
    , mSpinExRows(NULL)
    , mSpinExCols(NULL)
    , mRadioCosFun(NULL)
    , mRadioAvgDif(NULL)
    , mRadioInvPow(NULL)
    , mRadioMinCrv(NULL)
    , mButtonGroupExpand(NULL)
    , mSpinProfile(NULL)
    , mSpinBaseLine(NULL)
    , mSpinDti0(NULL)
    , mSpinDtd0(NULL)
    , mSpinMagnetization(NULL)
    , mSpinDensity(NULL)
    , mSpinCosine(NULL)
    , mEditSaveFile(NULL)
    , mBtnSaveFile(NULL)
    , mBtnSaveFileView(NULL)
    , mBtnOk(NULL)
    , mBtnCancel(NULL)
    , mRowsNum(0)
    , mColsNum(0)
    , mColStep(0.0)
    , mRowStep(0.0)
    , mExRows(0)
    , mExCols(0)
    , mMinExRows(1)
    , mMinExCols(1)
    , mExpandMethod(ExpandCosFun)
    , mProfile(90.0)
    , mBaseLine(0.0)
    , mDti0(90.0)
    , mDtd0(0.0)
    , mMagnetization(1.0)
    , mDensity(0.1)
    , mCosine(1)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CFreqDomainPsudoGraDlg::~CFreqDomainPsudoGraDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CFreqDomainPsudoGraDlg::initUi()
{
    setWindowTitle(QStringLiteral("磁源重力异常"));
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
    // 对应 .rc 中的 IDC_RowsNum / IDC_ColsNum / IDC_RowStep / IDC_ColStep
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

    // ================= 数据扩边信息 =================
    // 对应 .rc 中的 IDC_ExRows、IDC_ExCols 及对应微调钮
    QGroupBox* pGroupExtend = new QGroupBox(QStringLiteral("数据扩边信息"), this);
    QGridLayout* pExtendLayout = new QGridLayout(pGroupExtend);
    pExtendLayout->addWidget(new QLabel(QStringLiteral("扩边行数"), pGroupExtend), 0, 0);
    mSpinExRows = new QSpinBox(pGroupExtend);
    mSpinExRows->setRange(1, sMaxExtendSize);
    mSpinExRows->setValue(1);
    mSpinExRows->setAlignment(Qt::AlignCenter);
    mSpinExRows->setFixedWidth(100);
    pExtendLayout->addWidget(mSpinExRows, 0, 1);
    pExtendLayout->addWidget(new QLabel(QStringLiteral("扩边列数"), pGroupExtend), 0, 2);
    mSpinExCols = new QSpinBox(pGroupExtend);
    mSpinExCols->setRange(1, sMaxExtendSize);
    mSpinExCols->setValue(1);
    mSpinExCols->setAlignment(Qt::AlignCenter);
    mSpinExCols->setFixedWidth(100);
    pExtendLayout->addWidget(mSpinExCols, 0, 3);
    pMainLayout->addWidget(pGroupExtend);

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

    // ================= 处理参数（两行）=================
    // 对应 .rc 中的 IDC_Profile / IDC_BaseLine / IDC_DTI0 / IDC_DTD0 /
    //           IDC_Magnetization / IDC_Density / IDC_Cosine
    // （取值范围与原工程 DDV_MinMaxFloat / DDV_MinMaxInt 一致）
    QGroupBox* pGroupParams = new QGroupBox(QStringLiteral("处理参数（磁化强度为 A/m，剩余密度为 g/cm^3）"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    // 第一行：测线方向 / 基线方向 / 磁化倾角 / 磁化偏角
    pParamsLayout->addWidget(new QLabel(QStringLiteral("测线方向"), pGroupParams), 0, 0);
    mSpinProfile = new QDoubleSpinBox(pGroupParams);
    mSpinProfile->setRange(-360.0, 360.0);
    mSpinProfile->setDecimals(3);
    mSpinProfile->setSingleStep(1.0);
    mSpinProfile->setValue(mProfile);
    mSpinProfile->setAlignment(Qt::AlignCenter);
    mSpinProfile->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinProfile, 0, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("基线方向"), pGroupParams), 0, 2);
    mSpinBaseLine = new QDoubleSpinBox(pGroupParams);
    mSpinBaseLine->setRange(-360.0, 360.0);
    mSpinBaseLine->setDecimals(3);
    mSpinBaseLine->setSingleStep(1.0);
    mSpinBaseLine->setValue(mBaseLine);
    mSpinBaseLine->setAlignment(Qt::AlignCenter);
    mSpinBaseLine->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinBaseLine, 0, 3);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("磁化倾角"), pGroupParams), 0, 4);
    mSpinDti0 = new QDoubleSpinBox(pGroupParams);
    mSpinDti0->setRange(0.0, 180.0);
    mSpinDti0->setDecimals(3);
    mSpinDti0->setSingleStep(1.0);
    mSpinDti0->setValue(mDti0);
    mSpinDti0->setAlignment(Qt::AlignCenter);
    mSpinDti0->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinDti0, 0, 5);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("磁化偏角"), pGroupParams), 0, 6);
    mSpinDtd0 = new QDoubleSpinBox(pGroupParams);
    mSpinDtd0->setRange(-360.0, 360.0);
    mSpinDtd0->setDecimals(3);
    mSpinDtd0->setSingleStep(1.0);
    mSpinDtd0->setValue(mDtd0);
    mSpinDtd0->setAlignment(Qt::AlignCenter);
    mSpinDtd0->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinDtd0, 0, 7);
    // 第二行：磁化强度 / 剩余密度 / 衰减因子
    pParamsLayout->addWidget(new QLabel(QStringLiteral("磁化强度"), pGroupParams), 1, 0);
    mSpinMagnetization = new QDoubleSpinBox(pGroupParams);
    mSpinMagnetization->setRange(0.0, 1.0e+12);
    mSpinMagnetization->setDecimals(3);
    mSpinMagnetization->setSingleStep(1.0);
    mSpinMagnetization->setValue(mMagnetization);
    mSpinMagnetization->setAlignment(Qt::AlignCenter);
    mSpinMagnetization->setFixedWidth(130);
    pParamsLayout->addWidget(mSpinMagnetization, 1, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("剩余密度"), pGroupParams), 1, 2);
    mSpinDensity = new QDoubleSpinBox(pGroupParams);
    mSpinDensity->setRange(-5.0, 5.0);
    mSpinDensity->setDecimals(3);
    mSpinDensity->setSingleStep(0.1);
    mSpinDensity->setValue(mDensity);
    mSpinDensity->setAlignment(Qt::AlignCenter);
    mSpinDensity->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinDensity, 1, 3);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("衰减因子"), pGroupParams), 1, 4);
    mSpinCosine = new QSpinBox(pGroupParams);
    mSpinCosine->setRange(1, 20);
    mSpinCosine->setValue(mCosine);
    mSpinCosine->setAlignment(Qt::AlignCenter);
    mSpinCosine->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinCosine, 1, 5);
    pMainLayout->addWidget(pGroupParams);

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

    // 输入文件
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CFreqDomainPsudoGraDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CFreqDomainPsudoGraDlg::onOpenFileViewClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CFreqDomainPsudoGraDlg::onOpenFileTextEdited);

    // 数据文件输出
    connect(mBtnSaveFile, &QPushButton::clicked, this, &CFreqDomainPsudoGraDlg::onSaveFileClicked);
    connect(mBtnSaveFileView, &QPushButton::clicked, this, &CFreqDomainPsudoGraDlg::onSaveFileViewClicked);
    // 输出文件名编辑框（手工输入时同步成员变量，对应原工程 EN_CHANGE→UpdateData）
    connect(mEditSaveFile, &QLineEdit::textEdited, this, &CFreqDomainPsudoGraDlg::onSaveFileTextEdited);

    // 扩边行数/列数微调
    connect(mSpinExRows, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainPsudoGraDlg::onExRowsValueChanged);
    connect(mSpinExCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainPsudoGraDlg::onExColsValueChanged);

    // 扩边方法单选组（id 与 ExpandMethod 枚举一致）
    connect(mButtonGroupExpand, &QButtonGroup::idClicked, this, &CFreqDomainPsudoGraDlg::onExpandMethodClicked);

    // 处理参数（角度/物性微调框）
    connect(mSpinProfile, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainPsudoGraDlg::onProfileValueChanged);
    connect(mSpinBaseLine, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainPsudoGraDlg::onBaseLineValueChanged);
    connect(mSpinDti0, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainPsudoGraDlg::onDti0ValueChanged);
    connect(mSpinDtd0, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainPsudoGraDlg::onDtd0ValueChanged);
    connect(mSpinMagnetization, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainPsudoGraDlg::onMagnetizationValueChanged);
    connect(mSpinDensity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainPsudoGraDlg::onDensityValueChanged);
    connect(mSpinCosine, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainPsudoGraDlg::onCosineValueChanged);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CFreqDomainPsudoGraDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CFreqDomainPsudoGraDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：选择输入数据文件（对应原工程 OnOpenFile）
void CFreqDomainPsudoGraDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeOpenFile）
void CFreqDomainPsudoGraDlg::onOpenFileTextEdited(const QString& strText)
{
    mInputFilePath = strText;
}

// 功能：读取文件头并刷新网格信息、默认值（对应原工程 ReadData）
void CFreqDomainPsudoGraDlg::loadFile(const QString& strFilePath)
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

    // 默认扩边尺寸（2 的幂，规则见 FrontendUtils.h，与原工程一致）
    mMinExCols = suggestExtendSize(mColsNum);
    mMinExRows = suggestExtendSize(mRowsNum);
    setSpinExtendSize(mSpinExCols, mMinExCols);
    setSpinExtendSize(mSpinExRows, mMinExRows);
}

// 功能：生成默认输出文件名（对应原工程 ReadData 中 _Psudo.grd 命名规则）
void CFreqDomainPsudoGraDlg::updateDefaultSavePaths(const QString& strFilePath)
{
    // 取最后一个点号之前的内容作为基准名（与原工程 ReverseFind('.') 一致）
    int nPos = strFilePath.lastIndexOf('.');
    QString strBase = strFilePath;
    if (nPos > 0)
    {
        strBase = strFilePath.left(nPos);
    }

    mOutputFilePath = strBase + QStringLiteral("_Psudo.grd");

    if (mEditSaveFile != NULL)
    {
        mEditSaveFile->setText(mOutputFilePath);
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CFreqDomainPsudoGraDlg::updateGridInfoDisplay()
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

// 功能：同步设置扩边微调框下限与数值（下限即当前文件要求的最小扩边尺寸）
void CFreqDomainPsudoGraDlg::setSpinExtendSize(QSpinBox* pSpin, int nSize)
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
void CFreqDomainPsudoGraDlg::onSaveFileClicked()
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
void CFreqDomainPsudoGraDlg::onSaveFileTextEdited(const QString& strText)
{
    mOutputFilePath = strText;
}

// 功能：弹出打开文件对话框（.grd 网格数据文件）
QString CFreqDomainPsudoGraDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框（.grd 网格数据文件）
QString CFreqDomainPsudoGraDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath,
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：检查文件存在并发出发送显示请求（对应原工程 OnOpenFileView / OnSaveFileView）
void CFreqDomainPsudoGraDlg::viewGridFile(const QString& strFilePath)
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
void CFreqDomainPsudoGraDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示处理结果文件等值线
void CFreqDomainPsudoGraDlg::onSaveFileViewClicked()
{
    viewGridFile(mOutputFilePath);
}

// 功能：扩边行数变化（同步成员变量）
void CFreqDomainPsudoGraDlg::onExRowsValueChanged(int nValue)
{
    mExRows = nValue;
}

// 功能：扩边列数变化（同步成员变量）
void CFreqDomainPsudoGraDlg::onExColsValueChanged(int nValue)
{
    mExCols = nValue;
}

// 功能：数据扩边方法切换（同步成员变量）
void CFreqDomainPsudoGraDlg::onExpandMethodClicked(int nMethodId)
{
    mExpandMethod = nMethodId;
}

// 功能：测线方向变化（同步成员变量）
void CFreqDomainPsudoGraDlg::onProfileValueChanged(double dValue)
{
    mProfile = dValue;
}

// 功能：基线方向变化（同步成员变量）
void CFreqDomainPsudoGraDlg::onBaseLineValueChanged(double dValue)
{
    mBaseLine = dValue;
}

// 功能：磁化倾角变化（同步成员变量）
void CFreqDomainPsudoGraDlg::onDti0ValueChanged(double dValue)
{
    mDti0 = dValue;
}

// 功能：磁化偏角变化（同步成员变量）
void CFreqDomainPsudoGraDlg::onDtd0ValueChanged(double dValue)
{
    mDtd0 = dValue;
}

// 功能：磁化强度变化（同步成员变量）
void CFreqDomainPsudoGraDlg::onMagnetizationValueChanged(double dValue)
{
    mMagnetization = dValue;
}

// 功能：剩余密度变化（同步成员变量）
void CFreqDomainPsudoGraDlg::onDensityValueChanged(double dValue)
{
    mDensity = dValue;
}

// 功能：衰减因子变化（同步成员变量）
void CFreqDomainPsudoGraDlg::onCosineValueChanged(int nValue)
{
    mCosine = nValue;
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的空值检查，并给出更明确的提示）
bool CFreqDomainPsudoGraDlg::validateInputs(QString& strError)
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
    // 扩边尺寸检查（必须不小于读文件时计算的最小尺寸）
    if (mExCols < mMinExCols || mExRows < mMinExRows)
    {
        strError = QStringLiteral("扩边行数/列数不得小于网格数据要求的最小尺寸。");
        return false;
    }
    return true;
}

// 功能：组装参数并调用后端 processPsudoGra（对应原工程 OnOK 主体逻辑）
void CFreqDomainPsudoGraDlg::runProcess()
{
    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), QStringLiteral("后端接口未初始化。"));
        return;
    }

    // 组装处理参数（对应原工程界面上的全部输入项）
    PsudoGraParams params;
    params.inputFilePath = toBackendString(mInputFilePath);
    params.outputFilePath = toBackendString(mOutputFilePath);
    params.exRows = mExRows;
    params.exCols = mExCols;
    params.expandMethod = (ExpandMethod)mExpandMethod;
    params.profile = (float)mProfile;
    params.baseLine = (float)mBaseLine;
    params.dti0 = (float)mDti0;
    params.dtd0 = (float)mDtd0;
    params.magnetization = (float)mMagnetization;
    params.density = (float)mDensity;
    params.cosine = mCosine;

    // 同步调用后端处理（处理期间显示等待光标，与原工程 BeginWaitCursor 一致）
    BackendError error;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bOk = pBackend->processPsudoGra(params, error);
    QApplication::restoreOverrideCursor();

    if (bOk)
    {
        // 处理完成提示（与原工程一致，处理完成后对话框不关闭，由用户点“取消”退出）
        QMessageBox::information(this, QStringLiteral("处理完成"),
            QStringLiteral("磁源重力异常换算结束!"));
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), fromBackendString(error.message));
    }
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CFreqDomainPsudoGraDlg::onOkClicked()
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
void CFreqDomainPsudoGraDlg::onCancelClicked()
{
    reject();
}

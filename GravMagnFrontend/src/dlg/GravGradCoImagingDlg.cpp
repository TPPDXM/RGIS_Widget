// GravGradCoImagingDlg.cpp : 三维重力异常和梯度相关成像对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_GravGradCoImaging；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）；
//   * 后端接口（processGravGradCoImaging）由后端提供后接入，当前“确定”提示占位信息。

#include "GravGradCoImagingDlg.h"

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

#include "FrontendUtils.h"

// 场值类型枚举（与单选按钮组 id 对应，见类注释）
enum EFieldType { FieldTypeGz = 0, FieldTypeGxz = 1, FieldTypeGyz = 2, FieldTypeGzz = 3 };

// 滤波选择枚举（与单选按钮组 id 对应）
enum EFilterType { FilterNonEntropy = 0, FilterEntropy = 1 };

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CGravGradCoImagingDlg::CGravGradCoImagingDlg(QWidget* pParent)
    : QDialog(pParent)
    , mEditOpenFile(NULL)
    , mBtnOpenFile(NULL)
    , mBtnOpenFileView(NULL)
    , mEditRowsNum(NULL)
    , mEditColsNum(NULL)
    , mEditRowStep(NULL)
    , mEditColStep(NULL)
    , mRadioEntropyFilter(NULL)
    , mRadioNonEntropyFilter(NULL)
    , mButtonGroupFilter(NULL)
    , mRadioGz(NULL)
    , mRadioGxz(NULL)
    , mRadioGyz(NULL)
    , mRadioGzz(NULL)
    , mButtonGroupField(NULL)
    , mSpinZObs(NULL)
    , mSpinLayStep(NULL)
    , mSpinZMin(NULL)
    , mSpinZMax(NULL)
    , mEditSaveFile(NULL)
    , mBtnSaveFile(NULL)
    , mBtnSaveFileView(NULL)
    , mEditSaveInfoFile(NULL)
    , mBtnSaveInfoFile(NULL)
    , mBtnSaveInfoFileView(NULL)
    , mBtnOk(NULL)
    , mBtnCancel(NULL)
    , mRowsNum(0)
    , mColsNum(0)
    , mColStep(0.0)
    , mRowStep(0.0)
    , mZObs(0.0)
    , mLayStep(0.0)
    , mZMin(0.0)
    , mZMax(0.0)
    , mFieldType(FieldTypeGz)
    , mFilterType(FilterNonEntropy)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CGravGradCoImagingDlg::~CGravGradCoImagingDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CGravGradCoImagingDlg::initUi()
{
    setWindowTitle(QStringLiteral("三维重力异常和梯度相关成像"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 重力异常和梯度网格数据输入 =================
    QGroupBox* pGroupInput = new QGroupBox(QStringLiteral("重力异常和梯度网格数据输入"), this);
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

    // ================= 滤波选择（单选）=================
    // 对应 .rc 中的 IDC_EntropyFilter / IDC_NonEntropyFilter（默认不进行熵滤波）
    QGroupBox* pGroupFilter = new QGroupBox(QStringLiteral("滤波选择"), this);
    QHBoxLayout* pFilterLayout = new QHBoxLayout(pGroupFilter);
    mButtonGroupFilter = new QButtonGroup(this);
    mRadioEntropyFilter = new QRadioButton(QStringLiteral(" 进行熵滤波"), pGroupFilter);
    mRadioNonEntropyFilter = new QRadioButton(QStringLiteral("不进行熵滤波"), pGroupFilter);
    mButtonGroupFilter->addButton(mRadioEntropyFilter, FilterEntropy);
    mButtonGroupFilter->addButton(mRadioNonEntropyFilter, FilterNonEntropy);
    mRadioNonEntropyFilter->setChecked(true);   // 默认不进行熵滤波（与原工程 OnInitDialog 一致）
    mFilterType = FilterNonEntropy;
    pFilterLayout->addWidget(mRadioEntropyFilter);
    pFilterLayout->addWidget(mRadioNonEntropyFilter);
    pFilterLayout->addStretch(1);
    pMainLayout->addWidget(pGroupFilter);

    // ================= 场值类型（单选，4 种）=================
    // 对应 .rc 中的 IDC_Gz / IDC_Gxz / IDC_Gyz / IDC_Gzz（默认 Gz）
    QGroupBox* pGroupField = new QGroupBox(QStringLiteral("场值类型"), this);
    QHBoxLayout* pFieldLayout = new QHBoxLayout(pGroupField);
    mButtonGroupField = new QButtonGroup(this);
    mRadioGz = new QRadioButton(QStringLiteral("Gz"), pGroupField);
    mRadioGxz = new QRadioButton(QStringLiteral(" Gxz"), pGroupField);
    mRadioGyz = new QRadioButton(QStringLiteral("Gyz"), pGroupField);
    mRadioGzz = new QRadioButton(QStringLiteral("Gzz"), pGroupField);
    mButtonGroupField->addButton(mRadioGz, FieldTypeGz);
    mButtonGroupField->addButton(mRadioGxz, FieldTypeGxz);
    mButtonGroupField->addButton(mRadioGyz, FieldTypeGyz);
    mButtonGroupField->addButton(mRadioGzz, FieldTypeGzz);
    mRadioGz->setChecked(true);     // 默认 Gz（重力异常）
    mFieldType = FieldTypeGz;
    pFieldLayout->addWidget(mRadioGz);
    pFieldLayout->addWidget(mRadioGxz);
    pFieldLayout->addWidget(mRadioGyz);
    pFieldLayout->addWidget(mRadioGzz);
    pFieldLayout->addStretch(1);
    pMainLayout->addWidget(pGroupField);

    // ================= 处理参数 =================
    // 对应 .rc 中的 IDC_ZObs（观测面高度）/ IDC_LayStep（深度层间距）/
    //           IDC_ZMin（深度层起点）/ IDC_ZMax（深度层终点）
    QGroupBox* pGroupParams = new QGroupBox(QStringLiteral("处理参数"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("观测面高度"), pGroupParams), 0, 0);
    mSpinZObs = new QDoubleSpinBox(pGroupParams);
    mSpinZObs->setRange(-9999999.0, 9999999.0);
    mSpinZObs->setDecimals(3);
    mSpinZObs->setSingleStep(1.0);
    mSpinZObs->setValue(mZObs);
    mSpinZObs->setAlignment(Qt::AlignCenter);
    mSpinZObs->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinZObs, 0, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("深度层间距"), pGroupParams), 0, 2);
    mSpinLayStep = new QDoubleSpinBox(pGroupParams);
    mSpinLayStep->setRange(0.000001, 9999999.0);
    mSpinLayStep->setDecimals(3);
    mSpinLayStep->setSingleStep(1.0);
    mSpinLayStep->setValue(mLayStep);
    mSpinLayStep->setAlignment(Qt::AlignCenter);
    mSpinLayStep->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinLayStep, 0, 3);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("深度层起点"), pGroupParams), 1, 0);
    mSpinZMin = new QDoubleSpinBox(pGroupParams);
    mSpinZMin->setRange(-9999999.0, 9999999.0);
    mSpinZMin->setDecimals(3);
    mSpinZMin->setSingleStep(1.0);
    mSpinZMin->setValue(mZMin);
    mSpinZMin->setAlignment(Qt::AlignCenter);
    mSpinZMin->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinZMin, 1, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("深度层终点"), pGroupParams), 1, 2);
    mSpinZMax = new QDoubleSpinBox(pGroupParams);
    mSpinZMax->setRange(-9999999.0, 9999999.0);
    mSpinZMax->setDecimals(3);
    mSpinZMax->setSingleStep(1.0);
    mSpinZMax->setValue(mZMax);
    mSpinZMax->setAlignment(Qt::AlignCenter);
    mSpinZMax->setFixedWidth(110);
    pParamsLayout->addWidget(mSpinZMax, 1, 3);
    pMainLayout->addWidget(pGroupParams);

    // ================= 相关系数体数据文件输出 =================
    // 对应 .rc 中的 IDC_SaveFile / ID_SaveFile / ID_SaveFileView（.vol 体数据文件）
    QGroupBox* pGroupOutput = new QGroupBox(QStringLiteral("相关系数体数据文件输出"), this);
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

    // ================= 相关成像信息文件输出 =================
    // 对应 .rc 中的 IDC_SaveInfoFile / ID_SaveInfoFile（.log 信息文件）
    QGroupBox* pGroupInfoFile = new QGroupBox(QStringLiteral("相关成像信息文件输出"), this);
    QGridLayout* pInfoFileLayout = new QGridLayout(pGroupInfoFile);
    mEditSaveInfoFile = new QLineEdit(pGroupInfoFile);
    mBtnSaveInfoFile = new QPushButton(QStringLiteral("..."), pGroupInfoFile);
    mBtnSaveInfoFile->setFixedWidth(32);
    mBtnSaveInfoFileView = new QPushButton(QStringLiteral("显示"), pGroupInfoFile);
    mBtnSaveInfoFileView->setFixedWidth(48);
    pInfoFileLayout->addWidget(mEditSaveInfoFile, 0, 0);
    pInfoFileLayout->addWidget(mBtnSaveInfoFile, 0, 1);
    pInfoFileLayout->addWidget(mBtnSaveInfoFileView, 0, 2);
    pInfoFileLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupInfoFile);

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
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CGravGradCoImagingDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CGravGradCoImagingDlg::onOpenFileViewClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CGravGradCoImagingDlg::onOpenFileTextEdited);

    // 体数据 / 信息文件输出
    connect(mBtnSaveFile, &QPushButton::clicked, this, &CGravGradCoImagingDlg::onSaveFileClicked);
    connect(mBtnSaveFileView, &QPushButton::clicked, this, &CGravGradCoImagingDlg::onSaveFileViewClicked);
    connect(mEditSaveFile, &QLineEdit::textEdited, this, &CGravGradCoImagingDlg::onSaveFileTextEdited);
    connect(mBtnSaveInfoFile, &QPushButton::clicked, this, &CGravGradCoImagingDlg::onSaveInfoFileClicked);
    connect(mBtnSaveInfoFileView, &QPushButton::clicked, this, &CGravGradCoImagingDlg::onSaveInfoFileViewClicked);
    connect(mEditSaveInfoFile, &QLineEdit::textEdited, this, &CGravGradCoImagingDlg::onSaveInfoFileTextEdited);

    // 场值类型 / 滤波选择单选组
    connect(mButtonGroupField, &QButtonGroup::idClicked, this, &CGravGradCoImagingDlg::onFieldTypeClicked);
    connect(mButtonGroupFilter, &QButtonGroup::idClicked, this, &CGravGradCoImagingDlg::onFilterTypeClicked);

    // 处理参数
    connect(mSpinZObs, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravGradCoImagingDlg::onZObsValueChanged);
    connect(mSpinLayStep, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravGradCoImagingDlg::onLayStepValueChanged);
    connect(mSpinZMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravGradCoImagingDlg::onZMinValueChanged);
    connect(mSpinZMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGravGradCoImagingDlg::onZMaxValueChanged);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CGravGradCoImagingDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CGravGradCoImagingDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：选择输入数据文件（对应原工程 OnOpenFile）
void CGravGradCoImagingDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeOpenFile）
void CGravGradCoImagingDlg::onOpenFileTextEdited(const QString& strText)
{
    mInputFilePath = strText;
}

// 功能：读取文件头并刷新网格信息、默认值（对应原工程 ReadData）
void CGravGradCoImagingDlg::loadFile(const QString& strFilePath)
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

    updateGridInfoDisplay();

    // 输入文件基准名（用于生成默认输出文件名）与默认输出文件名
    int nPos = strFilePath.lastIndexOf('.');
    mFilePathBase = strFilePath;
    if (nPos > 0)
    {
        mFilePathBase = strFilePath.left(nPos);
    }
    updateDefaultSavePaths(mFilePathBase);

    // 默认深度层间距 = (列距 + 行距) / 2、默认深度层起点 = (列距 + 行距) / 4
    //（与原工程 ReadData 中的 "%.3f", (xStep + yStep) / 2.0f / 4.0f 一致）
    mLayStep = (mColStep + mRowStep) / 2.0;
    mZMin = (mColStep + mRowStep) / 4.0;
    if (mSpinLayStep != NULL)
    {
        mSpinLayStep->setValue(mLayStep);
    }
    if (mSpinZMin != NULL)
    {
        mSpinZMin->setValue(mZMin);
    }
}

// 功能：按场值类型生成默认输出文件名（对应原工程 ReadData/OnGz 等 4 个单选回调）
// 参数：strFileBase —— 输入文件去扩展名的基准名（原 m_strFilePath）
void CGravGradCoImagingDlg::updateDefaultSavePaths(const QString& strFileBase)
{
    if (strFileBase.isEmpty())
    {
        return;
    }

    // 按场值类型选择默认后缀：Gz→_Z、Gxz→_XZ、Gyz→_YZ、Gzz→_ZZ（体数据 .vol / 信息 .log）
    QString strSuffix;
    if (mFieldType == FieldTypeGz)
    {
        strSuffix = QStringLiteral("_Z");
    }
    else if (mFieldType == FieldTypeGxz)
    {
        strSuffix = QStringLiteral("_XZ");
    }
    else if (mFieldType == FieldTypeGyz)
    {
        strSuffix = QStringLiteral("_YZ");
    }
    else
    {
        strSuffix = QStringLiteral("_ZZ");
    }

    mOutputFilePath = strFileBase + strSuffix + QStringLiteral(".vol");
    mInfoFilePath = strFileBase + strSuffix + QStringLiteral(".log");

    if (mEditSaveFile != NULL)
    {
        mEditSaveFile->setText(mOutputFilePath);
    }
    if (mEditSaveInfoFile != NULL)
    {
        mEditSaveInfoFile->setText(mInfoFilePath);
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CGravGradCoImagingDlg::updateGridInfoDisplay()
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

// 功能：场值类型切换（同步成员变量并刷新默认输出名，对应原工程 OnGz/OnGxz/OnGyz/OnGzz）
void CGravGradCoImagingDlg::onFieldTypeClicked(int nFieldTypeId)
{
    mFieldType = nFieldTypeId;
    updateDefaultSavePaths(mFilePathBase);
}

// 功能：滤波选择切换（同步成员变量，对应原工程 OnEntropyFilter/OnNonEntropyFilter）
void CGravGradCoImagingDlg::onFilterTypeClicked(int nFilterTypeId)
{
    mFilterType = nFilterTypeId;
}

// 功能：观测面高度变化（同步成员变量）
void CGravGradCoImagingDlg::onZObsValueChanged(double dValue)
{
    mZObs = dValue;
}

// 功能：深度层间距变化（同步成员变量）
void CGravGradCoImagingDlg::onLayStepValueChanged(double dValue)
{
    mLayStep = dValue;
}

// 功能：深度层起点变化（同步成员变量）
void CGravGradCoImagingDlg::onZMinValueChanged(double dValue)
{
    mZMin = dValue;
}

// 功能：深度层终点变化（同步成员变量）
void CGravGradCoImagingDlg::onZMaxValueChanged(double dValue)
{
    mZMax = dValue;
}

// 功能：选择相关系数体数据文件（.vol，对应原工程 OnSaveFile）
void CGravGradCoImagingDlg::onSaveFileClicked()
{
    QString strFilePath = askSaveFilePath(QStringLiteral("请输入成像结果数据文件名"), mOutputFilePath,
        QStringLiteral("体数据文件 (*.vol);;所有数据文件 (*.*)"));
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

// 功能：手动输入体数据文件名（同步成员变量，对应原工程 OnChangeSaveFile）
void CGravGradCoImagingDlg::onSaveFileTextEdited(const QString& strText)
{
    mOutputFilePath = strText;
}

// 功能：选择相关成像信息文件（.log，对应原工程 OnSaveInfoFile）
void CGravGradCoImagingDlg::onSaveInfoFileClicked()
{
    QString strFilePath = askSaveFilePath(QStringLiteral("请输入成像信息数据文件名"), mInfoFilePath,
        QStringLiteral("信息文件 (*.log);;所有数据文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mInfoFilePath = strFilePath;
    if (mEditSaveInfoFile != NULL)
    {
        mEditSaveInfoFile->setText(strFilePath);
    }
}

// 功能：手动输入信息文件名（同步成员变量，对应原工程 OnChangeSaveInfoFile）
void CGravGradCoImagingDlg::onSaveInfoFileTextEdited(const QString& strText)
{
    mInfoFilePath = strText;
}

// 功能：弹出打开文件对话框（.grd 网格数据文件）
QString CGravGradCoImagingDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框
// 参数：strFilter —— 文件过滤器（*.vol / *.log 等）
QString CGravGradCoImagingDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath,
                                               const QString& strFilter)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath, strFilter);
}

// 功能：检查网格文件存在并发出发送显示请求（对应原工程 OnOpenFileView）
void CGravGradCoImagingDlg::viewGridFile(const QString& strFilePath)
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

// 功能：检查体数据文件存在并发出发送显示请求（对应原工程 OnSaveFileView 的 CVolumeDataViewDlg）
void CGravGradCoImagingDlg::viewVolumeFile(const QString& strFilePath)
{
    if (strFilePath.isEmpty())
    {
        return;
    }
    // 文件不存在时提示并返回
    if (!QFile::exists(strFilePath))
    {
        QMessageBox::warning(this, QStringLiteral("文件不存在"),
            QStringLiteral("文件") + strFilePath + QStringLiteral("不存在!"));
        return;
    }
    // 体数据显示窗口由前端后续版本实现，此处交由宿主处理
    emit viewVolumeFileRequested(strFilePath);
}

// 功能：显示输入数据文件等值线
void CGravGradCoImagingDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示相关系数体数据文件三维视图
void CGravGradCoImagingDlg::onSaveFileViewClicked()
{
    viewVolumeFile(mOutputFilePath);
}

// 功能：显示相关成像信息文件（交由宿主处理）
void CGravGradCoImagingDlg::onSaveInfoFileViewClicked()
{
    viewVolumeFile(mInfoFilePath);
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的空值与深度检查，并给出更明确的提示）
bool CGravGradCoImagingDlg::validateInputs(QString& strError)
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
    // 体数据输出文件检查
    if (mOutputFilePath.isEmpty())
    {
        strError = QStringLiteral("请输入相关系数体数据文件路径。");
        return false;
    }
    // 信息输出文件检查
    if (mInfoFilePath.isEmpty())
    {
        strError = QStringLiteral("请输入相关成像信息文件路径。");
        return false;
    }
    // 深度层终点必须大于起点（与原工程 "深度层最大值小于或等于最小值，请确认。" 一致）
    if (mZMax <= mZMin)
    {
        strError = QStringLiteral("深度层最大值小于或等于最小值，请确认。");
        return false;
    }
    // 成像深度应不超过网格最小长度/宽度的一半（与原工程 OnOK 中的 m_fTmp3 检查一致）
    double dMinSpan = 0.0;
    double dSpanX = mColsNum > 0 ? mColStep * (mColsNum - 1) : 0.0;
    double dSpanY = mRowsNum > 0 ? mRowStep * (mRowsNum - 1) : 0.0;
    if (dSpanY <= dSpanX)
    {
        dMinSpan = dSpanY;
    }
    else
    {
        dMinSpan = dSpanX;
    }
    if ((mZMax - mZMin) >= dMinSpan / 2.0)
    {
        strError = QStringLiteral("成像深度应小于或等于网格数据的最小长度或宽度，请确认。");
        return false;
    }
    return true;
}

// 功能：调用后端 processGravGradCoImaging（对应原工程 OnOK 主体逻辑）
// 说明：后端接口（processGravGradCoImaging）尚未由后端提供，本函数当前仅做参数校验与占位提示；
//       后端提供接口后，在下方注释位置接入
//       IRgisBackend* pBackend = CBackendService::rgisBackend();
//       pBackend->processGravGradCoImaging(params, error);
void CGravGradCoImagingDlg::runProcess()
{
    // TODO(后端对接)：后端接口 processGravGradCoImaging 提供后在此接入：
    //   组装 GravGradCoImagingParams 参数（输入文件、输出体数据/信息文件、
    //   场值类型、滤波选择、zObs/layStep/zMin/zMax），调用后端完成处理，
    //   完成后提示“相关成像结束!”（与原工程一致，处理完成后对话框不关闭）。
    // 当前：前端界面已完成，后端未接入，给出占位提示。
    QMessageBox::warning(this, QStringLiteral("处理失败"),
        QStringLiteral("算法后端尚未接入：三维重力异常和梯度相关成像功能等待后端实现。"));
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CGravGradCoImagingDlg::onOkClicked()
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
void CGravGradCoImagingDlg::onCancelClicked()
{
    reject();
}

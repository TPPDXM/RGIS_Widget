// FreqDomainIterDrapeDlg.cpp : 频率域迭代曲化平（迭代下延计算）对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_FreqDomainIterDrape；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查。

#include "FreqDomainIterDrapeDlg.h"
#include "backend/RgisBackend.h"
#include "FrontendUtils.h"

#include <vector>

#include <QAbstractItemView>
#include <QApplication>
#include <QButtonGroup>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include <QSpinBox>

// 最大扩边尺寸（与原 MFC 工程 OnDeltaposSpinExCols 中 <= 65536 的约束一致）
static const int sMaxExtendSize = 65536;

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CFreqDomainIterDrapeDlg::CFreqDomainIterDrapeDlg(QWidget* pParent)
    : QDialog(pParent)
    , mEditOpenFile(NULL)
    , mBtnOpenFile(NULL)
    , mBtnOpenFileView(NULL)
    , mEditOpenDemFile(NULL)
    , mBtnOpenDemFile(NULL)
    , mBtnOpenDemFileView(NULL)
    , mEditRowsNum(NULL)
    , mEditColsNum(NULL)
    , mEditRowStep(NULL)
    , mEditColStep(NULL)
    , mEditHMin(NULL)
    , mEditHMax(NULL)
    , mSpinExRows(NULL)
    , mSpinExCols(NULL)
    , mRadioCosFun(NULL)
    , mRadioAvgDif(NULL)
    , mRadioInvPow(NULL)
    , mButtonGroupExpand(NULL)
    , mSpinHeight(NULL)
    , mSpinPlaneNums(NULL)
    , mSpinIterError(NULL)
    , mSpinIterFactor(NULL)
    , mTablePlanes(NULL)
    , mEditSaveFile(NULL)
    , mBtnSaveFile(NULL)
    , mBtnSaveFileView(NULL)
    , mBtnOk(NULL)
    , mBtnCancel(NULL)
    , mRowsNum(0)
    , mColsNum(0)
    , mColStep(0.0)
    , mRowStep(0.0)
    , mHMin(0.0)
    , mHMax(0.0)
    , mExRows(0)
    , mExCols(0)
    , mMinExRows(1)
    , mMinExCols(1)
    , mHeight(0.0)
    , mPlaneNums(5)
    , mIterError(0.5)
    , mIterFactor(1.5)
    , mExpandMethod(ExpandCosFun)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CFreqDomainIterDrapeDlg::~CFreqDomainIterDrapeDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CFreqDomainIterDrapeDlg::initUi()
{
    setWindowTitle(QStringLiteral("迭代曲化平"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 重磁场值数据文件输入 =================
    QGroupBox* pGroupInput = new QGroupBox(QStringLiteral("重磁场值数据文件输入"), this);
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

    // ================= 地形高程数据文件输入 =================
    QGroupBox* pGroupDemInput = new QGroupBox(QStringLiteral("地形高程数据文件输入"), this);
    QGridLayout* pDemInputLayout = new QGridLayout(pGroupDemInput);
    mEditOpenDemFile = new QLineEdit(pGroupDemInput);
    mBtnOpenDemFile = new QPushButton(QStringLiteral("..."), pGroupDemInput);
    mBtnOpenDemFile->setFixedWidth(32);
    mBtnOpenDemFileView = new QPushButton(QStringLiteral("显示"), pGroupDemInput);
    mBtnOpenDemFileView->setFixedWidth(48);
    pDemInputLayout->addWidget(mEditOpenDemFile, 0, 0);
    pDemInputLayout->addWidget(mBtnOpenDemFile, 0, 1);
    pDemInputLayout->addWidget(mBtnOpenDemFileView, 0, 2);
    pDemInputLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupDemInput);

    // ================= 网格数据信息（只读显示 + 高程范围）=================
    // 对应 .rc 中的 IDC_RowsNum / IDC_ColsNum / IDC_RowStep / IDC_ColStep / IDC_hMin / IDC_hMax
    QGroupBox* pGroupInfo = new QGroupBox(QStringLiteral("网格数据信息"), this);
    QGridLayout* pInfoLayout = new QGridLayout(pGroupInfo);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格行数"), pGroupInfo), 0, 0);
    mEditRowsNum = new QLineEdit(pGroupInfo);
    mEditRowsNum->setReadOnly(true);
    mEditRowsNum->setAlignment(Qt::AlignCenter);
    mEditRowsNum->setFixedWidth(100);
    pInfoLayout->addWidget(mEditRowsNum, 0, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格列数"), pGroupInfo), 0, 2);
    mEditColsNum = new QLineEdit(pGroupInfo);
    mEditColsNum->setReadOnly(true);
    mEditColsNum->setAlignment(Qt::AlignCenter);
    mEditColsNum->setFixedWidth(100);
    pInfoLayout->addWidget(mEditColsNum, 0, 3);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格行距"), pGroupInfo), 1, 0);
    mEditRowStep = new QLineEdit(pGroupInfo);
    mEditRowStep->setReadOnly(true);
    mEditRowStep->setAlignment(Qt::AlignCenter);
    mEditRowStep->setFixedWidth(100);
    pInfoLayout->addWidget(mEditRowStep, 1, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格列距"), pGroupInfo), 1, 2);
    mEditColStep = new QLineEdit(pGroupInfo);
    mEditColStep->setReadOnly(true);
    mEditColStep->setAlignment(Qt::AlignCenter);
    mEditColStep->setFixedWidth(100);
    pInfoLayout->addWidget(mEditColStep, 1, 3);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("高程最小值"), pGroupInfo), 2, 0);
    mEditHMin = new QLineEdit(pGroupInfo);
    mEditHMin->setAlignment(Qt::AlignCenter);
    mEditHMin->setFixedWidth(100);
    pInfoLayout->addWidget(mEditHMin, 2, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("高程最大值"), pGroupInfo), 2, 2);
    mEditHMax = new QLineEdit(pGroupInfo);
    mEditHMax->setAlignment(Qt::AlignCenter);
    mEditHMax->setFixedWidth(100);
    pInfoLayout->addWidget(mEditHMax, 2, 3);
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

    // ================= 数据扩边方法（单选，仅 3 种）=================
    // 对应 .rc 中的 IDC_CosFun / IDC_AvgDif / IDC_InvPow
    QGroupBox* pGroupMethod = new QGroupBox(QStringLiteral("数据扩边方法"), this);
    QHBoxLayout* pMethodLayout = new QHBoxLayout(pGroupMethod);
    mButtonGroupExpand = new QButtonGroup(this);
    mRadioCosFun = new QRadioButton(QStringLiteral("余弦函数衰减"), pGroupMethod);
    mRadioAvgDif = new QRadioButton(QStringLiteral("平均值差分"), pGroupMethod);
    mRadioInvPow = new QRadioButton(QStringLiteral("反距离加权"), pGroupMethod);
    mButtonGroupExpand->addButton(mRadioCosFun, ExpandCosFun);
    mButtonGroupExpand->addButton(mRadioAvgDif, ExpandAvgDif);
    mButtonGroupExpand->addButton(mRadioInvPow, ExpandInvPow);
    mRadioCosFun->setChecked(true);     // 默认选中“余弦函数衰减”（与原工程 OnInitDialog 一致）
    mExpandMethod = ExpandCosFun;
    pMethodLayout->addWidget(mRadioCosFun);
    pMethodLayout->addWidget(mRadioAvgDif);
    pMethodLayout->addWidget(mRadioInvPow);
    pMethodLayout->addStretch(1);
    pMainLayout->addWidget(pGroupMethod);

    // ================= 网格数据信息（延拓与迭代参数 + 切割平面高程表格）=================
    // 左侧：对应 .rc 中的 IDC_Height / IDC_PlaneNums / IDC_IterFactor / IDC_IterError
    // 右侧：对应 .rc 中的 MSFlexGrid（平面序号 / 平面高程）
    QHBoxLayout* pParamAreaLayout = new QHBoxLayout();
    QGroupBox* pGroupParams = new QGroupBox(QStringLiteral("网格数据信息"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("延拓高度"), pGroupParams), 0, 0);
    mSpinHeight = new QDoubleSpinBox(pGroupParams);
    mSpinHeight->setRange(0.0, 999999.0);
    mSpinHeight->setDecimals(3);
    mSpinHeight->setSingleStep(1.0);
    mSpinHeight->setValue(mHeight);
    mSpinHeight->setAlignment(Qt::AlignCenter);
    mSpinHeight->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinHeight, 0, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("切割平面数"), pGroupParams), 1, 0);
    mSpinPlaneNums = new QSpinBox(pGroupParams);
    mSpinPlaneNums->setRange(1, 99);
    mSpinPlaneNums->setValue(mPlaneNums);
    mSpinPlaneNums->setAlignment(Qt::AlignCenter);
    mSpinPlaneNums->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinPlaneNums, 1, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("迭代因子"), pGroupParams), 2, 0);
    mSpinIterFactor = new QDoubleSpinBox(pGroupParams);
    mSpinIterFactor->setRange(0.0, 10.0);
    mSpinIterFactor->setDecimals(3);
    mSpinIterFactor->setSingleStep(0.1);
    mSpinIterFactor->setValue(mIterFactor);
    mSpinIterFactor->setAlignment(Qt::AlignCenter);
    mSpinIterFactor->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinIterFactor, 2, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("迭代均方差"), pGroupParams), 3, 0);
    mSpinIterError = new QDoubleSpinBox(pGroupParams);
    mSpinIterError->setRange(0.0, 9999.0);
    mSpinIterError->setDecimals(3);
    mSpinIterError->setSingleStep(0.1);
    mSpinIterError->setValue(mIterError);
    mSpinIterError->setAlignment(Qt::AlignCenter);
    mSpinIterError->setFixedWidth(100);
    pParamsLayout->addWidget(mSpinIterError, 3, 1);
    pParamAreaLayout->addWidget(pGroupParams);

    // 切割平面高程表格（对应原工程 MSFlexGrid，列：平面序号 / 平面高程）
    mTablePlanes = new QTableWidget(this);
    mTablePlanes->setColumnCount(2);
    QStringList strHeaders;
    strHeaders << QStringLiteral("平面序号") << QStringLiteral("平面高程");
    mTablePlanes->setHorizontalHeaderLabels(strHeaders);
    mTablePlanes->verticalHeader()->setVisible(false);
    mTablePlanes->setColumnWidth(0, 70);
    mTablePlanes->setColumnWidth(1, 130);
    mTablePlanes->setSelectionMode(QAbstractItemView::SingleSelection);
    mTablePlanes->setFixedWidth(210);
    // 固定高度：默认显示 6 行（序号 0 ~ 5），更多行时通过滚动条查看（与原工程 MSFlexGrid 固定高度一致）
    mTablePlanes->setFixedHeight(240);
    pParamAreaLayout->addWidget(mTablePlanes);
    pParamAreaLayout->addStretch(1);
    pMainLayout->addLayout(pParamAreaLayout);

    // ================= 曲化平结果数据文件输出 =================
    // 对应 .rc 中的 IDC_SaveFile / ID_SaveFile / ID_SaveFileView
    QGroupBox* pGroupOutput = new QGroupBox(QStringLiteral("曲化平结果数据文件输出"), this);
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

    // 重磁场值输入文件
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CFreqDomainIterDrapeDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CFreqDomainIterDrapeDlg::onOpenFileViewClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CFreqDomainIterDrapeDlg::onOpenFileTextEdited);

    // 地形高程输入文件
    connect(mBtnOpenDemFile, &QPushButton::clicked, this, &CFreqDomainIterDrapeDlg::onOpenDemFileClicked);
    connect(mBtnOpenDemFileView, &QPushButton::clicked, this, &CFreqDomainIterDrapeDlg::onOpenDemFileViewClicked);
    connect(mEditOpenDemFile, &QLineEdit::textEdited, this, &CFreqDomainIterDrapeDlg::onOpenDemFileTextEdited);

    // 曲化平结果数据文件输出
    connect(mBtnSaveFile, &QPushButton::clicked, this, &CFreqDomainIterDrapeDlg::onSaveFileClicked);
    connect(mBtnSaveFileView, &QPushButton::clicked, this, &CFreqDomainIterDrapeDlg::onSaveFileViewClicked);
    connect(mEditSaveFile, &QLineEdit::textEdited, this, &CFreqDomainIterDrapeDlg::onSaveFileTextEdited);

    // 扩边行数/列数微调
    connect(mSpinExRows, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainIterDrapeDlg::onExRowsValueChanged);
    connect(mSpinExCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainIterDrapeDlg::onExColsValueChanged);

    // 扩边方法单选组（id 与 ExpandMethod 枚举一致）
    connect(mButtonGroupExpand, &QButtonGroup::idClicked, this, &CFreqDomainIterDrapeDlg::onExpandMethodClicked);

    // 处理参数
    connect(mSpinHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainIterDrapeDlg::onHeightValueChanged);
    connect(mSpinPlaneNums, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainIterDrapeDlg::onPlaneNumsValueChanged);
    connect(mSpinIterError, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainIterDrapeDlg::onIterErrorValueChanged);
    connect(mSpinIterFactor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainIterDrapeDlg::onIterFactorValueChanged);

    // 高程最小/最大值（输入完成后重建表格默认高程，与原工程点击表格重填规则一致）
    connect(mEditHMin, &QLineEdit::editingFinished, this, &CFreqDomainIterDrapeDlg::onHMinEdited);
    connect(mEditHMax, &QLineEdit::editingFinished, this, &CFreqDomainIterDrapeDlg::onHMaxEdited);

    // 切割平面高程表格点击（行内容为空时填入默认高程）
    connect(mTablePlanes, &QTableWidget::cellClicked, this, &CFreqDomainIterDrapeDlg::onPlaneTableClicked);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CFreqDomainIterDrapeDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CFreqDomainIterDrapeDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();

    // 按默认切割平面数（5）重建切割平面高程表格（对应原工程 OnInitDialog 中的表格初始化）
    rebuildPlaneTable();
}

// 功能：选择重磁场值输入数据文件（对应原工程 OnOpenFile）
void CFreqDomainIterDrapeDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：手动输入重磁场值文件名（同步成员变量，对应原工程 OnChangeOpenFile）
void CFreqDomainIterDrapeDlg::onOpenFileTextEdited(const QString& strText)
{
    mInputFilePath = strText;
}

// 功能：读取重磁场值文件头并刷新网格信息、默认值（对应原工程 OnOpenFile 中的读文件逻辑）
void CFreqDomainIterDrapeDlg::loadFile(const QString& strFilePath)
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
    mGridHead = head;
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

    // 默认延拓高度：1 倍列距（与原工程 ReadData 中 "%.3f", xStep 一致）
    // 注意：原工程该字段不参与迭代计算（平面高程由切割平面高程表格给定），此处仅保留界面显示
    mHeight = mColStep;
    if (mSpinHeight != NULL)
    {
        mSpinHeight->setValue(mHeight);
    }
}

// 功能：选择地形高程输入数据文件（对应原工程 OnOpenDEMFile）
void CFreqDomainIterDrapeDlg::onOpenDemFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择地形数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadDemFile(strFilePath);
}

// 功能：手动输入地形高程文件名（同步成员变量，对应原工程 OnChangeOpenDEMFile）
void CFreqDomainIterDrapeDlg::onOpenDemFileTextEdited(const QString& strText)
{
    mDemFilePath = strText;
}

// 功能：读取地形高程文件头并校验一致性、刷新高程范围（对应原工程 OnOpenDEMFile）
void CFreqDomainIterDrapeDlg::loadDemFile(const QString& strFilePath)
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
    if (strFilePath == mLoadedDemFilePath)
    {
        return;
    }
    // 必须已加载重磁场值数据文件后才能进行一致性校验（原工程实际使用顺序一致）
    if (mGridHead.rows <= 0 || mGridHead.cols <= 0)
    {
        QMessageBox::warning(this, QStringLiteral("读取文件失败"),
            QStringLiteral("请先加载重磁场值数据文件，再加载地形高程数据文件。"));
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

    // 与重磁场值数据文件一致性校验（与原工程 OnOpenDEMFile 中的校验一致）
    if (mGridHead.rows != head.rows || mGridHead.cols != head.cols)
    {
        QMessageBox::warning(this, QStringLiteral("数据不一致"),
            QStringLiteral("您读入的地形和重磁场网格数据文件行列数不一致!"));
        return;
    }
    if (mGridHead.xMin != head.xMin || mGridHead.xMax != head.xMax
        || mGridHead.yMin != head.yMin || mGridHead.yMax != head.yMax)
    {
        QMessageBox::warning(this, QStringLiteral("数据不一致"),
            QStringLiteral("您读入的地形和重磁场网格数据文件坐标位置不一致!"));
        return;
    }

    // 记录加载状态并同步输入框显示
    mLoadedDemFilePath = strFilePath;
    mDemFilePath = strFilePath;
    mDemHead = head;
    if (mEditOpenDemFile != NULL)
    {
        mEditOpenDemFile->setText(strFilePath);
    }

    // 高程范围默认取地形数据的 z 范围（与原工程 Str.Format("%.3f", dbTmin/dbTmax) 一致）
    mHMin = head.zMin;
    mHMax = head.zMax;
    if (mEditHMin != NULL)
    {
        mEditHMin->setText(QString::number(mHMin, 'f', 3));
    }
    if (mEditHMax != NULL)
    {
        mEditHMax->setText(QString::number(mHMax, 'f', 3));
    }

    // 用新的高程范围重建切割平面高程表格
    rebuildPlaneTable();
}

// 功能：生成默认输出文件名（对应原工程 OnOpenFile 中 _Pro.grd 命名规则）
void CFreqDomainIterDrapeDlg::updateDefaultSavePaths(const QString& strFilePath)
{
    // 取最后一个点号之前的内容作为基准名（与原工程 ReverseFind('.') 一致）
    int nPos = strFilePath.lastIndexOf('.');
    QString strBase = strFilePath;
    if (nPos > 0)
    {
        strBase = strFilePath.left(nPos);
    }

    mOutputFilePath = strBase + QStringLiteral("_Pro.grd");

    if (mEditSaveFile != NULL)
    {
        mEditSaveFile->setText(mOutputFilePath);
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CFreqDomainIterDrapeDlg::updateGridInfoDisplay()
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
void CFreqDomainIterDrapeDlg::setSpinExtendSize(QSpinBox* pSpin, int nSize)
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

// 功能：按切割平面数与高程范围重建表格行并填入默认高程
// 对应原工程 OnClickMsFlexGrid 的填表逻辑：
//   各平面高程 = 高程最小值 + 序号 * (高程最大值 - 高程最小值) / 切割平面数
void CFreqDomainIterDrapeDlg::rebuildPlaneTable()
{
    if (mTablePlanes == NULL)
    {
        return;
    }
    if (mPlaneNums < 1)
    {
        mPlaneNums = 1;
    }

    // 数据行数 = 切割平面数 + 1（序号 0 ~ 切割平面数，与原工程 MSFlexGrid 行数规则一致）
    mTablePlanes->setRowCount(mPlaneNums + 1);

    double dDelta = (mHMax - mHMin) / mPlaneNums;

    for (int i = 0; i <= mPlaneNums; i++)
    {
        // 第 0 列：平面序号（只读）
        QTableWidgetItem* pItemIndex = new QTableWidgetItem(QString::number(i));
        pItemIndex->setFlags(pItemIndex->flags() & ~Qt::ItemIsEditable);
        pItemIndex->setTextAlignment(Qt::AlignCenter);
        mTablePlanes->setItem(i, 0, pItemIndex);

        // 第 1 列：平面高程（默认线性插值，与原工程 "%.3f" 一致）
        QTableWidgetItem* pItemHeight = new QTableWidgetItem(
            QString::number(mHMin + i * dDelta, 'f', 3));
        pItemHeight->setTextAlignment(Qt::AlignCenter);
        mTablePlanes->setItem(i, 1, pItemHeight);
    }
}

// 功能：点击切割平面高程表格（行内容为空时填入默认值；已编辑过的行保留，避免覆盖用户输入）
void CFreqDomainIterDrapeDlg::onPlaneTableClicked()
{
    if (mTablePlanes == NULL)
    {
        return;
    }

    bool bNeedFill = false;
    for (int i = 0; i < mTablePlanes->rowCount(); i++)
    {
        QTableWidgetItem* pItem = mTablePlanes->item(i, 1);
        if (pItem == NULL || pItem->text().isEmpty())
        {
            bNeedFill = true;
            break;
        }
    }

    if (bNeedFill)
    {
        rebuildPlaneTable();
    }
}

// 功能：读取表格各平面高程并校验（对应原工程 OnOK 前 MSFlexGrid 数据的读取与校验）
// 参数：strError —— 校验失败时的错误描述
// 返回：true 校验通过；false 校验失败（strError 给出原因）
bool CFreqDomainIterDrapeDlg::readPlaneTable(QString& strError)
{
    if (mTablePlanes == NULL)
    {
        strError = QStringLiteral("切割平面高程表格未初始化。");
        return false;
    }

    for (int i = 0; i <= mPlaneNums; i++)
    {
        QTableWidgetItem* pItemHeight = mTablePlanes->item(i, 1);
        QString strHeight = (pItemHeight != NULL) ? pItemHeight->text().trimmed() : QString();
        if (strHeight.isEmpty())
        {
            strError = QStringLiteral("第 ") + QString::number(i)
                + QStringLiteral(" 个切割平面高程参数输入没有完成！");
            return false;
        }
        bool bOkHeight = false;
        double dHeight = strHeight.toDouble(&bOkHeight);
        if (!bOkHeight)
        {
            strError = QStringLiteral("第 ") + QString::number(i)
                + QStringLiteral(" 个切割平面高程参数输入错误！");
            return false;
        }
        // 高程数值校验通过即可（具体数值在 runProcess 中统一读入后端参数）
        (void)dHeight;
    }
    return true;
}

// 功能：选择曲化平结果数据文件（对应原工程 OnSaveFile）
void CFreqDomainIterDrapeDlg::onSaveFileClicked()
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
void CFreqDomainIterDrapeDlg::onSaveFileTextEdited(const QString& strText)
{
    mOutputFilePath = strText;
}

// 功能：弹出打开文件对话框（.grd 网格数据文件）
QString CFreqDomainIterDrapeDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框（.grd 网格数据文件）
QString CFreqDomainIterDrapeDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath,
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：检查文件存在并发出发送显示请求（对应原工程各“显示”按钮）
void CFreqDomainIterDrapeDlg::viewGridFile(const QString& strFilePath)
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

// 功能：显示重磁场值输入文件等值线
void CFreqDomainIterDrapeDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示地形高程输入文件等值线
void CFreqDomainIterDrapeDlg::onOpenDemFileViewClicked()
{
    viewGridFile(mDemFilePath);
}

// 功能：显示曲化平结果文件等值线
void CFreqDomainIterDrapeDlg::onSaveFileViewClicked()
{
    viewGridFile(mOutputFilePath);
}

// 功能：扩边行数变化（同步成员变量）
void CFreqDomainIterDrapeDlg::onExRowsValueChanged(int nValue)
{
    mExRows = nValue;
}

// 功能：扩边列数变化（同步成员变量）
void CFreqDomainIterDrapeDlg::onExColsValueChanged(int nValue)
{
    mExCols = nValue;
}

// 功能：数据扩边方法切换（同步成员变量）
void CFreqDomainIterDrapeDlg::onExpandMethodClicked(int nMethodId)
{
    mExpandMethod = nMethodId;
}

// 功能：延拓高度变化（同步成员变量）
void CFreqDomainIterDrapeDlg::onHeightValueChanged(double dValue)
{
    mHeight = dValue;
}

// 功能：切割平面数变化（重建切割平面高程表格）
void CFreqDomainIterDrapeDlg::onPlaneNumsValueChanged(int nValue)
{
    mPlaneNums = nValue;
    rebuildPlaneTable();
}

// 功能：迭代均方差变化（同步成员变量）
void CFreqDomainIterDrapeDlg::onIterErrorValueChanged(double dValue)
{
    mIterError = dValue;
}

// 功能：迭代因子变化（同步成员变量）
void CFreqDomainIterDrapeDlg::onIterFactorValueChanged(double dValue)
{
    mIterFactor = dValue;
}

// 功能：高程最小值输入完成（重建切割平面高程表格，对应原工程点击表格后的重填规则）
void CFreqDomainIterDrapeDlg::onHMinEdited()
{
    if (mEditHMin == NULL)
    {
        return;
    }
    bool bOk = false;
    double dValue = mEditHMin->text().toDouble(&bOk);
    if (bOk)
    {
        mHMin = dValue;
        rebuildPlaneTable();
    }
}

// 功能：高程最大值输入完成（重建切割平面高程表格，对应原工程点击表格后的重填规则）
void CFreqDomainIterDrapeDlg::onHMaxEdited()
{
    if (mEditHMax == NULL)
    {
        return;
    }
    bool bOk = false;
    double dValue = mEditHMax->text().toDouble(&bOk);
    if (bOk)
    {
        mHMax = dValue;
        rebuildPlaneTable();
    }
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的检查，并给出更明确的提示）
bool CFreqDomainIterDrapeDlg::validateInputs(QString& strError)
{
    // 重磁场值输入数据文件检查
    if (mInputFilePath.isEmpty())
    {
        strError = QStringLiteral("请先选择重磁场值输入数据文件。");
        return false;
    }
    if (!QFile::exists(mInputFilePath))
    {
        strError = QStringLiteral("重磁场值输入数据文件不存在：") + mInputFilePath;
        return false;
    }
    // 地形高程输入数据文件检查
    if (mDemFilePath.isEmpty())
    {
        strError = QStringLiteral("请先选择地形高程输入数据文件。");
        return false;
    }
    if (!QFile::exists(mDemFilePath))
    {
        strError = QStringLiteral("地形高程输入数据文件不存在：") + mDemFilePath;
        return false;
    }
    // 曲化平结果输出文件检查
    if (mOutputFilePath.isEmpty())
    {
        strError = QStringLiteral("请输入曲化平结果数据文件路径。");
        return false;
    }
    // 两个文件均已正确加载（文件头缓存有效）检查
    if (mGridHead.rows <= 0 || mGridHead.cols <= 0 || mDemHead.rows <= 0 || mDemHead.cols <= 0)
    {
        strError = QStringLiteral("重磁场值或地形高程数据文件尚未正确加载。");
        return false;
    }
    // 重磁场值与地形数据一致性检查（与原工程 OnOK 中的检查一致）
    if (mGridHead.rows != mDemHead.rows || mGridHead.cols != mDemHead.cols)
    {
        strError = QStringLiteral("您读入的地形和重磁场网格数据文件行列数不一致!");
        return false;
    }
    if (mGridHead.xMin != mDemHead.xMin || mGridHead.xMax != mDemHead.xMax
        || mGridHead.yMin != mDemHead.yMin || mGridHead.yMax != mDemHead.yMax)
    {
        strError = QStringLiteral("您读入的地形和重磁场网格数据文件坐标位置不一致!");
        return false;
    }
    // 高程范围检查（与原工程 OnClickMsFlexGrid 中的“高程参数输入错误”一致）
    if (mHMax - mHMin <= 0.0)
    {
        strError = QStringLiteral("高程参数输入错误！");
        return false;
    }
    // 扩边尺寸检查（必须不小于读文件时计算的最小尺寸）
    if (mExCols < mMinExCols || mExRows < mMinExRows)
    {
        strError = QStringLiteral("扩边行数/列数不得小于网格数据要求的最小尺寸。");
        return false;
    }
    // 切割平面高程表格检查
    if (!readPlaneTable(strError))
    {
        return false;
    }
    return true;
}

// 功能：组装参数并调用后端 processIterDrape（对应原工程 OnOK 主体逻辑）
void CFreqDomainIterDrapeDlg::runProcess()
{
    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), QStringLiteral("后端接口未初始化。"));
        return;
    }

    // 组装处理参数（对应原工程界面上的全部输入项）
    IterDrapeParams params;
    params.inputFilePath = toBackendString(mInputFilePath);
    params.demFilePath = toBackendString(mDemFilePath);
    params.outputFilePath = toBackendString(mOutputFilePath);
    params.exRows = mExRows;
    params.exCols = mExCols;
    params.expandMethod = (ExpandMethod)mExpandMethod;
    params.planeNums = mPlaneNums;
    params.demHMin = (float)mHMin;
    params.demHMax = (float)mHMax;
    params.iterError = (float)mIterError;
    params.iterFactor = (float)mIterFactor;

    // 读取切割平面高程表格（读取前已通过校验，直接生成后端参数）
    params.planeAltitudes.resize(mPlaneNums + 1);
    for (int i = 0; i <= mPlaneNums; i++)
    {
        QTableWidgetItem* pItemHeight = mTablePlanes->item(i, 1);
        double dHeight = (pItemHeight != NULL) ? pItemHeight->text().toDouble() : 0.0;
        params.planeAltitudes[i] = (float)dHeight;
    }

    // 同步调用后端处理（处理期间显示等待光标，与原工程 BeginWaitCursor 一致）
    // 出参：迭代次数与最终均方差（原工程完成提示中显示）
    BackendError error;
    int iterations = 0;
    float finalError = 0.0f;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bOk = pBackend->processIterDrape(params, iterations, finalError, error);
    QApplication::restoreOverrideCursor();

    if (bOk)
    {
        // 处理完成提示（与原工程一致，处理完成后对话框不关闭，由用户点“取消”退出）
        QString strMessage = QStringLiteral("迭代次数为 ") + QString::number(iterations)
            + QStringLiteral(" , 迭代均方差为: ")
            + QString::number(finalError, 'f', 6)
            + QStringLiteral(" , 迭代下延计算结束！");
        QMessageBox::information(this, QStringLiteral("处理完成"), strMessage);
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), fromBackendString(error.message));
    }
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CFreqDomainIterDrapeDlg::onOkClicked()
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
void CFreqDomainIterDrapeDlg::onCancelClicked()
{
    reject();
}

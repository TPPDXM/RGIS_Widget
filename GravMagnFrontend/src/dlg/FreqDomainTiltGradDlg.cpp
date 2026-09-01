// FreqDomainTiltGradDlg.cpp : 频率域 Tilt 梯度（斜导数）计算对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_FreqDomainTiltGrad；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律用 QStringLiteral 构造。

#include "FreqDomainTiltGradDlg.h"

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


// 最大扩边尺寸（与原 MFC 工程 OnDeltaposSpinExCols 中 <= 65536 的约束一致）
static const int sMaxExtendSize = 65536;

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CFreqDomainTiltGradDlg::CFreqDomainTiltGradDlg(QWidget* pParent)
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
    , mButtonGroupExpand(NULL)
    , mRadioFilter(NULL)
    , mRadioNonFilter(NULL)
    , mSpinBeta(NULL)
    , mSpinComNum(NULL)
    , mEditSaveTdrFile(NULL)
    , mEditSaveTdxFile(NULL)
    , mEditSaveTdyFile(NULL)
    , mEditSaveThdFile(NULL)
    , mEditSaveTtdFile(NULL)
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
    , mFilterBefore(false)
    , mComNum(20)
    , mBeta(150.0)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CFreqDomainTiltGradDlg::~CFreqDomainTiltGradDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CFreqDomainTiltGradDlg::initUi()
{
    setWindowTitle(QStringLiteral("Tilt 梯度(斜导数)计算"));
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

    // ================= 数据扩边方法（单选，3 种，无最小曲率）=================
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

    // ================= 梯度计算前是否滤波（单选）=================
    // 对应 .rc 中的 IDC_Filter / IDC_NonFilter（默认不滤波，与原工程 OnInitDialog 一致）
    QGroupBox* pGroupFilter = new QGroupBox(QStringLiteral("梯度计算前是否滤波"), this);
    QHBoxLayout* pFilterLayout = new QHBoxLayout(pGroupFilter);
    mRadioNonFilter = new QRadioButton(QStringLiteral("梯度计算前不滤波"), pGroupFilter);
    mRadioFilter = new QRadioButton(QStringLiteral("梯度计算前滤波"), pGroupFilter);
    mRadioNonFilter->setChecked(true);
    mFilterBefore = false;
    pFilterLayout->addWidget(mRadioNonFilter);
    pFilterLayout->addWidget(mRadioFilter);
    pFilterLayout->addStretch(1);
    pMainLayout->addWidget(pGroupFilter);

    // ================= 处理参数 =================
    // 对应 .rc 中的 IDC_Beta（指数因子）、IDC_ComNum（补偿因子）
    QGroupBox* pGroupParams = new QGroupBox(QStringLiteral("处理参数"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("指数因子"), pGroupParams), 0, 0);
    mSpinBeta = new QDoubleSpinBox(pGroupParams);
    mSpinBeta->setRange(0.0, 999999.0);
    mSpinBeta->setDecimals(3);
    mSpinBeta->setSingleStep(1.0);
    mSpinBeta->setValue(mBeta);
    mSpinBeta->setAlignment(Qt::AlignCenter);
    mSpinBeta->setFixedWidth(140);
    pParamsLayout->addWidget(mSpinBeta, 0, 1);
    pParamsLayout->addWidget(new QLabel(QStringLiteral("补偿因子"), pGroupParams), 0, 2);
    mSpinComNum = new QSpinBox(pGroupParams);
    mSpinComNum->setRange(1, 999999);
    mSpinComNum->setValue(mComNum);
    mSpinComNum->setAlignment(Qt::AlignCenter);
    mSpinComNum->setFixedWidth(120);
    pParamsLayout->addWidget(mSpinComNum, 0, 3);
    pMainLayout->addWidget(pGroupParams);

    // ================= 数据文件输出（5 行结果文件）=================
    // 对应 .rc 中的 IDC_SaveTdrFile / IDC_SaveTdxFile / IDC_SaveTdyFile /
    //           IDC_SaveThdFile / IDC_SaveTtdFile（原工程无“...”浏览按钮，
    //           行标题按钮仅作名称显示，同梯度计算对话框的处理方式）
    QGroupBox* pGroupOutput = new QGroupBox(QStringLiteral("数据文件输出"), this);
    QGridLayout* pOutputLayout = new QGridLayout(pGroupOutput);
    mEditSaveTdrFile = new QLineEdit(pGroupOutput);
    // 各行“显示”按钮指针（addOutputRow 返回，供信号槽连接，对应原工程 5 个“显示”按钮）
    QPushButton* pBtnSaveTdrFileView = addOutputRow(pOutputLayout, 0, mEditSaveTdrFile, QStringLiteral("Tilt 梯度"));
    mEditSaveTdxFile = new QLineEdit(pGroupOutput);
    QPushButton* pBtnSaveTdxFileView = addOutputRow(pOutputLayout, 1, mEditSaveTdxFile, QStringLiteral("斜导数 X 方向导数"));
    mEditSaveTdyFile = new QLineEdit(pGroupOutput);
    QPushButton* pBtnSaveTdyFileView = addOutputRow(pOutputLayout, 2, mEditSaveTdyFile, QStringLiteral("斜导数 Y 方向导数"));
    mEditSaveThdFile = new QLineEdit(pGroupOutput);
    QPushButton* pBtnSaveThdFileView = addOutputRow(pOutputLayout, 3, mEditSaveThdFile, QStringLiteral("斜导数的水平梯度"));
    mEditSaveTtdFile = new QLineEdit(pGroupOutput);
    QPushButton* pBtnSaveTtdFileView = addOutputRow(pOutputLayout, 4, mEditSaveTtdFile, QStringLiteral("斜导数的斜导数"));
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
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CFreqDomainTiltGradDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CFreqDomainTiltGradDlg::onOpenFileViewClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CFreqDomainTiltGradDlg::onOpenFileTextEdited);

    // 输出文件名编辑框（手工输入时同步成员变量，对应原工程 EN_CHANGE→UpdateData）
    connect(mEditSaveTdrFile, &QLineEdit::textEdited, this, &CFreqDomainTiltGradDlg::onSaveTdrFileTextEdited);
    connect(mEditSaveTdxFile, &QLineEdit::textEdited, this, &CFreqDomainTiltGradDlg::onSaveTdxFileTextEdited);
    connect(mEditSaveTdyFile, &QLineEdit::textEdited, this, &CFreqDomainTiltGradDlg::onSaveTdyFileTextEdited);
    connect(mEditSaveThdFile, &QLineEdit::textEdited, this, &CFreqDomainTiltGradDlg::onSaveThdFileTextEdited);
    connect(mEditSaveTtdFile, &QLineEdit::textEdited, this, &CFreqDomainTiltGradDlg::onSaveTtdFileTextEdited);

    // 5 个结果文件“显示”按钮（对应原工程 ON_BN_CLICKED(ID_SaveTdrFileView, OnSaveTdrFileView) 等 5 条映射）
    if (pBtnSaveTdrFileView != NULL)
    {
        connect(pBtnSaveTdrFileView, &QPushButton::clicked, this, &CFreqDomainTiltGradDlg::onSaveTdrFileViewClicked);
    }
    if (pBtnSaveTdxFileView != NULL)
    {
        connect(pBtnSaveTdxFileView, &QPushButton::clicked, this, &CFreqDomainTiltGradDlg::onSaveTdxFileViewClicked);
    }
    if (pBtnSaveTdyFileView != NULL)
    {
        connect(pBtnSaveTdyFileView, &QPushButton::clicked, this, &CFreqDomainTiltGradDlg::onSaveTdyFileViewClicked);
    }
    if (pBtnSaveThdFileView != NULL)
    {
        connect(pBtnSaveThdFileView, &QPushButton::clicked, this, &CFreqDomainTiltGradDlg::onSaveThdFileViewClicked);
    }
    if (pBtnSaveTtdFileView != NULL)
    {
        connect(pBtnSaveTtdFileView, &QPushButton::clicked, this, &CFreqDomainTiltGradDlg::onSaveTtdFileViewClicked);
    }

    // 扩边行数/列数微调
    connect(mSpinExRows, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainTiltGradDlg::onExRowsValueChanged);
    connect(mSpinExCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainTiltGradDlg::onExColsValueChanged);

    // 扩边方法单选组（id 与 ExpandMethod 枚举一致）
    connect(mButtonGroupExpand, &QButtonGroup::idClicked, this, &CFreqDomainTiltGradDlg::onExpandMethodClicked);

    // 梯度计算前是否滤波（单选，两个按钮均为成员回调）
    connect(mRadioFilter, &QRadioButton::clicked, this, &CFreqDomainTiltGradDlg::onFilterClicked);
    connect(mRadioNonFilter, &QRadioButton::clicked, this, &CFreqDomainTiltGradDlg::onNonFilterClicked);

    // 处理参数
    connect(mSpinBeta, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainTiltGradDlg::onBetaValueChanged);
    connect(mSpinComNum, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainTiltGradDlg::onComNumValueChanged);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CFreqDomainTiltGradDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CFreqDomainTiltGradDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：添加一个输出文件行（编辑框 + 行标题按钮 + 显示按钮）
// 参数：pLayout —— 输出文件分组网格布局
// 参数：nRow —— 行号（0 起）
// 参数：pEdit —— 输出文件路径编辑框
// 参数：strTitle —— 行标题（原工程为无功能的按钮，仅作行名称显示）
// 返回：该行的“显示”按钮指针（供信号槽连接）
QPushButton* CFreqDomainTiltGradDlg::addOutputRow(QGridLayout* pLayout, int nRow, QLineEdit* pEdit,
                                                  const QString& strTitle)
{
    if (pLayout == NULL || pEdit == NULL)
    {
        return NULL;
    }

    pLayout->addWidget(pEdit, nRow, 0);

    // 行标题按钮（原工程该按钮无功能，仅作行名称显示；此处以按下不可用的形式呈现）
    QPushButton* pTitle = new QPushButton(strTitle, this);
    pTitle->setFixedWidth(156);
    pTitle->setEnabled(false);
    pLayout->addWidget(pTitle, nRow, 1);

    QPushButton* pView = new QPushButton(QStringLiteral("显示"), this);
    pView->setFixedWidth(48);
    pLayout->addWidget(pView, nRow, 2);

    pLayout->setColumnStretch(0, 1);
    return pView;
}

// 功能：选择输入数据文件（对应原工程 OnOpenFile）
void CFreqDomainTiltGradDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeOpenFile）
void CFreqDomainTiltGradDlg::onOpenFileTextEdited(const QString& strText)
{
    mInputFilePath = strText;
}

// 功能：读取文件头并刷新网格信息、默认值（对应原工程 ReadData）
void CFreqDomainTiltGradDlg::loadFile(const QString& strFilePath)
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

// 功能：生成默认输出文件名（对应原工程 ReadData 中 Tdr/Tdx/Tdy/Thd/Ttd.grd 命名规则）
void CFreqDomainTiltGradDlg::updateDefaultSavePaths(const QString& strFilePath)
{
    // 取最后一个点号之前的内容作为基准名（与原工程 ReverseFind('.') 一致）
    int nPos = strFilePath.lastIndexOf('.');
    QString strBase = strFilePath;
    if (nPos > 0)
    {
        strBase = strFilePath.left(nPos);
    }

    mSaveTdrFilePath = strBase + QStringLiteral("Tdr.grd");
    mSaveTdxFilePath = strBase + QStringLiteral("Tdx.grd");
    mSaveTdyFilePath = strBase + QStringLiteral("Tdy.grd");
    mSaveThdFilePath = strBase + QStringLiteral("Thd.grd");
    mSaveTtdFilePath = strBase + QStringLiteral("Ttd.grd");

    if (mEditSaveTdrFile != NULL)
    {
        mEditSaveTdrFile->setText(mSaveTdrFilePath);
    }
    if (mEditSaveTdxFile != NULL)
    {
        mEditSaveTdxFile->setText(mSaveTdxFilePath);
    }
    if (mEditSaveTdyFile != NULL)
    {
        mEditSaveTdyFile->setText(mSaveTdyFilePath);
    }
    if (mEditSaveThdFile != NULL)
    {
        mEditSaveThdFile->setText(mSaveThdFilePath);
    }
    if (mEditSaveTtdFile != NULL)
    {
        mEditSaveTtdFile->setText(mSaveTtdFilePath);
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CFreqDomainTiltGradDlg::updateGridInfoDisplay()
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
void CFreqDomainTiltGradDlg::setSpinExtendSize(QSpinBox* pSpin, int nSize)
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

// 功能：弹出打开文件对话框（.grd 网格数据文件）
QString CFreqDomainTiltGradDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：检查文件存在并发出发送显示请求（对应原工程 OnOpenFileView / OnSaveXxxFileView）
void CFreqDomainTiltGradDlg::viewGridFile(const QString& strFilePath)
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
void CFreqDomainTiltGradDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示 Tilt 梯度结果文件等值线
void CFreqDomainTiltGradDlg::onSaveTdrFileViewClicked()
{
    viewGridFile(mSaveTdrFilePath);
}

// 功能：显示斜导数 X 方向导数结果文件等值线
void CFreqDomainTiltGradDlg::onSaveTdxFileViewClicked()
{
    viewGridFile(mSaveTdxFilePath);
}

// 功能：显示斜导数 Y 方向导数结果文件等值线
void CFreqDomainTiltGradDlg::onSaveTdyFileViewClicked()
{
    viewGridFile(mSaveTdyFilePath);
}

// 功能：显示斜导数的水平梯度结果文件等值线
void CFreqDomainTiltGradDlg::onSaveThdFileViewClicked()
{
    viewGridFile(mSaveThdFilePath);
}

// 功能：显示斜导数的斜导数结果文件等值线
void CFreqDomainTiltGradDlg::onSaveTtdFileViewClicked()
{
    viewGridFile(mSaveTtdFilePath);
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeSaveXxxFile）
void CFreqDomainTiltGradDlg::onSaveTdrFileTextEdited(const QString& strText)
{
    mSaveTdrFilePath = strText;
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeSaveXxxFile）
void CFreqDomainTiltGradDlg::onSaveTdxFileTextEdited(const QString& strText)
{
    mSaveTdxFilePath = strText;
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeSaveXxxFile）
void CFreqDomainTiltGradDlg::onSaveTdyFileTextEdited(const QString& strText)
{
    mSaveTdyFilePath = strText;
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeSaveXxxFile）
void CFreqDomainTiltGradDlg::onSaveThdFileTextEdited(const QString& strText)
{
    mSaveThdFilePath = strText;
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeSaveXxxFile）
void CFreqDomainTiltGradDlg::onSaveTtdFileTextEdited(const QString& strText)
{
    mSaveTtdFilePath = strText;
}

// 功能：扩边行数变化（同步成员变量）
void CFreqDomainTiltGradDlg::onExRowsValueChanged(int nValue)
{
    mExRows = nValue;
}

// 功能：扩边列数变化（同步成员变量）
void CFreqDomainTiltGradDlg::onExColsValueChanged(int nValue)
{
    mExCols = nValue;
}

// 功能：数据扩边方法切换（同步成员变量）
void CFreqDomainTiltGradDlg::onExpandMethodClicked(int nMethodId)
{
    mExpandMethod = nMethodId;
}

// 功能：选择“梯度计算前滤波”
void CFreqDomainTiltGradDlg::onFilterClicked()
{
    mFilterBefore = true;
}

// 功能：选择“梯度计算前不滤波”（默认）
void CFreqDomainTiltGradDlg::onNonFilterClicked()
{
    mFilterBefore = false;
}

// 功能：指数因子变化（同步成员变量）
void CFreqDomainTiltGradDlg::onBetaValueChanged(double dValue)
{
    mBeta = dValue;
}

// 功能：补偿因子变化（同步成员变量）
void CFreqDomainTiltGradDlg::onComNumValueChanged(int nValue)
{
    mComNum = nValue;
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的空值检查，并给出更明确的提示）
bool CFreqDomainTiltGradDlg::validateInputs(QString& strError)
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
    // 5 个结果文件检查（原工程只检查输入文件，此处补充校验）
    if (mSaveTdrFilePath.isEmpty() || mSaveTdxFilePath.isEmpty() ||
        mSaveTdyFilePath.isEmpty() || mSaveThdFilePath.isEmpty() ||
        mSaveTtdFilePath.isEmpty())
    {
        strError = QStringLiteral("请填写完整的输出数据文件路径。");
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

// 功能：组装参数并调用后端 processTiltGrad（对应原工程 OnOK 主体逻辑）
void CFreqDomainTiltGradDlg::runProcess()
{
    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), QStringLiteral("后端接口未初始化。"));
        return;
    }

    // 组装处理参数（对应原工程界面上的全部输入项）
    TiltGradParams params;
    params.inputFilePath = toBackendString(mInputFilePath);
    params.tiltGradFilePath = toBackendString(mSaveTdrFilePath);
    params.tiltDxFilePath = toBackendString(mSaveTdxFilePath);
    params.tiltDyFilePath = toBackendString(mSaveTdyFilePath);
    params.tiltHorzGradFilePath = toBackendString(mSaveThdFilePath);
    params.tiltTiltFilePath = toBackendString(mSaveTtdFilePath);
    params.exRows = mExRows;
    params.exCols = mExCols;
    params.expandMethod = (ExpandMethod)mExpandMethod;
    params.filterBefore = mFilterBefore;
    params.comNum = mComNum;
    params.beta = (float)mBeta;

    // 同步调用后端处理（处理期间显示等待光标，与原工程 BeginWaitCursor 一致）
    BackendError error;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bOk = pBackend->processTiltGrad(params, error);
    QApplication::restoreOverrideCursor();

    if (bOk)
    {
        // 处理完成提示（与原工程一致，处理完成后对话框不关闭，由用户点“取消”退出）
        QMessageBox::information(this, QStringLiteral("处理完成"),
            QStringLiteral("梯度计算结束!"));
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), fromBackendString(error.message));
    }
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CFreqDomainTiltGradDlg::onOkClicked()
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
void CFreqDomainTiltGradDlg::onCancelClicked()
{
    reject();
}

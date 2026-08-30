// FreqDomainGradientDlg.cpp : 频率域梯度（梯度计算）对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_FreqDomainGradient；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查。

#include "FreqDomainGradientDlg.h"
#include "backend/RgisBackend.h"

#include <QApplication>
#include <QButtonGroup>
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

#include "core/BackendConvert.h"
#include "core/BackendService.h"
#include "core/ExtendGridSize.h"
#include <QSpinBox>

// 最大扩边尺寸（与原 MFC 工程 OnDeltaposSpinExCols 中 <= 65536 的约束一致）
static const int sMaxExtendSize = 65536;

// 构造函数：初始化全部成员并创建界面
CFreqDomainGradientDlg::CFreqDomainGradientDlg(QWidget* pParent)
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
    , mEditSaveHDFile(NULL)
    , mEditSaveFDFile(NULL)
    , mEditSaveZTFile(NULL)
    , mEditSaveTDFile(NULL)
    , mEditSaveTHFile(NULL)
    , mEditSaveFile6(NULL)
    , mBtnSaveHDFileView(NULL)
    , mBtnSaveFDFileView(NULL)
    , mBtnSaveZTFileView(NULL)
    , mBtnSaveTDFileView(NULL)
    , mBtnSaveTHFileView(NULL)
    , mBtnSaveFile6(NULL)
    , mBtnSaveFile6View(NULL)
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
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CFreqDomainGradientDlg::~CFreqDomainGradientDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CFreqDomainGradientDlg::initUi()
{
    setWindowTitle(QStringLiteral("梯度计算"));
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

    // ================= 数据文件输出（一）：水平梯度 / 总梯度 / Theta 图 =================
    // 对应 .rc 中的 IDC_SaveHDFile / IDC_SaveFDFile / IDC_SaveZTFile 三行
    QGroupBox* pGroupOutput1 = new QGroupBox(QStringLiteral("数据文件输出"), this);
    QGridLayout* pOutput1Layout = new QGridLayout(pGroupOutput1);
    mEditSaveHDFile = new QLineEdit(pGroupOutput1);
    mEditSaveFDFile = new QLineEdit(pGroupOutput1);
    mEditSaveZTFile = new QLineEdit(pGroupOutput1);
    mBtnSaveHDFileView = addOutputRow(pOutput1Layout, 0, mEditSaveHDFile, QStringLiteral("水平梯度"), false);
    mBtnSaveFDFileView = addOutputRow(pOutput1Layout, 1, mEditSaveFDFile, QStringLiteral("总梯度"), false);
    mBtnSaveZTFileView = addOutputRow(pOutput1Layout, 2, mEditSaveZTFile, QStringLiteral("Theta 图"), false);
    pMainLayout->addWidget(pGroupOutput1);

    // ================= 数据文件输出（二）：Tilt 导数 / 斜导数水平梯度 / 预留行 =================
    // 对应 .rc 中的 IDC_SaveTDFile / IDC_SaveTHFile / IDC_SaveFile6 三行
    QGroupBox* pGroupOutput2 = new QGroupBox(QStringLiteral("数据文件输出"), this);
    QGridLayout* pOutput2Layout = new QGridLayout(pGroupOutput2);
    // 第 1、2 行的行标题为“Tilt(斜)导数”“斜导数的水平梯度”（原工程为无功能按钮，此处同样不加功能）
    mEditSaveTDFile = new QLineEdit(pGroupOutput2);
    mEditSaveTHFile = new QLineEdit(pGroupOutput2);
    mBtnSaveTDFileView = addOutputRow(pOutput2Layout, 0, mEditSaveTDFile, QStringLiteral("Tilt(斜)导数"), false);
    mBtnSaveTHFileView = addOutputRow(pOutput2Layout, 1, mEditSaveTHFile, QStringLiteral("斜导数的水平梯度"), false);
    // 第 3 行：预留输出行，带“...”浏览按钮（原工程未使用该行，此处保留布局）
    mEditSaveFile6 = new QLineEdit(pGroupOutput2);
    mBtnSaveFile6 = new QPushButton(QStringLiteral("..."), pGroupOutput2);
    mBtnSaveFile6->setFixedWidth(32);
    mBtnSaveFile6View = new QPushButton(QStringLiteral("显示"), pGroupOutput2);
    mBtnSaveFile6View->setFixedWidth(48);
    pOutput2Layout->addWidget(mEditSaveFile6, 2, 0);
    pOutput2Layout->addWidget(mBtnSaveFile6, 2, 1);
    pOutput2Layout->addWidget(mBtnSaveFile6View, 2, 2);
    pOutput2Layout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupOutput2);

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
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onOpenFileViewClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CFreqDomainGradientDlg::onOpenFileTextEdited);

    // 输出文件“显示”按钮
    connect(mBtnSaveHDFileView, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onSaveHDFileViewClicked);
    connect(mBtnSaveFDFileView, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onSaveFDFileViewClicked);
    connect(mBtnSaveZTFileView, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onSaveZTFileViewClicked);
    connect(mBtnSaveTDFileView, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onSaveTDFileViewClicked);
    connect(mBtnSaveTHFileView, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onSaveTHFileViewClicked);
    connect(mBtnSaveFile6View, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onSaveFile6ViewClicked);

    // 输出文件编辑框（手工输入时同步成员变量，对应原工程 EN_CHANGE→UpdateData）
    connect(mEditSaveHDFile, &QLineEdit::textEdited, this, &CFreqDomainGradientDlg::onSaveHDFileTextEdited);
    connect(mEditSaveFDFile, &QLineEdit::textEdited, this, &CFreqDomainGradientDlg::onSaveFDFileTextEdited);
    connect(mEditSaveZTFile, &QLineEdit::textEdited, this, &CFreqDomainGradientDlg::onSaveZTFileTextEdited);
    connect(mEditSaveTDFile, &QLineEdit::textEdited, this, &CFreqDomainGradientDlg::onSaveTDFileTextEdited);
    connect(mEditSaveTHFile, &QLineEdit::textEdited, this, &CFreqDomainGradientDlg::onSaveTHFileTextEdited);
    connect(mEditSaveFile6, &QLineEdit::textEdited, this, &CFreqDomainGradientDlg::onSaveFile6TextEdited);
    connect(mBtnSaveFile6, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onSaveFile6Clicked);

    // 扩边行数/列数微调
    connect(mSpinExRows, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainGradientDlg::onExRowsValueChanged);
    connect(mSpinExCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainGradientDlg::onExColsValueChanged);

    // 扩边方法单选组（id 与 ExpandMethod 枚举一致）
    connect(mButtonGroupExpand, &QButtonGroup::idClicked, this, &CFreqDomainGradientDlg::onExpandMethodClicked);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CFreqDomainGradientDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：添加一个输出文件行（编辑框 + 行标题/浏览按钮 + 显示按钮）
// 参数：pLayout —— 输出文件分组网格布局
// 参数：nRow —— 行号（0 起）
// 参数：pEdit —— 输出文件路径编辑框
// 参数：strTitle —— 行标题（原工程为无功能的按钮，“...”浏览按钮的行用 strTitle 为空表示）
// 参数：bHasBrowse —— 该行是否带“...”浏览按钮（原工程仅第 6 行带）
// 返回：该行的“显示”按钮指针（供信号槽连接）
QPushButton* CFreqDomainGradientDlg::addOutputRow(QGridLayout* pLayout, int nRow, QLineEdit* pEdit,
                                                  const QString& strTitle, bool bHasBrowse)
{
    if (pLayout == NULL || pEdit == NULL)
    {
        return NULL;
    }

    pLayout->addWidget(pEdit, nRow, 0);

    if (bHasBrowse)
    {
        // 带浏览按钮的行：“...”按钮 + “显示”按钮
        QPushButton* pBrowse = new QPushButton(QStringLiteral("..."), this);
        pBrowse->setFixedWidth(32);
        pLayout->addWidget(pBrowse, nRow, 1);
    }
    else
    {
        // 普通行：行标题按钮（原工程该按钮无功能，仅作行名称显示；此处以按下不可用的形式呈现）
        QPushButton* pTitle = new QPushButton(strTitle, this);
        pTitle->setFixedWidth(156);
        pTitle->setEnabled(false);
        pLayout->addWidget(pTitle, nRow, 1);
    }

    QPushButton* pView = new QPushButton(QStringLiteral("显示"), this);
    pView->setFixedWidth(48);
    pLayout->addWidget(pView, nRow, 2);

    pLayout->setColumnStretch(0, 1);
    return pView;
}

// 功能：选择输入数据文件（对应原工程 OnOpenFile）
void CFreqDomainGradientDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeOpenFile）
void CFreqDomainGradientDlg::onOpenFileTextEdited(const QString& strText)
{
    mInputFilePath = strText;
}

// 功能：读取文件头并刷新网格信息、默认值（对应原工程 OnOpenFile 中的读文件逻辑）
void CFreqDomainGradientDlg::loadFile(const QString& strFilePath)
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

    // 默认扩边尺寸（2 的幂，规则见 ExtendGridSize.h，与原工程一致）
    mMinExCols = suggestExtendSize(mColsNum);
    mMinExRows = suggestExtendSize(mRowsNum);
    setSpinExtendSize(mSpinExCols, mMinExCols);
    setSpinExtendSize(mSpinExRows, mMinExRows);
}

// 功能：生成默认输出文件名（与原工程 ReadData 中 _HD/_FD/_ZT/_TD/_TH.grd 命名规则一致）
void CFreqDomainGradientDlg::updateDefaultSavePaths(const QString& strFilePath)
{
    // 取最后一个点号之前的内容作为基准名（与原工程 ReverseFind('.') 一致）
    int nPos = strFilePath.lastIndexOf('.');
    QString strBase = strFilePath;
    if (nPos > 0)
    {
        strBase = strFilePath.left(nPos);
    }

    mSaveHDFilePath = strBase + QStringLiteral("_HD.grd");
    mSaveFDFilePath = strBase + QStringLiteral("_FD.grd");
    mSaveZTFilePath = strBase + QStringLiteral("_ZT.grd");
    mSaveTDFilePath = strBase + QStringLiteral("_TD.grd");
    mSaveTHFilePath = strBase + QStringLiteral("_TH.grd");

    if (mEditSaveHDFile != NULL)
    {
        mEditSaveHDFile->setText(mSaveHDFilePath);
    }
    if (mEditSaveFDFile != NULL)
    {
        mEditSaveFDFile->setText(mSaveFDFilePath);
    }
    if (mEditSaveZTFile != NULL)
    {
        mEditSaveZTFile->setText(mSaveZTFilePath);
    }
    if (mEditSaveTDFile != NULL)
    {
        mEditSaveTDFile->setText(mSaveTDFilePath);
    }
    if (mEditSaveTHFile != NULL)
    {
        mEditSaveTHFile->setText(mSaveTHFilePath);
    }
    // 第 6 个输出文件原工程不生成默认名，保持为空
}

// 功能：把网格信息成员变量刷到界面显示
void CFreqDomainGradientDlg::updateGridInfoDisplay()
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
void CFreqDomainGradientDlg::setSpinExtendSize(QSpinBox* pSpin, int nSize)
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
QString CFreqDomainGradientDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框（.grd 网格数据文件）
QString CFreqDomainGradientDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath,
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：检查文件存在并发出发送显示请求（对应原工程各“显示”按钮）
void CFreqDomainGradientDlg::viewGridFile(const QString& strFilePath)
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
void CFreqDomainGradientDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示水平梯度结果文件等值线
void CFreqDomainGradientDlg::onSaveHDFileViewClicked()
{
    viewGridFile(mSaveHDFilePath);
}

// 功能：显示总梯度结果文件等值线
void CFreqDomainGradientDlg::onSaveFDFileViewClicked()
{
    viewGridFile(mSaveFDFilePath);
}

// 功能：显示 Theta 图结果文件等值线
void CFreqDomainGradientDlg::onSaveZTFileViewClicked()
{
    viewGridFile(mSaveZTFilePath);
}

// 功能：显示 Tilt(斜)导数结果文件等值线
void CFreqDomainGradientDlg::onSaveTDFileViewClicked()
{
    viewGridFile(mSaveTDFilePath);
}

// 功能：显示斜导数水平梯度结果文件等值线
void CFreqDomainGradientDlg::onSaveTHFileViewClicked()
{
    viewGridFile(mSaveTHFilePath);
}

// 功能：显示第 6 个输出文件等值线（原工程预留行）
void CFreqDomainGradientDlg::onSaveFile6ViewClicked()
{
    viewGridFile(mSaveFile6Path);
}

// 功能：选择第 6 个输出文件（原工程预留行，原“...”按钮无功能，此处提供完整交互）
void CFreqDomainGradientDlg::onSaveFile6Clicked()
{
    QString strFilePath = askSaveFilePath(QStringLiteral("请输入处理结果数据文件名"), mSaveFile6Path);
    if (strFilePath.isEmpty())
    {
        return;
    }
    mSaveFile6Path = strFilePath;
    if (mEditSaveFile6 != NULL)
    {
        mEditSaveFile6->setText(strFilePath);
    }
}

// 功能：手动输入水平梯度路径（同步成员变量，对应原工程 OnChangeSaveHDFile）
void CFreqDomainGradientDlg::onSaveHDFileTextEdited(const QString& strText)
{
    mSaveHDFilePath = strText;
}

// 功能：手动输入总梯度路径（同步成员变量，对应原工程 OnChangeSaveFDFile）
void CFreqDomainGradientDlg::onSaveFDFileTextEdited(const QString& strText)
{
    mSaveFDFilePath = strText;
}

// 功能：手动输入 Theta 图路径（同步成员变量，对应原工程 OnChangeSaveZTFile）
void CFreqDomainGradientDlg::onSaveZTFileTextEdited(const QString& strText)
{
    mSaveZTFilePath = strText;
}

// 功能：手动输入 Tilt(斜)导数路径（同步成员变量，对应原工程 OnChangeSaveTDFile）
void CFreqDomainGradientDlg::onSaveTDFileTextEdited(const QString& strText)
{
    mSaveTDFilePath = strText;
}

// 功能：手动输入斜导数水平梯度路径（同步成员变量，对应原工程 OnChangeSaveTHFile）
void CFreqDomainGradientDlg::onSaveTHFileTextEdited(const QString& strText)
{
    mSaveTHFilePath = strText;
}

// 功能：手动输入第 6 个输出文件路径（同步成员变量，对应原工程预留行）
void CFreqDomainGradientDlg::onSaveFile6TextEdited(const QString& strText)
{
    mSaveFile6Path = strText;
}

// 功能：扩边行数变化（同步成员变量）
void CFreqDomainGradientDlg::onExRowsValueChanged(int nValue)
{
    mExRows = nValue;
}

// 功能：扩边列数变化（同步成员变量）
void CFreqDomainGradientDlg::onExColsValueChanged(int nValue)
{
    mExCols = nValue;
}

// 功能：数据扩边方法切换（同步成员变量）
void CFreqDomainGradientDlg::onExpandMethodClicked(int nMethodId)
{
    mExpandMethod = nMethodId;
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的检查，并给出更明确的提示）
bool CFreqDomainGradientDlg::validateInputs(QString& strError)
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
    // 5 个结果文件路径检查（原工程 OnOK 仅检查输入文件，此处给出明确提示）
    if (mSaveHDFilePath.isEmpty() || mSaveFDFilePath.isEmpty() || mSaveZTFilePath.isEmpty()
        || mSaveTDFilePath.isEmpty() || mSaveTHFilePath.isEmpty())
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

// 功能：组装参数并调用后端 processGradient（对应原工程 OnOK 主体逻辑）
void CFreqDomainGradientDlg::runProcess()
{
    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        QMessageBox::warning(this, QStringLiteral("处理失败"), QStringLiteral("后端接口未初始化。"));
        return;
    }

    // 组装处理参数（对应原工程界面上的全部输入项与输出文件）
    GradientParams params;
    params.inputFilePath = toBackendString(mInputFilePath);
    params.horizontalGradientFilePath = toBackendString(mSaveHDFilePath);
    params.totalGradientFilePath = toBackendString(mSaveFDFilePath);
    params.thetaFilePath = toBackendString(mSaveZTFilePath);
    params.tiltFilePath = toBackendString(mSaveTDFilePath);
    params.tiltGradientFilePath = toBackendString(mSaveTHFilePath);
    params.exRows = mExRows;
    params.exCols = mExCols;
    params.expandMethod = (ExpandMethod)mExpandMethod;

    // 同步调用后端处理（处理期间显示等待光标，与原工程 BeginWaitCursor 一致）
    BackendError error;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bOk = pBackend->processGradient(params, error);
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
void CFreqDomainGradientDlg::onOkClicked()
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
void CFreqDomainGradientDlg::onCancelClicked()
{
    reject();
}

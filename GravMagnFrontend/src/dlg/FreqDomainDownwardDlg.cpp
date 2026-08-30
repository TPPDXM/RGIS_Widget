// FreqDomainDownwardDlg.cpp : 频率域向下延拓对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_FreqDomainDownward；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查。

#include "FreqDomainDownwardDlg.h"
#include "backend/RgisBackend.h"

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
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

#include "core/BackendConvert.h"
#include "core/BackendService.h"
#include "core/ExtendGridSize.h"
#include "core/Pow2SpinBox.h"

// 最大扩边尺寸（与原 MFC 工程 OnDeltaposSpinExCols 中 <= 65536 的约束一致）
static const int sMaxExtendSize = 65536;

// 构造函数：初始化全部成员并创建界面
// 参数：strFileNames —— 候选数据文件列表（分号分隔，与原 MFC 工程构造参数一致）
// 参数：pParent —— 父窗口（可为 NULL）
CFreqDomainDownwardDlg::CFreqDomainDownwardDlg(const QString& strFileNames, QWidget* pParent)
    : QDialog(pParent)
    , mComboOpenFile(NULL)
    , mBtnOpenFile(NULL)
    , mBtnOpenFileView(NULL)
    , mEditRowsNum(NULL)
    , mEditColsNum(NULL)
    , mEditColStep(NULL)
    , mEditRowStep(NULL)
    , mSpinExRows(NULL)
    , mSpinExCols(NULL)
    , mRadioCosFun(NULL)
    , mRadioAvgDif(NULL)
    , mRadioInvPow(NULL)
    , mRadioMinCrv(NULL)
    , mButtonGroupExpand(NULL)
    , mSpinHeight(NULL)
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
    , mHeight(0.0)
    , mExpandMethod(ExpandCosFun)
{
    // 候选文件列表加入“数据文件输入”下拉框（分号分隔，与原工程 AddFilesToComboBox 一致）
    addCandidateFiles(strFileNames);

    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CFreqDomainDownwardDlg::~CFreqDomainDownwardDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CFreqDomainDownwardDlg::initUi()
{
    setWindowTitle(QString::fromUtf8("频率域向下延拓"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 数据文件输入 =================
    QGroupBox* pGroupInput = new QGroupBox(QString::fromUtf8("数据文件输入"), this);
    QGridLayout* pInputLayout = new QGridLayout(pGroupInput);
    mComboOpenFile = new QComboBox(pGroupInput);
    mComboOpenFile->setEditable(true);
    mComboOpenFile->setInsertPolicy(QComboBox::NoInsert);
    mBtnOpenFile = new QPushButton(QString::fromUtf8("..."), pGroupInput);
    mBtnOpenFile->setFixedWidth(32);
    mBtnOpenFileView = new QPushButton(QString::fromUtf8("显示"), pGroupInput);
    mBtnOpenFileView->setFixedWidth(48);
    pInputLayout->addWidget(mComboOpenFile, 0, 0);
    pInputLayout->addWidget(mBtnOpenFile, 0, 1);
    pInputLayout->addWidget(mBtnOpenFileView, 0, 2);
    pInputLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupInput);

    // ================= 网格数据信息（只读显示）=================
    // 对应 .rc 中的 IDC_RowsNum / IDC_ColsNum / IDC_RowStep / IDC_ColStep
    QGroupBox* pGroupInfo = new QGroupBox(QString::fromUtf8("网格数据信息"), this);
    QGridLayout* pInfoLayout = new QGridLayout(pGroupInfo);
    pInfoLayout->addWidget(new QLabel(QString::fromUtf8("网格行数"), pGroupInfo), 0, 0);
    mEditRowsNum = new QLineEdit(pGroupInfo);
    mEditRowsNum->setReadOnly(true);
    mEditRowsNum->setAlignment(Qt::AlignCenter);
    mEditRowsNum->setFixedWidth(110);
    pInfoLayout->addWidget(mEditRowsNum, 0, 1);
    pInfoLayout->addWidget(new QLabel(QString::fromUtf8("网格列数"), pGroupInfo), 0, 2);
    mEditColsNum = new QLineEdit(pGroupInfo);
    mEditColsNum->setReadOnly(true);
    mEditColsNum->setAlignment(Qt::AlignCenter);
    mEditColsNum->setFixedWidth(110);
    pInfoLayout->addWidget(mEditColsNum, 0, 3);
    pInfoLayout->addWidget(new QLabel(QString::fromUtf8("网格行距"), pGroupInfo), 0, 4);
    mEditRowStep = new QLineEdit(pGroupInfo);
    mEditRowStep->setReadOnly(true);
    mEditRowStep->setAlignment(Qt::AlignCenter);
    mEditRowStep->setFixedWidth(110);
    pInfoLayout->addWidget(mEditRowStep, 0, 5);
    pInfoLayout->addWidget(new QLabel(QString::fromUtf8("网格列距"), pGroupInfo), 0, 6);
    mEditColStep = new QLineEdit(pGroupInfo);
    mEditColStep->setReadOnly(true);
    mEditColStep->setAlignment(Qt::AlignCenter);
    mEditColStep->setFixedWidth(110);
    pInfoLayout->addWidget(mEditColStep, 0, 7);
    pInfoLayout->setColumnStretch(1, 1);
    pInfoLayout->setColumnStretch(3, 1);
    pInfoLayout->setColumnStretch(5, 1);
    pInfoLayout->setColumnStretch(7, 1);
    pMainLayout->addWidget(pGroupInfo);

    // ================= 数据扩边信息 =================
    // 对应 .rc 中的 IDC_ExRows、IDC_ExCols 及对应微调钮
    QGroupBox* pGroupExtend = new QGroupBox(QString::fromUtf8("数据扩边信息"), this);
    QGridLayout* pExtendLayout = new QGridLayout(pGroupExtend);
    pExtendLayout->addWidget(new QLabel(QString::fromUtf8("扩边行数"), pGroupExtend), 0, 0);
    mSpinExRows = new CPow2SpinBox(pGroupExtend);
    mSpinExRows->setFixedWidth(100);
    pExtendLayout->addWidget(mSpinExRows, 0, 1);
    pExtendLayout->addWidget(new QLabel(QString::fromUtf8("扩边列数"), pGroupExtend), 0, 2);
    mSpinExCols = new CPow2SpinBox(pGroupExtend);
    mSpinExCols->setFixedWidth(100);
    pExtendLayout->addWidget(mSpinExCols, 0, 3);
    pExtendLayout->setColumnStretch(1, 1);
    pExtendLayout->setColumnStretch(3, 1);
    pMainLayout->addWidget(pGroupExtend);

    // ================= 数据扩边方法（单选）=================
    // 对应 .rc 中的 IDC_CosFun / IDC_AvgDif / IDC_InvPow / IDC_MinCrv
    QGroupBox* pGroupMethod = new QGroupBox(QString::fromUtf8("数据扩边方法"), this);
    QHBoxLayout* pMethodLayout = new QHBoxLayout(pGroupMethod);
    mButtonGroupExpand = new QButtonGroup(this);
    mRadioCosFun = new QRadioButton(QString::fromUtf8(" 余弦函数衰减"), pGroupMethod);
    mRadioAvgDif = new QRadioButton(QString::fromUtf8(" 平均值差分"), pGroupMethod);
    mRadioInvPow = new QRadioButton(QString::fromUtf8(" 反距离加权"), pGroupMethod);
    mRadioMinCrv = new QRadioButton(QString::fromUtf8(" 最小曲率"), pGroupMethod);
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

    // ================= 处理参数 =================
    // 对应 .rc 中的 IDC_UpHeight（延拓高度）
    QGroupBox* pGroupParams = new QGroupBox(QString::fromUtf8("处理参数"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    pParamsLayout->addWidget(new QLabel(QString::fromUtf8("延拓高度"), pGroupParams), 0, 0);
    mSpinHeight = new QDoubleSpinBox(pGroupParams);
    mSpinHeight->setRange(0.0, 999999.0);
    mSpinHeight->setDecimals(3);
    mSpinHeight->setSingleStep(1.0);
    mSpinHeight->setValue(mHeight);
    mSpinHeight->setAlignment(Qt::AlignCenter);
    mSpinHeight->setFixedWidth(140);
    pParamsLayout->addWidget(mSpinHeight, 0, 1);
    pParamsLayout->setColumnStretch(1, 1);
    pMainLayout->addWidget(pGroupParams);

    // ================= 数据文件输出 =================
    // 对应 .rc 中的 IDC_SaveFile / ID_SaveFile / ID_SaveFileView
    QGroupBox* pGroupOutput = new QGroupBox(QString::fromUtf8("数据文件输出"), this);
    QGridLayout* pOutputLayout = new QGridLayout(pGroupOutput);
    mEditSaveFile = new QLineEdit(pGroupOutput);
    mBtnSaveFile = new QPushButton(QString::fromUtf8("..."), pGroupOutput);
    mBtnSaveFile->setFixedWidth(32);
    mBtnSaveFileView = new QPushButton(QString::fromUtf8("显示"), pGroupOutput);
    mBtnSaveFileView->setFixedWidth(48);
    pOutputLayout->addWidget(mEditSaveFile, 0, 0);
    pOutputLayout->addWidget(mBtnSaveFile, 0, 1);
    pOutputLayout->addWidget(mBtnSaveFileView, 0, 2);
    pOutputLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupOutput);

    // ================= 确定 / 取消 =================
    QHBoxLayout* pButtonLayout = new QHBoxLayout();
    mBtnOk = new QPushButton(QString::fromUtf8("确  定"), this);
    mBtnCancel = new QPushButton(QString::fromUtf8("取  消"), this);
    mBtnOk->setDefault(true);
    // 确定按钮在左下角、取消按钮在右下角（与原工程布局一致，见示例图片 SubWindow.png）
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
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CFreqDomainDownwardDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CFreqDomainDownwardDlg::onOpenFileViewClicked);
    connect(mComboOpenFile, QOverload<int>::of(&QComboBox::activated), this, &CFreqDomainDownwardDlg::onFileComboActivated);
    connect(mComboOpenFile->lineEdit(), &QLineEdit::editingFinished, this, &CFreqDomainDownwardDlg::onFileEditFinished);

    // 扩边行数/列数（2 的幂微调）
    connect(mSpinExRows, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainDownwardDlg::onExRowsValueChanged);
    connect(mSpinExCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainDownwardDlg::onExColsValueChanged);

    // 扩边方法单选组（id 与 ExpandMethod 枚举一致）
    connect(mButtonGroupExpand, &QButtonGroup::idClicked, this, &CFreqDomainDownwardDlg::onExpandMethodClicked);

    // 处理参数
    connect(mSpinHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainDownwardDlg::onHeightValueChanged);

    // 数据文件输出
    connect(mBtnSaveFile, &QPushButton::clicked, this, &CFreqDomainDownwardDlg::onSaveFileClicked);
    connect(mBtnSaveFileView, &QPushButton::clicked, this, &CFreqDomainDownwardDlg::onSaveFileViewClicked);
    // 输出文件名编辑框（手工输入时同步成员变量，对应原工程 EN_CHANGE→UpdateData）
    connect(mEditSaveFile, &QLineEdit::textEdited, this, &CFreqDomainDownwardDlg::onSaveFileTextEdited);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CFreqDomainDownwardDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CFreqDomainDownwardDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：把分号分隔的候选文件加入下拉框（对应原工程 AddFilesToComboBox）
void CFreqDomainDownwardDlg::addCandidateFiles(const QString& strFileNames)
{
    if (mComboOpenFile == NULL)
    {
        return;
    }

    QStringList fileList = strFileNames.split(';', Qt::SkipEmptyParts);
    for (int i = 0; i < fileList.size(); i++)
    {
        QString strFile = fileList.at(i).trimmed();
        if (strFile.isEmpty())
        {
            continue;
        }
        // 下拉框中显示完整路径（与原工程 GetFileTitleFromFileName 显示内容一致）
        if (!mCandidateFiles.contains(strFile))
        {
            mCandidateFiles.append(strFile);
            mComboOpenFile->addItem(strFile);
        }
    }
}

// 功能：选择输入数据文件（对应原工程 OnOpenFile）
void CFreqDomainDownwardDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QString::fromUtf8("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }

    // 候选列表已有该文件则直接选中，否则追加到下拉框
    int nIndex = mCandidateFiles.indexOf(strFilePath);
    if (nIndex >= 0)
    {
        mComboOpenFile->setCurrentIndex(nIndex);
    }
    else
    {
        mCandidateFiles.append(strFilePath);
        mComboOpenFile->addItem(strFilePath);
        mComboOpenFile->setCurrentIndex(mComboOpenFile->count() - 1);
    }

    loadFile(strFilePath);
}

// 功能：下拉选择输入文件后重新载入数据（对应原工程 OnSelchangeCombo1）
void CFreqDomainDownwardDlg::onFileComboActivated(int nIndex)
{
    if (nIndex < 0 || nIndex >= mCandidateFiles.size())
    {
        return;
    }
    loadFile(mCandidateFiles.at(nIndex));
}

// 功能：手工输入文件名（回车/失焦）后重新载入数据
void CFreqDomainDownwardDlg::onFileEditFinished()
{
    if (mComboOpenFile == NULL)
    {
        return;
    }
    QString strFilePath = mComboOpenFile->currentText().trimmed();
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：读取文件头并刷新网格信息、默认输出名、默认扩边尺寸（对应原工程 ReadData）
void CFreqDomainDownwardDlg::loadFile(const QString& strFilePath)
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
        QMessageBox::warning(this, QString::fromUtf8("读取文件失败"), fromBackendString(error.message));
        return;
    }

    // 记录加载状态并同步下拉框显示（与原工程 SetWindowText 一致）
    mLoadedFilePath = strFilePath;
    mInputFilePath = strFilePath;
    if (mComboOpenFile != NULL && mComboOpenFile->currentText() != strFilePath)
    {
        mComboOpenFile->setEditText(strFilePath);
    }

    // 网格信息（与原工程 ReadData 中的 xStep/yStep 计算一致）
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

    // 默认延拓高度：2 倍列距（与原工程 ReadData 中 "%.3f", 2 * xStep 一致）
    mHeight = 2.0 * mColStep;
    if (mSpinHeight != NULL)
    {
        mSpinHeight->setValue(mHeight);
    }
}

// 功能：生成默认输出文件名（对应原工程 ReadData 中 _Pro.grd 命名规则）
void CFreqDomainDownwardDlg::updateDefaultSavePaths(const QString& strFilePath)
{
    // 取最后一个点号之前的内容作为基准名（与原工程 ReverseFind('.') 一致）
    int nPos = strFilePath.lastIndexOf('.');
    QString strBase = strFilePath;
    if (nPos > 0)
    {
        strBase = strFilePath.left(nPos);
    }

    mOutputFilePath = strBase + QString::fromUtf8("_Pro.grd");

    if (mEditSaveFile != NULL)
    {
        mEditSaveFile->setText(mOutputFilePath);
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CFreqDomainDownwardDlg::updateGridInfoDisplay()
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
void CFreqDomainDownwardDlg::setSpinExtendSize(CPow2SpinBox* pSpin, int nSize)
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
void CFreqDomainDownwardDlg::onSaveFileClicked()
{
    QString strFilePath = askSaveFilePath(QString::fromUtf8("请输入处理结果数据文件名"), mOutputFilePath);
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
void CFreqDomainDownwardDlg::onSaveFileTextEdited(const QString& strText)
{
    mOutputFilePath = strText;
}

// 功能：弹出打开文件对话框（.grd 网格数据文件）
QString CFreqDomainDownwardDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QString::fromUtf8("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框（.grd 网格数据文件）
QString CFreqDomainDownwardDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath,
        QString::fromUtf8("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：检查文件存在并发出发送显示请求（对应原工程 OnOpenFileView / OnSaveFileView）
void CFreqDomainDownwardDlg::viewGridFile(const QString& strFilePath)
{
    if (strFilePath.isEmpty())
    {
        return;
    }
    // 文件不存在时提示并返回（与原工程 GetFileAttributes == -1 判断一致）
    if (!QFile::exists(strFilePath))
    {
        QMessageBox::warning(this, QString::fromUtf8("文件不存在"),
            QString::fromUtf8("文件") + strFilePath + QString::fromUtf8("不存在!"));
        return;
    }
    // 等值线显示窗口由前端后续版本实现，此处交由宿主处理
    emit viewGridFileRequested(strFilePath);
}

// 功能：显示输入数据文件等值线
void CFreqDomainDownwardDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示处理结果文件等值线
void CFreqDomainDownwardDlg::onSaveFileViewClicked()
{
    viewGridFile(mOutputFilePath);
}

// 功能：扩边行数变化（同步成员变量）
void CFreqDomainDownwardDlg::onExRowsValueChanged(int nValue)
{
    mExRows = nValue;
}

// 功能：扩边列数变化（同步成员变量）
void CFreqDomainDownwardDlg::onExColsValueChanged(int nValue)
{
    mExCols = nValue;
}

// 功能：数据扩边方法切换（同步成员变量）
void CFreqDomainDownwardDlg::onExpandMethodClicked(int nMethodId)
{
    mExpandMethod = nMethodId;
}

// 功能：延拓高度变化（同步成员变量）
void CFreqDomainDownwardDlg::onHeightValueChanged(double dValue)
{
    mHeight = dValue;
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的空值检查，并给出更明确的提示）
bool CFreqDomainDownwardDlg::validateInputs(QString& strError)
{
    // 输入数据文件检查
    if (mInputFilePath.isEmpty())
    {
        strError = QString::fromUtf8("请先选择输入数据文件。");
        return false;
    }
    if (!QFile::exists(mInputFilePath))
    {
        strError = QString::fromUtf8("输入数据文件不存在：") + mInputFilePath;
        return false;
    }
    // 输出文件检查
    if (mOutputFilePath.isEmpty())
    {
        strError = QString::fromUtf8("请输入处理结果数据文件路径。");
        return false;
    }
    // 扩边尺寸检查（必须不小于读文件时计算的最小尺寸）
    if (mExCols < mMinExCols || mExRows < mMinExRows)
    {
        strError = QString::fromUtf8("扩边行数/列数不得小于网格数据要求的最小尺寸。");
        return false;
    }
    return true;
}

// 功能：组装参数并调用后端 processDownward（对应原工程 OnOK 主体逻辑）
void CFreqDomainDownwardDlg::runProcess()
{
    // 后端接口按需从全局服务获取（不缓存为成员指针）
    IRgisBackend* pBackend = CBackendService::rgisBackend();
    if (pBackend == NULL)
    {
        QMessageBox::warning(this, QString::fromUtf8("处理失败"), QString::fromUtf8("后端接口未初始化。"));
        return;
    }

    // 组装处理参数（对应原工程界面上的全部输入项）
    DownwardParams params;
    params.inputFilePath = toBackendString(mInputFilePath);
    params.outputFilePath = toBackendString(mOutputFilePath);
    params.exRows = mExRows;
    params.exCols = mExCols;
    params.expandMethod = (ExpandMethod)mExpandMethod;
    params.height = (float)mHeight;

    // 同步调用后端处理（处理期间显示等待光标，与原工程 BeginWaitCursor 一致）
    BackendError error;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bOk = pBackend->processDownward(params, error);
    QApplication::restoreOverrideCursor();

    if (bOk)
    {
        // 处理完成提示（注意：与原工程一致，处理完成后对话框不关闭，由用户点“取消”退出）
        QMessageBox::information(this, QString::fromUtf8("处理完成"),
            QString::fromUtf8("频率域向下延拓处理结束!"));
    }
    else
    {
        QMessageBox::warning(this, QString::fromUtf8("处理失败"), fromBackendString(error.message));
    }
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CFreqDomainDownwardDlg::onOkClicked()
{
    QString strError;
    if (!validateInputs(strError))
    {
        QMessageBox::warning(this, QString::fromUtf8("参数错误"), strError);
        return;
    }
    runProcess();
}

// 功能：“取消”——关闭对话框（对应原工程 OnCancel）
void CFreqDomainDownwardDlg::onCancelClicked()
{
    reject();
}

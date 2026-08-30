// FreqDomainGradwardDlg.cpp : 频率域逐次向下延拓（正则化滤波）对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_FreqDomainGradward；
//   * 逐次延拓参数表格对应原工程 MSFlexGrid（3 列：序号 / 延拓高度 / 水平几何尺度）；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查。

#include "FreqDomainGradwardDlg.h"

#include <QApplication>
#include <QButtonGroup>
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
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include "core/BackendConvert.h"
#include "core/BackendService.h"
#include "core/ExtendGridSize.h"
#include "core/Pow2SpinBox.h"

// 最大扩边尺寸（与原 MFC 工程 OnDeltaposSpinExCols 中 <= 65536 的约束一致）
static const int sMaxExtendSize = 65536;

// 构造函数：初始化全部成员并创建界面
CFreqDomainGradwardDlg::CFreqDomainGradwardDlg(QWidget* pParent)
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
    , mSpinHeight(NULL)
    , mSpinTimes(NULL)
    , mTableSteps(NULL)
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
    , mTimes(5)
    , mExpandMethod(ExpandCosFun)
    , mBackend(NULL)
{
    // 取后端接口指针（从未注入时由服务返回内置占位实现，保证 mBackend 永不为 NULL）
    mBackend = CBackendService::rgisBackend();
    if (mBackend == NULL)
    {
        return;
    }

    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CFreqDomainGradwardDlg::~CFreqDomainGradwardDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CFreqDomainGradwardDlg::initUi()
{
    setWindowTitle(QString::fromUtf8("频率域逐次向下延拓(正则化滤波)"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 数据文件输入 =================
    QGroupBox* pGroupInput = new QGroupBox(QString::fromUtf8("数据文件输入"), this);
    QGridLayout* pInputLayout = new QGridLayout(pGroupInput);
    mEditOpenFile = new QLineEdit(pGroupInput);
    mBtnOpenFile = new QPushButton(QString::fromUtf8("..."), pGroupInput);
    mBtnOpenFile->setFixedWidth(32);
    mBtnOpenFileView = new QPushButton(QString::fromUtf8("显示"), pGroupInput);
    mBtnOpenFileView->setFixedWidth(48);
    pInputLayout->addWidget(mEditOpenFile, 0, 0);
    pInputLayout->addWidget(mBtnOpenFile, 0, 1);
    pInputLayout->addWidget(mBtnOpenFileView, 0, 2);
    pInputLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupInput);

    // ================= 网格数据信息（只读显示，2 行 2 列）=================
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
    pInfoLayout->addWidget(new QLabel(QString::fromUtf8("网格行距"), pGroupInfo), 1, 0);
    mEditRowStep = new QLineEdit(pGroupInfo);
    mEditRowStep->setReadOnly(true);
    mEditRowStep->setAlignment(Qt::AlignCenter);
    mEditRowStep->setFixedWidth(110);
    pInfoLayout->addWidget(mEditRowStep, 1, 1);
    pInfoLayout->addWidget(new QLabel(QString::fromUtf8("网格列距"), pGroupInfo), 1, 2);
    mEditColStep = new QLineEdit(pGroupInfo);
    mEditColStep->setReadOnly(true);
    mEditColStep->setAlignment(Qt::AlignCenter);
    mEditColStep->setFixedWidth(110);
    pInfoLayout->addWidget(mEditColStep, 1, 3);
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
    pMainLayout->addWidget(pGroupExtend);

    // ================= 数据扩边方法（单选，仅 3 种）=================
    // 对应 .rc 中的 IDC_CosFun / IDC_AvgDif / IDC_InvPow
    QGroupBox* pGroupMethod = new QGroupBox(QString::fromUtf8("数据扩边方法"), this);
    QHBoxLayout* pMethodLayout = new QHBoxLayout(pGroupMethod);
    mButtonGroupExpand = new QButtonGroup(this);
    mRadioCosFun = new QRadioButton(QString::fromUtf8(" 余弦函数衰减"), pGroupMethod);
    mRadioAvgDif = new QRadioButton(QString::fromUtf8(" 平均值差分"), pGroupMethod);
    mRadioInvPow = new QRadioButton(QString::fromUtf8(" 反距离加权"), pGroupMethod);
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

    // ================= 延拓参数（含逐次延拓参数表格）=================
    // 对应 .rc 中的 IDC_UpHeight（延拓高度）、IDC_Times（延拓次数）、IDC_MSFLEXGRID（参数表格）
    QGroupBox* pGroupParams = new QGroupBox(QString::fromUtf8("延拓参数"), this);
    QGridLayout* pParamsLayout = new QGridLayout(pGroupParams);
    // 第一行：延拓高度 / 延拓次数
    pParamsLayout->addWidget(new QLabel(QString::fromUtf8("延拓高度"), pGroupParams), 0, 0);
    mSpinHeight = new QDoubleSpinBox(pGroupParams);
    mSpinHeight->setRange(0.0, 999999.0);
    mSpinHeight->setDecimals(3);
    mSpinHeight->setSingleStep(1.0);
    mSpinHeight->setValue(mHeight);
    mSpinHeight->setAlignment(Qt::AlignCenter);
    mSpinHeight->setFixedWidth(140);
    pParamsLayout->addWidget(mSpinHeight, 0, 1);
    pParamsLayout->addWidget(new QLabel(QString::fromUtf8("延拓次数"), pGroupParams), 0, 2);
    mSpinTimes = new QSpinBox(pGroupParams);
    mSpinTimes->setRange(1, 9999);
    mSpinTimes->setValue(mTimes);
    mSpinTimes->setAlignment(Qt::AlignCenter);
    mSpinTimes->setFixedWidth(120);
    pParamsLayout->addWidget(mSpinTimes, 0, 3);
    pParamsLayout->setColumnStretch(1, 1);
    pParamsLayout->setColumnStretch(3, 1);

    // 第二行：逐次延拓参数表格（序号 / 延拓高度 / 水平几何尺度，对应原工程 MSFlexGrid）
    mTableSteps = new QTableWidget(pGroupParams);
    mTableSteps->setColumnCount(3);
    mTableSteps->setHorizontalHeaderLabels(QStringList()
        << QString::fromUtf8("序号") << QString::fromUtf8("延拓高度") << QString::fromUtf8("水平几何尺度"));
    mTableSteps->horizontalHeader()->setStretchLastSection(true);
    mTableSteps->setColumnWidth(0, 70);
    mTableSteps->setColumnWidth(1, 130);
    mTableSteps->setColumnWidth(2, 130);
    mTableSteps->verticalHeader()->hide();
    mTableSteps->setSelectionBehavior(QAbstractItemView::SelectItems);
    mTableSteps->setSelectionMode(QAbstractItemView::SingleSelection);
    mTableSteps->setMinimumHeight(150);
    pParamsLayout->addWidget(mTableSteps, 1, 0, 1, 4);
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

    // 输入 / 输出文件
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CFreqDomainGradwardDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CFreqDomainGradwardDlg::onOpenFileViewClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CFreqDomainGradwardDlg::onOpenFileTextEdited);
    connect(mBtnSaveFile, &QPushButton::clicked, this, &CFreqDomainGradwardDlg::onSaveFileClicked);
    connect(mBtnSaveFileView, &QPushButton::clicked, this, &CFreqDomainGradwardDlg::onSaveFileViewClicked);
    connect(mEditSaveFile, &QLineEdit::textEdited, this, &CFreqDomainGradwardDlg::onSaveFileTextEdited);

    // 扩边行数/列数（2 的幂微调）
    connect(mSpinExRows, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainGradwardDlg::onExRowsValueChanged);
    connect(mSpinExCols, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainGradwardDlg::onExColsValueChanged);

    // 扩边方法单选组（id 与 ExpandMethod 枚举一致）
    connect(mButtonGroupExpand, &QButtonGroup::idClicked, this, &CFreqDomainGradwardDlg::onExpandMethodClicked);

    // 延拓参数（延拓次数变化时重建表格默认值）
    connect(mSpinHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CFreqDomainGradwardDlg::onHeightValueChanged);
    connect(mSpinTimes, QOverload<int>::of(&QSpinBox::valueChanged), this, &CFreqDomainGradwardDlg::onTimesValueChanged);
    connect(mTableSteps, &QTableWidget::cellClicked, this, &CFreqDomainGradwardDlg::onTableClicked);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CFreqDomainGradwardDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CFreqDomainGradwardDlg::onCancelClicked);

    // 对话框初始尺寸（含表格，允许一定伸缩）
    setMinimumWidth(520);
    adjustSize();

    // 初始化逐次延拓参数表格（按初始延拓次数填入默认值）
    rebuildStepTable();
}

// 功能：选择输入数据文件（对应原工程 OnOpenFile）
void CFreqDomainGradwardDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QString::fromUtf8("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：手动输入文件名（同步成员变量，对应原工程 OnChangeOpenFile）
void CFreqDomainGradwardDlg::onOpenFileTextEdited(const QString& strText)
{
    mInputFilePath = strText;
}

// 功能：读取文件头并刷新网格信息、默认值（对应原工程 OnOpenFile 中的读文件逻辑）
void CFreqDomainGradwardDlg::loadFile(const QString& strFilePath)
{
    if (strFilePath.isEmpty())
    {
        return;
    }
    if (mBackend == NULL)
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
    if (!mBackend->readGridFileHead(toBackendString(strFilePath), head, error))
    {
        QMessageBox::warning(this, QString::fromUtf8("读取文件失败"), fromBackendString(error.message));
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

    // 默认总延拓高度：10 倍列距（与原工程 "%.3f", 10 * xStep 一致）
    mHeight = 10.0 * mColStep;
    if (mSpinHeight != NULL)
    {
        mSpinHeight->setValue(mHeight);
    }

    // 重建逐次延拓参数表格默认值
    rebuildStepTable();
}

// 功能：生成默认输出文件名（与原工程 ReadData 中 + "OutN.grd" 命名规则一致）
void CFreqDomainGradwardDlg::updateDefaultSavePaths(const QString& strFilePath)
{
    // 取最后一个点号之前的内容作为基准名（与原工程 ReverseFind('.') 一致）
    int nPos = strFilePath.lastIndexOf('.');
    QString strBase = strFilePath;
    if (nPos > 0)
    {
        strBase = strFilePath.left(nPos);
    }

    mOutputFilePath = strBase + QString::fromUtf8("OutN.grd");

    if (mEditSaveFile != NULL)
    {
        mEditSaveFile->setText(mOutputFilePath);
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CFreqDomainGradwardDlg::updateGridInfoDisplay()
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
void CFreqDomainGradwardDlg::setSpinExtendSize(CPow2SpinBox* pSpin, int nSize)
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

// 功能：按延拓次数重建表格行并填入默认值（对应原工程 OnClickMsFlexGrid 的填表逻辑）
void CFreqDomainGradwardDlg::rebuildStepTable()
{
    if (mTableSteps == NULL)
    {
        return;
    }
    if (mTimes < 1)
    {
        mTimes = 1;
    }

    // 数据行数 = 延拓次数（表头由 QTableWidget 表头承载，占一行，与原工程 0 行为表头一致）
    mTableSteps->setRowCount(mTimes);

    for (int i = 0; i < mTimes; i++)
    {
        // 第 0 列：序号（只读）
        QTableWidgetItem* pItemIndex = new QTableWidgetItem(QString::number(i + 1));
        pItemIndex->setFlags(pItemIndex->flags() & ~Qt::ItemIsEditable);
        pItemIndex->setTextAlignment(Qt::AlignCenter);
        mTableSteps->setItem(i, 0, pItemIndex);

        // 第 1 列：该步延拓高度（默认 总高度/延拓次数，与原工程 "%.2f" 一致）
        double dStepHeight = (mTimes > 0) ? (mHeight / mTimes) : 0.0;
        QTableWidgetItem* pItemHeight = new QTableWidgetItem(QString::number(dStepHeight, 'f', 2));
        pItemHeight->setTextAlignment(Qt::AlignCenter);
        mTableSteps->setItem(i, 1, pItemHeight);

        // 第 2 列：水平几何尺度（默认 5 + 序号，与原工程 5+i 一致）
        QTableWidgetItem* pItemScale = new QTableWidgetItem(QString::number(5 + i + 1));
        pItemScale->setTextAlignment(Qt::AlignCenter);
        mTableSteps->setItem(i, 2, pItemScale);
    }
}

// 功能：点击逐次延拓参数表格（行内容为空时填入默认值；已编辑过的行保留，避免覆盖用户输入）
void CFreqDomainGradwardDlg::onTableClicked()
{
    if (mTableSteps == NULL)
    {
        return;
    }

    bool bNeedFill = false;
    for (int i = 0; i < mTableSteps->rowCount(); i++)
    {
        QTableWidgetItem* pItem = mTableSteps->item(i, 1);
        if (pItem == NULL || pItem->text().isEmpty())
        {
            bNeedFill = true;
            break;
        }
    }

    if (bNeedFill)
    {
        rebuildStepTable();
    }
}

// 功能：读取表格各步参数并校验（对应原工程 OnOK 中的表格读取与校验）
// 参数：steps —— 输出各步参数
// 参数：strError —— 校验失败时的错误描述
// 返回：true 校验通过；false 校验失败（strError 给出原因）
bool CFreqDomainGradwardDlg::readStepTable(QString& strError)
{
    if (mTableSteps == NULL)
    {
        strError = QString::fromUtf8("逐次延拓参数表格未初始化。");
        return false;
    }

    mStepValues.resize(mTimes);
    for (int i = 0; i < mTimes; i++)
    {
        // ===== 读第 1 列：该步延拓高度 =====
        QTableWidgetItem* pItemHeight = mTableSteps->item(i, 1);
        QString strHeight = (pItemHeight != NULL) ? pItemHeight->text().trimmed() : QString();
        if (strHeight.isEmpty())
        {
            strError = QString::fromUtf8("逐次延拓高度参数输入没有完成！");
            return false;
        }
        bool bOkHeight = false;
        double dHeight = strHeight.toDouble(&bOkHeight);
        if (!bOkHeight)
        {
            strError = QString::fromUtf8("逐次延拓高度参数输入错误！");
            return false;
        }
        if (dHeight < 0.0 || dHeight > mHeight)
        {
            strError = QString::fromUtf8("逐次延拓高度参数输入错误！");
            return false;
        }
        mStepValues[i].mHeight = dHeight;

        // ===== 读第 2 列：水平几何尺度 =====
        QTableWidgetItem* pItemScale = mTableSteps->item(i, 2);
        QString strScale = (pItemScale != NULL) ? pItemScale->text().trimmed() : QString();
        if (strScale.isEmpty())
        {
            strError = QString::fromUtf8("水平几何尺度参数输入没有完成！");
            return false;
        }
        bool bOkScale = false;
        double dScale = strScale.toDouble(&bOkScale);
        if (!bOkScale || dScale < 0.0)
        {
            strError = QString::fromUtf8("水平几何尺度参数输入错误！");
            return false;
        }
        // 与原工程一致：先取整再使用（(int)atof 截断）
        mStepValues[i].mScale = (int)dScale;
    }
    return true;
}

// 功能：选择处理结果数据文件（对应原工程 OnSaveFile）
void CFreqDomainGradwardDlg::onSaveFileClicked()
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
void CFreqDomainGradwardDlg::onSaveFileTextEdited(const QString& strText)
{
    mOutputFilePath = strText;
}

// 功能：弹出打开文件对话框（.grd 网格数据文件）
QString CFreqDomainGradwardDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QString::fromUtf8("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框（.grd 网格数据文件）
QString CFreqDomainGradwardDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath,
        QString::fromUtf8("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：检查文件存在并发出发送显示请求（对应原工程 OnOpenFileView / OnSaveFileView）
void CFreqDomainGradwardDlg::viewGridFile(const QString& strFilePath)
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
void CFreqDomainGradwardDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示处理结果文件等值线
void CFreqDomainGradwardDlg::onSaveFileViewClicked()
{
    viewGridFile(mOutputFilePath);
}

// 功能：扩边行数变化（同步成员变量）
void CFreqDomainGradwardDlg::onExRowsValueChanged(int nValue)
{
    mExRows = nValue;
}

// 功能：扩边列数变化（同步成员变量）
void CFreqDomainGradwardDlg::onExColsValueChanged(int nValue)
{
    mExCols = nValue;
}

// 功能：数据扩边方法切换（同步成员变量）
void CFreqDomainGradwardDlg::onExpandMethodClicked(int nMethodId)
{
    mExpandMethod = nMethodId;
}

// 功能：延拓高度变化（同步成员变量）
void CFreqDomainGradwardDlg::onHeightValueChanged(double dValue)
{
    mHeight = dValue;
}

// 功能：延拓次数变化（重建逐次延拓参数表格默认值，对应原工程 OnChangeTimes）
void CFreqDomainGradwardDlg::onTimesValueChanged(int nValue)
{
    mTimes = nValue;
    rebuildStepTable();
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的检查，并给出更明确的提示）
bool CFreqDomainGradwardDlg::validateInputs(QString& strError)
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
    // 逐次延拓参数表格检查
    if (!readStepTable(strError))
    {
        return false;
    }
    return true;
}

// 功能：组装参数并调用后端 processGradward（对应原工程 OnOK 主体逻辑）
void CFreqDomainGradwardDlg::runProcess()
{
    if (mBackend == NULL)
    {
        QMessageBox::warning(this, QString::fromUtf8("处理失败"), QString::fromUtf8("后端接口未初始化。"));
        return;
    }

    // 组装处理参数（对应原工程界面上的全部输入项）
    GradwardParams params;
    params.inputFilePath = toBackendString(mInputFilePath);
    params.outputFilePath = toBackendString(mOutputFilePath);
    params.exRows = mExRows;
    params.exCols = mExCols;
    params.expandMethod = (ExpandMethod)mExpandMethod;
    params.maxHeight = (float)mHeight;

    // 逐次延拓各步参数来自表格
    for (int i = 0; i < mTimes; i++)
    {
        GradwardStepParams step;
        step.height = (float)mStepValues[i].mHeight;
        step.scaleOfRow = mStepValues[i].mScale;
        params.steps.push_back(step);
    }

    // 同步调用后端处理（处理期间显示等待光标，与原工程 BeginWaitCursor 一致）
    BackendError error;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool bOk = mBackend->processGradward(params, error);
    QApplication::restoreOverrideCursor();

    if (bOk)
    {
        // 处理完成提示（与原工程一致，处理完成后对话框不关闭，由用户点“取消”退出）
        QMessageBox::information(this, QString::fromUtf8("处理完成"),
            QString::fromUtf8("频率域向下延拓处理结束!"));
    }
    else
    {
        QMessageBox::warning(this, QString::fromUtf8("处理失败"), fromBackendString(error.message));
    }
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CFreqDomainGradwardDlg::onOkClicked()
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
void CFreqDomainGradwardDlg::onCancelClicked()
{
    reject();
}

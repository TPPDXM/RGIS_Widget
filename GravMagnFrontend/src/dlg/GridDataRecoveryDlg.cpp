// GridDataRecoveryDlg.cpp : 网格数据空白区还原对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_GridDataRecovery；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）。
//   * 后端接口（processGridDataRecovery）尚未由后端提供：runProcess 当前仅做
//     占位提示，处理调用点以 TODO(后端对接) 预留（与三维重力/磁异常相关成像
//     对话框的处理方式一致）。

#include "GridDataRecoveryDlg.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "FrontendUtils.h"

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CGridDataRecoveryDlg::CGridDataRecoveryDlg(QWidget* pParent)
    : QDialog(pParent)
    , mEditOpenFile(NULL)
    , mBtnOpenFile(NULL)
    , mBtnOpenFileView(NULL)
    , mEditOpenMissingFile(NULL)
    , mBtnOpenMissingFile(NULL)
    , mBtnOpenMissingFileView(NULL)
    , mEditRowsNum(NULL)
    , mEditColsNum(NULL)
    , mEditRowStep(NULL)
    , mEditColStep(NULL)
    , mEditSaveFile(NULL)
    , mBtnSaveFile(NULL)
    , mBtnSaveFileView(NULL)
    , mBtnOk(NULL)
    , mBtnCancel(NULL)
    , mRowsNum(0)
    , mColsNum(0)
    , mColStep(0.0)
    , mRowStep(0.0)
    , mHasDataHead(false)
    , mHasMissingHead(false)
    , mDataRows(0)
    , mDataCols(0)
    , mDataXMin(0.0)
    , mDataXMax(0.0)
    , mDataYMin(0.0)
    , mDataYMax(0.0)
    , mMissingRows(0)
    , mMissingCols(0)
    , mMissingXMin(0.0)
    , mMissingXMax(0.0)
    , mMissingYMin(0.0)
    , mMissingYMax(0.0)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CGridDataRecoveryDlg::~CGridDataRecoveryDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CGridDataRecoveryDlg::initUi()
{
    setWindowTitle(QStringLiteral("网格数据空白区还原"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 还原数据输入 =================
    QGroupBox* pGroupInput = new QGroupBox(QStringLiteral("还原数据输入"), this);
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

    // ================= 带空区的网格数据输入 =================
    QGroupBox* pGroupMissing = new QGroupBox(QStringLiteral("带空区的网格数据输入"), this);
    QGridLayout* pMissingLayout = new QGridLayout(pGroupMissing);
    mEditOpenMissingFile = new QLineEdit(pGroupMissing);
    mBtnOpenMissingFile = new QPushButton(QStringLiteral("..."), pGroupMissing);
    mBtnOpenMissingFile->setFixedWidth(32);
    mBtnOpenMissingFileView = new QPushButton(QStringLiteral("显示"), pGroupMissing);
    mBtnOpenMissingFileView->setFixedWidth(48);
    pMissingLayout->addWidget(mEditOpenMissingFile, 0, 0);
    pMissingLayout->addWidget(mBtnOpenMissingFile, 0, 1);
    pMissingLayout->addWidget(mBtnOpenMissingFileView, 0, 2);
    pMissingLayout->setColumnStretch(0, 1);
    pMainLayout->addWidget(pGroupMissing);

    // ================= 网格数据信息（只读显示，2 行 2 列）=================
    // 对应 .rc 中的 IDC_RowsNum / IDC_ColsNum / IDC_RowStep / IDC_ColStep
    // 说明：原工程 ReadData 中把 nRows 显示到"网格列数"框、nCols 显示到"网格行数"框
    //       （交换笔误），此处按标签语义正确显示（行数→网格行数、列数→网格列数）。
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

    // ================= 还原后的网格数据输出 =================
    // 对应 .rc 中的 IDC_SaveFile / ID_SaveFile / ID_SaveFileView（默认 基准名 + "_Pro.grd"）
    QGroupBox* pGroupOutput = new QGroupBox(QStringLiteral("还原后的网格数据输出"), this);
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

    // 还原数据输入
    connect(mBtnOpenFile, &QPushButton::clicked, this, &CGridDataRecoveryDlg::onOpenFileClicked);
    connect(mBtnOpenFileView, &QPushButton::clicked, this, &CGridDataRecoveryDlg::onOpenFileViewClicked);
    connect(mEditOpenFile, &QLineEdit::textEdited, this, &CGridDataRecoveryDlg::onOpenFileTextEdited);

    // 带空区数据输入
    connect(mBtnOpenMissingFile, &QPushButton::clicked, this, &CGridDataRecoveryDlg::onOpenMissingFileClicked);
    connect(mBtnOpenMissingFileView, &QPushButton::clicked, this, &CGridDataRecoveryDlg::onOpenMissingFileViewClicked);
    connect(mEditOpenMissingFile, &QLineEdit::textEdited, this, &CGridDataRecoveryDlg::onOpenMissingFileTextEdited);

    // 还原结果输出
    connect(mBtnSaveFile, &QPushButton::clicked, this, &CGridDataRecoveryDlg::onSaveFileClicked);
    connect(mBtnSaveFileView, &QPushButton::clicked, this, &CGridDataRecoveryDlg::onSaveFileViewClicked);
    // 输出文件名编辑框（手工输入时同步成员变量，对应原工程 EN_CHANGE→UpdateData）
    connect(mEditSaveFile, &QLineEdit::textEdited, this, &CGridDataRecoveryDlg::onSaveFileTextEdited);

    // 确定 / 取消
    connect(mBtnOk, &QPushButton::clicked, this, &CGridDataRecoveryDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CGridDataRecoveryDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致，见示例图片 SubWindow.png）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：选择还原数据文件（对应原工程 OnOpenFile）
void CGridDataRecoveryDlg::onOpenFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择处理数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadFile(strFilePath);
}

// 功能：手动输入还原数据文件名（同步成员变量，对应原工程 OnChangeOpenFile）
void CGridDataRecoveryDlg::onOpenFileTextEdited(const QString& strText)
{
    mInputFilePath = strText;
}

// 功能：读取还原数据文件头并刷新网格信息、默认输出名（对应原工程 ReadData）
void CGridDataRecoveryDlg::loadFile(const QString& strFilePath)
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

    // 网格信息（与原工程 ReadData 计算一致；
    // 注：原工程把 nRows 显示到"网格列数"框、nCols 显示到"网格行数"框（交换笔误），
    //     此处按标签语义正确显示）
    mColsNum = head.cols;
    mRowsNum = head.rows;
    mColStep = (head.cols > 1) ? (head.xMax - head.xMin) / (head.cols - 1) : 0.0;
    mRowStep = (head.rows > 1) ? (head.yMax - head.yMin) / (head.rows - 1) : 0.0;
    updateGridInfoDisplay();

    // 缓存文件头（供两文件一致性校验，对应原工程 m_obFile1）
    mHasDataHead = true;
    mDataRows = head.rows;
    mDataCols = head.cols;
    mDataXMin = head.xMin;
    mDataXMax = head.xMax;
    mDataYMin = head.yMin;
    mDataYMax = head.yMax;

    // 默认输出文件名（对应原工程 ReadData 中 _Pro.grd 命名规则）
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

    // 若空区文件已加载，则补充做一致性校验
    if (mHasMissingHead)
    {
        checkHeadConsistency();
    }
}

// 功能：选择带空区网格数据文件（对应原工程 OnOpenMissingFile）
void CGridDataRecoveryDlg::onOpenMissingFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择含空区网格数据文件"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadMissingFile(strFilePath);
}

// 功能：手动输入带空区文件名（同步成员变量，对应原工程 OnChangeOpenMissingFile）
void CGridDataRecoveryDlg::onOpenMissingFileTextEdited(const QString& strText)
{
    mMissingFilePath = strText;
}

// 功能：读取带空区文件头并做一致性校验（对应原工程 ReadMissingData）
// 说明：原工程还统计空白区点数（值 < zmin 或 > zmax），需要读数据体，后端实现时校验
//       （提示“输入数据没有空白区!”）。
void CGridDataRecoveryDlg::loadMissingFile(const QString& strFilePath)
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
    if (strFilePath == mLoadedMissingFilePath)
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
    mLoadedMissingFilePath = strFilePath;
    mMissingFilePath = strFilePath;
    if (mEditOpenMissingFile != NULL)
    {
        mEditOpenMissingFile->setText(strFilePath);
    }

    // 缓存文件头（供两文件一致性校验，对应原工程 m_obFile2）
    mHasMissingHead = true;
    mMissingRows = head.rows;
    mMissingCols = head.cols;
    mMissingXMin = head.xMin;
    mMissingXMax = head.xMax;
    mMissingYMin = head.yMin;
    mMissingYMax = head.yMax;

    // 一致性校验（还原数据已加载时执行，对应原工程 ReadMissingData 中的行列/坐标比较）
    if (mHasDataHead)
    {
        checkHeadConsistency();
    }
}

// 功能：两个输入文件头一致性校验（对应原工程 ReadMissingData 末尾的比较逻辑）
void CGridDataRecoveryDlg::checkHeadConsistency()
{
    // 行列数不一致（提示语与原工程一致）
    if (mDataRows != mMissingRows || mDataCols != mMissingCols)
    {
        QMessageBox::warning(this, QStringLiteral("数据不一致"),
            QStringLiteral("您读入的两个用于空区还原的网格数据文件行列数不一致!"));
        return;
    }
    // 坐标位置不一致（提示语与原工程一致）
    if (mDataXMax != mMissingXMax || mDataYMax != mMissingYMax ||
        mDataXMin != mMissingXMin || mDataYMin != mMissingYMin)
    {
        QMessageBox::warning(this, QStringLiteral("数据不一致"),
            QStringLiteral("您读入的两个用于空区还原的网格数据文件坐标位置不一致!"));
        return;
    }
}

// 功能：把网格信息成员变量刷到界面显示
void CGridDataRecoveryDlg::updateGridInfoDisplay()
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

// 功能：选择还原结果数据文件（对应原工程 OnSaveFile）
void CGridDataRecoveryDlg::onSaveFileClicked()
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
void CGridDataRecoveryDlg::onSaveFileTextEdited(const QString& strText)
{
    mOutputFilePath = strText;
}

// 功能：弹出打开文件对话框（.grd 网格数据文件）
QString CGridDataRecoveryDlg::askOpenFilePath(const QString& strTitle)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(),
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：弹出保存文件对话框（.grd 网格数据文件）
QString CGridDataRecoveryDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath,
        QStringLiteral("网格数据文件 (*.grd);;所有数据文件 (*.*)"));
}

// 功能：检查文件存在并发出发送显示请求（对应原工程 OnOpenFileView / OnSaveFileView / OnOpenMissingFileView）
void CGridDataRecoveryDlg::viewGridFile(const QString& strFilePath)
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

// 功能：显示还原数据文件等值线
void CGridDataRecoveryDlg::onOpenFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示带空区网格数据文件等值线
void CGridDataRecoveryDlg::onOpenMissingFileViewClicked()
{
    viewGridFile(mMissingFilePath);
}

// 功能：显示还原结果文件等值线
void CGridDataRecoveryDlg::onSaveFileViewClicked()
{
    viewGridFile(mOutputFilePath);
}

// 功能：确定前校验输入项（对应原工程 OnOK 中的文件打开检查，并给出更明确的提示）
bool CGridDataRecoveryDlg::validateInputs(QString& strError)
{
    // 还原数据文件检查
    if (mInputFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择还原数据文件。");
        return false;
    }
    if (!QFile::exists(mInputFilePath))
    {
        strError = QStringLiteral("还原数据文件不存在：") + mInputFilePath;
        return false;
    }
    // 带空区文件检查
    if (mMissingFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择带空区的网格数据文件。");
        return false;
    }
    if (!QFile::exists(mMissingFilePath))
    {
        strError = QStringLiteral("带空区网格数据文件不存在：") + mMissingFilePath;
        return false;
    }
    // 输出文件检查
    if (mOutputFilePath.isEmpty())
    {
        strError = QStringLiteral("请输入还原后的网格数据文件路径。");
        return false;
    }
    return true;
}

// 功能：调用后端 processGridDataRecovery（对应原工程 OnOK 主体逻辑）
// 说明：后端接口（processGridDataRecovery）尚未由后端提供，本函数当前仅做参数校验与占位提示；
//       后端提供接口后，在下方注释位置接入：
//       IRgisBackend* pBackend = CBackendService::rgisBackend();
//       组装 GridDataRecoveryParams（还原数据文件、带空区文件、输出文件），
//       调用 pBackend->processGridDataRecovery(params, error) 完成处理，
//       完成后提示“空白区还原结束!”（与原工程一致，处理完成后对话框不关闭）。
//       后端处理约定（与原工程 OnOK 一致）：
//         1. 校验两文件行列数、坐标范围一致（不一致报错）；
//         2. 校验空区数据存在空白区（值 < zmin 或 > zmax，无则报错“输入数据没有空白区!”）；
//         3. 结果 = 还原数据逐点拷贝，空区点（值超出空区文件 zmin/zmax 范围者）
//            用空区数据对应值替换（原工程 IsValidData 逻辑）；
//         4. 按输出文件写出结果（DSBB 格式），完成提示“空白区还原结束!”。
void CGridDataRecoveryDlg::runProcess()
{
    // TODO(后端对接)：后端接口 processGridDataRecovery 提供后在此接入（见上方说明）。
    // 当前：前端界面已完成，后端未接入，给出占位提示。
    QMessageBox::warning(this, QStringLiteral("处理失败"),
        QStringLiteral("算法后端尚未接入：网格数据空白区还原功能等待后端实现。"));
}

// 功能：“确定”——校验参数并调用后端处理（对应原工程 OnOK）
void CGridDataRecoveryDlg::onOkClicked()
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
void CGridDataRecoveryDlg::onCancelClicked()
{
    reject();
}

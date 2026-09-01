// GridFileNameDlg.cpp : 网格文件名对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_DLG_INVERSION_PARAMS；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）；
//   * 后端算法接口（processGravMagnVolumeInv）由后端提供后接入，"开始反演"当前为占位提示。

#include "GridFileNameDlg.h"

#include <QApplication>
#include <QCheckBox>
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
#include <QVBoxLayout>

#include "FrontendUtils.h"
#include "GravMagnVolumeInvDlg.h"

// 构造函数：初始化全部成员并创建界面
// 参数：strFileNames —— 候选数据文件列表（分号分隔，可为空）
// 参数：pParent —— 父窗口（可为 NULL）
CGridFileNameDlg::CGridFileNameDlg(const QString& strFileNames, QWidget* pParent)
    : QDialog(pParent)
    , mComboOriginalFile(NULL)
    , mBtnOpenOriginalFile(NULL)
    , mBtnOpenOriginalFileView(NULL)
    , mLabelSurfaceFile(NULL)
    , mComboSurfaceFile(NULL)
    , mBtnOpenSurfaceFile(NULL)
    , mBtnOpenSurfaceFileView(NULL)
    , mEditRowsNum(NULL)
    , mEditColsNum(NULL)
    , mEditXMin(NULL)
    , mEditXMax(NULL)
    , mEditYMin(NULL)
    , mEditYMax(NULL)
    , mCheckGravityData(NULL)
    , mCheckKm(NULL)
    , mLabelMagInc(NULL)
    , mSpinInclination(NULL)
    , mLabelMagDec(NULL)
    , mSpinDeclination(NULL)
    , mLabelProfile(NULL)
    , mSpinProfileDec(NULL)
    , mLabelBaseline(NULL)
    , mSpinBaselineDec(NULL)
    , mCheckInitModels(NULL)
    , mEditInitModels(NULL)
    , mBtnOpenInitModels(NULL)
    , mBtnOpenInitModelsView(NULL)
    , mEditCommandFile(NULL)
    , mBtnOpenCommandFile(NULL)
    , mEditModXMin(NULL)
    , mEditModXMax(NULL)
    , mEditModYMin(NULL)
    , mEditModYMax(NULL)
    , mSpinModZMin(NULL)
    , mSpinModZMax(NULL)
    , mEditModXStep(NULL)
    , mEditModYStep(NULL)
    , mSpinModZStep(NULL)
    , mEditForwardFieldsFile(NULL)
    , mBtnOpenModelsFile(NULL)
    , mBtnOpenModelsFileView(NULL)
    , mEditModelPropertiesFile(NULL)
    , mBtnOpenModelsPropertiesFile(NULL)
    , mBtnOpenModelsPropertiesFileView(NULL)
    , mEditInvertInfoFile(NULL)
    , mBtnSaveInvertInfoFile(NULL)
    , mBtnOk(NULL)
    , mBtnCancel(NULL)
    , mRowsNum(0)
    , mColsNum(0)
    , mDataXMin(0.0)
    , mDataXMax(0.0)
    , mDataYMin(0.0)
    , mDataYMax(0.0)
    , mModXMin(0.0)
    , mModXMax(0.0)
    , mModYMin(0.0)
    , mModYMax(0.0)
    , mModZMin(0.0)
    , mModZMax(0.0)
    , mModXStep(0.0)
    , mModYStep(0.0)
    , mModZStep(0.0)
    , mGravityData(true)
    , mKm(false)
    , mInclination(90.0)
    , mDeclination(0.0)
    , mProfileDec(90.0)
    , mBaselineDec(0.0)
    , mPrevModelsAsInit(false)
    , mProc(NULL)
{
    // 候选文件列表加入两个文件下拉框（分号分隔，与原工程 AddFilesToComboBox 一致）
    QStringList fileList = strFileNames.split(';', Qt::SkipEmptyParts);
    for (int i = 0; i < fileList.size(); i++)
    {
        QString strFile = fileList.at(i).trimmed();
        if (strFile.isEmpty())
        {
            continue;
        }
        mCandidateFiles.append(strFile);
    }

    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CGridFileNameDlg::~CGridFileNameDlg()
{
}

// 功能：设置反演处理流程类指针（"开始反演"时回调其 doInvsProcess）
void CGridFileNameDlg::setGravMagnVolumeInvProc(CGravMagnVolumeInvProc* pProc)
{
    mProc = pProc;
}

// ===== 默认参数设置（模态打开前由流程类调用）=====

void CGridFileNameDlg::setGravityData(bool bGravity)
{
    mGravityData = bGravity;
    if (mCheckGravityData != NULL)
    {
        mCheckGravityData->setChecked(mGravityData);
    }
    updateVisibilities();
}

void CGridFileNameDlg::setKm(bool bKm)
{
    mKm = bKm;
    if (mCheckKm != NULL)
    {
        mCheckKm->setChecked(mKm);
    }
}

void CGridFileNameDlg::setInclination(double dValue)
{
    mInclination = dValue;
    if (mSpinInclination != NULL)
    {
        mSpinInclination->setValue(dValue);
    }
}

void CGridFileNameDlg::setDeclination(double dValue)
{
    mDeclination = dValue;
    if (mSpinDeclination != NULL)
    {
        mSpinDeclination->setValue(dValue);
    }
}

void CGridFileNameDlg::setProfileDec(double dValue)
{
    mProfileDec = dValue;
    if (mSpinProfileDec != NULL)
    {
        mSpinProfileDec->setValue(dValue);
    }
}

void CGridFileNameDlg::setBaselineDec(double dValue)
{
    mBaselineDec = dValue;
    if (mSpinBaselineDec != NULL)
    {
        mSpinBaselineDec->setValue(dValue);
    }
}

void CGridFileNameDlg::setPrevModelsAsInit(bool bUsed)
{
    mPrevModelsAsInit = bUsed;
    if (mCheckInitModels != NULL)
    {
        mCheckInitModels->setChecked(mPrevModelsAsInit);
    }
    updateVisibilities();
}

// ===== 参数读取 =====

bool CGridFileNameDlg::gravityData() const    { return mGravityData; }
bool CGridFileNameDlg::km() const             { return mKm; }
double CGridFileNameDlg::inclination() const  { return mInclination; }
double CGridFileNameDlg::declination() const  { return mDeclination; }
double CGridFileNameDlg::profileDec() const   { return mProfileDec; }
double CGridFileNameDlg::baselineDec() const  { return mBaselineDec; }
bool CGridFileNameDlg::prevModelsAsInit() const { return mPrevModelsAsInit; }
QString CGridFileNameDlg::originalFieldsFilePath() const { return mInputFilePath; }
QString CGridFileNameDlg::originalSurfaceFilePath() const { return mSurfaceFilePath; }
QString CGridFileNameDlg::commandFilePath() const { return mCommandFilePath; }
QString CGridFileNameDlg::forwardFieldsFilePath() const { return mForwardFieldsFilePath; }
QString CGridFileNameDlg::modelPropertiesFilePath() const { return mModelPropertiesFilePath; }
QString CGridFileNameDlg::invertInfoFilePath() const { return mInvertInfoFilePath; }
QString CGridFileNameDlg::prevModelsFilePath() const { return mPrevModelsFilePath; }
double CGridFileNameDlg::modXmin() const { return mModXMin; }
double CGridFileNameDlg::modXmax() const { return mModXMax; }
double CGridFileNameDlg::modYmin() const { return mModYMin; }
double CGridFileNameDlg::modYmax() const { return mModYMax; }
double CGridFileNameDlg::modZmin() const { return mModZMin; }
double CGridFileNameDlg::modZmax() const { return mModZMax; }
double CGridFileNameDlg::modXstep() const { return mModXStep; }
double CGridFileNameDlg::modYstep() const { return mModYStep; }
double CGridFileNameDlg::modZstep() const { return mModZStep; }

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CGridFileNameDlg::initUi()
{
    setWindowTitle(QStringLiteral("三维重磁异常自动反演参数设置"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 场值和地形网格数据输入 =================
    QGroupBox* pGroupInput = new QGroupBox(QStringLiteral("场值和地形网格数据输入"), this);
    QGridLayout* pInputLayout = new QGridLayout(pGroupInput);
    pInputLayout->addWidget(new QLabel(QStringLiteral("场值数据文件"), pGroupInput), 0, 0);
    mComboOriginalFile = new QComboBox(pGroupInput);
    mComboOriginalFile->setEditable(true);
    mComboOriginalFile->setInsertPolicy(QComboBox::NoInsert);
    mComboOriginalFile->addItems(mCandidateFiles);
    mBtnOpenOriginalFile = new QPushButton(QStringLiteral("..."), pGroupInput);
    mBtnOpenOriginalFile->setFixedWidth(32);
    mBtnOpenOriginalFileView = new QPushButton(QStringLiteral("显示"), pGroupInput);
    mBtnOpenOriginalFileView->setFixedWidth(48);
    pInputLayout->addWidget(mComboOriginalFile, 0, 1);
    pInputLayout->addWidget(mBtnOpenOriginalFile, 0, 2);
    pInputLayout->addWidget(mBtnOpenOriginalFileView, 0, 3);
    mLabelSurfaceFile = new QLabel(QStringLiteral("地形数据文件"), pGroupInput);
    pInputLayout->addWidget(mLabelSurfaceFile, 1, 0);
    mComboSurfaceFile = new QComboBox(pGroupInput);
    mComboSurfaceFile->setEditable(true);
    mComboSurfaceFile->setInsertPolicy(QComboBox::NoInsert);
    mComboSurfaceFile->addItems(mCandidateFiles);
    mBtnOpenSurfaceFile = new QPushButton(QStringLiteral("..."), pGroupInput);
    mBtnOpenSurfaceFile->setFixedWidth(32);
    mBtnOpenSurfaceFileView = new QPushButton(QStringLiteral("显示"), pGroupInput);
    mBtnOpenSurfaceFileView->setFixedWidth(48);
    pInputLayout->addWidget(mComboSurfaceFile, 1, 1);
    pInputLayout->addWidget(mBtnOpenSurfaceFile, 1, 2);
    pInputLayout->addWidget(mBtnOpenSurfaceFileView, 1, 3);
    pInputLayout->setColumnStretch(1, 1);
    pMainLayout->addWidget(pGroupInput);

    // ================= 网格数据信息（只读显示）=================
    // 对应 .rc 中的 IDC_NumRows / IDC_NumCols / IDC_X_MIN / IDC_X_MAX / IDC_Y_MIN / IDC_Y_MAX
    QGroupBox* pGroupInfo = new QGroupBox(QStringLiteral("网格数据信息"), this);
    QGridLayout* pInfoLayout = new QGridLayout(pGroupInfo);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格数据行数"), pGroupInfo), 0, 0);
    mEditRowsNum = new QLineEdit(pGroupInfo);
    mEditRowsNum->setReadOnly(true);
    mEditRowsNum->setAlignment(Qt::AlignCenter);
    mEditRowsNum->setFixedWidth(90);
    pInfoLayout->addWidget(mEditRowsNum, 0, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("X 坐标最小值"), pGroupInfo), 0, 2);
    mEditXMin = new QLineEdit(pGroupInfo);
    mEditXMin->setReadOnly(true);
    mEditXMin->setAlignment(Qt::AlignCenter);
    mEditXMin->setFixedWidth(100);
    pInfoLayout->addWidget(mEditXMin, 0, 3);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("X 坐标最大值"), pGroupInfo), 0, 4);
    mEditXMax = new QLineEdit(pGroupInfo);
    mEditXMax->setReadOnly(true);
    mEditXMax->setAlignment(Qt::AlignCenter);
    mEditXMax->setFixedWidth(100);
    pInfoLayout->addWidget(mEditXMax, 0, 5);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("网格数据列数"), pGroupInfo), 1, 0);
    mEditColsNum = new QLineEdit(pGroupInfo);
    mEditColsNum->setReadOnly(true);
    mEditColsNum->setAlignment(Qt::AlignCenter);
    mEditColsNum->setFixedWidth(90);
    pInfoLayout->addWidget(mEditColsNum, 1, 1);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("Y 坐标最小值"), pGroupInfo), 1, 2);
    mEditYMin = new QLineEdit(pGroupInfo);
    mEditYMin->setReadOnly(true);
    mEditYMin->setAlignment(Qt::AlignCenter);
    mEditYMin->setFixedWidth(100);
    pInfoLayout->addWidget(mEditYMin, 1, 3);
    pInfoLayout->addWidget(new QLabel(QStringLiteral("Y 坐标最大值"), pGroupInfo), 1, 4);
    mEditYMax = new QLineEdit(pGroupInfo);
    mEditYMax->setReadOnly(true);
    mEditYMax->setAlignment(Qt::AlignCenter);
    mEditYMax->setFixedWidth(100);
    pInfoLayout->addWidget(mEditYMax, 1, 5);
    pMainLayout->addWidget(pGroupInfo);

    // ================= 场值类型选择和参数输入 =================
    QGroupBox* pGroupType = new QGroupBox(QStringLiteral("场值类型选择和参数输入"), this);
    QGridLayout* pTypeLayout = new QGridLayout(pGroupType);
    mCheckGravityData = new QCheckBox(QStringLiteral("  重力数据"), pGroupType);
    pTypeLayout->addWidget(mCheckGravityData, 0, 0);
    mCheckKm = new QCheckBox(QStringLiteral("  公里"), pGroupType);
    pTypeLayout->addWidget(mCheckKm, 1, 0);
    mLabelMagInc = new QLabel(QStringLiteral("磁化倾角"), pGroupType);
    pTypeLayout->addWidget(mLabelMagInc, 0, 1);
    mSpinInclination = new QDoubleSpinBox(pGroupType);
    mSpinInclination->setRange(-360.0, 360.0);
    mSpinInclination->setDecimals(3);
    mSpinInclination->setSingleStep(1.0);
    mSpinInclination->setValue(mInclination);
    mSpinInclination->setAlignment(Qt::AlignCenter);
    mSpinInclination->setFixedWidth(100);
    pTypeLayout->addWidget(mSpinInclination, 0, 2);
    mLabelProfile = new QLabel(QStringLiteral("测线方位角"), pGroupType);
    pTypeLayout->addWidget(mLabelProfile, 0, 3);
    mSpinProfileDec = new QDoubleSpinBox(pGroupType);
    mSpinProfileDec->setRange(-360.0, 360.0);
    mSpinProfileDec->setDecimals(3);
    mSpinProfileDec->setSingleStep(1.0);
    mSpinProfileDec->setValue(mProfileDec);
    mSpinProfileDec->setAlignment(Qt::AlignCenter);
    mSpinProfileDec->setFixedWidth(100);
    pTypeLayout->addWidget(mSpinProfileDec, 0, 4);
    mLabelMagDec = new QLabel(QStringLiteral("磁化偏角"), pGroupType);
    pTypeLayout->addWidget(mLabelMagDec, 1, 1);
    mSpinDeclination = new QDoubleSpinBox(pGroupType);
    mSpinDeclination->setRange(-360.0, 360.0);
    mSpinDeclination->setDecimals(3);
    mSpinDeclination->setSingleStep(1.0);
    mSpinDeclination->setValue(mDeclination);
    mSpinDeclination->setAlignment(Qt::AlignCenter);
    mSpinDeclination->setFixedWidth(100);
    pTypeLayout->addWidget(mSpinDeclination, 1, 2);
    mLabelBaseline = new QLabel(QStringLiteral("基线方位角"), pGroupType);
    pTypeLayout->addWidget(mLabelBaseline, 1, 3);
    mSpinBaselineDec = new QDoubleSpinBox(pGroupType);
    mSpinBaselineDec->setRange(-360.0, 360.0);
    mSpinBaselineDec->setDecimals(3);
    mSpinBaselineDec->setSingleStep(1.0);
    mSpinBaselineDec->setValue(mBaselineDec);
    mSpinBaselineDec->setAlignment(Qt::AlignCenter);
    mSpinBaselineDec->setFixedWidth(100);
    pTypeLayout->addWidget(mSpinBaselineDec, 1, 4);
    pTypeLayout->setColumnStretch(2, 1);
    pTypeLayout->setColumnStretch(4, 1);
    pMainLayout->addWidget(pGroupType);

    // ================= 初始模型和反演控制参数文件输入 =================
    QGroupBox* pGroupControl = new QGroupBox(QStringLiteral("初始模型和反演控制参数文件输入"), this);
    QGridLayout* pControlLayout = new QGridLayout(pGroupControl);
    mCheckInitModels = new QCheckBox(QStringLiteral("  初始模型"), pGroupControl);
    pControlLayout->addWidget(mCheckInitModels, 0, 0);
    mEditInitModels = new QLineEdit(pGroupControl);
    mBtnOpenInitModels = new QPushButton(QStringLiteral("..."), pGroupControl);
    mBtnOpenInitModels->setFixedWidth(32);
    mBtnOpenInitModelsView = new QPushButton(QStringLiteral("显示"), pGroupControl);
    mBtnOpenInitModelsView->setFixedWidth(48);
    pControlLayout->addWidget(mEditInitModels, 0, 1);
    pControlLayout->addWidget(mBtnOpenInitModels, 0, 2);
    pControlLayout->addWidget(mBtnOpenInitModelsView, 0, 3);
    pControlLayout->addWidget(new QLabel(QStringLiteral("控制参数文件"), pGroupControl), 1, 0);
    mEditCommandFile = new QLineEdit(pGroupControl);
    mBtnOpenCommandFile = new QPushButton(QStringLiteral("..."), pGroupControl);
    mBtnOpenCommandFile->setFixedWidth(32);
    pControlLayout->addWidget(mEditCommandFile, 1, 1);
    pControlLayout->addWidget(mBtnOpenCommandFile, 1, 2);
    pControlLayout->setColumnStretch(1, 1);
    pMainLayout->addWidget(pGroupControl);

    // ================= 网格剖分信息 =================
    QGroupBox* pGroupMesh = new QGroupBox(QStringLiteral("网格剖分信息"), this);
    QGridLayout* pMeshLayout = new QGridLayout(pGroupMesh);
    // 第一行：X 最小 / X 最大 / X 方向网格距（X 项由场值文件自动生成，只读）
    pMeshLayout->addWidget(new QLabel(QStringLiteral("X 坐标最小值"), pGroupMesh), 0, 0);
    mEditModXMin = new QLineEdit(pGroupMesh);
    mEditModXMin->setReadOnly(true);
    mEditModXMin->setAlignment(Qt::AlignCenter);
    mEditModXMin->setFixedWidth(100);
    pMeshLayout->addWidget(mEditModXMin, 0, 1);
    pMeshLayout->addWidget(new QLabel(QStringLiteral("X 坐标最大值"), pGroupMesh), 0, 2);
    mEditModXMax = new QLineEdit(pGroupMesh);
    mEditModXMax->setReadOnly(true);
    mEditModXMax->setAlignment(Qt::AlignCenter);
    mEditModXMax->setFixedWidth(100);
    pMeshLayout->addWidget(mEditModXMax, 0, 3);
    pMeshLayout->addWidget(new QLabel(QStringLiteral("X  方向网格距"), pGroupMesh), 0, 4);
    mEditModXStep = new QLineEdit(pGroupMesh);
    mEditModXStep->setReadOnly(true);
    mEditModXStep->setAlignment(Qt::AlignCenter);
    mEditModXStep->setFixedWidth(100);
    pMeshLayout->addWidget(mEditModXStep, 0, 5);
    // 第二行：Y 最小 / Y 最大 / Y 方向网格距（Y 项由场值文件自动生成，只读）
    pMeshLayout->addWidget(new QLabel(QStringLiteral("Y 坐标最小值"), pGroupMesh), 1, 0);
    mEditModYMin = new QLineEdit(pGroupMesh);
    mEditModYMin->setReadOnly(true);
    mEditModYMin->setAlignment(Qt::AlignCenter);
    mEditModYMin->setFixedWidth(100);
    pMeshLayout->addWidget(mEditModYMin, 1, 1);
    pMeshLayout->addWidget(new QLabel(QStringLiteral("Y 坐标最大值"), pGroupMesh), 1, 2);
    mEditModYMax = new QLineEdit(pGroupMesh);
    mEditModYMax->setReadOnly(true);
    mEditModYMax->setAlignment(Qt::AlignCenter);
    mEditModYMax->setFixedWidth(100);
    pMeshLayout->addWidget(mEditModYMax, 1, 3);
    pMeshLayout->addWidget(new QLabel(QStringLiteral("Y  方向网格距"), pGroupMesh), 1, 4);
    mEditModYStep = new QLineEdit(pGroupMesh);
    mEditModYStep->setReadOnly(true);
    mEditModYStep->setAlignment(Qt::AlignCenter);
    mEditModYStep->setFixedWidth(100);
    pMeshLayout->addWidget(mEditModYStep, 1, 5);
    // 第三行：Z 最小 / Z 最大 / Z 方向网格距（Z 项可编辑，默认由场值文件生成）
    pMeshLayout->addWidget(new QLabel(QStringLiteral("Z 坐标最小值"), pGroupMesh), 2, 0);
    mSpinModZMin = new QDoubleSpinBox(pGroupMesh);
    mSpinModZMin->setRange(-9999999.0, 9999999.0);
    mSpinModZMin->setDecimals(3);
    mSpinModZMin->setSingleStep(1.0);
    mSpinModZMin->setValue(mModZMin);
    mSpinModZMin->setAlignment(Qt::AlignCenter);
    mSpinModZMin->setFixedWidth(100);
    pMeshLayout->addWidget(mSpinModZMin, 2, 1);
    pMeshLayout->addWidget(new QLabel(QStringLiteral("Z 坐标最大值"), pGroupMesh), 2, 2);
    mSpinModZMax = new QDoubleSpinBox(pGroupMesh);
    mSpinModZMax->setRange(-9999999.0, 9999999.0);
    mSpinModZMax->setDecimals(3);
    mSpinModZMax->setSingleStep(1.0);
    mSpinModZMax->setValue(mModZMax);
    mSpinModZMax->setAlignment(Qt::AlignCenter);
    mSpinModZMax->setFixedWidth(100);
    pMeshLayout->addWidget(mSpinModZMax, 2, 3);
    pMeshLayout->addWidget(new QLabel(QStringLiteral("Z  方向网格距"), pGroupMesh), 2, 4);
    mSpinModZStep = new QDoubleSpinBox(pGroupMesh);
    mSpinModZStep->setRange(0.000001, 9999999.0);
    mSpinModZStep->setDecimals(3);
    mSpinModZStep->setSingleStep(1.0);
    mSpinModZStep->setValue(mModZStep);
    mSpinModZStep->setAlignment(Qt::AlignCenter);
    mSpinModZStep->setFixedWidth(100);
    pMeshLayout->addWidget(mSpinModZStep, 2, 5);
    pMainLayout->addWidget(pGroupMesh);

    // ================= 反演结果输出 =================
    QGroupBox* pGroupOutput = new QGroupBox(QStringLiteral("反演结果输出"), this);
    QGridLayout* pOutputLayout = new QGridLayout(pGroupOutput);
    pOutputLayout->addWidget(new QLabel(QStringLiteral("正演场值文件"), pGroupOutput), 0, 0);
    mEditForwardFieldsFile = new QLineEdit(pGroupOutput);
    mBtnOpenModelsFile = new QPushButton(QStringLiteral("..."), pGroupOutput);
    mBtnOpenModelsFile->setFixedWidth(32);
    mBtnOpenModelsFileView = new QPushButton(QStringLiteral("显示"), pGroupOutput);
    mBtnOpenModelsFileView->setFixedWidth(48);
    pOutputLayout->addWidget(mEditForwardFieldsFile, 0, 1);
    pOutputLayout->addWidget(mBtnOpenModelsFile, 0, 2);
    pOutputLayout->addWidget(mBtnOpenModelsFileView, 0, 3);
    pOutputLayout->addWidget(new QLabel(QStringLiteral("模型数据文件"), pGroupOutput), 1, 0);
    mEditModelPropertiesFile = new QLineEdit(pGroupOutput);
    mBtnOpenModelsPropertiesFile = new QPushButton(QStringLiteral("..."), pGroupOutput);
    mBtnOpenModelsPropertiesFile->setFixedWidth(32);
    mBtnOpenModelsPropertiesFileView = new QPushButton(QStringLiteral("显示"), pGroupOutput);
    mBtnOpenModelsPropertiesFileView->setFixedWidth(48);
    pOutputLayout->addWidget(mEditModelPropertiesFile, 1, 1);
    pOutputLayout->addWidget(mBtnOpenModelsPropertiesFile, 1, 2);
    pOutputLayout->addWidget(mBtnOpenModelsPropertiesFileView, 1, 3);
    pOutputLayout->addWidget(new QLabel(QStringLiteral("反演信息文件"), pGroupOutput), 2, 0);
    mEditInvertInfoFile = new QLineEdit(pGroupOutput);
    mBtnSaveInvertInfoFile = new QPushButton(QStringLiteral("..."), pGroupOutput);
    mBtnSaveInvertInfoFile->setFixedWidth(32);
    pOutputLayout->addWidget(mEditInvertInfoFile, 2, 1);
    pOutputLayout->addWidget(mBtnSaveInvertInfoFile, 2, 2);
    pOutputLayout->setColumnStretch(1, 1);
    pMainLayout->addWidget(pGroupOutput);

    // ================= 开始反演 / 退出反演 =================
    QHBoxLayout* pButtonLayout = new QHBoxLayout();
    mBtnOk = new QPushButton(QStringLiteral("开始反演"), this);
    mBtnCancel = new QPushButton(QStringLiteral("退出反演"), this);
    mBtnOk->setDefault(true);
    pButtonLayout->addWidget(mBtnOk);
    pButtonLayout->addStretch(1);
    pButtonLayout->addWidget(mBtnCancel);
    pMainLayout->addLayout(pButtonLayout);

    // ================= 信号槽连接（全部使用成员函数引用，不使用 lambda）=================
    // 分组框标题居中（对应原工程 GROUPBOX 的 BS_CENTER 样式）
    for (QGroupBox* pGroup : findChildren<QGroupBox*>())
    {
        pGroup->setAlignment(Qt::AlignCenter);
    }

    // 场值 / 地形文件
    connect(mBtnOpenOriginalFile, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenOriginalFileClicked);
    connect(mBtnOpenOriginalFileView, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenOriginalFileViewClicked);
    connect(mComboOriginalFile, QOverload<int>::of(&QComboBox::activated), this, &CGridFileNameDlg::onOriginalComboActivated);
    connect(mBtnOpenSurfaceFile, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenSurfaceFileClicked);
    connect(mBtnOpenSurfaceFileView, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenSurfaceFileViewClicked);
    connect(mComboSurfaceFile, QOverload<int>::of(&QComboBox::activated), this, &CGridFileNameDlg::onSurfaceComboActivated);

    // 控制参数 / 初始模型
    connect(mBtnOpenCommandFile, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenCommandFileClicked);
    connect(mCheckInitModels, &QCheckBox::toggled, this, &CGridFileNameDlg::onInitModelsToggled);
    connect(mBtnOpenInitModels, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenInitModelsClicked);
    connect(mBtnOpenInitModelsView, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenInitModelsViewClicked);

    // 场值类型 / 参数
    connect(mCheckGravityData, &QCheckBox::toggled, this, &CGridFileNameDlg::onGravityDataToggled);
    connect(mCheckKm, &QCheckBox::toggled, this, &CGridFileNameDlg::onKmToggled);
    connect(mSpinInclination, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGridFileNameDlg::onInclinationChanged);
    connect(mSpinDeclination, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGridFileNameDlg::onDeclinationChanged);
    connect(mSpinProfileDec, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGridFileNameDlg::onProfileDecChanged);
    connect(mSpinBaselineDec, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGridFileNameDlg::onBaselineDecChanged);

    // 网格剖分（Z 项可编辑）
    connect(mSpinModZMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGridFileNameDlg::onModZMinChanged);
    connect(mSpinModZMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGridFileNameDlg::onModZMaxChanged);
    connect(mSpinModZStep, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CGridFileNameDlg::onModZStepChanged);

    // 反演结果输出
    connect(mBtnOpenModelsFile, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenModelsFileClicked);
    connect(mBtnOpenModelsFileView, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenModelsFileViewClicked);
    connect(mBtnOpenModelsPropertiesFile, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenModelsPropertiesFileClicked);
    connect(mBtnOpenModelsPropertiesFileView, &QPushButton::clicked, this, &CGridFileNameDlg::onOpenModelsPropertiesFileViewClicked);
    connect(mBtnSaveInvertInfoFile, &QPushButton::clicked, this, &CGridFileNameDlg::onSaveInvertInfoFileClicked);

    // 开始 / 退出
    connect(mBtnOk, &QPushButton::clicked, this, &CGridFileNameDlg::onOkClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CGridFileNameDlg::onCancelClicked);

    // 初始复选框状态与成员初值一致（重力数据默认勾选；初始模型默认不勾选）
    if (mCheckGravityData != NULL)
    {
        mCheckGravityData->setChecked(mGravityData);
    }
    if (mCheckKm != NULL)
    {
        mCheckKm->setChecked(mKm);
    }
    if (mCheckInitModels != NULL)
    {
        mCheckInitModels->setChecked(mPrevModelsAsInit);
    }

    // 初始显隐状态（与原工程 OnInitDialog 一致：
    //   重力数据默认勾选 → 公里显示、磁参隐藏；初始模型行隐藏；地形行隐藏；开始反演隐藏）
    updateVisibilities();
    updateStartButtonVisibility();

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：按当前选项刷新各分组的显隐状态（对应原工程 OnCheckGravData / OnCheckInitModels 的显隐逻辑）
void CGridFileNameDlg::updateVisibilities()
{
    // 重力数据勾选时：显示"公里"、隐藏磁参（倾角/偏角/测线方位角/基线方位角）；否则相反
    bool bGravity = mCheckGravityData != NULL ? mCheckGravityData->isChecked() : mGravityData;
    if (mCheckKm != NULL)
    {
        mCheckKm->setVisible(bGravity);
    }
    if (mLabelMagInc != NULL) { mLabelMagInc->setVisible(!bGravity); }
    if (mSpinInclination != NULL) { mSpinInclination->setVisible(!bGravity); }
    if (mLabelMagDec != NULL) { mLabelMagDec->setVisible(!bGravity); }
    if (mSpinDeclination != NULL) { mSpinDeclination->setVisible(!bGravity); }
    if (mLabelProfile != NULL) { mLabelProfile->setVisible(!bGravity); }
    if (mSpinProfileDec != NULL) { mSpinProfileDec->setVisible(!bGravity); }
    if (mLabelBaseline != NULL) { mLabelBaseline->setVisible(!bGravity); }
    if (mSpinBaselineDec != NULL) { mSpinBaselineDec->setVisible(!bGravity); }

    // 初始模型勾选时显示：初始模型文件输入行（编辑框 + "..." + "显示"）
    bool bInit = mCheckInitModels != NULL ? mCheckInitModels->isChecked() : mPrevModelsAsInit;
    if (mEditInitModels != NULL)
    {
        mEditInitModels->setVisible(bInit);
    }
    if (mBtnOpenInitModels != NULL)
    {
        mBtnOpenInitModels->setVisible(bInit);
    }
    if (mBtnOpenInitModelsView != NULL)
    {
        mBtnOpenInitModelsView->setVisible(bInit);
    }

    // 地形数据文件行：选完场值数据文件后显示（与原工程 m_strOriginalFieldsFileName 非空判断一致）
    bool bFieldLoaded = !mInputFilePath.isEmpty();
    if (mLabelSurfaceFile != NULL)
    {
        mLabelSurfaceFile->setVisible(bFieldLoaded);
    }
    if (mComboSurfaceFile != NULL)
    {
        mComboSurfaceFile->setVisible(bFieldLoaded);
    }
    if (mBtnOpenSurfaceFile != NULL)
    {
        mBtnOpenSurfaceFile->setVisible(bFieldLoaded);
    }
    if (mBtnOpenSurfaceFileView != NULL)
    {
        mBtnOpenSurfaceFileView->setVisible(bFieldLoaded);
    }
}

// 功能：按 场值/地形/控制参数文件 是否齐全刷新"开始反演"显隐（与原工程 IDOK 显隐条件一致）
void CGridFileNameDlg::updateStartButtonVisibility()
{
    if (mBtnOk == NULL)
    {
        return;
    }
    bool bReady = !mCommandFilePath.isEmpty()
        && !mInputFilePath.isEmpty()
        && !mSurfaceFilePath.isEmpty();
    mBtnOk->setVisible(bReady);
    // 隐藏开始反演时，提示行首控件可获焦（仅当隐藏时：原工程隐藏 IDOK；为防误触设为不可用）
    mBtnOk->setEnabled(bReady);
}

// 功能："..."选择场值数据文件（对应原工程 OnOpenOriginalFile）
void CGridFileNameDlg::onOpenOriginalFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择重磁异常网格数据文件"),
        QStringLiteral("网格化文件 (*.grd);;所有文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadGridFile(strFilePath);
}

// 功能：下拉选择场值数据文件（对应原工程 OnSelchangeCombo1）
void CGridFileNameDlg::onOriginalComboActivated(int nIndex)
{
    if (nIndex < 0 || nIndex >= mCandidateFiles.size())
    {
        return;
    }
    loadGridFile(mCandidateFiles.at(nIndex));
}

// 功能：读取场值文件头并刷新网格信息、默认值（对应原工程 ReadData）
void CGridFileNameDlg::loadGridFile(const QString& strFilePath)
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
        QMessageBox::warning(this, QStringLiteral("读取文件失败"),
            QStringLiteral("不能打开") + strFilePath + QStringLiteral("文件！"));
        return;
    }

    // 记录加载状态并同步下拉框显示
    mLoadedFilePath = strFilePath;
    mInputFilePath = strFilePath;
    if (mComboOriginalFile != NULL)
    {
        mComboOriginalFile->setEditText(strFilePath);
    }

    // 网格信息（与原工程 xStep/yStep 计算一致）
    mRowsNum = head.rows;
    mColsNum = head.cols;
    mDataXMin = head.xMin;
    mDataXMax = head.xMax;
    mDataYMin = head.yMin;
    mDataYMax = head.yMax;
    double dXStep = (head.cols > 1) ? (head.xMax - head.xMin) / (head.cols - 1) : 0.0;
    double dYStep = (head.rows > 1) ? (head.yMax - head.yMin) / (head.rows - 1) : 0.0;

    // 模型范围/网格距默认值（与原工程 ReadData 一致）：
    //   X/Y 范围 = 数据范围，X/Y 网格距 = 数据点/线距，Z 网格距 = X 网格距，
    //   Z 最小值 = X 网格距，Z 最大值 = Zmin + Zstep * (cols 为偶 ? cols : cols-1) / 2
    mModXMin = mDataXMin;
    mModXMax = mDataXMax;
    mModYMin = mDataYMin;
    mModYMax = mDataYMax;
    mModXStep = dXStep;
    mModYStep = dYStep;
    mModZStep = dXStep;
    mModZMin = dXStep;
    if (mColsNum / 2 * 2 == mColsNum)
    {
        mModZMax = mModZMin + mModZStep * mColsNum / 2;
    }
    else
    {
        mModZMax = mModZMin + mModZStep * (mColsNum - 1) / 2;
    }

    updateGridInfoDisplay();
    updateModExtentDisplay();

    // 默认输出文件名（对应原工程 ReadData 末尾的 _Fwd.grd / _Vol.vol / _Inf.txt）
    int nPos = strFilePath.lastIndexOf('.');
    mFilePathBase = strFilePath;
    if (nPos > 0)
    {
        mFilePathBase = strFilePath.left(nPos);
    }
    mForwardFieldsFilePath = mFilePathBase + QStringLiteral("_Fwd.grd");
    mModelPropertiesFilePath = mFilePathBase + QStringLiteral("_Vol.vol");
    mInvertInfoFilePath = mFilePathBase + QStringLiteral("_Inf.txt");
    if (mEditForwardFieldsFile != NULL)
    {
        mEditForwardFieldsFile->setText(mForwardFieldsFilePath);
    }
    if (mEditModelPropertiesFile != NULL)
    {
        mEditModelPropertiesFile->setText(mModelPropertiesFilePath);
    }
    if (mEditInvertInfoFile != NULL)
    {
        mEditInvertInfoFile->setText(mInvertInfoFilePath);
    }

    // 地形数据文件行与"开始反演"按文件齐全情况显隐（与原工程一致）
    updateVisibilities();
    updateStartButtonVisibility();
}

// 功能：把网格信息成员变量刷到界面显示
void CGridFileNameDlg::updateGridInfoDisplay()
{
    if (mEditRowsNum != NULL)
    {
        mEditRowsNum->setText(QString::number(mRowsNum));
    }
    if (mEditColsNum != NULL)
    {
        mEditColsNum->setText(QString::number(mColsNum));
    }
    if (mEditXMin != NULL)
    {
        mEditXMin->setText(QString::number(mDataXMin, 'f', 2));
    }
    if (mEditXMax != NULL)
    {
        mEditXMax->setText(QString::number(mDataXMax, 'f', 2));
    }
    if (mEditYMin != NULL)
    {
        mEditYMin->setText(QString::number(mDataYMin, 'f', 2));
    }
    if (mEditYMax != NULL)
    {
        mEditYMax->setText(QString::number(mDataYMax, 'f', 2));
    }
}

// 功能：把模型范围/网格距成员变量刷到界面显示
void CGridFileNameDlg::updateModExtentDisplay()
{
    if (mEditModXMin != NULL)
    {
        mEditModXMin->setText(QString::number(mModXMin, 'f', 3));
    }
    if (mEditModXMax != NULL)
    {
        mEditModXMax->setText(QString::number(mModXMax, 'f', 3));
    }
    if (mEditModYMin != NULL)
    {
        mEditModYMin->setText(QString::number(mModYMin, 'f', 3));
    }
    if (mEditModYMax != NULL)
    {
        mEditModYMax->setText(QString::number(mModYMax, 'f', 3));
    }
    if (mEditModXStep != NULL)
    {
        mEditModXStep->setText(QString::number(mModXStep, 'f', 3));
    }
    if (mEditModYStep != NULL)
    {
        mEditModYStep->setText(QString::number(mModYStep, 'f', 3));
    }
    if (mSpinModZMin != NULL)
    {
        mSpinModZMin->setValue(mModZMin);
    }
    if (mSpinModZMax != NULL)
    {
        mSpinModZMax->setValue(mModZMax);
    }
    if (mSpinModZStep != NULL)
    {
        mSpinModZStep->setValue(mModZStep);
    }
}

// 功能："..."选择地形高程数据文件（对应原工程 OnBnClickedOpenSurfaceFile）
void CGridFileNameDlg::onOpenSurfaceFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择地形高程网格数据文件"),
        QStringLiteral("网格化文件 (*.grd);;所有文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    loadDemFile(strFilePath);
}

// 功能：下拉选择地形数据文件（对应原工程 OnSelchangeCombo2）
void CGridFileNameDlg::onSurfaceComboActivated(int nIndex)
{
    if (nIndex < 0 || nIndex >= mCandidateFiles.size())
    {
        return;
    }
    loadDemFile(mCandidateFiles.at(nIndex));
}

// 功能：读取地形文件头并做坐标一致性校验（对应原工程 ReadDEMData）
void CGridFileNameDlg::loadDemFile(const QString& strFilePath)
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

    // 调用后端读取网格文件头（仅文件头，不读取数据体）
    GridFileHead head;
    BackendError error;
    if (!pBackend->readGridFileHead(toBackendString(strFilePath), head, error))
    {
        QMessageBox::warning(this, QStringLiteral("读取文件失败"),
            QStringLiteral("地形数据文件 ") + strFilePath + QStringLiteral("格式错误！"));
        return;
    }

    // 记录地形文件路径并同步下拉框显示（与原工程先赋值后校验的次序一致）
    mSurfaceFilePath = strFilePath;
    if (mComboSurfaceFile != NULL)
    {
        mComboSurfaceFile->setEditText(strFilePath);
    }
    updateStartButtonVisibility();

    // 坐标一致性校验（与原工程 "地形数据文件 X 和场值数据文件坐标不一致！" 一致）
    if (mDataXMin != head.xMin || mDataXMax != head.xMax
        || mDataYMin != head.yMin || mDataYMax != head.yMax)
    {
        QMessageBox::warning(this, QStringLiteral("坐标不一致"),
            QStringLiteral("地形数据文件 ") + strFilePath
                + QStringLiteral(" 和场值数据文件坐标不一致！"));
    }
}

// 功能："..."选择控制参数文件（.par，对应原工程 OnOpenCommandFile）
void CGridFileNameDlg::onOpenCommandFileClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择控制参数文件"),
        QStringLiteral("参数文件 (*.par);;所有文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mCommandFilePath = strFilePath;
    if (mEditCommandFile != NULL)
    {
        mEditCommandFile->setText(strFilePath);
    }
    updateStartButtonVisibility();
}

// 功能："..."选择正演场值文件（.grd，保存，对应原工程 OnOpenModelsFile）
void CGridFileNameDlg::onOpenModelsFileClicked()
{
    QString strFilePath = askSaveFilePath(QStringLiteral("请输入正演场值文件名"), mForwardFieldsFilePath,
        QStringLiteral("网格化文件 (*.grd);;所有文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mForwardFieldsFilePath = strFilePath;
    if (mEditForwardFieldsFile != NULL)
    {
        mEditForwardFieldsFile->setText(strFilePath);
    }
}

// 功能："..."选择模型数据文件（.vol，保存，对应原工程 OnOpenModelsPropertiesFile）
void CGridFileNameDlg::onOpenModelsPropertiesFileClicked()
{
    QString strFilePath = askSaveFilePath(QStringLiteral("请输入模型数据文件名"), mModelPropertiesFilePath,
        QStringLiteral("三维数据体文件 (*.vol);;所有文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mModelPropertiesFilePath = strFilePath;
    if (mEditModelPropertiesFile != NULL)
    {
        mEditModelPropertiesFile->setText(strFilePath);
    }
}

// 功能："..."选择先前模型物性文件（.vol，打开，对应原工程 OnOpenInitModels）
void CGridFileNameDlg::onOpenInitModelsClicked()
{
    QString strFilePath = askOpenFilePath(QStringLiteral("请选择先前模型物性数据文件"),
        QStringLiteral("模型体数据文件 (*.vol);;所有文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mPrevModelsFilePath = strFilePath;
    if (mEditInitModels != NULL)
    {
        mEditInitModels->setText(strFilePath);
    }
}

// 功能："..."选择反演信息文件（.txt，保存，对应原工程 OnSaveInvertInformationFile）
void CGridFileNameDlg::onSaveInvertInfoFileClicked()
{
    QString strFilePath = askSaveFilePath(QStringLiteral("请输入反演信息文件名"), mInvertInfoFilePath,
        QStringLiteral("反演信息文件 (*.txt);;所有文件 (*.*)"));
    if (strFilePath.isEmpty())
    {
        return;
    }
    mInvertInfoFilePath = strFilePath;
    if (mEditInvertInfoFile != NULL)
    {
        mEditInvertInfoFile->setText(strFilePath);
    }
}

// 功能：重力数据复选框切换（显隐"公里"与磁参，对应原工程 OnCheckGravData）
void CGridFileNameDlg::onGravityDataToggled(bool bChecked)
{
    mGravityData = bChecked;
    updateVisibilities();
}

// 功能：公里复选框切换（同步成员变量，对应原工程 OnCheckKm）
void CGridFileNameDlg::onKmToggled(bool bChecked)
{
    mKm = bChecked;
}

// 功能：初始模型复选框切换（显隐初始模型文件行，对应原工程 OnCheckInitModels）
void CGridFileNameDlg::onInitModelsToggled(bool bChecked)
{
    mPrevModelsAsInit = bChecked;
    updateVisibilities();
}

// 功能：磁化倾角变化（同步成员变量）
void CGridFileNameDlg::onInclinationChanged(double dValue)
{
    mInclination = dValue;
}

// 功能：磁化偏角变化（同步成员变量）
void CGridFileNameDlg::onDeclinationChanged(double dValue)
{
    mDeclination = dValue;
}

// 功能：测线方位角变化（同步成员变量）
void CGridFileNameDlg::onProfileDecChanged(double dValue)
{
    mProfileDec = dValue;
}

// 功能：基线方位角变化（同步成员变量）
void CGridFileNameDlg::onBaselineDecChanged(double dValue)
{
    mBaselineDec = dValue;
}

// 功能：模型 Z 最小值变化（同步成员变量）
void CGridFileNameDlg::onModZMinChanged(double dValue)
{
    mModZMin = dValue;
}

// 功能：模型 Z 最大值变化（同步成员变量）
void CGridFileNameDlg::onModZMaxChanged(double dValue)
{
    mModZMax = dValue;
}

// 功能：模型 Z 网格距变化（同步成员变量）
void CGridFileNameDlg::onModZStepChanged(double dValue)
{
    mModZStep = dValue;
}

// 功能：弹出打开文件对话框
QString CGridFileNameDlg::askOpenFilePath(const QString& strTitle, const QString& strFilter)
{
    return QFileDialog::getOpenFileName(this, strTitle, QString(), strFilter);
}

// 功能：弹出保存文件对话框
QString CGridFileNameDlg::askSaveFilePath(const QString& strTitle, const QString& strDefaultPath,
                                          const QString& strFilter)
{
    return QFileDialog::getSaveFileName(this, strTitle, strDefaultPath, strFilter);
}

// 功能：检查网格文件存在并发出发送显示请求
void CGridFileNameDlg::viewGridFile(const QString& strFilePath)
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
    // 等值线显示窗口由前端后续版本实现，此处交由宿主处理
    emit viewGridFileRequested(strFilePath);
}

// 功能：检查体数据文件存在并发出显示请求
void CGridFileNameDlg::viewVolumeFile(const QString& strFilePath)
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

// 功能：显示场值数据文件等值线
void CGridFileNameDlg::onOpenOriginalFileViewClicked()
{
    viewGridFile(mInputFilePath);
}

// 功能：显示地形数据文件等值线
void CGridFileNameDlg::onOpenSurfaceFileViewClicked()
{
    viewGridFile(mSurfaceFilePath);
}

// 功能：显示正演场值文件等值线
void CGridFileNameDlg::onOpenModelsFileViewClicked()
{
    viewGridFile(mForwardFieldsFilePath);
}

// 功能：显示模型数据文件三维视图
void CGridFileNameDlg::onOpenModelsPropertiesFileViewClicked()
{
    viewVolumeFile(mModelPropertiesFilePath);
}

// 功能：显示先前模型物性文件三维视图
void CGridFileNameDlg::onOpenInitModelsViewClicked()
{
    viewVolumeFile(mPrevModelsFilePath);
}

// 功能：开始反演前校验输入项
bool CGridFileNameDlg::validateInputs(QString& strError)
{
    // 场值数据文件检查（原工程 OnOK 仅检查场值文件，其余在 OnOK 中给默认名）
    if (mInputFilePath.isEmpty())
    {
        strError = QStringLiteral("请先选择场值数据文件。");
        return false;
    }
    if (!QFile::exists(mInputFilePath))
    {
        strError = QStringLiteral("场值数据文件不存在：") + mInputFilePath;
        return false;
    }
    // 地形高程文件检查（"开始反演"按钮可见条件之一）
    if (mSurfaceFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择地形高程数据文件。");
        return false;
    }
    // 控制参数文件检查（"开始反演"按钮可见条件之一）
    if (mCommandFilePath.isEmpty())
    {
        strError = QStringLiteral("请选择控制参数文件。");
        return false;
    }
    // 模型 Z 最大值必须大于最小值
    if (mModZMax <= mModZMin)
    {
        strError = QStringLiteral("模型 Z 坐标最大值应大于最小值，请确认。");
        return false;
    }
    return true;
}

// 功能："开始反演"——校验参数并回调流程类（对应原工程 OnOK）
void CGridFileNameDlg::onOkClicked()
{
    QString strError;
    if (!validateInputs(strError))
    {
        QMessageBox::warning(this, QStringLiteral("参数错误"), strError);
        return;
    }

    if (mProc == NULL)
    {
        // 未接入流程类时给出占位提示（后端接口待提供）
        QMessageBox::warning(this, QStringLiteral("处理失败"),
            QStringLiteral("算法后端尚未接入：重磁三维体反演功能等待后端实现。"));
        return;
    }

    // 交由流程类读取参数并调用后端完成反演（对应原工程 m_pMyGravMagnVolumeInvProc->DoInvsProcess）
    mProc->doInvsProcess(this);
}

// 功能："退出反演"——关闭对话框
void CGridFileNameDlg::onCancelClicked()
{
    reject();
}

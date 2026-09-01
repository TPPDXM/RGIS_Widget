// MagnIntensityCalculationDlg.cpp : 磁化强度计算对话框实现
//
// 说明：
//   * 界面布局对照 RGISGravMagnDataProcessing.rc 中 IDD_MagnIntensityCalculation；
//   * 所有回调均为成员函数引用（connect 的第四个参数为 &类名::槽函数），不使用 lambda；
//   * 全程不使用 try/catch，错误一律通过返回值 + 消息框反馈，做足空指针检查；
//   * 中文字符串一律使用 QStringLiteral（源码 UTF-8）；
//   * 本对话框为纯参数计算（无后端接口）：计算公式与原 MFC 工程 OnOK 一致。

#include "MagnIntensityCalculationDlg.h"

#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include <cmath>

// 构造函数：初始化全部成员并创建界面
// 参数：pParent —— 父窗口（可为 NULL）
CMagnIntensityCalculationDlg::CMagnIntensityCalculationDlg(QWidget* pParent)
    : QDialog(pParent)
    , mRadioInduced(NULL)
    , mRadioTotal(NULL)
    , mRadioEffective(NULL)
    , mButtonGroupQuantity(NULL)
    , mSpinGeoMagIntensity(NULL)
    , mSpinSusceptibility(NULL)
    , mEditMagneticIntensity(NULL)
    , mSpinIndMagIntensity(NULL)
    , mSpinIndMagInclination(NULL)
    , mSpinIndMagDeclination(NULL)
    , mSpinResMagIntensity(NULL)
    , mSpinResMagInclination(NULL)
    , mSpinResMagDeclination(NULL)
    , mEditTotMagIntensity(NULL)
    , mEditTotMagInclination(NULL)
    , mEditTotMagDeclination(NULL)
    , mSpinMagIntensity(NULL)
    , mSpinInclination(NULL)
    , mSpinDeclination(NULL)
    , mSpinProfileAzimuth(NULL)
    , mEditEffectiveIntensity(NULL)
    , mEditEffectiveInclination(NULL)
    , mBtnCompute(NULL)
    , mBtnCancel(NULL)
    , mGeoMagIntensity(55000.0)
    , mSusceptibility(1000.0)
    , mIndMagIntensity(2.0)
    , mIndMagInclination(60.0)
    , mIndMagDeclination(5.0)
    , mResMagIntensity(3.0)
    , mResMagInclination(30.0)
    , mResMagDeclination(-2.0)
    , mMagIntensity(2.0)
    , mInclination(60.0)
    , mDeclination(10.0)
    , mProfileAzimuth(30.0)
    , mQuantity(QuantityInduced)
{
    // 创建界面控件并连接信号槽
    initUi();
}

// 析构函数：界面控件由 Qt 父子机制自动释放，无需手工释放
CMagnIntensityCalculationDlg::~CMagnIntensityCalculationDlg()
{
}

// 功能：按 .rc 布局创建全部控件并连接信号槽
void CMagnIntensityCalculationDlg::initUi()
{
    setWindowTitle(QStringLiteral("磁化强度计算"));
    setModal(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // ================= 计算物理量选择（单选）=================
    // 对应 .rc 中的 IDC_RADIO1 / IDC_RADIO2 / IDC_RADIO3（默认感应磁化强度）
    QGroupBox* pGroupQuantity = new QGroupBox(QStringLiteral("计算物理量选择"), this);
    QHBoxLayout* pQuantityLayout = new QHBoxLayout(pGroupQuantity);
    mButtonGroupQuantity = new QButtonGroup(this);
    mRadioInduced = new QRadioButton(QStringLiteral("感应磁化强度"), pGroupQuantity);
    mRadioTotal = new QRadioButton(QStringLiteral("总磁化强度"), pGroupQuantity);
    mRadioEffective = new QRadioButton(QStringLiteral("有效磁化强度"), pGroupQuantity);
    mButtonGroupQuantity->addButton(mRadioInduced, QuantityInduced);
    mButtonGroupQuantity->addButton(mRadioTotal, QuantityTotal);
    mButtonGroupQuantity->addButton(mRadioEffective, QuantityEffective);
    mRadioInduced->setChecked(true);    // 默认感应磁化强度（与原工程 OnInitDialog 一致）
    mQuantity = QuantityInduced;
    pQuantityLayout->addWidget(mRadioInduced);
    pQuantityLayout->addWidget(mRadioTotal);
    pQuantityLayout->addWidget(mRadioEffective);
    pQuantityLayout->addStretch(1);
    pMainLayout->addWidget(pGroupQuantity);

    // ================= 感应磁化强度计算 =================
    // 对应 .rc 中的 IDC_GeoMagIntensity / IDC_Susceptibility / IDC_MagneticIntensity（结果只读）
    QGroupBox* pGroupInduced = new QGroupBox(
        QStringLiteral("感应磁化强度（地磁场强度单位：nT，磁化率单位：0.00001 SI，感应磁化强度单位：A/m）"), this);
    QGridLayout* pInducedLayout = new QGridLayout(pGroupInduced);
    pInducedLayout->addWidget(new QLabel(QStringLiteral("地磁场强度"), pGroupInduced), 0, 0);
    mSpinGeoMagIntensity = new QDoubleSpinBox(pGroupInduced);
    mSpinGeoMagIntensity->setRange(0.0, 1.0e12);
    mSpinGeoMagIntensity->setDecimals(3);
    mSpinGeoMagIntensity->setSingleStep(1.0);
    mSpinGeoMagIntensity->setValue(mGeoMagIntensity);
    mSpinGeoMagIntensity->setAlignment(Qt::AlignCenter);
    mSpinGeoMagIntensity->setFixedWidth(110);
    pInducedLayout->addWidget(mSpinGeoMagIntensity, 0, 1);
    pInducedLayout->addWidget(new QLabel(QStringLiteral("磁化率"), pGroupInduced), 0, 2);
    mSpinSusceptibility = new QDoubleSpinBox(pGroupInduced);
    mSpinSusceptibility->setRange(0.0, 1.0e12);
    mSpinSusceptibility->setDecimals(3);
    mSpinSusceptibility->setSingleStep(1.0);
    mSpinSusceptibility->setValue(mSusceptibility);
    mSpinSusceptibility->setAlignment(Qt::AlignCenter);
    mSpinSusceptibility->setFixedWidth(110);
    pInducedLayout->addWidget(mSpinSusceptibility, 0, 3);
    pInducedLayout->addWidget(new QLabel(QStringLiteral("感应磁化强度"), pGroupInduced), 0, 4);
    mEditMagneticIntensity = new QLineEdit(pGroupInduced);
    mEditMagneticIntensity->setReadOnly(true);
    mEditMagneticIntensity->setAlignment(Qt::AlignCenter);
    mEditMagneticIntensity->setFixedWidth(110);
    pInducedLayout->addWidget(mEditMagneticIntensity, 0, 5);
    pMainLayout->addWidget(pGroupInduced);

    // ================= 总磁化强度计算 =================
    // 对应 .rc 中的 IDC_IndMagIntensity / IDC_IndMagInclination / IDC_IndMagDeclination /
    //           IDC_ResMagIntensity / IDC_ResMagInclination / IDC_ResMagDeclination /
    //           IDC_TotMagIntensity / IDC_TotMagInclination / IDC_TotMagDeclination（结果只读）
    QGroupBox* pGroupTotal = new QGroupBox(
        QStringLiteral("总磁化强度计算（磁化强度单位：A/m，角度单位：度）"), this);
    QGridLayout* pTotalLayout = new QGridLayout(pGroupTotal);
    pTotalLayout->addWidget(new QLabel(QStringLiteral("感应磁化强度"), pGroupTotal), 0, 0);
    mSpinIndMagIntensity = new QDoubleSpinBox(pGroupTotal);
    mSpinIndMagIntensity->setRange(0.0, 1.0e12);
    mSpinIndMagIntensity->setDecimals(3);
    mSpinIndMagIntensity->setSingleStep(1.0);
    mSpinIndMagIntensity->setValue(mIndMagIntensity);
    mSpinIndMagIntensity->setAlignment(Qt::AlignCenter);
    mSpinIndMagIntensity->setFixedWidth(110);
    pTotalLayout->addWidget(mSpinIndMagIntensity, 0, 1);
    pTotalLayout->addWidget(new QLabel(QStringLiteral("感应磁化倾角"), pGroupTotal), 0, 2);
    mSpinIndMagInclination = new QDoubleSpinBox(pGroupTotal);
    mSpinIndMagInclination->setRange(-360.0, 360.0);
    mSpinIndMagInclination->setDecimals(3);
    mSpinIndMagInclination->setSingleStep(1.0);
    mSpinIndMagInclination->setValue(mIndMagInclination);
    mSpinIndMagInclination->setAlignment(Qt::AlignCenter);
    mSpinIndMagInclination->setFixedWidth(110);
    pTotalLayout->addWidget(mSpinIndMagInclination, 0, 3);
    pTotalLayout->addWidget(new QLabel(QStringLiteral("感应磁化偏角"), pGroupTotal), 0, 4);
    mSpinIndMagDeclination = new QDoubleSpinBox(pGroupTotal);
    mSpinIndMagDeclination->setRange(-360.0, 360.0);
    mSpinIndMagDeclination->setDecimals(3);
    mSpinIndMagDeclination->setSingleStep(1.0);
    mSpinIndMagDeclination->setValue(mIndMagDeclination);
    mSpinIndMagDeclination->setAlignment(Qt::AlignCenter);
    mSpinIndMagDeclination->setFixedWidth(110);
    pTotalLayout->addWidget(mSpinIndMagDeclination, 0, 5);
    pTotalLayout->addWidget(new QLabel(QStringLiteral("剩余磁化强度"), pGroupTotal), 1, 0);
    mSpinResMagIntensity = new QDoubleSpinBox(pGroupTotal);
    mSpinResMagIntensity->setRange(0.0, 1.0e12);
    mSpinResMagIntensity->setDecimals(3);
    mSpinResMagIntensity->setSingleStep(1.0);
    mSpinResMagIntensity->setValue(mResMagIntensity);
    mSpinResMagIntensity->setAlignment(Qt::AlignCenter);
    mSpinResMagIntensity->setFixedWidth(110);
    pTotalLayout->addWidget(mSpinResMagIntensity, 1, 1);
    pTotalLayout->addWidget(new QLabel(QStringLiteral("剩余磁化倾角"), pGroupTotal), 1, 2);
    mSpinResMagInclination = new QDoubleSpinBox(pGroupTotal);
    mSpinResMagInclination->setRange(-360.0, 360.0);
    mSpinResMagInclination->setDecimals(3);
    mSpinResMagInclination->setSingleStep(1.0);
    mSpinResMagInclination->setValue(mResMagInclination);
    mSpinResMagInclination->setAlignment(Qt::AlignCenter);
    mSpinResMagInclination->setFixedWidth(110);
    pTotalLayout->addWidget(mSpinResMagInclination, 1, 3);
    pTotalLayout->addWidget(new QLabel(QStringLiteral("剩余磁化偏角"), pGroupTotal), 1, 4);
    mSpinResMagDeclination = new QDoubleSpinBox(pGroupTotal);
    mSpinResMagDeclination->setRange(-360.0, 360.0);
    mSpinResMagDeclination->setDecimals(3);
    mSpinResMagDeclination->setSingleStep(1.0);
    mSpinResMagDeclination->setValue(mResMagDeclination);
    mSpinResMagDeclination->setAlignment(Qt::AlignCenter);
    mSpinResMagDeclination->setFixedWidth(110);
    pTotalLayout->addWidget(mSpinResMagDeclination, 1, 5);
    pTotalLayout->addWidget(new QLabel(QStringLiteral("总磁化强度"), pGroupTotal), 2, 0);
    mEditTotMagIntensity = new QLineEdit(pGroupTotal);
    mEditTotMagIntensity->setReadOnly(true);
    mEditTotMagIntensity->setAlignment(Qt::AlignCenter);
    mEditTotMagIntensity->setFixedWidth(110);
    pTotalLayout->addWidget(mEditTotMagIntensity, 2, 1);
    pTotalLayout->addWidget(new QLabel(QStringLiteral("总磁化倾角"), pGroupTotal), 2, 2);
    mEditTotMagInclination = new QLineEdit(pGroupTotal);
    mEditTotMagInclination->setReadOnly(true);
    mEditTotMagInclination->setAlignment(Qt::AlignCenter);
    mEditTotMagInclination->setFixedWidth(110);
    pTotalLayout->addWidget(mEditTotMagInclination, 2, 3);
    pTotalLayout->addWidget(new QLabel(QStringLiteral("总磁化偏角"), pGroupTotal), 2, 4);
    mEditTotMagDeclination = new QLineEdit(pGroupTotal);
    mEditTotMagDeclination->setReadOnly(true);
    mEditTotMagDeclination->setAlignment(Qt::AlignCenter);
    mEditTotMagDeclination->setFixedWidth(110);
    pTotalLayout->addWidget(mEditTotMagDeclination, 2, 5);
    pMainLayout->addWidget(pGroupTotal);

    // ================= 有效磁化强度计算 =================
    // 对应 .rc 中的 IDC_MagIntensity / IDC_Inclination / IDC_Declination /
    //           IDC_ProfileAzimuth / IDC_EffectiveIntensity / IDC_EffectiveInclination（结果只读）
    QGroupBox* pGroupEffective = new QGroupBox(
        QStringLiteral("有效磁化强度计算（磁化强度单位：A/m，角度单位：度）"), this);
    QGridLayout* pEffectiveLayout = new QGridLayout(pGroupEffective);
    pEffectiveLayout->addWidget(new QLabel(QStringLiteral("总磁化强度"), pGroupEffective), 0, 0);
    mSpinMagIntensity = new QDoubleSpinBox(pGroupEffective);
    mSpinMagIntensity->setRange(0.0, 1.0e12);
    mSpinMagIntensity->setDecimals(3);
    mSpinMagIntensity->setSingleStep(1.0);
    mSpinMagIntensity->setValue(mMagIntensity);
    mSpinMagIntensity->setAlignment(Qt::AlignCenter);
    mSpinMagIntensity->setFixedWidth(110);
    pEffectiveLayout->addWidget(mSpinMagIntensity, 0, 1);
    pEffectiveLayout->addWidget(new QLabel(QStringLiteral("总磁化倾角"), pGroupEffective), 0, 2);
    mSpinInclination = new QDoubleSpinBox(pGroupEffective);
    mSpinInclination->setRange(-360.0, 360.0);
    mSpinInclination->setDecimals(3);
    mSpinInclination->setSingleStep(1.0);
    mSpinInclination->setValue(mInclination);
    mSpinInclination->setAlignment(Qt::AlignCenter);
    mSpinInclination->setFixedWidth(110);
    pEffectiveLayout->addWidget(mSpinInclination, 0, 3);
    pEffectiveLayout->addWidget(new QLabel(QStringLiteral("总磁化偏角"), pGroupEffective), 0, 4);
    mSpinDeclination = new QDoubleSpinBox(pGroupEffective);
    mSpinDeclination->setRange(-360.0, 360.0);
    mSpinDeclination->setDecimals(3);
    mSpinDeclination->setSingleStep(1.0);
    mSpinDeclination->setValue(mDeclination);
    mSpinDeclination->setAlignment(Qt::AlignCenter);
    mSpinDeclination->setFixedWidth(110);
    pEffectiveLayout->addWidget(mSpinDeclination, 0, 5);
    pEffectiveLayout->addWidget(new QLabel(QStringLiteral("剖面方位角"), pGroupEffective), 1, 0);
    mSpinProfileAzimuth = new QDoubleSpinBox(pGroupEffective);
    mSpinProfileAzimuth->setRange(-360.0, 360.0);
    mSpinProfileAzimuth->setDecimals(3);
    mSpinProfileAzimuth->setSingleStep(1.0);
    mSpinProfileAzimuth->setValue(mProfileAzimuth);
    mSpinProfileAzimuth->setAlignment(Qt::AlignCenter);
    mSpinProfileAzimuth->setFixedWidth(110);
    pEffectiveLayout->addWidget(mSpinProfileAzimuth, 1, 1);
    pEffectiveLayout->addWidget(new QLabel(QStringLiteral("有效磁化强度"), pGroupEffective), 1, 2);
    mEditEffectiveIntensity = new QLineEdit(pGroupEffective);
    mEditEffectiveIntensity->setReadOnly(true);
    mEditEffectiveIntensity->setAlignment(Qt::AlignCenter);
    mEditEffectiveIntensity->setFixedWidth(110);
    pEffectiveLayout->addWidget(mEditEffectiveIntensity, 1, 3);
    pEffectiveLayout->addWidget(new QLabel(QStringLiteral("有效磁化倾角"), pGroupEffective), 1, 4);
    mEditEffectiveInclination = new QLineEdit(pGroupEffective);
    mEditEffectiveInclination->setReadOnly(true);
    mEditEffectiveInclination->setAlignment(Qt::AlignCenter);
    mEditEffectiveInclination->setFixedWidth(110);
    pEffectiveLayout->addWidget(mEditEffectiveInclination, 1, 5);
    pMainLayout->addWidget(pGroupEffective);

    // ================= 计算 / 取消 =================
    QHBoxLayout* pButtonLayout = new QHBoxLayout();
    mBtnCompute = new QPushButton(QStringLiteral("计  算"), this);
    mBtnCancel = new QPushButton(QStringLiteral("取  消"), this);
    mBtnCompute->setDefault(true);
    pButtonLayout->addWidget(mBtnCompute);
    pButtonLayout->addStretch(1);
    pButtonLayout->addWidget(mBtnCancel);
    pMainLayout->addLayout(pButtonLayout);

    // ================= 信号槽连接（全部使用成员函数引用，不使用 lambda）=================
    // 分组框标题居中（对应原工程 GROUPBOX 的 BS_CENTER 样式，见示例图片 SubWindow.png）
    for (QGroupBox* pGroup : findChildren<QGroupBox*>())
    {
        pGroup->setAlignment(Qt::AlignCenter);
    }

    // 计算物理量单选组
    connect(mButtonGroupQuantity, &QButtonGroup::idClicked, this, &CMagnIntensityCalculationDlg::onQuantityClicked);

    // 计算 / 取消
    connect(mBtnCompute, &QPushButton::clicked, this, &CMagnIntensityCalculationDlg::onComputeClicked);
    connect(mBtnCancel, &QPushButton::clicked, this, &CMagnIntensityCalculationDlg::onCancelClicked);

    // 对话框按内容固定尺寸（与原工程 DS_MODALFRAME 固定大小对话框一致）
    setFixedSize(sizeHint());
    setSizeGripEnabled(false);
    adjustSize();
}

// 功能：计算物理量切换（同步成员变量，对应原工程 OnRad1/OnRad2/OnRad3）
void CMagnIntensityCalculationDlg::onQuantityClicked(int nQuantityId)
{
    mQuantity = nQuantityId;
}

// 功能：感应磁化强度输入校验（对应原工程 OnOK 感应分支）
bool CMagnIntensityCalculationDlg::validateInduced(QString& strError)
{
    if (mGeoMagIntensity < 0 || mGeoMagIntensity > 70000)
    {
        strError = QStringLiteral("地磁场强度数据越界!");
        return false;
    }
    if (mSusceptibility < 0)
    {
        strError = QStringLiteral("磁化率数据必须大于零!");
        return false;
    }
    return true;
}

// 功能：总磁化强度输入校验（对应原工程 OnOK 总磁化强度分支）
bool CMagnIntensityCalculationDlg::validateTotal(QString& strError)
{
    if (mIndMagInclination < -90 || mIndMagInclination > 90 ||
        mResMagInclination < -90 || mResMagInclination > 90)
    {
        strError = QStringLiteral("磁化倾角数据越界!");
        return false;
    }
    if (mIndMagDeclination < -180 || mIndMagDeclination > 180 ||
        mResMagDeclination < -180 || mResMagDeclination > 180)
    {
        strError = QStringLiteral("磁化偏角数据越界!");
        return false;
    }
    if (mIndMagIntensity < 0 || mResMagIntensity < 0)
    {
        strError = QStringLiteral("磁化强度数据必须大于零!");
        return false;
    }
    return true;
}

// 功能：有效磁化强度输入校验（对应原工程 OnOK 有效磁化强度分支）
bool CMagnIntensityCalculationDlg::validateEffective(QString& strError)
{
    if (mInclination < -90 || mInclination > 90)
    {
        strError = QStringLiteral("磁化倾角数据越界!");
        return false;
    }
    if (mDeclination < -180 || mDeclination > 180)
    {
        strError = QStringLiteral("磁化偏角数据越界!");
        return false;
    }
    if (mMagIntensity < 0)
    {
        strError = QStringLiteral("磁化强度数据必须大于零!");
        return false;
    }
    return true;
}

// 功能：计算感应磁化强度并显示（对应原工程 OnOK 感应分支公式）
void CMagnIntensityCalculationDlg::computeInduced()
{
    if (mEditMagneticIntensity == NULL)
    {
        return;
    }
    const double kPi = 3.141593;
    double dMag = mGeoMagIntensity / 100.0;
    double dSup = mSusceptibility / (4.0 * kPi);
    double dResult = dMag * dSup / 100000.0;
    mEditMagneticIntensity->setText(QString::number(dResult, 'f', 3));
}

// 功能：计算总磁化强度（矢量合成）并显示（对应原工程 OnOK 总磁化强度分支公式）
void CMagnIntensityCalculationDlg::computeTotal()
{
    if (mEditTotMagIntensity == NULL || mEditTotMagInclination == NULL || mEditTotMagDeclination == NULL)
    {
        return;
    }
    const double kPi = 3.141593;
    double dIntInc = mIndMagInclination * kPi / 180.0;
    double dIntDec = mIndMagDeclination * kPi / 180.0;
    double dResInc = mResMagInclination * kPi / 180.0;
    double dResDec = mResMagDeclination * kPi / 180.0;
    double dInt = mIndMagIntensity;
    double dInt2 = dInt * dInt;
    double dRes = mResMagIntensity;
    double dRes2 = dRes * dRes;
    double dIntRes = dInt * dRes;

    double dTotIntensity = sqrt(dInt2 + dRes2 +
        2.0 * dIntRes * cos(dIntDec - dResDec) * cos(dIntInc) * cos(dResInc) +
        2.0 * dIntRes * sin(dIntInc) * sin(dResInc));
    double dTotInclination = atan((dInt * sin(dIntInc) + dRes * sin(dResInc)) /
        sqrt(dInt2 * cos(dIntInc) * cos(dIntInc) + dRes2 * cos(dResInc) * cos(dResInc) +
             2.0 * dIntRes * cos(dIntDec - dResDec) * cos(dIntInc) * cos(dResInc))) * 180.0 / kPi;
    double dTotDeclination = atan((dInt * cos(dIntInc) * sin(dIntDec) + dRes * cos(dResInc) * sin(dResDec)) /
        (dInt * cos(dIntInc) * cos(dIntDec) + dRes * cos(dResInc) * cos(dResDec))) * 180.0 / kPi;

    mEditTotMagIntensity->setText(QString::number(dTotIntensity, 'f', 3));
    mEditTotMagInclination->setText(QString::number(dTotInclination, 'f', 3));
    mEditTotMagDeclination->setText(QString::number(dTotDeclination, 'f', 3));
}

// 功能：计算有效磁化强度（剖面方向投影）并显示（对应原工程 OnOK 有效磁化强度分支公式）
void CMagnIntensityCalculationDlg::computeEffective()
{
    if (mEditEffectiveIntensity == NULL || mEditEffectiveInclination == NULL)
    {
        return;
    }
    const double kPi = 3.141593;
    double dAllInc = mInclination * kPi / 180.0;
    double dAllDec = mDeclination * kPi / 180.0;
    double dPrfAng = mProfileAzimuth * kPi / 180.0;
    double dPrfDec = dPrfAng - dAllDec;
    if (dPrfDec > kPi)
    {
        dPrfDec = 2.0 * kPi - dPrfDec;
    }
    if (dPrfDec < -kPi)
    {
        dPrfDec = 2.0 * kPi + dPrfDec;
    }
    if (dPrfDec > kPi / 2.0)
    {
        dPrfDec = -kPi + dPrfDec;
    }
    if (dPrfDec < -kPi / 2.0)
    {
        dPrfDec = kPi + dPrfDec;
    }

    double dEffectiveIntensity = mMagIntensity * sqrt(
        cos(dAllInc) * cos(dAllInc) * cos(dPrfAng - dAllDec) * cos(dPrfAng - dAllDec) +
        sin(dAllInc) * sin(dAllInc));
    double dEffectiveInclination = atan(tan(dAllInc) / cos(dPrfAng - dAllDec)) * 180.0 / kPi;

    mEditEffectiveIntensity->setText(QString::number(dEffectiveIntensity, 'f', 3));
    mEditEffectiveInclination->setText(QString::number(dEffectiveInclination, 'f', 3));
}

// 功能：“计  算”——校验参数并按选中物理量计算（对应原工程 OnOK）
void CMagnIntensityCalculationDlg::onComputeClicked()
{
    QString strError;
    bool bValid = false;

    if (mQuantity == QuantityInduced)
    {
        bValid = validateInduced(strError);
        if (bValid)
        {
            computeInduced();
            return;
        }
    }
    else if (mQuantity == QuantityTotal)
    {
        bValid = validateTotal(strError);
        if (bValid)
        {
            computeTotal();
            return;
        }
    }
    else
    {
        bValid = validateEffective(strError);
        if (bValid)
        {
            computeEffective();
            return;
        }
    }

    if (!bValid)
    {
        QMessageBox::warning(this, QStringLiteral("参数错误"), strError);
    }
}

// 功能：“取  消”——关闭对话框（对应原工程 OnCancel）
void CMagnIntensityCalculationDlg::onCancelClicked()
{
    reject();
}

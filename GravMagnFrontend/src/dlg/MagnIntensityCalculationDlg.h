// MagnIntensityCalculationDlg.h : 磁化强度计算对话框（Qt 前端）
//
// 对应原 MFC 工程：CMagnIntensityCalculationDlg（IDD_MagnIntensityCalculation）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_MagnIntensityCalculation 绘制：
//   计算物理量选择（感应磁化强度 / 总磁化强度 / 有效磁化强度）/
//   感应磁化强度计算 / 总磁化强度计算 / 有效磁化强度计算
//
// 说明：本对话框为纯参数计算工具（无网格数据文件输入/输出），
//       计算逻辑与原 MFC 工程 OnOK 中的公式一致，由前端直接完成，
//       不依赖后端算法接口（磁化强度换算为地磁/磁化参数间的标量/矢量换算）。
//
// 前端职责：
//   1. 输入三个物理量分组的参数（默认值与原工程 AFX_DATA_INIT 一致）；
//   2. 选中某物理量后点“计算”：校验输入并计算，结果填入对应只读结果框；
//   3. 校验提示字符串与原工程一致（如“地磁场强度数据越界!”等）。

#pragma once

#include <QDialog>

// 前置声明（减少头文件之间的相互包含）
class QLineEdit;
class QPushButton;
class QRadioButton;
class QDoubleSpinBox;
class QButtonGroup;

// 磁化强度计算对话框
class CMagnIntensityCalculationDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CMagnIntensityCalculationDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CMagnIntensityCalculationDlg() override;

signals:
    // 功能：请求宿主显示指定文本/数据文件（本对话框无文件输出，预留；由宿主处理）
    void viewFileRequested(const QString& strFilePath);

private slots:
    // ===== 计算物理量单选回调 =====
    void onQuantityClicked(int nQuantityId);    // 功能：计算物理量切换（同步成员变量）

    // ===== 计算 / 取消 =====
    void onComputeClicked();                    // 功能：“计  算”——校验参数并按选中物理量计算
    void onCancelClicked();                     // 功能：“取  消”——关闭对话框

private:
    // 计算物理量类型枚举（对应原工程 IDC_RADIO1 / IDC_RADIO2 / IDC_RADIO3）
    enum EQuantity { QuantityInduced = 0, QuantityTotal = 1, QuantityEffective = 2 };

    // ===== 界面初始化 =====
    void initUi();                              // 功能：按 .rc 布局创建全部控件并连接信号槽

    // ===== 计算 =====
    bool validateInduced(QString& strError);    // 功能：感应磁化强度输入校验
    bool validateTotal(QString& strError);      // 功能：总磁化强度输入校验
    bool validateEffective(QString& strError);  // 功能：有效磁化强度输入校验
    void computeInduced();                      // 功能：计算感应磁化强度并显示
    void computeTotal();                        // 功能：计算总磁化强度（矢量合成）并显示
    void computeEffective();                    // 功能：计算有效磁化强度（剖面投影）并显示

    // ===== 界面控件（计算物理量选择，单选）=====
    QRadioButton*   mRadioInduced;      // 感应磁化强度（默认选中）
    QRadioButton*   mRadioTotal;        // 总磁化强度
    QRadioButton*   mRadioEffective;    // 有效磁化强度
    QButtonGroup*   mButtonGroupQuantity;  // 计算物理量单选按钮组

    // ===== 界面控件（感应磁化强度计算组）=====
    QDoubleSpinBox* mSpinGeoMagIntensity;   // 地磁场强度（nT，默认 55000）
    QDoubleSpinBox* mSpinSusceptibility;    // 磁化率（0.00001 SI，默认 1000）
    QLineEdit*      mEditMagneticIntensity; // 感应磁化强度结果（只读，A/m）

    // ===== 界面控件（总磁化强度计算组）=====
    QDoubleSpinBox* mSpinIndMagIntensity;   // 感应磁化强度（A/m，默认 2.0）
    QDoubleSpinBox* mSpinIndMagInclination; // 感应磁化倾角（度，默认 60）
    QDoubleSpinBox* mSpinIndMagDeclination; // 感应磁化偏角（度，默认 5）
    QDoubleSpinBox* mSpinResMagIntensity;   // 剩余磁化强度（A/m，默认 3.0）
    QDoubleSpinBox* mSpinResMagInclination; // 剩余磁化倾角（度，默认 30）
    QDoubleSpinBox* mSpinResMagDeclination; // 剩余磁化偏角（度，默认 -2）
    QLineEdit*      mEditTotMagIntensity;   // 总磁化强度结果（只读）
    QLineEdit*      mEditTotMagInclination; // 总磁化倾角结果（只读）
    QLineEdit*      mEditTotMagDeclination; // 总磁化偏角结果（只读）

    // ===== 界面控件（有效磁化强度计算组）=====
    QDoubleSpinBox* mSpinMagIntensity;      // 总磁化强度（A/m，默认 2.0）
    QDoubleSpinBox* mSpinInclination;       // 总磁化倾角（度，默认 60）
    QDoubleSpinBox* mSpinDeclination;       // 总磁化偏角（度，默认 10）
    QDoubleSpinBox* mSpinProfileAzimuth;    // 剖面方位角（度，默认 30）
    QLineEdit*      mEditEffectiveIntensity;    // 有效磁化强度结果（只读）
    QLineEdit*      mEditEffectiveInclination;  // 有效磁化倾角结果（只读）

    // ===== 界面控件（计算 / 取消）=====
    QPushButton* mBtnCompute;       // “计  算”按钮（校验并计算）
    QPushButton* mBtnCancel;        // “取  消”按钮（关闭对话框）

    // ===== 状态数据 =====
    double  mGeoMagIntensity;       // 地磁场强度（原 m_fGeoMagIntensity，默认 55000）
    double  mSusceptibility;        // 磁化率（原 m_fSusceptibility，默认 1000）
    double  mIndMagIntensity;       // 感应磁化强度（原 m_fIndIntensity，默认 2.0）
    double  mIndMagInclination;     // 感应磁化倾角（原 m_fIndMagInclination，默认 60）
    double  mIndMagDeclination;     // 感应磁化偏角（原 m_fIndMagDeclination，默认 5）
    double  mResMagIntensity;       // 剩余磁化强度（原 m_fResMagIntensity，默认 3.0）
    double  mResMagInclination;     // 剩余磁化倾角（原 m_fResMagInclination，默认 30）
    double  mResMagDeclination;     // 剩余磁化偏角（原 m_fResMagDeclination，默认 -2）
    double  mMagIntensity;          // 总磁化强度（原 m_fMagIntensity，默认 2.0）
    double  mInclination;           // 总磁化倾角（原 m_fInclination，默认 60）
    double  mDeclination;           // 总磁化偏角（原 m_fDeclination，默认 10）
    double  mProfileAzimuth;        // 剖面方位角（原 m_fProfileAzimuth，默认 30）

    int     mQuantity;              // 当前计算物理量（0=感应、1=总、2=有效，默认感应）
};

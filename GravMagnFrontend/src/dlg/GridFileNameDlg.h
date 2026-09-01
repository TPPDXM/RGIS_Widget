// GridFileNameDlg.h : 网格文件名对话框（三维重磁异常自动反演参数设置）（Qt 前端）
//
// 对应原 MFC 工程：CGridFileNameDlg（IDD_DLG_INVERSION_PARAMS，标题"三维重磁异常自动反演参数设置"）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_DLG_INVERSION_PARAMS 绘制：
//   场值和地形网格数据输入 / 网格数据信息 / 场值类型选择和参数输入 /
//   初始模型和反演控制参数文件输入 / 网格剖分信息 / 反演结果输出
//
// 职责：
//   1. 收集重磁三维体反演全部输入参数（场值/地形/控制参数文件、模型范围与网格距、
//      场值类型（重力/磁力）与磁参、初始模型、输出文件等）；
//   2. 读入场值/地形文件头，显示网格信息并生成模型默认剖分与默认输出文件名；
//   3. "开始反演"时调用 CGravMagnVolumeInvProc::doInvsProcess 完成反演
//      （算法在后端，接口待后端提供，当前为占位提示）。
//
// 说明：原工程中本对话框还被二维/三维界面反演等功能共用（通用文件参数对话框）。

#pragma once

#include <QDialog>

class QComboBox;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class CGravMagnVolumeInvProc;

// 网格文件名对话框（三维重磁异常自动反演参数设置）
class CGridFileNameDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：strFileNames —— 候选数据文件列表（分号分隔，可为空，填入两个文件下拉框）
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CGridFileNameDlg(const QString& strFileNames, QWidget* pParent = nullptr);

    // 析构函数
    ~CGridFileNameDlg() override;

    // ===== 流程类接入 =====
    // 功能：设置反演处理流程类指针（"开始反演"时回调其 doInvsProcess）
    void setGravMagnVolumeInvProc(CGravMagnVolumeInvProc* pProc);

    // ===== 默认参数设置（在模态打开前由流程类调用，对应原工程 OnDoInvsProcess 中的赋值）=====
    void setGravityData(bool bGravity);         // 功能：设置是否为重力数据（true 显示"公里"、隐藏磁参）
    void setKm(bool bKm);                       // 功能：设置长度单位是否为公里
    void setInclination(double dValue);         // 功能：设置磁化倾角（度）
    void setDeclination(double dValue);         // 功能：设置磁化偏角（度）
    void setProfileDec(double dValue);          // 功能：设置测线方位角（度）
    void setBaselineDec(double dValue);         // 功能：设置基线方位角（度）
    void setPrevModelsAsInit(bool bUsed);       // 功能：设置是否有先前模型作为初始模型

    // ===== 参数读取（供 CGravMagnVolumeInvProc::doInvsProcess 取用）=====
    bool        gravityData() const;            // 功能：取重力数据标志
    bool        km() const;                     // 功能：取公里单位标志
    double      inclination() const;            // 功能：取磁化倾角（度）
    double      declination() const;            // 功能：取磁化偏角（度）
    double      profileDec() const;             // 功能：取测线方位角（度）
    double      baselineDec() const;            // 功能：取基线方位角（度）
    bool        prevModelsAsInit() const;       // 功能：取初始模型使用标志
    QString     originalFieldsFilePath() const;  // 功能：取场值数据文件路径
    QString     originalSurfaceFilePath() const; // 功能：取地形高程数据文件路径
    QString     commandFilePath() const;          // 功能：取控制参数文件路径
    QString     forwardFieldsFilePath() const;    // 功能：取正演场值文件路径（默认 _Fwd.grd）
    QString     modelPropertiesFilePath() const;  // 功能：取模型数据文件路径（默认 _Vol.vol）
    QString     invertInfoFilePath() const;       // 功能：取反演信息文件路径（默认 _Inf.txt）
    QString     prevModelsFilePath() const;       // 功能：取先前模型物性文件路径
    double      modXmin() const;               // 功能：取模型 X 坐标最小值
    double      modXmax() const;               // 功能：取模型 X 坐标最大值
    double      modYmin() const;               // 功能：取模型 Y 坐标最小值
    double      modYmax() const;               // 功能：取模型 Y 坐标最大值
    double      modZmin() const;               // 功能：取模型 Z 坐标最小值
    double      modZmax() const;               // 功能：取模型 Z 坐标最大值
    double      modXstep() const;              // 功能：取模型 X 方向网格距
    double      modYstep() const;              // 功能：取模型 Y 方向网格距
    double      modZstep() const;              // 功能：取模型 Z 方向网格距

signals:
    // 功能：请求宿主显示指定网格文件的等值线图（对应原 MFC 工程"显示"按钮）
    void viewGridFileRequested(const QString& strFilePath);

    // 功能：请求宿主显示指定体数据文件的三维视图（对应原 MFC 工程 CVolumeDataViewDlg）
    void viewVolumeFileRequested(const QString& strFilePath);

private slots:
    // ===== 文件选择 / 下拉回调 =====
    void onOpenOriginalFileClicked();           // 功能："..."选择场值数据文件
    void onOpenSurfaceFileClicked();            // 功能："..."选择地形高程数据文件
    void onOpenCommandFileClicked();            // 功能："..."选择控制参数文件（.par）
    void onOpenModelsFileClicked();             // 功能："..."选择正演场值文件（.grd，保存）
    void onOpenModelsPropertiesFileClicked();   // 功能："..."选择模型数据文件（.vol，保存）
    void onOpenInitModelsClicked();             // 功能："..."选择先前模型物性文件（.vol，打开）
    void onSaveInvertInfoFileClicked();         // 功能："..."选择反演信息文件（.txt，保存）
    void onOriginalComboActivated(int nIndex);  // 功能：下拉选择场值数据文件（对应原 OnSelchangeCombo1）
    void onSurfaceComboActivated(int nIndex);   // 功能：下拉选择地形数据文件（对应原 OnSelchangeCombo2）

    // ===== 显示回调 =====
    void onOpenOriginalFileViewClicked();       // 功能：显示场值数据文件等值线
    void onOpenSurfaceFileViewClicked();        // 功能：显示地形数据文件等值线
    void onOpenModelsFileViewClicked();         // 功能：显示正演场值文件等值线
    void onOpenModelsPropertiesFileViewClicked();   // 功能：显示模型数据文件三维视图
    void onOpenInitModelsViewClicked();         // 功能：显示先前模型物性文件三维视图

    // ===== 选项回调 =====
    void onGravityDataToggled(bool bChecked);   // 功能：重力数据复选框切换（显隐"公里"与磁参）
    void onKmToggled(bool bChecked);            // 功能：公里复选框切换（同步成员变量）
    void onInitModelsToggled(bool bChecked);    // 功能：初始模型复选框切换（显隐初始模型文件行）
    void onInclinationChanged(double dValue);   // 功能：磁化倾角变化（同步成员变量）
    void onDeclinationChanged(double dValue);   // 功能：磁化偏角变化（同步成员变量）
    void onProfileDecChanged(double dValue);    // 功能：测线方位角变化（同步成员变量）
    void onBaselineDecChanged(double dValue);   // 功能：基线方位角变化（同步成员变量）

    // ===== 处理参数回调 =====
    void onModZMinChanged(double dValue);       // 功能：模型 Z 最小值变化（同步成员变量）
    void onModZMaxChanged(double dValue);       // 功能：模型 Z 最大值变化（同步成员变量）
    void onModZStepChanged(double dValue);      // 功能：模型 Z 网格距变化（同步成员变量）

    // ===== 确定 / 取消 =====
    void onOkClicked();                         // 功能："开始反演"——校验参数并回调流程类
    void onCancelClicked();                     // 功能："退出反演"——关闭对话框

private:
    // ===== 界面初始化 =====
    void initUi();                              // 功能：按 .rc 布局创建全部控件并连接信号槽
    void updateVisibilities();                  // 功能：按当前选项刷新各分组的显隐状态
    void updateStartButtonVisibility();         // 功能：按 场值/地形/控制参数文件 是否齐全刷新"开始反演"显隐

    // ===== 数据加载 =====
    void loadGridFile(const QString& strFilePath);              // 功能：读取场值文件头并刷新网格信息、默认值（原 ReadData）
    void loadDemFile(const QString& strFilePath);               // 功能：读取地形文件头并做坐标一致性校验（原 ReadDEMData）
    void updateGridInfoDisplay();               // 功能：把网格信息成员变量刷到界面显示
    void updateModExtentDisplay();              // 功能：把模型范围/网格距成员变量刷到界面显示

    // ===== 文件选择 / 显示 =====
    QString askOpenFilePath(const QString& strTitle, const QString& strFilter);   // 功能：弹出打开文件对话框
    QString askSaveFilePath(const QString& strTitle, const QString& strDefaultPath,
                            const QString& strFilter);                            // 功能：弹出保存文件对话框
    void viewGridFile(const QString& strFilePath);              // 功能：检查网格文件存在并发出发送显示请求
    void viewVolumeFile(const QString& strFilePath);            // 功能：检查体数据文件存在并发出显示请求

    // ===== 处理 =====
    bool validateInputs(QString& strError);     // 功能：开始反演前校验输入项，返回 false 表示校验失败

    // ===== 界面控件（场值和地形网格数据输入）=====
    QComboBox*      mComboOriginalFile;     // 场值数据文件下拉框（可编辑，对应原 IDC_EDIT_ORIGINAL_DATA_FILENAME）
    QPushButton*    mBtnOpenOriginalFile;   // 场值文件"..."按钮
    QPushButton*    mBtnOpenOriginalFileView;  // 场值文件"显示"按钮
    QLabel*         mLabelSurfaceFile;      // "地形数据文件"标签（初始隐藏，选完场值文件后显示）
    QComboBox*      mComboSurfaceFile;      // 地形高程数据文件下拉框（初始隐藏）
    QPushButton*    mBtnOpenSurfaceFile;    // 地形文件"..."按钮（初始隐藏）
    QPushButton*    mBtnOpenSurfaceFileView;// 地形文件"显示"按钮（初始隐藏）

    // ===== 界面控件（网格数据信息，只读显示）=====
    QLineEdit*  mEditRowsNum;       // 网格数据行数（只读）
    QLineEdit*  mEditColsNum;       // 网格数据列数（只读）
    QLineEdit*  mEditXMin;          // X 坐标最小值（只读，%.2f）
    QLineEdit*  mEditXMax;          // X 坐标最大值（只读，%.2f）
    QLineEdit*  mEditYMin;          // Y 坐标最小值（只读，%.2f）
    QLineEdit*  mEditYMax;          // Y 坐标最大值（只读，%.2f）

    // ===== 界面控件（场值类型选择和参数输入）=====
    QCheckBox*      mCheckGravityData;   // "重力数据"复选框（勾选=重力模式，默认勾选）
    QCheckBox*      mCheckKm;            // "公里"复选框（仅重力数据时显示）
    QLabel*         mLabelMagInc;        // "磁化倾角"标签（重力模式隐藏）
    QDoubleSpinBox* mSpinInclination;    // 磁化倾角（度，默认 90）
    QLabel*         mLabelMagDec;        // "磁化偏角"标签（重力模式隐藏）
    QDoubleSpinBox* mSpinDeclination;    // 磁化偏角（度，默认 0）
    QLabel*         mLabelProfile;       // "测线方位角"标签（重力模式隐藏）
    QDoubleSpinBox* mSpinProfileDec;     // 测线方位角（度，默认 90）
    QLabel*         mLabelBaseline;      // "基线方位角"标签（重力模式隐藏）
    QDoubleSpinBox* mSpinBaselineDec;    // 基线方位角（度，默认 0）

    // ===== 界面控件（初始模型和反演控制参数文件输入）=====
    QCheckBox*  mCheckInitModels;        // "初始模型"复选框（默认不勾选）
    QLineEdit*  mEditInitModels;         // 先前模型物性文件路径（初始隐藏）
    QPushButton* mBtnOpenInitModels;     // 初始模型"..."按钮（初始隐藏）
    QPushButton* mBtnOpenInitModelsView; // 初始模型"显示"按钮（初始隐藏）
    QLineEdit*  mEditCommandFile;        // 控制参数文件路径
    QPushButton* mBtnOpenCommandFile;    // 控制参数文件"..."按钮

    // ===== 界面控件（网格剖分信息）=====
    QLineEdit*      mEditModXMin;    // 模型 X 坐标最小值（只读）
    QLineEdit*      mEditModXMax;    // 模型 X 坐标最大值（只读）
    QLineEdit*      mEditModYMin;    // 模型 Y 坐标最小值（只读）
    QLineEdit*      mEditModYMax;    // 模型 Y 坐标最大值（只读）
    QDoubleSpinBox* mSpinModZMin;    // 模型 Z 坐标最小值（可编辑）
    QDoubleSpinBox* mSpinModZMax;    // 模型 Z 坐标最大值（可编辑）
    QLineEdit*      mEditModXStep;   // 模型 X 方向网格距（只读）
    QLineEdit*      mEditModYStep;   // 模型 Y 方向网格距（只读）
    QDoubleSpinBox* mSpinModZStep;   // 模型 Z 方向网格距（可编辑）

    // ===== 界面控件（反演结果输出）=====
    QLineEdit*  mEditForwardFieldsFile;      // 正演场值文件路径（默认 基准名 + "_Fwd.grd"）
    QPushButton* mBtnOpenModelsFile;         // 正演场值文件"..."按钮
    QPushButton* mBtnOpenModelsFileView;     // 正演场值文件"显示"按钮
    QLineEdit*  mEditModelPropertiesFile;    // 模型数据文件路径（默认 基准名 + "_Vol.vol"）
    QPushButton* mBtnOpenModelsPropertiesFile;   // 模型数据文件"..."按钮
    QPushButton* mBtnOpenModelsPropertiesFileView; // 模型数据文件"显示"按钮
    QLineEdit*  mEditInvertInfoFile;         // 反演信息文件路径（默认 基准名 + "_Inf.txt"）
    QPushButton* mBtnSaveInvertInfoFile;     // 反演信息文件"..."按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // "开始反演"按钮（初始隐藏，文件齐全后显示）
    QPushButton* mBtnCancel;         // "退出反演"按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mInputFilePath;          // 场值数据文件路径（对应原 m_strOriginalFieldsFileName）
    QString mSurfaceFilePath;        // 地形高程数据文件路径（对应原 m_strOriginalSurfaceFileName）
    QString mCommandFilePath;        // 控制参数文件路径（对应原 m_strCommandFileName）
    QString mForwardFieldsFilePath;  // 正演场值文件路径（对应原 m_strModelsFieldsFileName 绑定控件）
    QString mModelPropertiesFilePath;// 模型数据文件路径（对应原 m_strModelsPropertiesFileName 绑定控件）
    QString mInvertInfoFilePath;     // 反演信息文件路径（对应原 m_strInvertInformationFileName）
    QString mPrevModelsFilePath;     // 先前模型物性文件路径（对应原 m_strPreviousModelsPropertiesFileName）
    QString mFilePathBase;           // 输入文件去扩展名的基准名（原 m_strFilePath）
    QString mLoadedFilePath;         // 最近一次成功读取文件头的场值文件路径（防止重复加载）
    QStringList mCandidateFiles;     // 候选文件列表（对应原 m_strArrayFileNames）

    int     mRowsNum;        // 网格数据行数（对应原 m_nRows 显示值）
    int     mColsNum;        // 网格数据列数（对应原 m_nCols 显示值）
    double  mDataXMin;       // 场值数据 X 坐标最小值（原 fDataXmin）
    double  mDataXMax;       // 场值数据 X 坐标最大值（原 fDataXmax）
    double  mDataYMin;       // 场值数据 Y 坐标最小值（原 fDataYmin）
    double  mDataYMax;       // 场值数据 Y 坐标最大值（原 fDataYmax）

    double  mModXMin;        // 模型 X 坐标最小值（原 m_fModXmin）
    double  mModXMax;        // 模型 X 坐标最大值（原 m_fModXmax）
    double  mModYMin;        // 模型 Y 坐标最小值（原 m_fModYmin）
    double  mModYMax;        // 模型 Y 坐标最大值（原 m_fModYmax）
    double  mModZMin;        // 模型 Z 坐标最小值（原 m_fModZmin）
    double  mModZMax;        // 模型 Z 坐标最大值（原 m_fModZmax）
    double  mModXStep;       // 模型 X 方向网格距（原 m_fModXstep）
    double  mModYStep;       // 模型 Y 方向网格距（原 m_fModYstep）
    double  mModZStep;       // 模型 Z 方向网格距（原 m_fModZstep）

    bool    mGravityData;    // 是否为重力数据（对应原 m_bGravityData，默认 true）
    bool    mKm;             // 长度单位是否为公里（对应原 m_bKm，默认 false）
    double  mInclination;    // 磁化倾角（对应原 m_Inclination，默认 90）
    double  mDeclination;    // 磁化偏角（对应原 m_Declination，默认 0）
    double  mProfileDec;     // 测线方位角（对应原 m_DeclinationOfProfile，默认 90）
    double  mBaselineDec;    // 基线方位角（对应原 m_DeclinationOfBaseLine，默认 0）
    bool    mPrevModelsAsInit;   // 是否有先前模型物性分布作为初始模型（对应原 m_bPrevModelsAsInit，默认 false）

    CGravMagnVolumeInvProc* mProc;  // 反演处理流程类指针（"开始反演"时回调；可为 NULL）
};

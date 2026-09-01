// MagnGradCoImagingDlg.h : 三维磁异常和梯度相关成像对话框（Qt 前端）
//
// 对应原 MFC 工程：CMagnGradCoImagingDlg（IDD_MagnGradCoImaging）
// 界面布局依据 RGISGravMagnDataProcessing.rc 中 IDD_MagnGradCoImaging 绘制：
//   磁异常和梯度网格数据输入 / 网格数据信息 / 滤波选择 / 场值类型 / 磁场参数 /
//   处理参数（观测面高度、深度层起点/终点/间距）/
//   相关系数体数据文件输出（.vol）/ 相关成像信息文件输出（.log）
//
// 前端职责（本对话框只做界面与交互，算法/数据读写在后端）：
//   1. 输入数据文件的选择、网格信息显示、默认输出文件名生成
//      （按场值类型 _T/_X/_Y/_Z.vol 与 _T/_X/_Y/_Z.log）；
//   2. 场值类型单选（△T/△Tx/△Ty/△Tz，默认 △T）、滤波选择（默认进行熵滤波）、
//      磁场参数（地磁倾角/偏角、磁化倾角/偏角）、
//      观测面高度/深度层起点/终点/间距输入；
//   3. “确定”时校验输入（深度终点>起点、成像深度不超过网格最小尺寸的一半），
//      调用后端 processMagnGradCoImaging 完成处理（接口由后端提供后接入，
//      当前显示“后端未接入”占位提示）；
//   4. “显示”按钮请求查看等值线/体数据（通过 viewGridFileRequested /
//      viewVolumeFileRequested 信号交由宿主处理）。

#pragma once

#include <QDialog>

#include "backend/RgisBackend.h"

// 前置声明（减少头文件之间的相互包含）
class QLineEdit;
class QPushButton;
class QRadioButton;
class QDoubleSpinBox;
class QButtonGroup;

// 三维磁异常和梯度相关成像对话框
class CMagnGradCoImagingDlg : public QDialog
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：pParent —— 父窗口（可为 NULL）
    explicit CMagnGradCoImagingDlg(QWidget* pParent = nullptr);

    // 析构函数
    ~CMagnGradCoImagingDlg() override;

signals:
    // 功能：请求宿主显示指定网格文件的等值线图（对应原 MFC 工程“显示”按钮
    //       弹出的 CDrawOpenGLContourDlg，等值线显示窗口由前端后续版本实现）
    void viewGridFileRequested(const QString& strFilePath);

    // 功能：请求宿主显示指定体数据文件的三维视图（对应原 MFC 工程 CVolumeDataViewDlg，
    //       体数据显示窗口由前端后续版本实现）
    void viewVolumeFileRequested(const QString& strFilePath);

private slots:
    // ===== 输入/输出文件回调 =====
    void onOpenFileClicked();               // 功能：“...”选择输入数据文件
    void onOpenFileViewClicked();           // 功能：显示输入数据文件等值线
    void onOpenFileTextEdited(const QString& strText);  // 功能：手动输入文件名（同步成员变量）
    void onSaveFileClicked();               // 功能：“...”选择相关系数体数据文件（.vol）
    void onSaveFileViewClicked();           // 功能：显示相关系数体数据文件三维视图
    void onSaveFileTextEdited(const QString& strText);  // 功能：手动输入体数据文件名（同步成员变量）
    void onSaveInfoFileClicked();           // 功能：“...”选择相关成像信息文件（.log）
    void onSaveInfoFileViewClicked();       // 功能：显示相关成像信息文件（文本，由宿主处理）
    void onSaveInfoFileTextEdited(const QString& strText);  // 功能：手动输入信息文件名（同步成员变量）

    // ===== 场值类型单选回调 =====
    void onFieldTypeClicked(int nFieldTypeId);  // 功能：场值类型切换（同步成员变量并刷新默认输出名）

    // ===== 滤波选择单选回调 =====
    void onFilterTypeClicked(int nFilterTypeId);    // 功能：滤波选择切换（同步成员变量）

    // ===== 磁场参数回调 =====
    void onGeoInclineValueChanged(double dValue);   // 功能：地磁倾角变化（同步成员变量）
    void onGeoDeclineValueChanged(double dValue);   // 功能：地磁偏角变化（同步成员变量）
    void onMagInclineValueChanged(double dValue);   // 功能：磁化倾角变化（同步成员变量）
    void onMagDeclineValueChanged(double dValue);   // 功能：磁化偏角变化（同步成员变量）

    // ===== 处理参数回调 =====
    void onZObsValueChanged(double dValue);     // 功能：观测面高度变化（同步成员变量）
    void onLayStepValueChanged(double dValue);  // 功能：深度层间距变化（同步成员变量）
    void onZMinValueChanged(double dValue);     // 功能：深度层起点变化（同步成员变量）
    void onZMaxValueChanged(double dValue);     // 功能：深度层终点变化（同步成员变量）

    // ===== 确定 / 取消 =====
    void onOkClicked();                         // 功能：“确定”——校验参数并调用后端处理
    void onCancelClicked();                     // 功能：“取消”——关闭对话框

private:
    // 功能类型枚举（决定场值类型、滤波选择的取值）
    enum EFieldType { FieldTypeDt = 0, FieldTypeDtx = 1, FieldTypeDty = 2, FieldTypeDtz = 3 };
    enum EFilterType { FilterEntropy = 0, FilterNonEntropy = 1 };

    // ===== 界面初始化 =====
    void initUi();                              // 功能：按 .rc 布局创建全部控件并连接信号槽

    // ===== 数据加载 =====
    void loadFile(const QString& strFilePath);              // 功能：读取文件头并刷新网格信息、默认值
    void updateDefaultSavePaths(const QString& strFilePath);// 功能：按场值类型生成默认输出文件名
    void updateGridInfoDisplay();               // 功能：把网格信息成员变量刷到界面显示

    // ===== 文件选择 / 显示 =====
    QString askOpenFilePath(const QString& strTitle);       // 功能：弹出打开文件对话框
    QString askSaveFilePath(const QString& strTitle, const QString& strDefaultPath,
                            const QString& strFilter);      // 功能：弹出保存文件对话框
    void viewGridFile(const QString& strFilePath);          // 功能：检查文件存在并发出发送显示请求
    void viewVolumeFile(const QString& strFilePath);        // 功能：检查体数据文件存在并发出显示请求

    // ===== 处理 =====
    bool validateInputs(QString& strError);     // 功能：确定前校验输入项，返回 false 表示校验失败
    void runProcess();                          // 功能：调用后端 processMagnGradCoImaging（接口待后端提供）

    // ===== 界面控件（磁异常和梯度网格数据输入）=====
    QLineEdit*      mEditOpenFile;      // 输入数据文件路径（对应原 IDC_OpenFile）
    QPushButton*    mBtnOpenFile;       // 输入文件“...”按钮
    QPushButton*    mBtnOpenFileView;   // 输入文件“显示”按钮

    // ===== 界面控件（网格数据信息，只读显示）=====
    QLineEdit*  mEditRowsNum;       // 网格行数（只读）
    QLineEdit*  mEditColsNum;       // 网格列数（只读）
    QLineEdit*  mEditRowStep;       // 网格行距（只读）
    QLineEdit*  mEditColStep;       // 网格列距（只读）

    // ===== 界面控件（滤波选择，单选）=====
    QRadioButton*   mRadioEntropyFilter;     // 进行熵滤波（默认选中）
    QRadioButton*   mRadioNonEntropyFilter;  // 不进行熵滤波
    QButtonGroup*   mButtonGroupFilter;      // 滤波单选按钮组

    // ===== 界面控件（场值类型，单选）=====
    QRadioButton*   mRadioDt;        // △T（默认选中）
    QRadioButton*   mRadioDtx;       // △Tx
    QRadioButton*   mRadioDty;       // △Ty
    QRadioButton*   mRadioDtz;       // △Tz
    QButtonGroup*   mButtonGroupField;   // 场值类型单选按钮组

    // ===== 界面控件（磁场参数，对应 .rc 中 IDC_GeoIncline / IDC_GeoDecline /
    //           IDC_MagIncline / IDC_MagDecline）=====
    QDoubleSpinBox* mSpinGeoIncline;    // 地磁倾角（度，默认 90.0）
    QDoubleSpinBox* mSpinGeoDecline;    // 地磁偏角（度，默认 0.0）
    QDoubleSpinBox* mSpinMagIncline;    // 磁化倾角（度，默认 90.0）
    QDoubleSpinBox* mSpinMagDecline;    // 磁化偏角（度，默认 0.0）

    // ===== 界面控件（处理参数，对应 .rc 中 IDC_ZObs / IDC_LayStep / IDC_ZMin / IDC_ZMax）=====
    QDoubleSpinBox* mSpinZObs;       // 观测面高度（默认 0.0）
    QDoubleSpinBox* mSpinLayStep;    // 深度层间距（读文件后默认 (列距+行距)/2）
    QDoubleSpinBox* mSpinZMin;       // 深度层起点（读文件后默认 (列距+行距)/4）
    QDoubleSpinBox* mSpinZMax;       // 深度层终点（默认 0.0，需大于起点）

    // ===== 界面控件（相关系数体数据文件输出）=====
    QLineEdit*  mEditSaveFile;       // 相关系数体数据文件名（默认 _T/_X/_Y/_Z.vol）
    QPushButton* mBtnSaveFile;       // 体数据文件“...”按钮
    QPushButton* mBtnSaveFileView;   // 体数据文件“显示”按钮

    // ===== 界面控件（相关成像信息文件输出）=====
    QLineEdit*  mEditSaveInfoFile;   // 相关成像信息文件名（默认 _T/_X/_Y/_Z.log）
    QPushButton* mBtnSaveInfoFile;   // 信息文件“...”按钮
    QPushButton* mBtnSaveInfoFileView;   // 信息文件“显示”按钮

    // ===== 界面控件（确定 / 取消）=====
    QPushButton* mBtnOk;             // “确定”按钮（校验并调用后端处理）
    QPushButton* mBtnCancel;         // “取消”按钮（关闭对话框）

    // ===== 状态数据 =====
    QString mInputFilePath;          // 当前输入数据文件路径（对应原 m_strOpenFile）
    QString mOutputFilePath;         // 体数据输出文件路径（对应原 m_strSaveFile）
    QString mInfoFilePath;           // 信息输出文件路径（对应原 m_strSaveInfoFile）
    QString mFilePathBase;           // 输入文件去扩展名的基准名（原 m_strFilePath）
    QString mLoadedFilePath;         // 最近一次成功读取文件头的文件路径（防止重复加载）

    int     mRowsNum;       // 网格行数（原 m_nRowsNum）
    int     mColsNum;       // 网格列数（原 m_nColsNum）
    double  mColStep;       // 网格列距（原 m_fColStep）
    double  mRowStep;       // 网格行距（原 m_fRowStep）

    double  mGeoIncline;    // 地磁倾角（原 m_fGeoIncline，默认 90.0）
    double  mGeoDecline;    // 地磁偏角（原 m_fGeoDecline，默认 0.0）
    double  mMagIncline;    // 磁化倾角（原 m_fMagIncline，默认 90.0）
    double  mMagDecline;    // 磁化偏角（原 m_fMagDecline，默认 0.0）

    double  mZObs;          // 观测面高度（原 m_fZObs，默认 0.0）
    double  mLayStep;       // 深度层间距（原 m_fLayStep）
    double  mZMin;          // 深度层起点（原 m_fZMin）
    double  mZMax;          // 深度层终点（原 m_fZMax）

    int     mFieldType;     // 场值类型（0=△T、1=△Tx、2=△Ty、3=△Tz，默认 △T）
    int     mFilterType;    // 滤波选择（0=进行熵滤波（默认）、1=不进行熵滤波）
};

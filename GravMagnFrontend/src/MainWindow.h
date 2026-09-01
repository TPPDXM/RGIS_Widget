// MainWindow.h : 前端宿主主窗口（重磁数据预处理和处理）
//
// 功能：
//   按原 MFC 工程 TESTGravMagnDataProcessing 主对话框
//   （IDD_TESTGravMagnDataProcessing，见 TESTGravMagnDataProcessing.rc）
//   的布局实现主窗口：五组功能按钮（每组 8 列），右下角“退出”。
//   对应示例图片 example_img/MainWindow.png。
//
// 说明：
//   已实现的功能（补偿圆滑滤波、频率域向下延拓、频率域逐次向下延拓、
//   频率域总水平方向导数、频率域迭代曲化平、频率域迭代向下延拓、
//   频率域最大水平方向导数、频率域任意方向分量转换、频率域正则化滤波、
//   频率域一阶导数、频率域伪重力（磁源重力异常）、频率域剩余化极（化极）、
//   频率域构造(小子域滤波/线性构造增强)、频率域三分量转换、频率域总梯度(解析信号)、
//   频率域二阶导数、频率域向上延拓）
//   打开真实对话框，
//   其余功能按钮提示“尚未开发”（按迁移计划逐个推进后替换为真实对话框）。

#pragma once

#include <QMainWindow>

class QGridLayout;
class QVBoxLayout;

// 前端宿主主窗口（重磁数据预处理和处理）
class CGravMagnMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 构造函数
    // 参数：strFileNames —— 候选数据文件列表（分号分隔，可为空）
    explicit CGravMagnMainWindow(const QString& strFileNames);

    // 析构函数
    ~CGravMagnMainWindow() override;

private slots:
    void onOpenCmpsFilterClicked();     // 功能：打开“补偿圆滑滤波”（频率域组合滤波）对话框
    void onOpenDownwardClicked();       // 功能：打开“频率域向下延拓”对话框
    void onOpenGradwardClicked();       // 功能：打开“频率域逐次向下延拓(正则化滤波)”对话框
    void onOpenHorzGradClicked();       // 功能：打开“频率域总水平方向导数”对话框
    void onOpenIterDrapeClicked();      // 功能：打开“频率域迭代曲化平”对话框
    void onOpenIterwardClicked();       // 功能：打开“频率域迭代向下延拓”对话框
    void onOpenMaxiGradClicked();       // 功能：打开“频率域最大水平方向导数”对话框
    void onOpenMultiCpnClicked();       // 功能：打开“频率域任意方向分量转换(多分量)”对话框
    void onOpenNormFilterClicked();     // 功能：打开“频率域正则化滤波”对话框
    void onOpenOneDerivClicked();       // 功能：打开“频率域一阶导数”对话框
    void onOpenPsudoGraClicked();       // 功能：打开“频率域伪重力(磁源重力异常)”对话框
    void onOpenReToPoleClicked();       // 功能：打开“频率域剩余化极(化极)”对话框
    void onOpenStructureClicked();      // 功能：打开“频率域构造(小子域滤波/线性构造增强)”对话框
    void onOpenThreeCpnClicked();       // 功能：打开“频率域三分量转换”对话框
    void onOpenTotlGradClicked();       // 功能：打开“频率域总梯度(解析信号)”对话框
    void onOpenTwoDerivClicked();       // 功能：打开“频率域二阶导数”对话框
    void onOpenUpwardClicked();         // 功能：打开“频率域向上延拓”对话框
    void onOpenGravGradCoImagingClicked(); // 功能：打开“三维重力异常和梯度相关成像”对话框
    void onOpenGravMagnVolumeInvClicked();  // 功能：打开“重磁三维体反演(网格文件名对话框)”流程
    void onOpenGravMidTerrainClicked();   // 功能：打开“重力中区地形改正”对话框
    void onOpenGravUnionTerrainClicked(); // 功能：打开“重力联合(平面带)地形改正”对话框
    void onOpenMagnGradCoImagingClicked();// 功能：打开“三维磁异常和梯度相关成像”对话框
    void onOpenMagnIntensityClicked();    // 功能：打开“磁化强度计算”对话框
    void onOpenGridDataRecoveryClicked(); // 功能：打开“网格数据空白区还原”对话框
    void onNotImplementedClicked();     // 功能：提示该功能前端尚未开发（未实现功能统一回调）
    void onExitClicked();               // 功能：“退出”按钮——关闭主窗口退出程序
    void onViewGridFileRequested(const QString& strFilePath);  // 功能：处理对话框的“显示”请求（等值线显示由后续版本实现）
    void onViewVolumeFileRequested(const QString& strFilePath); // 功能：处理对话框的体数据“显示”请求（三维体数据视图由后续版本实现）

private:
    // 功能类型枚举（决定按钮点击后打开哪个功能）
    enum EFunctionType
    {
        FunctionNotImplemented = 0,     // 该功能前端尚未开发
        FunctionCmpsFilter = 1,         // 补偿圆滑滤波（已实现）
        FunctionDownward = 2,           // 频率域向下延拓（已实现）
        FunctionGradward = 3,           // 频率域逐次向下延拓(正则化滤波)（已实现）
        FunctionHorzGrad = 4,           // 频率域总水平方向导数（已实现）
        FunctionIterDrape = 5,          // 频率域迭代曲化平（已实现）
        FunctionIterward = 6,           // 频率域迭代向下延拓（已实现）
        FunctionMaxiGrad = 7,           // 频率域最大水平方向导数（已实现）
        FunctionMultiCpn = 8,           // 频率域任意方向分量转换(多分量)（已实现）
        FunctionNormFilter = 9,         // 频率域正则化滤波（已实现）
        FunctionOneDeriv = 10,          // 频率域一阶导数（已实现）
        FunctionPsudoGra = 11,          // 频率域伪重力(磁源重力异常)（已实现）
        FunctionReToPole = 12,          // 频率域剩余化极(化极)（已实现）
        FunctionStructure = 13,         // 频率域构造(小子域滤波/线性构造增强)（已实现）
        FunctionThreeCpn = 14,          // 频率域三分量转换（已实现）
        FunctionTotlGrad = 15,          // 频率域总梯度(解析信号)（已实现）
        FunctionTwoDeriv = 16,          // 频率域二阶导数（已实现）
        FunctionUpward = 17,            // 频率域向上延拓（已实现）
        FunctionGravGradCoImaging = 18, // 三维重力异常和梯度相关成像（已实现）
        FunctionGravMagnVolumeInv = 19, // 重磁三维体反演（已实现）
        FunctionGravMidTerrain = 20,   // 重力中区地形改正（已实现）
        FunctionGravUnionTerrain = 21,  // 重力联合(平面带)地形改正（已实现）
        FunctionMagnGradCoImaging = 22, // 三维磁异常和梯度相关成像（已实现）
        FunctionMagnIntensity = 23,     // 磁化强度计算（已实现）
        FunctionGridDataRecovery = 24   // 网格数据空白区还原（已实现）
    };

    // ===== 界面初始化 =====
    void initUi();                                      // 功能：按参考布局创建全部功能按钮与分组
    QGridLayout* createGroupLayout(QVBoxLayout* pMainLayout, const QString& strTitle);  // 功能：创建一个功能分组（标题居中，返回按钮网格布局）
    void addFunctionButton(QGridLayout* pLayout, int nRow, int nCol,
                           const QString& strText, EFunctionType eType);   // 功能：在分组网格的指定位置添加功能按钮并连接回调

    // ===== 打开功能对话框 =====
    void openCmpsFilterDlg();                           // 功能：打开补偿圆滑滤波对话框（模态）
    void openDownwardDlg();                             // 功能：打开频率域向下延拓对话框（模态）
    void openGradwardDlg();                             // 功能：打开频率域逐次向下延拓(正则化滤波)对话框（模态）
    void openHorzGradDlg();                             // 功能：打开频率域总水平方向导数对话框（模态）
    void openIterDrapeDlg();                            // 功能：打开频率域迭代曲化平对话框（模态）
    void openIterwardDlg();                             // 功能：打开频率域迭代向下延拓对话框（模态）
    void openMaxiGradDlg();                             // 功能：打开频率域最大水平方向导数对话框（模态）
    void openMultiCpnDlg();                             // 功能：打开频率域任意方向分量转换(多分量)对话框（模态）
    void openNormFilterDlg();                           // 功能：打开频率域正则化滤波对话框（模态）
    void openOneDerivDlg();                             // 功能：打开频率域一阶导数对话框（模态）
    void openPsudoGraDlg();                             // 功能：打开频率域伪重力(磁源重力异常)对话框（模态）
    void openReToPoleDlg();                             // 功能：打开频率域剩余化极(化极)对话框（模态）
    void openStructureDlg();                            // 功能：打开频率域构造(小子域滤波/线性构造增强)对话框（模态）
    void openThreeCpnDlg();                             // 功能：打开频率域三分量转换对话框（模态）
    void openTotlGradDlg();                             // 功能：打开频率域总梯度(解析信号)对话框（模态）
    void openGravMidTerrainDlg();                       // 功能：打开重力中区地形改正对话框（模态）
    void openGravUnionTerrainDlg();                     // 功能：打开重力联合(平面带)地形改正对话框（模态）
    void openTwoDerivDlg();                             // 功能：打开频率域二阶导数对话框（模态）
    void openUpwardDlg();                               // 功能：打开频率域向上延拓对话框（模态）
    void openGridDataRecoveryDlg();                     // 功能：打开网格数据空白区还原对话框（模态）
    void openGravGradCoImagingDlg();                    // 功能：打开三维重力异常和梯度相关成像对话框（模态）
    void openGravMagnVolumeInvDlg();                    // 功能：打开重磁三维体反演参数设置对话框（模态）
    void openMagnGradCoImagingDlg();                    // 功能：打开三维磁异常和梯度相关成像对话框（模态）
    void openMagnIntensityDlg();                        // 功能：打开磁化强度计算对话框（模态）

    QString mStrFileNames;  // 候选数据文件列表（分号分隔，传递给功能对话框）
};

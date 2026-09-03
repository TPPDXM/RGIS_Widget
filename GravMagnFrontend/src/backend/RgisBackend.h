// RgisBackend.h : 前端与后端(算法核心)之间的对接接口
//
// 说明：
//   本文件是"前端(Qt 界面) —— 后端(数据处理算法)"的分界线。
//   前端只依赖本接口（IRgisBackend），不关心后端的具体实现方式；
//   后端(算法/数据层)只需实现本接口，并在宿主程序中通过
//   CBackendService::setRgisBackend() 注入即可完成对接。
//
//   为保证后端可以独立于 Qt 编译（例如以 DLL/静态库形式交付），
//   本文件不含任何 Qt 头文件，仅使用标准 C++ 类型。
//   文件路径统一使用 UTF-8 编码的 std::string 传递。
//
//   对应原 MFC 工程（RGISGravMagnDataProcessing）模块：
//   - readGridFileHead   —— 对应 CContourFile::Read / Get_FileHead（仅文件头）
//   - processCmpsFilter  —— 对应 CFreqDomainCmpsFilterDlg::OnOK
//   - processDownward    —— 对应 CFreqDomainDownwardDlg::OnOK

#pragma once

#include <string>
#include <vector>

//---------------------------------------------------------------------------
// 数据结构定义
//---------------------------------------------------------------------------

// 网格文件头信息（对应原 MFC 工程中的 ContourFileHead 结构体）
struct GridFileHead
{
    int    cols;    // 列数（X 方向数据点个数）
    int    rows;    // 行数（Y 方向数据线个数）
    double xMin;    // X 坐标最小值
    double xMax;    // X 坐标最大值
    double yMin;    // Y 坐标最小值
    double yMax;    // Y 坐标最大值
    double zMin;    // 数据值最小值
    double zMax;    // 数据值最大值

    // 构造函数：全部清零，防止未初始化的垃圾值
    GridFileHead()
        : cols(0)
        , rows(0)
        , xMin(0.0)
        , xMax(0.0)
        , yMin(0.0)
        , yMax(0.0)
        , zMin(0.0)
        , zMax(0.0)
    {
    }
};

// 数据扩边方法（对应原 MFC 工程 IDC_CosFun / IDC_AvgDif / IDC_InvPow / IDC_MinCrv 单选按钮）
enum ExpandMethod
{
    ExpandCosFun = 0,   // 余弦函数衰减
    ExpandAvgDif = 1,   // 平均值差分
    ExpandInvPow = 2,   // 反距离加权
    ExpandMinCrv = 3    // 最小曲率
};

// 后端调用返回的错误信息（code == 0 表示无错误）
struct BackendError
{
    int         code;       // 错误码（0 = 无错误，非 0 = 出错）
    std::string message;    // 错误描述（可直接显示给用户）

    BackendError()
        : code(0)
        , message("")
    {
    }
};

// 频率域组合滤波（补偿圆滑滤波）处理参数
// 对应原 MFC 工程 CFreqDomainCmpsFilterDlg 界面上的全部输入项
struct CmpsFilterParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string regionalFilePath;   // 区域场结果数据文件输出路径（对应“区域场数据文件输出”）
    std::string residualFilePath;   // 剩余场结果数据文件输出路径（对应“剩余场数据文件输出”）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组）
    int         comNum;             // 补偿因子（界面“补偿因子”，默认 20）
    float       beta;               // 指数因子（界面“指数因子”，默认 150.0）

    CmpsFilterParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , comNum(20)
        , beta(150.0f)
    {
    }
};

// 频率域向下延拓处理参数
// 对应原 MFC 工程 CFreqDomainDownwardDlg 界面上的全部输入项
struct DownwardParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（对应“数据文件输出”）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组）
    float       height;             // 延拓高度（界面“延拓高度”，向下延拓为正，默认 2 倍列距）

    DownwardParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , height(0.0f)
    {
    }
};

// 频率域梯度（梯度计算）处理参数
// 对应原 MFC 工程 CFreqDomainGradientDlg 界面上的全部输出项
struct GradientParams
{
    std::string inputFilePath;              // 输入网格数据文件路径（.grd）
    std::string horizontalGradientFilePath; // 水平梯度结果文件路径（对应“水平梯度”，默认 _HD.grd）
    std::string totalGradientFilePath;      // 总梯度结果文件路径（对应“总梯度”，默认 _FD.grd）
    std::string thetaFilePath;              // Theta 图结果文件路径（对应“Theta 图”，默认 _ZT.grd）
    std::string tiltFilePath;               // Tilt(斜)导数结果文件路径（对应“Tilt(斜)导数”，默认 _TD.grd）
    std::string tiltGradientFilePath;       // 斜导数的水平梯度结果文件路径（对应“斜导数的水平梯度”，默认 _TH.grd）
    int         exRows;                     // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;                     // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;              // 数据扩边方法（界面“数据扩边方法”单选组，仅 3 种）

    GradientParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
    {
    }
};

// 频率域逐次向下延拓（正则化滤波）的单个延拓步参数
// 对应原 MFC 工程 CFreqDomainGradwardDlg 中表格（MSFlexGrid）每一行的输入
struct GradwardStepParams
{
    float height;       // 该步的向下延拓高度增量（0 ~ 总延拓高度）
    int   scaleOfRow;   // 水平几何尺度（正则化滤波参数，>= 0）

    GradwardStepParams()
        : height(0.0f)
        , scaleOfRow(0)
    {
    }
};

// 频率域逐次向下延拓（正则化滤波）处理参数
// 对应原 MFC 工程 CFreqDomainGradwardDlg 界面上的全部输入项
struct GradwardParams
{
    std::string inputFilePath;              // 输入网格数据文件路径（.grd）
    std::string outputFilePath;             // 最终结果数据文件输出路径（默认 基准名 + "OutN.grd"）
    int         exRows;                     // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;                     // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;              // 数据扩边方法（界面“数据扩边方法”单选组，仅 3 种）
    float       maxHeight;                  // 总延拓高度（界面“延拓高度”，各步高度增量不得超过该值）
    std::vector<GradwardStepParams> steps;  // 逐次延拓各步参数（长度 = 延拓次数，由界面表格给定）

    GradwardParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , maxHeight(0.0f)
    {
    }
};

// 频率域总水平方向导数处理参数
// 对应原 MFC 工程 CFreqDomainHorzGradDlg 界面上的全部输入项
struct HorzGradParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_THDR.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）

    HorzGradParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
    {
    }
};

// 频率域迭代曲化平（迭代下延计算）处理参数
// 对应原 MFC 工程 CFreqDomainIterDrapeDlg 界面上的全部输入项
struct IterDrapeParams
{
    std::string inputFilePath;              // 重磁场值输入网格数据文件路径（.grd）
    std::string demFilePath;                // 地形高程输入网格数据文件路径（.grd）
    std::string outputFilePath;             // 曲化平结果数据文件输出路径（默认 基准名 + "_Pro.grd"）
    int         exRows;                     // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;                     // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;              // 数据扩边方法（界面“数据扩边方法”单选组，3 种）
    int         planeNums;                  // 切割平面数（界面“切割平面数”，默认 5）
    std::vector<float> planeAltitudes;      // 各切割平面高程（长度 = planeNums + 1，序号 0 ~ planeNums，
                                            //   由界面表格“平面高程”列给定；后端点高程 = 平面高程 - 平面 0 高程）
    float       demHMin;                    // 高程最小值（界面“高程最小值”，默认取地形文件 z 最小值）
    float       demHMax;                    // 高程最大值（界面“高程最大值”，默认取地形文件 z 最大值）
    float       iterError;                  // 迭代均方差阈值（界面“迭代均方差”，默认 0.5）
    float       iterFactor;                 // 迭代因子（界面“迭代因子”，默认 1.5）

    IterDrapeParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , planeNums(5)
        , demHMin(0.0f)
        , demHMax(0.0f)
        , iterError(0.5f)
        , iterFactor(1.5f)
    {
    }
};

// 频率域迭代向下延拓处理参数
// 对应原 MFC 工程 CFreqDomainIterwardDlg 界面上的全部输入项
struct IterwardParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "Pro.grd"，无下划线，与原工程一致）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）
    float       height;             // 延拓高度（界面“延拓高度”，读文件后默认 2 倍列距）
    float       iterError;          // 迭代均方差阈值（界面“迭代方差”，读文件后默认 (zmax-zmin)*0.01）
    float       iterFactor;         // 迭代因子（界面“迭代因子”，默认 1.5）
    int         maxIterTimes;       // 最大迭代次数（界面“迭代次数”，默认 100）

    IterwardParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , height(0.0f)
        , iterError(0.1f)
        , iterFactor(1.5f)
        , maxIterTimes(100)
    {
    }
};

// 频率域最大梯度（最大水平方向导数）处理参数
// 对应原 MFC 工程 CFreqDomainMaxiGradDlg 界面上的全部输入项
struct MaxiGradParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_Pro.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，仅 3 种，无最小曲率）

    MaxiGradParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
    {
    }
};

// 频率域任意方向分量转换（多分量）处理参数
// 对应原 MFC 工程 CFreqDomainMultiCpnDlg 界面上的全部输入项
struct MultiCpnParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_Pro.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）
    float       baseLine;           // 测线基线方位角（界面“基线方向”，度，原 DirectBaseline，默认 0.0）
    float       profile;            // 测线方向方位角（界面“测线方向”，度，原 DirectProfile，默认 90.0）
    float       transInc;           // 转换方向倾角（界面“转换倾角”，度，原 TransInc，默认 90.0）
    float       transDec;           // 转换方向偏角（界面“转换偏角”，度，原 TransDec，默认 0.0）
    float       dtd0;               // 磁化偏角（界面“磁化偏角”，度，原 dTD0，默认 0.0）
    float       dti0;               // 磁化倾角（界面“磁化倾角”，度，原 dTI0，默认 90.0）

    MultiCpnParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , baseLine(0.0f)
        , profile(90.0f)
        , transInc(90.0f)
        , transDec(0.0f)
        , dtd0(0.0f)
        , dti0(90.0f)
    {
    }
};

// 频率域正则化滤波处理参数
// 对应原 MFC 工程 CFreqDomainNormFilterDlg 界面上的全部输入项
struct NormFilterParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string regionalFilePath;   // 区域场结果数据文件输出路径（对应“区域场数据文件输出”，默认 基准名 + "_Reg.grd"）
    std::string residualFilePath;   // 剩余场结果数据文件输出路径（对应“剩余场数据文件输出”，默认 基准名 + "_Res.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）
    int         scaleOfRow;         // 水平几何尺度（界面“水平几何尺度”，原 ScaleOfRow，默认 10）

    NormFilterParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , scaleOfRow(10)
    {
    }
};

// 频率域一阶导数处理参数
// 对应原 MFC 工程 CFreqDomainOneDerivDlg 界面上的全部输入项
struct OneDerivParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_HDR_I.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）
    float       derivInc;           // 导数倾角（界面“导数倾角”，度，原 DerivInc，默认 0.0，范围 0~360）
    float       derivDec;           // 导数偏角（界面“导数偏角”，度，原 DerivDec，默认 0.0，范围 0~360）

    OneDerivParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , derivInc(0.0f)
        , derivDec(0.0f)
    {
    }
};

// 频率域伪重力（磁源重力异常）处理参数
// 对应原 MFC 工程 CFreqDomainPsudoGraDlg 界面上的全部输入项
struct PsudoGraParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_Psudo.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）
    float       profile;            // 测线方向（界面“测线方向”，度，原 DirectProfile，默认 90.0，范围 -360~360）
    float       baseLine;           // 基线方向（界面“基线方向”，度，原 DirectBaseline，默认 0.0，范围 -360~360）
    float       dti0;               // 磁化倾角（界面“磁化倾角”，度，原 dTI0，默认 90.0，范围 0~180）
    float       dtd0;               // 磁化偏角（界面“磁化偏角”，度，原 dTD0，默认 0.0，范围 -360~360）
    float       magnetization;      // 磁化强度（界面“磁化强度”，A/m，原 Magnetization，默认 1.0，范围 0~1e12）
    float       density;            // 剩余密度（界面“剩余密度”，g/cm^3，原 Density，默认 0.1，范围 -5~5）
    int         cosine;             // 衰减因子（界面“衰减因子”，原 nCosine，默认 1，范围 1~20）

    PsudoGraParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , profile(90.0f)
        , baseLine(0.0f)
        , dti0(90.0f)
        , dtd0(0.0f)
        , magnetization(1.0f)
        , density(0.1f)
        , cosine(1)
    {
    }
};

// 频率域剩余化极（化极）处理参数
// 对应原 MFC 工程 CFreqDomainReToPoleDlg 界面上的全部输入项
struct ReToPoleParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_RTP.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）
    float       profile;            // 测线方向（界面“测线方向”，度，原 DirectProfile，默认 90.0，范围 -360~360）
    float       baseLine;           // 基线方向（界面“基线方向”，度，原 DirectBaseline，默认 0.0，范围 -360~360）
    float       dti0;               // 磁化倾角（界面“磁化倾角”，度，原 dTI0，默认 90.0，范围 -90~90）
    float       dtd0;               // 磁化偏角（界面“磁化偏角”，度，原 dTD0，默认 0.0，范围 -360~360）

    ReToPoleParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , profile(90.0f)
        , baseLine(0.0f)
        , dti0(90.0f)
        , dtd0(0.0f)
    {
    }
};

// 频率域剩余磁化极（剩磁化极）处理参数
// 对应原 MFC 工程 CFreqDomainRmToPoleDlg 界面上的全部输入项
// 说明：原工程界面无“数据扩边方法”单选组（该参数由算法内部完成），本结构体亦不包含
struct RmToPoleParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_RTP.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    float       dti0;               // 地磁倾角（界面“地磁倾角”，度，原 dTI0，默认 60.0，范围 0~180）
    float       dtd0;               // 地磁偏角（界面“地磁偏角”，度，原 dTD0，默认 30.0，范围 -360~360）
    float       dti1;               // 磁化倾角（界面“磁化倾角”，度，原 dTI1，默认 60.0，范围 -90~90）
    float       dtd1;               // 磁化偏角（界面“磁化偏角”，度，原 dTD1，默认 30.0，范围 -360~360）

    RmToPoleParams()
        : exRows(0)
        , exCols(0)
        , dti0(60.0f)
        , dtd0(30.0f)
        , dti1(60.0f)
        , dtd1(30.0f)
    {
    }
};

// 频率域构造（小子域滤波/线性构造增强）处理参数
// 对应原 MFC 工程 CFreqDomainStructureDlg 界面上的全部输入项
// 说明：原工程界面无“数据扩边信息”“数据扩边方法”“处理参数”输入（算法为
//       空间域/频率域直接变换，扩边在算法内部完成），本结构体仅含输入输出文件
struct StructureParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_Str.grd"）

    StructureParams()
    {
    }
};

// 频率域三分量转换（磁数据三分量转换）处理参数
// 对应原 MFC 工程 CFreqDomainThreeCpnDlg 界面上的全部输入项
struct ThreeCpnParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_DT/_Za/_Hax/_Hay.grd"，随换算分量变化）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）
    int         oriDataType;        // 测量数据分量（界面“测量数据分量”单选组：0=ΔT、1=Za、2=Hax、3=Hay，默认 0）
    int         calDataType;        // 换算数据分量（界面“换算数据分量”单选组：0=ΔT、1=Za、2=Hax、3=Hay，默认 1）
    float       profile;            // 测线方向（界面“测线方向”，度，原 DirectProfile，默认 90.0，范围 -360~360）
    float       baseLine;           // 基线方向（界面“基线方向”，度，原 DirectBaseline，默认 0.0，范围 -360~360）
    float       dti0;               // 磁化倾角（界面“磁化倾角”，度，原 dTI0，默认 90.0，范围 0~180）
    float       dtd0;               // 磁化偏角（界面“磁化偏角”，度，原 dTD0，默认 0.0，范围 -360~360）

    ThreeCpnParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , oriDataType(0)
        , calDataType(1)
        , profile(90.0f)
        , baseLine(0.0f)
        , dti0(90.0f)
        , dtd0(0.0f)
    {
    }
};

// 频率域 Tilt 梯度（斜导数）处理参数
// 对应原 MFC 工程 CFreqDomainTiltGradDlg 界面上的全部输入项
struct TiltGradParams
{
    std::string inputFilePath;              // 输入网格数据文件路径（.grd）
    std::string tiltGradFilePath;           // Tilt 梯度（斜导数）结果文件路径（默认 基准名 + "Tdr.grd"）
    std::string tiltDxFilePath;             // 斜导数 X 方向导数结果文件路径（默认 基准名 + "Tdx.grd"）
    std::string tiltDyFilePath;             // 斜导数 Y 方向导数结果文件路径（默认 基准名 + "Tdy.grd"）
    std::string tiltHorzGradFilePath;       // 斜导数的水平梯度结果文件路径（默认 基准名 + "Thd.grd"）
    std::string tiltTiltFilePath;           // 斜导数的斜导数结果文件路径（默认 基准名 + "Ttd.grd"）
    int         exRows;                     // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;                     // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;              // 数据扩边方法（界面“数据扩边方法”单选组，仅 3 种，无最小曲率）
    bool        filterBefore;               // 梯度计算前是否滤波（界面“梯度计算前滤波/不滤波”，默认不滤波）
    int         comNum;                     // 补偿因子（滤波用，界面“补偿因子”，默认 20）
    float       beta;                       // 指数因子（滤波用，界面“指数因子”，默认 150.0）

    TiltGradParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , filterBefore(false)
        , comNum(20)
        , beta(150.0f)
    {
    }
};

// 频率域总梯度（解析信号）处理参数
// 对应原 MFC 工程 CFreqDomainTotlGradDlg 界面上的全部输入项
struct TotlGradParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_AS.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）

    TotlGradParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
    {
    }
};

// 频率域二阶导数处理参数
// 对应原 MFC 工程 CFreqDomainTwoDerivDlg 界面上的全部输入项
struct TwoDerivParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_HDR_II.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）
    float       derivInc;           // 导数倾角（界面“导数倾角”，度，原 DerivInc，默认 0.0，范围 0~360）
    float       derivDec;           // 导数偏角（界面“导数偏角”，度，原 DerivDec，默认 0.0，范围 0~360）

    TwoDerivParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , derivInc(0.0f)
        , derivDec(0.0f)
    {
    }
};

// 频率域向上延拓处理参数
// 对应原 MFC 工程 CFreqDomainUpwardDlg 界面上的全部输入项
struct UpwardParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_Pro.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    ExpandMethod expandMethod;      // 数据扩边方法（界面“数据扩边方法”单选组，4 种）
    float       height;             // 向上延拓高度（界面“延拓高度”，向上延拓为正，默认 2 倍列距，
                                    //   原工程 OnOK 对负数取绝对值，界面范围 0~1e7）

    UpwardParams()
        : exRows(0)
        , exCols(0)
        , expandMethod(ExpandCosFun)
        , height(0.0f)
    {
    }
};

// 重力中区地形改正处理参数
// 对应原 MFC 工程 CGravMidTerrainCorrectionDlg 界面上的全部输入项
struct MidTerrainParams
{
    std::string txtFilePath;      // 重力测点数据文件路径（.txt，界面“重力测点数据文件输入”）
    std::string grdFilePath;      // 地形高程网格数据文件路径（.grd，界面“高程网格数据文件输入”）
    std::string resFilePath;      // 地形改正结果数据文件输出路径（.txt，界面“地形改正结果数据文件输出”）
    float       startRadius;      // 起始半径（界面“起始半径”，原 m_startraid，默认 20）
    float       endRadius;        // 终止半径（界面“终止半径”，原 m_EndRaid，默认 500）
    float       rockDensity;      // 地壳密度（界面“地壳密度”，原 m_rockdentisy，默认 2.67）
    int         outerShape;       // 外接口形状（界面“外接口形状选择”，0=方形 1=圆形）
    int         innerShape;       // 内接口形状（界面“内接口形状选择”，0=方形 1=圆形）

    MidTerrainParams()
        : startRadius(20.0f)
        , endRadius(500.0f)
        , rockDensity(2.67f)
        , outerShape(0)
        , innerShape(0)
    {
    }
};

// 重力联合（平面带）地形改正处理参数
// 对应原 MFC 工程 CGravUnionTerrainCorrectionDlg 界面上的全部输入项
struct UnionTerrainParams
{
    std::string txtFilePath;      // 重力测点数据文件路径（.dat，界面“重力测点数据文件输入”）
    std::string grdFilePath;      // 地形高程网格数据文件路径（.grd，界面“高程网格数据文件输入”）
    std::string resFilePath;      // 地形改正结果数据文件输出路径（.dat，默认 测点文件基准 + "_Out" + 原扩展名）
    int         kindsType;        // 地形改正方法（0=常规计算 1=三观测列方差分解）
    int         shapeType;        // 地形改正形状（0=环形 1=回形 2=钱形 3=枷形）
    int         typesType;        // 地形改正类型（0=常规地形改正 1=陆岛地形改正 2=广义地形改正）
    float       minRadius;        // 地改内环半径（界面“地改内环半径”，原 m_fMinRads，默认 50）
    float       maxRadius;        // 地改外环半径（界面“地改外环半径”，原 m_fMaxRads，默认 2000）
    float       rockDensity;      // 地形改正密度（界面“地形改正密度”，原 m_fDensity，默认 2.67）
    int         aziNum1;          // 第一列方位数（界面“第一列方位数”，原 m_AziNum1，默认 72）
    int         aziNum2;          // 第二列方位数（界面“第二列方位数”，原 m_AziNum2，默认 36）
    int         aziNum3;          // 第三列方位数（界面“第三列方位数”，原 m_AziNum3，默认 24）

    UnionTerrainParams()
        : kindsType(0)
        , shapeType(0)
        , typesType(0)
        , minRadius(50.0f)
        , maxRadius(2000.0f)
        , rockDensity(2.67f)
        , aziNum1(72)
        , aziNum2(36)
        , aziNum3(24)
    {
    }
};

// 三维体数据（三维体数据视图用）
// 对应原 MFC 工程 CVolumeData 类（见 VolumeData.h）与 VolumeDataViewCtl 控件所加载的体数据
struct VolumeData
{
    int             layers;     // 层数（z 轴方向，原 m_nVol_layers）
    int             rows;       // 行数（y 轴方向，原 m_nVol_rows）
    int             cols;       // 列数（x 轴方向，原 m_nVol_columns）
    double          xMin;       // x 范围最小值（原 m_fVol_xmin）
    double          xMax;       // x 范围最大值（原 m_fVol_xmax）
    double          yMin;       // y 范围最小值（原 m_fVol_ymin）
    double          yMax;       // y 范围最大值（原 m_fVol_ymax）
    double          zMin;       // z 范围最小值（原 m_fVol_zmin）
    double          zMax;       // z 范围最大值（原 m_fVol_zmax）
    double          valueMin;   // 属性值最小值（原 m_fVol_minValue）
    double          valueMax;   // 属性值最大值（原 m_fVol_maxValue）
    std::vector<float> data;    // 属性数据体，长度 = layers * rows * cols，
                                //   索引 = (layer * rows + row) * cols + column（原 pppfVol_Properties[layer][row][column]）

    // 构造函数：全部清零，防止未初始化的垃圾值
    VolumeData()
        : layers(0)
        , rows(0)
        , cols(0)
        , xMin(0.0)
        , xMax(0.0)
        , yMin(0.0)
        , yMax(0.0)
        , zMin(0.0)
        , zMax(0.0)
        , valueMin(0.0)
        , valueMax(0.0)
    {
    }
};

//---------------------------------------------------------------------------

// 频率域 ΔZ 化极（Za 化极）处理参数
// 对应原 MFC 工程 CFreqDomainDzToPoleDlg 界面上的全部输入项
// 说明：原工程此对话框无“数据扩边方法”单选组（扩边由算法内部完成）
struct DzToPoleParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_RTP.grd"）
    int         exRows;             // 扩边后行数（2 的幂，由界面“扩边行数”给定）
    int         exCols;             // 扩边后列数（2 的幂，由界面“扩边列数”给定）
    float       dtd0;               // 磁化偏角（界面“磁化偏角”，度，原 dTD0，默认 30.0，范围 -360~360）
    float       dti0;               // 磁化倾角（界面“磁化倾角”，度，原 dTI0，默认 60.0，范围 -90~90）

    DzToPoleParams()
        : exRows(0)
        , exCols(0)
        , dtd0(30.0f)
        , dti0(60.0f)
    {
    }
};

// 频率域最大导数（最大水平方向导数）处理参数
// 对应原 MFC 工程 CFreqMaximumDerivativeDlg 界面上的全部输入项
// 说明：原工程此对话框无扩边信息（扩边由算法内部完成）
struct MaximumDerivativeParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_MaxHDR.grd"）
    int         azimuthStep;        // 水平方向角度间隔（界面“水平方向角度间隔”，度/步长，
                                    //   原 m_nAzimuthStep，默认 5，范围 1~20）

    MaximumDerivativeParams()
        : azimuthStep(5)
    {
    }
};

// 频率域归一化标准差（归一化标准方差）处理参数
// 对应原 MFC 工程 CFreqNormalizationSTDDlg 界面上的全部输入项
// 说明：原工程此对话框无扩边信息（扩边由算法内部完成）；算法经外部动态库
//       PfProcesses.dll 的 _PFPROCESSES@680 入口完成（iComType = 503）。
struct NormalizationSTDParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_NSTD.grd"）
    int         winRows;            // 窗口行数（界面“窗口行数”，原 m_nWinRows，默认 3）
    int         winCols;            // 窗口列数（界面“窗口列数”，原 m_nWinCols，默认 3）
    float       alfa;               // 正则化因子（界面“正则化因子”，原 m_fAlfa，默认 0.0001，范围 0~1）

    NormalizationSTDParams()
        : winRows(3)
        , winCols(3)
        , alfa(0.0001f)
    {
    }
};

// 频率域归一化标准差的垂直导数比（NVDR-of-NSTD）处理参数
// 对应原 MFC 工程 CFreqNVDRofNSTDDlg 界面上的全部输入项
// 说明：原工程此对话框无扩边信息（扩边由算法内部完成）；算法经外部动态库
//       PfProcesses.dll 的 _PFPROCESSES@680 入口完成（iComType = 504）。
struct NVDRofNSTDParams
{
    std::string inputFilePath;      // 输入网格数据文件路径（.grd）
    std::string outputFilePath;     // 处理结果数据文件输出路径（默认 基准名 + "_NVDR_NSTD.grd"）
    int         winRows;            // 窗口行数（界面“窗口行数”，原 m_nWinRows，默认 3）
    int         winCols;            // 窗口列数（界面“窗口列数”，原 m_nWinCols，默认 3）
    float       alfa;               // 正则化因子（界面“正则化因子”，原 m_fAlfa，默认 0.0001，范围 0~1）

    NVDRofNSTDParams()
        : winRows(3)
        , winCols(3)
        , alfa(0.0001f)
    {
    }
};

//---------------------------------------------------------------------------

// 后端算法接口（前端只通过本接口与后端交互，后端只需实现本接口即可对接）
class IRgisBackend
{
public:
    // 析构函数：虚析构，防止通过基类指针释放派生类对象时内存泄漏
    virtual ~IRgisBackend()
    {
    }

    // ===== 功能：读取网格数据文件头 =====
    // 用途：界面载入输入文件后，需要显示“网格数据信息”（行数/列数/行距/列距）
    //       并据此计算默认的扩边尺寸（2 的幂），本接口只返回文件头，不返回数据体。
    // 对应原 MFC 工程：CContourFile::Read + Get_FileHead
    // 支持格式（与原 MFC 工程一致）：
    //   DSAA  —— Surfer ASCII 网格文件（#GMD / #GEX 文本变体）
    //   DSBB  —— Surfer Binary 网格文件
    //   DSRB  —— Surfer 二进制（double 数据）网格文件
    //   #GMD / #GEX —— 其他二进制网格文件
    // 返回：true 成功（head 被填充）；false 失败（error 给出原因）
    virtual bool readGridFileHead(const std::string& filePath, GridFileHead& head, BackendError& error) = 0;

    // ===== 功能：频率域组合滤波（补偿圆滑滤波）处理 =====
    // 用途：完成“界面 —— 后端”衔接，处理流程（与原 MFC 工程 CFreqDomainCmpsFilterDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 MFC 工程 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols（原 CContourFile::CosFun_Expand2D /
    //      AvgDif_Expand2D / InvPow_Expand2D / MinCrv_Expand2D）；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 组合滤波算子（补偿圆滑滤波，参数 comNum / beta，原 FreqDomainCmpsFilterProcess）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 区域场 = 滤波结果，剩余场 = 原始数据 - 区域场；
    //   9. 缺失数据还原（原 MissingDataResume），写出两个结果文件：
    //      regionalFilePath（区域场）、residualFilePath（剩余场）。
    // 约定：
    //   - 结果文件建议按原 MFC 工程 CContourFile::Write 的 DSBB 格式写出；
    //   - 本函数为同步调用，前端在调用期间显示等待光标，请勿弹出阻塞式对话框；
    //   - 无论成功失败都不得抛出异常（严禁 try/catch），一律通过返回值 + BackendError 报告。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processCmpsFilter(const CmpsFilterParams& params, BackendError& error) = 0;

    // ===== 功能：频率域向下延拓处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainDownwardDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 向下延拓算子（参数 height，向下延拓为正，原 FreqDomainDownwardProcess）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processDownward(const DownwardParams& params, BackendError& error) = 0;

    // ===== 功能：频率域梯度（梯度计算）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainGradientDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 对扩边数据分别求三个方向的水平/垂直一阶导数（原 FreqDomainOneDerivCom：
    //      沿 X 方向 DerivDec=0/DerivInc=0、沿 Y 方向 DerivDec=90/DerivInc=0、
    //      垂直方向 DerivDec=0/DerivInc=90，经 FFT2D + FreqDomainOneDerivProcess）；
    //   5. 水平梯度 = sqrt(dX^2 + dY^2)（原 TwoArraySquare）；
    //   6. 总梯度 = sqrt(dX^2 + dY^2 + dZ^2)（原 TriArraySquare）；
    //   7. Theta 图 = 水平梯度 / 总梯度（原 TwoArrayDivide）；
    //   8. Tilt(斜)导数 = atan2(垂直导数, 水平梯度)（原 TwoArrayAtangle）；
    //   9. 斜导数的水平梯度 = 对 Tilt(斜)导数再求水平方向梯度（原 TwoArraySquare）；
    //  10. 各结果从扩边数据中截取原网格范围（原 GridDataExtractFFT），
    //      缺失数据还原（原 MissingDataResume）后分别写出：
    //      horizontalGradientFilePath、totalGradientFilePath、thetaFilePath、
    //      tiltFilePath、tiltGradientFilePath。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processGradient(const GradientParams& params, BackendError& error) = 0;

    // ===== 功能：频率域逐次向下延拓（正则化滤波）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainGradwardDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 对 steps 中的每一步依次处理（steps.size() = 延拓次数）：
    //      a. 正则化滤波（参数 scaleOfRow，原 FreqDomainNormFilterCom →
    //         FreqDomainNormFilterProcess）；
    //      b. 向下延拓（参数 height 为该步高度增量，原 FreqDomainDownwardCom →
    //         FreqDomainDownwardProcess）。
    //   5. 最终结果从扩边数据中截取原网格范围（原 GridDataExtractFFT），
    //      缺失数据还原（原 MissingDataResume）后写出 outputFilePath；
    //   6. 中间各步结果按输入文件基准名 + "Out1.grd" ~ "Out{步数-1}.grd" 写出
    //      （与原工程 SaveGridFile 命名一致），如输入 C:\a\test.grd →
    //      C:\a\testOut1.grd、testOut2.grd……，步数为 1 时不写中间结果。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processGradward(const GradwardParams& params, BackendError& error) = 0;

    // ===== 功能：频率域总水平方向导数处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainHorzGradDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 分别求 X 方向一阶导数（方向角 DerivDec=0/DerivInc=0）与
    //      Y 方向一阶导数（方向角 DerivDec=90/DerivInc=0）
    //      （原 FreqDomainOneDerivCom：扩边 + FFT2D + FreqDomainOneDerivProcess + 逆 FFT + 截取原网格）；
    //   5. 总水平方向导数 = sqrt(dX^2 + dY^2)（原 OnOK 中的平方和开方）；
    //   6. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processHorzGrad(const HorzGradParams& params, BackendError& error) = 0;

    // ===== 功能：频率域迭代向下延拓处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainIterwardDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 对当前场数据按 expandMethod 扩边到 exRows * exCols
    //      （原 FreqDomainUpwardCom：CosFun/AvgDif/InvPow/MinCrv 扩边）；
    //   4. 一次迭代（原 OnOK 中的 ContinueForward 循环）：
    //      a. 频率域向上延拓（FFT2D + FreqDomainUpForwrdProcess + 逆 FFT，
    //         参数 height 为延拓高度），截取原网格范围（原 GridDataExtractFFT）；
    //      b. 计算当前场与延拓结果的均方差（RMS，原 OnOK 中的平方和开方）；
    //      c. 若均方差 < iterError **且** 迭代次数 == maxIterTimes：结束迭代（原工程
    //         终止条件为两者同时满足，界面“迭代次数”为最大迭代次数上限）；
    //         否则当前场 += iterFactor * (当前场 - 延拓结果)（原 CalculateFittingSub +
    //         TwoArrayAddSacal），回到 a 继续下一次迭代。
    //   5. 缺失数据还原（原 MissingDataResume）后写出结果文件 outputFilePath
    //      （输出的是最终迭代场，原 SaveGridFile）。
    // 约定：
    //   - 同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）；
    //   - 原工程迭代循环无上限（goto ContinueForward），后端实现时建议设置
    //     安全迭代上限（如 10000 次），达到上限仍未满足终止条件按失败返回并在 error 中说明；
    //   - 实际迭代次数与最终均方差通过出参 iterations / finalError 返回，
    //     供前端显示原工程完成提示“迭代次数为 X , 迭代均方差为: Y , 迭代下延计算结束！”。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processIterward(const IterwardParams& params, int& iterations, float& finalError, BackendError& error) = 0;

    // ===== 功能：频率域最大梯度（最大水平方向导数）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainMaxiGradDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols（仅 3 种方法，无最小曲率）；
    //   4. 固定枚举 36 个方向（原工程 OnOK 硬编码 AngleStep=5、Layers=180/AngleStep=36，
    //      界面无方向步长输入，后端无需修改该参数）：
    //      对每个方向 k：k*5° 方向求一阶导数、k*5°+90° 方向求一阶导数
    //      （原 FreqDomainOneDerivCom：FFT2D + FreqDomainOneDerivProcess + 逆 FFT），
    //      得该方向的梯度模 = sqrt(dx²+dy²)；
    //   5. 各点取 36 个方向梯度模的最大值（原 OnOK 中的逐点取最大），写回 ResData；
    //   6. 缺失数据还原（原 MissingDataResume）后写出结果文件 outputFilePath
    //      （对应原工程提示“最大水平梯度计算结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processMaxiGrad(const MaxiGradParams& params, BackendError& error) = 0;

    // ===== 功能：频率域迭代曲化平（迭代下延计算）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainIterDrapeDlg::OnOK 一致）：
    //   1. 读取重磁场 inputFilePath 与地形高程 demFilePath 网格数据，
    //      校验两者行列数、坐标范围一致（不一致时报错返回）；
    //   2. 两者缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 迭代下延计算（原 OnOK 中的迭代循环，以下简称为“一次迭代”）：
    //      a. 当前重磁场按 expandMethod 扩边到 exRows * exCols；
    //      b. 对 i = 1 ~ planeNums 逐个切割平面：向下延拓高度 =
    //         planeAltitudes[i] - planeAltitudes[0]（原 FreqDomainDownwardCom：
    //         FFT2D + FreqDomainDownwardProcess + 逆 FFT），
    //         再截取原网格范围（原 GridDataExtractFFT）存入第 i 个平面；
    //      c. 沿平面高程序列对地形高程面插值（原 Interpolation3D，
    //         参数 demHMin/demHMax 与 demFilePath 的地形数据）；
    //      d. 计算拟合均方差（原 CalculateFittingError）；
    //      e. 若均方差 > iterError：当前重磁场 += iterFactor * (当前重磁场 - 插值结果)
    //         （原 CalculateFittingSub + TwoArrayAddSacal），回到 a 继续下一次迭代；
    //         否则执行第 4 步。
    //   4. 缺失数据还原（原 MissingDataResume）后写出结果文件 outputFilePath。
    // 约定：
    //   - 同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）；
    //   - 原工程迭代循环无上限（goto ContinueForward），后端实现时建议设置
    //     安全迭代上限（如 10000 次），达到上限仍未收敛按失败返回并在 error 中说明；
    //   - 迭代次数与最终均方差通过出参 iterations / finalError 返回，供前端显示
    //     （原工程完成提示“迭代次数为 X , 迭代均方差为: Y”）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processIterDrape(const IterDrapeParams& params, int& iterations, float& finalError, BackendError& error) = 0;

    // ===== 功能：频率域任意方向分量转换（多分量）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainMultiCpnDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 任意磁化方向分量转换算子（原 FreqDomainMultiCmpProcess，使用参数：
    //      dti0 磁化倾角、dtd0 磁化偏角、transInc 转换方向倾角、transDec 转换方向偏角、
    //      baseLine 测线基线方位角、profile 测线方向方位角）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“频率域任意方向分量转换结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processMultiCpn(const MultiCpnParams& params, BackendError& error) = 0;

    // ===== 功能：频率域正则化滤波处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainNormFilterDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 正则化滤波算子（参数 scaleOfRow 水平几何尺度，原 FreqDomainNormFilterProcess）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 区域场 = 滤波结果，剩余场 = 原始数据 - 区域场；
    //   9. 缺失数据还原（原 MissingDataResume），写出两个结果文件：
    //      regionalFilePath（区域场）、residualFilePath（剩余场）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processNormFilter(const NormFilterParams& params, BackendError& error) = 0;

    // ===== 功能：频率域一阶导数处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainOneDerivDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 方向导数算子（参数 derivInc 导数倾角、derivDec 导数偏角，原 FreqDomainOneDerivProcess）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“频率域一阶导数处理结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processOneDeriv(const OneDerivParams& params, BackendError& error) = 0;

    // ===== 功能：频率域伪重力（磁源重力异常）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainPsudoGraDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 伪重力（磁源重力异常）换算算子（原 FreqDomainPsudoGraProcess，使用参数：
    //      dti0 磁化倾角、dtd0 磁化偏角、baseLine 基线方位角、profile 测线方向方位角、
    //      density 剩余密度、magnetization 磁化强度、cosine 衰减因子）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“磁源重力异常换算结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processPsudoGra(const PsudoGraParams& params, BackendError& error) = 0;

    // ===== 功能：频率域剩余化极（化极）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainReToPoleDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 剩余化极算子（原 FreqDomainReToPoleProcess，使用参数：
    //      dti0 磁化倾角、dtd0 磁化偏角、baseLine 基线方位角、profile 测线方向方位角）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“化极处理结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processReToPole(const ReToPoleParams& params, BackendError& error) = 0;

    // ===== 功能：频率域剩余磁化极（剩磁化极）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainRmToPoleDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 数据一维化（原 FloatDataDimensionTranslation，网格按行展开为一维数组）；
    //   4. 剩磁化极计算（原工程经外部动态库 FDReRmToPole.dll 的 _FDRERMTOPOLE@64 入口，
    //      参数：行数/列数/列距/行距/dti0 地磁倾角/dtd0 地磁偏角/dti1 磁化倾角/dtd1 磁化偏角/
    //      log2(ExRows)/log2(ExCols)/ExRows/ExCols 等；后端实现时可桥接该 DLL 或等价重写）；
    //   5. 结果还原为二维网格（原 FloatDataDimensionTranslation 反向转换）；
    //   6. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“化极处理结束!”）。
    // 约定：
    //   - 同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）；
    //   - 原工程界面无“数据扩边方法”输入（扩边处理在算法内部完成）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processRmToPole(const RmToPoleParams& params, BackendError& error) = 0;

    // ===== 功能：频率域构造（小子域滤波/线性构造增强）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainStructureDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 线性构造增强计算（原工程经外部函数 MainProcess_2（Structures.h，
    //      入参 dataInput/dataOutput/rows/columns，0 失败 1 成功），后端实现时可
    //      等价重写该算法；原工程界面无扩边参数，扩边处理在算法内部完成）；
    //   4. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“线性构造增强处理结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processStructure(const StructureParams& params, BackendError& error) = 0;

    // ===== 功能：频率域三分量转换（磁数据三分量转换）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainThreeCpnDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 三分量转换算子（原 FreqDomainThreeCmpProcess，使用参数：
    //      oriDataType 测量数据分量（0=ΔT、1=Za、2=Hax、3=Hay）、
    //      calDataType 换算数据分量（0=ΔT、1=Za、2=Hax、3=Hay）、
    //      dti0 磁化倾角、dtd0 磁化偏角、baseLine 测线基线方位角、profile 测线方向方位角）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“频率域三分量转换结束!”，输出文件名默认随换算分量
    //      变化为 基准名 + "_DT/_Za/_Hax/_Hay.grd"）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processThreeCpn(const ThreeCpnParams& params, BackendError& error) = 0;

    // ===== 功能：频率域 Tilt 梯度（斜导数）计算处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainTiltGradDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols（仅 3 种方法，无最小曲率）；
    //   4. 若 filterBefore 为 true：先做补偿圆滑滤波（原 FreqDomainComFilterCom：
    //      FFT2D + FreqDomainCmpsFilterProcess，参数 comNum / beta + 逆 FFT）；
    //   5. 分别求 X 方向一阶导数（DerivDec=0/DerivInc=0）、Y 方向一阶导数
    //      （DerivDec=90/DerivInc=0）、垂直方向一阶导数（DerivDec=0/DerivInc=90）
    //      （原 FreqDomainOneDerivCom：FFT2D + FreqDomainOneDerivProcess + 逆 FFT）；
    //   6. 水平梯度 = sqrt(dX^2 + dY^2)（原 TwoArraySquare）；
    //      Tilt 梯度 = atan2(垂直导数, 水平梯度)（原 TwoArrayAtangle），
    //      截取原网格范围（原 GridDataExtractFFT）后缺失数据还原（原 MissingDataResume），
    //      写出 tiltGradFilePath；
    //   7. 若 filterBefore 为 true：对 Tilt 梯度再做一次补偿圆滑滤波；
    //   8. 对 Tilt 梯度求 X 方向导数 → tiltDxFilePath、
    //      Y 方向导数 → tiltDyFilePath、
    //      水平梯度 = sqrt(dX^2 + dY^2) → tiltHorzGradFilePath、
    //      斜导数的 Tilt 梯度 = atan2(垂直导数, 水平梯度) → tiltTiltFilePath
    //      （每步均截取原网格 + 缺失数据还原后写出）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processTiltGrad(const TiltGradParams& params, BackendError& error) = 0;

    // ===== 功能：频率域总梯度（解析信号）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainTotlGradDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 分别求 X 方向一阶导数（DerivDec=0/DerivInc=0）、Y 方向一阶导数
    //      （DerivDec=90/DerivInc=0）、垂直方向一阶导数（DerivDec=0/DerivInc=90）
    //      （原 FreqDomainOneDerivCom：按 expandMethod 扩边到 exRows * exCols +
    //      FFT2D + FreqDomainOneDerivProcess + 逆 FFT + 截取原网格）；
    //   4. 总梯度（解析信号）= sqrt(dX^2 + dY^2 + dZ^2)（原 OnOK 中的平方和开方）；
    //   5. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“解析信号计算结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processTotlGrad(const TotlGradParams& params, BackendError& error) = 0;

    // ===== 功能：频率域二阶导数处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainTwoDerivDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 二阶方向导数算子（参数 derivInc 导数倾角、derivDec 导数偏角，原 FreqDomainTwoDerivProcess）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“频率域二阶导数处理结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processTwoDeriv(const TwoDerivParams& params, BackendError& error) = 0;

    // ===== 功能：频率域向上延拓处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainUpwardDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按 expandMethod 对数据扩边到 exRows * exCols；
    //   4. 二维傅里叶正变换（原 CFastFourierTransform::FFT2D）；
    //   5. 向上延拓算子（参数 height 向上延拓高度，为正，原 FreqDomainUpForwrdProcess；
    //      原工程 OnOK 对传入高度取绝对值，界面输入范围 0~1e7 已保证非负）；
    //   6. 二维傅里叶逆变换（原 FFT2D）；
    //   7. 从扩边后数据中截取原网格范围（去边）；
    //   8. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“频率域向上延拓处理结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processUpward(const UpwardParams& params, BackendError& error) = 0;

    // ===== 功能：重力中区地形改正处理 =====
    // 用途：处理流程（与原 MFC 工程 CGravMidTerrainCorrectionDlg::OnOK 一致）：
    //   1. 读取 txtFilePath 重力测点数据（原 OnOpenFileTxt 解析：首行为列数/文件头，
    //      数据行含 x、y（、z）等），读取 grdFilePath 高程网格数据头；
    //   2. 校验：起始/终止半径 > 0、地壳密度 > 0、起始/终止半径能被高程网格距 hstep 整除
    //      （原工程 fmod 校验，余差 > 0.0001 时报错）；
    //   3. 判断各测点在高程数据范围内（超出范围的点剔除，记录到 resFilePath 同目录
    //      Exceptdata.txt，原工程 m_filenameExp）；
    //   4. 按外接口形状（outerShape：方形/圆形）计算外环 Cij 系数、内接口形状（innerShape）
    //      计算内环 Cij 系数（原工程 OnOK 中矩形/圆形接口的系数计算）；
    //   5. 逐测点计算地形改正：对环内节点加权（Aij = Cij * 密度 逐项累加，
    //      (1 - 1/sqrt(1+TempHR^2))），4 邻域节点双线性插值，(原 OnOK 循环)；
    //   6. 结果乘以 6.67 * 0.001 * hstep * hstep，写出 resFilePath；
    //   7. 超出范围的点记录到 Exceptdata.txt（原工程 m_filenameExp，位于 resFilePath 同目录）。
    // 约定：
    //   - 同步调用、不抛异常，一律通过返回值 + BackendError 报告；
    //   - 结果文件为文本格式（测点被改正后结果，逐行写出，与原工程一致）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processMidTerrain(const MidTerrainParams& params, BackendError& error) = 0;

    // ===== 功能：重力联合（平面带）地形改正处理 =====
    // 用途：处理流程（与原 MFC 工程 CGravUnionTerrainCorrectionDlg::OnOK 一致）：
    //   1. 读取 txtFilePath 重力测点数据（.dat），读取 grdFilePath 高程网格数据头；
    //   2. 按 kindsType（0=常规计算/1=三观测列方差分解）、shapeType
    //      （0=环形/1=回形/2=钱形/3=枷形）、typesType（0=常规地形改正/1=陆岛地形改正/
    //      2=广义地形改正）选择计算分支（原 OnOK 中 3 组单选按钮决定）；
    //   3. 使用 minRadius/maxRadius 地改内/外环半径、rockDensity 地形改正密度、
    //      aziNum1/2/3 三个方位列数进行分区带地形改正计算（原 OnOK 内 NFW/分带计算）；
    //   4. 写出 resFilePath（文本格式），原工程完成无固定提示。
    // 约定：同步调用、不抛异常，一律通过返回值 + BackendError 报告；结果文件为文本格式。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processUnionTerrain(const UnionTerrainParams& params, BackendError& error) = 0;

    // ===== 功能：频率域 ΔZ 化极（Za 化极）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqDomainDzToPoleDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 按原工程内部扩边规则对数据扩边到 exRows * exCols（原工程经外部动态库
    //      FDReRmToPole.dll 完成，后端实现时可桥接该 DLL 或等价重写；界面无扩边方法输入，
    //      扩边处理在算法内部完成）；
    //   4. ΔZ 化极计算（参数 dtd0 磁化偏角、dti0 磁化倾角，原工程经动态库 FDReRmToPole.dll 的
    //      入口处理）；结果还原为二维网格（原 FloatDataDimensionTranslation 反向转换）；
    //   5. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“化极处理结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processDzToPole(const DzToPoleParams& params, BackendError& error) = 0;

    // ===== 功能：频率域最大导数（最大水平方向导数）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqMaximumDerivativeDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 以 azimuthStep 为水平方向角度间隔（度），枚举多个方向求一阶导数并逐点取最大
    //      （原工程经外部动态库 PfProcesses.dll 的 _PFPROCESSES@680 入口处理，参数
    //      iAngleStep = azimuthStep；后端实现时可桥接该 DLL 或等价重写）；
    //   4. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“最大水平方向导数计算结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processMaximumDerivative(const MaximumDerivativeParams& params, BackendError& error) = 0;

    // ===== 功能：频率域归一化标准差（归一化标准方差）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqNormalizationSTDDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据（界面已校验：网格行列距需相等，
    //      否则前端给出“您读入的网格数据行列距不相等！”提示并不发起处理）；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 归一化标准差计算（原工程经外部动态库 PfProcesses.dll 的 _PFPROCESSES@680 入口完成，
    //      iComType = 503；参数：行数/列数/列距/行距/窗口行数 winRows/窗口列数 winCols/
    //      正则化因子 alfa 等；后端实现时可桥接该 DLL 或等价重写；界面无扩边输入，扩边在算法内部完成）；
    //   4. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“归一化标准方差计算结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processNormalizationSTD(const NormalizationSTDParams& params, BackendError& error) = 0;

    // ===== 功能：频率域归一化标准差的垂直导数比（NVDR-of-NSTD）处理 =====
    // 用途：处理流程（与原 MFC 工程 CFreqNVDRofNSTDDlg::OnOK 一致）：
    //   1. 读取 inputFilePath 网格数据（界面已校验：网格行列距需相等，
    //      否则前端给出“您读入的网格数据行列距不相等！”提示并不发起处理）；
    //   2. 缺失数据插值（原 CContourFile::MissingDataIntrepolation）；
    //   3. 归一化标准差的垂直导数比计算（原工程经外部动态库 PfProcesses.dll 的
    //      _PFPROCESSES@680 入口完成，iComType = 504；参数：行数/列数/列距/行距/
    //      窗口行数 winRows/窗口列数 winCols/正则化因子 alfa 等；后端实现时可桥接该 DLL
    //      或等价重写；界面无扩边输入，扩边在算法内部完成）；
    //   4. 缺失数据还原（原 MissingDataResume），写出结果文件 outputFilePath
    //      （对应原工程提示“归一化标准方差垂向导数计算结束!”）。
    // 约定：同 processCmpsFilter（同步调用、不抛异常、DSBB 格式写出）。
    // 返回：true 成功；false 失败（error 给出原因）
    virtual bool processNVDRofNSTD(const NVDRofNSTDParams& params, BackendError& error) = 0;

    // ===== 功能：读取三维体数据 =====
    // 用途：三维体数据视图对话框（原 MFC 工程 CVolumeDataViewDlg 内嵌 ActiveX 体渲染控件
    //       VolumeDataViewCtl 的 OnFileOpen）需要加载体数据文件进行 3D 显示。
    //       本接口返回体数据的尺寸（layers/rows/cols）、坐标范围（x/y/z min/max）、
    //       属性值范围（valueMin/valueMax）以及属性数据体 data（见 VolumeData 注释）。
    // 对应原 MFC 工程：VolumeDataViewCtl::OnFileOpen 内部读取体数据文件（原工程经 ActiveX 控件
    //       读取某三维体数据二进制格式），后端实现时应等价读取该格式并填充 VolumeData。
    // 约定：
    //   - 同步调用、不得抛出异常（严禁 try/catch），一律通过返回值 + BackendError 报告；
    //   - data 长度为 layers * rows * cols，顺序 (layer,row,column)，由后端负责读取并填充；
    //   - 若后端尚未接入，应返回 false 并给出“未接入”的错误信息（与其它占位实现一致）。
    // 返回：true 成功（vol 被填充）；false 失败（error 给出原因）
    virtual bool readVolumeData(const std::string& filePath, VolumeData& vol, BackendError& error) = 0;
};

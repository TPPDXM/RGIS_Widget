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

//---------------------------------------------------------------------------
// 后端接口定义
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
};

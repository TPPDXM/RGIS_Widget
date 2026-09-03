// NullBackend.cpp : 后端接口占位实现（界面联调用）
//
// 实现说明：.grd 文件头读取逻辑完全对照原 MFC 工程
//   CContourFile::AReadFile1 / AReadFile2 / AReadFile3 / AReadFile4，
//   保证界面显示的网格信息与原工程一致。

#include "NullBackend.h"

#include <cstdio>
#include <cstring>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

// 构造与析构：无资源需要管理
CNullRgisBackend::CNullRgisBackend()
{
}

CNullRgisBackend::~CNullRgisBackend()
{
}

// 功能：将 UTF-8 编码的路径转换为宽字符路径（Windows 下打开含中文的路径）
// 参数：utf8Path —— UTF-8 编码的路径字符串
// 参数：bufferSize —— 输出的宽字符缓冲区长度（含结尾的 '\0'）
// 返回：动态分配的宽字符路径（调用方负责释放）；失败返回 NULL
wchar_t* CNullRgisBackend::utf8ToWide(const std::string& utf8Path, size_t& bufferSize)
{
#ifdef _WIN32
    // 第一次调用只为获取所需缓冲区长度
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Path.c_str(), -1, NULL, 0);
    if (wideLen <= 0)
    {
        bufferSize = 0;
        return NULL;
    }

    wchar_t* pWide = new wchar_t[wideLen];
    if (pWide == NULL)
    {
        bufferSize = 0;
        return NULL;
    }

    MultiByteToWideChar(CP_UTF8, 0, utf8Path.c_str(), -1, pWide, wideLen);
    bufferSize = (size_t)wideLen;
    return pWide;
#else
    // 非 Windows 平台：直接使用 UTF-8 路径（Linux/macOS 默认使用 UTF-8 文件系统）
    (void)bufferSize;
    return NULL;
#endif
}

// 功能：按文件头魔数判断文件类型（返回 1~5，0 表示无法识别）
// 对应原 MFC 工程：CContourFile::AReadFileType
int CNullRgisBackend::detectFileType(const wchar_t* pWidePath)
{
    int nType = 0;

    FILE* fp = _wfopen(pWidePath, L"rb");
    if (fp == NULL)
    {
        return nType;
    }

    char head[4];
    size_t readCount = fread(head, sizeof(char), 4, fp);
    fclose(fp);
    if (readCount != 4)
    {
        return nType;
    }

    if (head[0] == 'D' && head[1] == 'S' && head[2] == 'A' && head[3] == 'A')
    {
        nType = 1;          // DSAA —— Surfer ASCII
    }
    else if (head[0] == 'D' && head[1] == 'S' && head[2] == 'B' && head[3] == 'B')
    {
        nType = 2;          // DSBB —— Surfer Binary
    }
    else if (head[0] == 'D' && head[1] == 'S' && head[2] == 'R' && head[3] == 'B')
    {
        nType = 3;          // DSRB —— Surfer Binary(double)
    }
    else if (head[0] == '#' && head[1] == 'G' && head[2] == 'M' && head[3] == 'D')
    {
        nType = 4;          // #GMD —— 自编二进制网格
    }
    else if (head[0] == '#' && head[1] == 'G' && head[2] == 'E' && head[3] == 'X')
    {
        nType = 5;          // #GEX —— 自编二进制网格
    }

    return nType;
}

// 功能：读取 Surfer ASCII 网格文件头（DSAA 文本格式）
// 布局：DSAA / 列数 行数 / xmin xmax / ymin ymax / zmin zmax / 数据……
bool CNullRgisBackend::readHeadFile1(const wchar_t* pWidePath, GridFileHead& head)
{
    FILE* fp = _wfopen(pWidePath, L"rt");
    if (fp == NULL)
    {
        return false;
    }

    char headToken[64] = { 0 };

    // 读取头一行（魔数 DSAA）与 列数/行数 / xmin xmax / ymin ymax / zmin zmax
    int cols = 0;
    int rows = 0;
    float xMin = 0.0f, xMax = 0.0f;
    float yMin = 0.0f, yMax = 0.0f;
    float zMin = 0.0f, zMax = 0.0f;

    int scanCount = fscanf(fp, "%63s %d %d %f %f %f %f %f %f",
        headToken, &cols, &rows, &xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
    fclose(fp);

    if (scanCount != 9)
    {
        return false;
    }

    head.cols = cols;
    head.rows = rows;
    head.xMin = xMin;
    head.xMax = xMax;
    head.yMin = yMin;
    head.yMax = yMax;
    head.zMin = zMin;
    head.zMax = zMax;

    if (head.cols <= 0 || head.rows <= 0)
    {
        return false;
    }

    return true;
}

// 功能：读取 Surfer Binary 网格文件头（DSBB 二进制格式）
// 布局：DSBB / unsigned short 列数 / unsigned short 行数 /
//       6 个 double（xmin xmax ymin ymax zmin zmax）/ 数据(float)……
bool CNullRgisBackend::readHeadFile2(const wchar_t* pWidePath, GridFileHead& head)
{
    FILE* fp = _wfopen(pWidePath, L"rb");
    if (fp == NULL)
    {
        return false;
    }

    char headMagic[4];
    unsigned short nCols = 0;
    unsigned short nRows = 0;
    double coords[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

    bool bOk = true;
    bOk = bOk && (fread(headMagic, sizeof(char), 4, fp) == 4);
    bOk = bOk && (fread(&nCols, sizeof(unsigned short), 1, fp) == 1);
    bOk = bOk && (fread(&nRows, sizeof(unsigned short), 1, fp) == 1);
    bOk = bOk && (fread(coords, sizeof(double), 6, fp) == 6);
    fclose(fp);

    if (!bOk)
    {
        return false;
    }
    if (headMagic[0] != 'D' || headMagic[1] != 'S' || headMagic[2] != 'B' || headMagic[3] != 'B')
    {
        return false;
    }

    head.cols = (int)nCols;
    head.rows = (int)nRows;
    head.xMin = coords[0];
    head.xMax = coords[1];
    head.yMin = coords[2];
    head.yMax = coords[3];
    head.zMin = coords[4];
    head.zMax = coords[5];

    if (head.cols <= 0 || head.rows <= 0)
    {
        return false;
    }

    return true;
}

// 功能：读取 DSRB 二进制网格文件头
// 布局（对照原 CContourFile::AReadFile3）：
//   DSRB / int _ / int _ / char[4] / int _ / int 行数 / int 列数 /
//   double xmin / double ymin / double dx / double dy / double zmin / double zmax /
//   double _ / double _ / char[4] / int _ / 数据(double)……
bool CNullRgisBackend::readHeadFile3(const wchar_t* pWidePath, GridFileHead& head)
{
    FILE* fp = _wfopen(pWidePath, L"rb");
    if (fp == NULL)
    {
        return false;
    }

    char headMagic[4];
    int tmpInt = 0;
    char tmpChars[5] = { 0 };
    int rows = 0;
    int cols = 0;
    double xMin = 0.0, yMin = 0.0, fDx = 0.0, fDy = 0.0;
    double zMin = 0.0, zMax = 0.0;
    double tmpD1 = 0.0, tmpD2 = 0.0;

    bool bOk = true;
    bOk = bOk && (fread(headMagic, sizeof(char), 4, fp) == 4);
    bOk = bOk && (fread(&tmpInt, sizeof(int), 1, fp) == 1);
    bOk = bOk && (fread(&tmpInt, sizeof(int), 1, fp) == 1);
    bOk = bOk && (fread(tmpChars, sizeof(char), 4, fp) == 4);
    bOk = bOk && (fread(&tmpInt, sizeof(int), 1, fp) == 1);
    bOk = bOk && (fread(&tmpInt, sizeof(int), 1, fp) == 1);
    rows = tmpInt;
    bOk = bOk && (fread(&tmpInt, sizeof(int), 1, fp) == 1);
    cols = tmpInt;
    bOk = bOk && (fread(&xMin, sizeof(double), 1, fp) == 1);
    bOk = bOk && (fread(&yMin, sizeof(double), 1, fp) == 1);
    bOk = bOk && (fread(&fDx, sizeof(double), 1, fp) == 1);
    bOk = bOk && (fread(&fDy, sizeof(double), 1, fp) == 1);
    bOk = bOk && (fread(&zMin, sizeof(double), 1, fp) == 1);
    bOk = bOk && (fread(&zMax, sizeof(double), 1, fp) == 1);
    bOk = bOk && (fread(&tmpD1, sizeof(double), 1, fp) == 1);
    bOk = bOk && (fread(&tmpD2, sizeof(double), 1, fp) == 1);
    bOk = bOk && (fread(tmpChars, sizeof(char), 4, fp) == 4);
    bOk = bOk && (fread(&tmpInt, sizeof(int), 1, fp) == 1);
    fclose(fp);

    if (!bOk)
    {
        return false;
    }
    if (headMagic[0] != 'D' || headMagic[1] != 'S' || headMagic[2] != 'R' || headMagic[3] != 'B')
    {
        return false;
    }

    head.cols = cols;
    head.rows = rows;
    head.xMin = xMin;
    head.xMax = xMin + (cols - 1) * fDx;
    head.yMin = yMin;
    head.yMax = yMin + (rows - 1) * fDy;
    head.zMin = zMin;
    head.zMax = zMax;

    if (head.cols <= 0 || head.rows <= 0)
    {
        return false;
    }

    return true;
}

// 功能：读取 #GMD / #GEX 二进制网格文件头
// 布局（对照原 CContourFile::AReadFile4）：
//   char[12] / char[20] / float xmin / float ymin / float _ / char[28] /
//   short 列数 / short 行数 / float dx / float dy / float _ /
//   float zmin / float zmax / long _ / 数据(float)……
bool CNullRgisBackend::readHeadFile4(const wchar_t* pWidePath, GridFileHead& head)
{
    FILE* fp = _wfopen(pWidePath, L"rb");
    if (fp == NULL)
    {
        return false;
    }

    char headChars[32] = { 0 };
    char descChars[32] = { 0 };
    float xMin = 0.0f, yMin = 0.0f, tmpF = 0.0f;
    short nCols = 0, nRows = 0;
    float fDx = 0.0f, fDy = 0.0f;
    float zMin = 0.0f, zMax = 0.0f;
    long tmpL = 0;

    bool bOk = true;
    bOk = bOk && (fread(headChars, sizeof(char), 12, fp) == 12);
    bOk = bOk && (fread(descChars, sizeof(char), 20, fp) == 20);
    bOk = bOk && (fread(&xMin, sizeof(float), 1, fp) == 1);
    bOk = bOk && (fread(&yMin, sizeof(float), 1, fp) == 1);
    bOk = bOk && (fread(&tmpF, sizeof(float), 1, fp) == 1);
    bOk = bOk && (fread(descChars, sizeof(char), 28, fp) == 28);
    bOk = bOk && (fread(&nCols, sizeof(short), 1, fp) == 1);
    bOk = bOk && (fread(&nRows, sizeof(short), 1, fp) == 1);
    bOk = bOk && (fread(&fDx, sizeof(float), 1, fp) == 1);
    bOk = bOk && (fread(&fDy, sizeof(float), 1, fp) == 1);
    bOk = bOk && (fread(&tmpF, sizeof(float), 1, fp) == 1);
    bOk = bOk && (fread(&zMin, sizeof(float), 1, fp) == 1);
    bOk = bOk && (fread(&zMax, sizeof(float), 1, fp) == 1);
    bOk = bOk && (fread(&tmpL, sizeof(long), 1, fp) == 1);
    fclose(fp);

    if (!bOk)
    {
        return false;
    }

    head.cols = (int)nCols;
    head.rows = (int)nRows;
    head.xMin = xMin;
    head.xMax = xMin + fDx * (nCols - 1);
    head.yMin = yMin;
    head.yMax = yMin + fDy * (nRows - 1);
    head.zMin = zMin;
    head.zMax = zMax;

    if (head.cols <= 0 || head.rows <= 0)
    {
        return false;
    }

    return true;
}

// ===== IRgisBackend 接口实现 =====

// 功能：读取网格数据文件头（仅文件头，不读取数据体）
bool CNullRgisBackend::readGridFileHead(const std::string& filePath, GridFileHead& head, BackendError& error)
{
    if (filePath.empty())
    {
        error.code = 1;
        error.message = "输入文件路径为空。";
        return false;
    }

    size_t bufferSize = 0;
    wchar_t* pWidePath = utf8ToWide(filePath, bufferSize);
    if (pWidePath == NULL)
    {
        error.code = 2;
        error.message = "输入文件路径转换失败。";
        return false;
    }

    int nType = detectFileType(pWidePath);
    if (nType == 0)
    {
        delete[] pWidePath;
        error.code = 3;
        error.message = "无法识别的网格文件格式（支持 DSAA / DSBB / DSRB / #GMD / #GEX）。";
        return false;
    }

    bool bOk = true;
    switch (nType)
    {
    case 1:
        bOk = readHeadFile1(pWidePath, head);
        break;
    case 2:
        bOk = readHeadFile2(pWidePath, head);
        break;
    case 3:
        bOk = readHeadFile3(pWidePath, head);
        break;
    case 4:
    case 5:
        bOk = readHeadFile4(pWidePath, head);
        break;
    default:
        bOk = false;
        break;
    }

    delete[] pWidePath;

    if (!bOk)
    {
        error.code = 4;
        error.message = "网格文件头读取失败。";
        return false;
    }

    error.code = 0;
    error.message = "";
    return true;
}

// 功能：频率域组合滤波（补偿圆滑滤波）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processCmpsFilter(const CmpsFilterParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域组合滤波（补偿圆滑滤波）功能等待后端实现。";
    return false;
}

// 功能：频率域向下延拓处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processDownward(const DownwardParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域向下延拓功能等待后端实现。";
    return false;
}

// 功能：频率域梯度（梯度计算）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processGradient(const GradientParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域梯度（梯度计算）功能等待后端实现。";
    return false;
}

// 功能：频率域逐次向下延拓（正则化滤波）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processGradward(const GradwardParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域逐次向下延拓（正则化滤波）功能等待后端实现。";
    return false;
}

// 功能：频率域总水平方向导数处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processHorzGrad(const HorzGradParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域总水平方向导数功能等待后端实现。";
    return false;
}

// 功能：频率域迭代曲化平（迭代下延计算）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processIterDrape(const IterDrapeParams& params, int& iterations, float& finalError, BackendError& error)
{
    (void)params;       // 参数暂不使用（占位实现）
    iterations = 0;     // 出参清零（占位实现不产生结果）
    finalError = 0.0f;  // 出参清零（占位实现不产生结果）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域迭代曲化平（迭代下延计算）功能等待后端实现。";
    return false;
}

// 功能：频率域迭代向下延拓处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processIterward(const IterwardParams& params, int& iterations, float& finalError, BackendError& error)
{
    (void)params;       // 参数暂不使用（占位实现）
    iterations = 0;     // 出参清零（占位实现不产生结果）
    finalError = 0.0f;  // 出参清零（占位实现不产生结果）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域迭代向下延拓功能等待后端实现。";
    return false;
}

// 功能：频率域最大梯度（最大水平方向导数）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processMaxiGrad(const MaxiGradParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域最大梯度（最大水平方向导数）功能等待后端实现。";
    return false;
}

// 功能：频率域任意方向分量转换（多分量）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processMultiCpn(const MultiCpnParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域任意方向分量转换（多分量）功能等待后端实现。";
    return false;
}

// 功能：频率域正则化滤波处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processNormFilter(const NormFilterParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域正则化滤波功能等待后端实现。";
    return false;
}

// 功能：频率域一阶导数处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processOneDeriv(const OneDerivParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域一阶导数功能等待后端实现。";
    return false;
}

// 功能：频率域伪重力（磁源重力异常）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processPsudoGra(const PsudoGraParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域伪重力（磁源重力异常）功能等待后端实现。";
    return false;
}

// 功能：频率域剩余化极（化极）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processReToPole(const ReToPoleParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域剩余化极（化极）功能等待后端实现。";
    return false;
}

// 功能：频率域剩余磁化极（剩磁化极）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processRmToPole(const RmToPoleParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域剩余磁化极（剩磁化极）功能等待后端实现。";
    return false;
}

// 功能：频率域构造（小子域滤波/线性构造增强）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processStructure(const StructureParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域构造（小子域滤波/线性构造增强）功能等待后端实现。";
    return false;
}

// 功能：频率域三分量转换（磁数据三分量转换）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processThreeCpn(const ThreeCpnParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域三分量转换（磁数据三分量转换）功能等待后端实现。";
    return false;
}

// 功能：频率域 Tilt 梯度（斜导数）计算处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processTiltGrad(const TiltGradParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域 Tilt 梯度（斜导数）计算功能等待后端实现。";
    return false;
}

// 功能：频率域总梯度（解析信号）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processTotlGrad(const TotlGradParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域总梯度（解析信号）功能等待后端实现。";
    return false;
}

// 功能：频率域二阶导数处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processTwoDeriv(const TwoDerivParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域二阶导数功能等待后端实现。";
    return false;
}

// 功能：频率域向上延拓处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processUpward(const UpwardParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域向上延拓功能等待后端实现。";
    return false;
}

// 功能：重力中区地形改正处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processMidTerrain(const MidTerrainParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：重力中区地形改正功能等待后端实现。";
    return false;
}

// 功能：重力联合（平面带）地形改正处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processUnionTerrain(const UnionTerrainParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：重力联合（平面带）地形改正功能等待后端实现。";
    return false;
}

// 功能：读取三维体数据（占位，等待正式算法后端实现）
bool CNullRgisBackend::readVolumeData(const std::string& filePath, VolumeData& vol, BackendError& error)
{
    (void)filePath; // 参数暂不使用（占位实现）
    (void)vol;      // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "数据后端尚未接入：三维体数据读取功能等待后端实现。";
    return false;
}

// 功能：频率域 ΔZ 化极（Za 化极）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processDzToPole(const DzToPoleParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域 ΔZ 化极（Za 化极）功能等待后端实现。";
    return false;
}

// 功能：频率域最大导数（最大水平方向导数）处理（占位，等待正式算法后端实现）
bool CNullRgisBackend::processMaximumDerivative(const MaximumDerivativeParams& params, BackendError& error)
{
    (void)params;   // 参数暂不使用（占位实现）

    error.code = 100;
    error.message = "算法后端尚未接入：频率域最大导数（最大水平方向导数）功能等待后端实现。";
    return false;
}

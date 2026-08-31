// NullBackend.h : 后端接口的占位实现（界面联调用）
//
// 功能：
//   1. 为实现完整的前端演示，本类实现了 readGridFileHead（.grd 文件头读取，
//      与原 MFC 工程 CContourFile 中 AReadFile1 ~ AReadFile4 保持一致），
//      使界面载入文件后能正确显示网格信息并计算默认扩边尺寸；
//   2. processCmpsFilter / processDownward 暂未实现，返回“后端未接入”错误，
//      等待正式算法后端（实现 IRgisBackend）接入。
//
// 注意：文件读取逻辑属于数据层，正式后端接入后应由后端统一实现，
//       本类中的读取实现仅用于界面联调与数据验证。

#pragma once

#include "RgisBackend.h"

// 后端占位实现类（默认后端，未注入真实后端时由 CBackendService 使用）
class CNullRgisBackend : public IRgisBackend
{
public:
    CNullRgisBackend();
    ~CNullRgisBackend() override;

    // ===== IRgisBackend 接口实现 =====
    bool readGridFileHead(const std::string& filePath, GridFileHead& head, BackendError& error) override;
    bool processCmpsFilter(const CmpsFilterParams& params, BackendError& error) override;
    bool processDownward(const DownwardParams& params, BackendError& error) override;
    bool processGradient(const GradientParams& params, BackendError& error) override;
    bool processGradward(const GradwardParams& params, BackendError& error) override;
    bool processHorzGrad(const HorzGradParams& params, BackendError& error) override;
    bool processIterDrape(const IterDrapeParams& params, int& iterations, float& finalError, BackendError& error) override;
    bool processIterward(const IterwardParams& params, int& iterations, float& finalError, BackendError& error) override;
    bool processMaxiGrad(const MaxiGradParams& params, BackendError& error) override;
    bool processMultiCpn(const MultiCpnParams& params, BackendError& error) override;
    bool processNormFilter(const NormFilterParams& params, BackendError& error) override;
    bool processOneDeriv(const OneDerivParams& params, BackendError& error) override;
    bool processPsudoGra(const PsudoGraParams& params, BackendError& error) override;
    bool processReToPole(const ReToPoleParams& params, BackendError& error) override;
    bool processRmToPole(const RmToPoleParams& params, BackendError& error) override;
    bool processStructure(const StructureParams& params, BackendError& error) override;
    bool processThreeCpn(const ThreeCpnParams& params, BackendError& error) override;
    bool processTiltGrad(const TiltGradParams& params, BackendError& error) override;
    bool processTotlGrad(const TotlGradParams& params, BackendError& error) override;
    bool processTwoDeriv(const TwoDerivParams& params, BackendError& error) override;
    bool processUpward(const UpwardParams& params, BackendError& error) override;

private:
    // ===== 私有工具函数（.grd 文件头解析，对应原 MFC 工程 CContourFile::AReadFile1~4）=====

    // 功能：按文件头魔数判断文件类型（返回 1~5，0 表示无法识别）
    // 对应原 MFC 工程：CContourFile::AReadFileType
    int detectFileType(const wchar_t* pWidePath);

    // 功能：读取 Surfer ASCII 网格文件头（DSAA 文本格式）
    bool readHeadFile1(const wchar_t* pWidePath, GridFileHead& head);

    // 功能：读取 Surfer Binary 网格文件头（DSBB 二进制格式）
    bool readHeadFile2(const wchar_t* pWidePath, GridFileHead& head);

    // 功能：读取 DSRB 二进制网格文件头
    bool readHeadFile3(const wchar_t* pWidePath, GridFileHead& head);

    // 功能：读取 #GMD / #GEX 二进制网格文件头
    bool readHeadFile4(const wchar_t* pWidePath, GridFileHead& head);

    // 功能：将 UTF-8 编码的路径转换为宽字符路径（Windows 下打开含中文的路径）
    static wchar_t* utf8ToWide(const std::string& utf8Path, size_t& bufferSize);
};

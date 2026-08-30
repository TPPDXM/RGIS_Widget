// ExtendGridSize.h : 网格扩边尺寸计算工具
//
// 功能：按原 MFC 工程（FreqDomainCmpsFilterDlg::ReadData / FreqDomainDownwardDlg::ReadData）
//       的规则计算默认扩边尺寸（2 的幂）：
//       1. 取 2 的幂 P，使 P 大于等于原始点数 n；
//       2. 若 (P - n) < P / 2（扩边区太小，影响 FFT 结果），则将 P 翻倍。
//       两个对话框共用本规则。

#pragma once

// 功能：计算默认扩边尺寸（最小的满足条件的 2 的幂）
// 参数：nPointCount —— 原始数据点数（行数或列数）
// 返回：默认扩边尺寸（>= 2，为 2 的幂）
inline int suggestExtendSize(int nPointCount)
{
    if (nPointCount <= 0)
    {
        return 2;
    }

    int nSize = 1;
    while (nSize < nPointCount)
    {
        nSize *= 2;
    }

    if ((nSize - nPointCount) < nSize / 2)
    {
        nSize *= 2;
    }

    return nSize;
}

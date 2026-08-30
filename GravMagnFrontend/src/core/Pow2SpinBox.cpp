// Pow2SpinBox.cpp : 2 的幂步进微调框实现

#include "Pow2SpinBox.h"

// 默认最大扩边尺寸（与原 MFC 工程 OnDeltaposSpinExCols 中 <= 65536 的约束一致）
static const int sMaxExtendSize = 65536;

// 构造函数：设置默认范围 [1, 65536] 与初始值
CPow2SpinBox::CPow2SpinBox(QWidget* pParent)
    : QSpinBox(pParent)
{
    setRange(1, sMaxExtendSize);
    setValue(1);
    setAlignment(Qt::AlignCenter);
}

// 功能：重写微调框的步进逻辑（向上翻倍、向下减半），不使用默认 +/-1 步进
// 参数：steps —— 步进方向（正数为向上步进）与步数
void CPow2SpinBox::stepBy(int steps)
{
    int curValue = value();

    if (steps > 0)
    {
        // 向上：翻倍（不超过上限 65536，与原工程一致）
        int nextValue = curValue * 2;
        setValue(nextValue > maximum() ? maximum() : nextValue);
    }
    else if (steps < 0)
    {
        // 向下：减半（不低于下限，即读文件时计算出的最小扩边尺寸，与原工程一致）
        int nextValue = curValue / 2;
        setValue(nextValue < minimum() ? minimum() : nextValue);
    }
}

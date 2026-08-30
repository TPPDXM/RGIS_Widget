// Pow2SpinBox.h : 2 的幂步进微调框（对应原 MFC 工程的“扩边行数/扩边列数”+ 上下微调钮）
//
// 功能：数值为 2 的幂的微调框。点击向上箭头数值翻倍，向下箭头数值减半，
//       对应原 MFC 工程 CFreqDomainCmpsFilterDlg::OnDeltaposSpinExCols 中的规则：
//       增加时 *2（上限 65536），减少时 /2（不得小于读文件时计算出的最小扩边尺寸）。

#pragma once

#include <QSpinBox>

// 2 的幂步进微调框控件
class CPow2SpinBox : public QSpinBox
{
    Q_OBJECT

public:
    // 构造函数：设置默认范围 [1, 65536] 与初始值
    explicit CPow2SpinBox(QWidget* pParent = nullptr);

protected:
    // 功能：重写微调框的步进逻辑（向上翻倍、向下减半），不使用默认 +/-1 步进
    // 参数：steps —— 步进方向（正数为向上步进）与步数
    void stepBy(int steps) override;
};

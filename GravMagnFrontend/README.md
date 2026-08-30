# GravMagnFrontend —— 重磁数据处理前端（Qt 5.15.2）

本工程是原 MFC 工程 `RGISGravMagnDataProcessing`（VC6/MFC）的 Qt 前端迁移工程，
遵循“**前端（界面）—— 后端（算法核心）**”分离的长期目标：

* **前端（本工程）**：只做界面与交互 —— 对话框布局（对照 `RGISGravMagnDataProcessing.rc`）、
  参数校验、默认值、与用户交互，以及向后端发起处理请求；
* **后端（算法核心，另行开发）**：数据文件读写、扩边、FFT、滤波/延拓等算法实现，
  只需实现 `src/backend/RgisBackend.h` 中定义的接口 `IRgisBackend` 即可对接。

---

## 一、目录结构

```
GravMagnFrontend/
├── CMakeLists.txt                     # CMake 工程文件（Qt Creator 可直接打开）
├── README.md                          # 本说明文件
├── data/                              # 联调用样例数据（sample_dsaa/dsbb/dsrb/gmd.grd）
└── src/
    ├── main.cpp                       # 程序入口（后端注入点）
    ├── MainWindow.h / .cpp            # 演示宿主主窗口（打开各功能对话框）
    ├── backend/
    │   ├── RgisBackend.h              # ★ 后端对接接口（纯 C++，无 Qt 依赖）
    │   ├── NullBackend.h / .cpp       # 占位后端（文件头可读，算法未实现）
    ├── core/
    │   ├── BackendService.h / .cpp    # 后端实例注册与获取（CBackendService）
    │   ├── BackendConvert.h           # QString <-> std::string(UTF-8) 转换
    │   ├── ExtendGridSize.h           # 默认扩边尺寸（2 的幂）计算
    │   └── Pow2SpinBox.h / .cpp       # 2 的幂步进微调框（扩边行/列数）
    └── dlg/
        ├── FreqDomainCmpsFilterDlg.h / .cpp   # ★ 频率域组合滤波（补偿圆滑滤波）
        ├── FreqDomainDownwardDlg.h / .cpp     # ★ 频率域向下延拓
        ├── FreqDomainGradientDlg.h / .cpp     # ★ 频率域梯度（梯度计算）
        ├── FreqDomainGradwardDlg.h / .cpp     # ★ 频率域逐次向下延拓（正则化滤波）
        ├── FreqDomainHorzGradDlg.h / .cpp     # ★ 频率域总水平方向导数
        ├── FreqDomainIterDrapeDlg.h / .cpp    # ★ 频率域迭代曲化平（迭代下延计算）
        ├── FreqDomainIterwardDlg.h / .cpp     # ★ 频率域迭代向下延拓
        ├── FreqDomainMaxiGradDlg.h / .cpp     # ★ 频率域最大水平方向导数
        ├── FreqDomainMultiCpnDlg.h / .cpp     # ★ 频率域任意方向分量转换（多分量）
        └── FreqDomainNormFilterDlg.h / .cpp   # ★ 频率域正则化滤波
```

## 二、已完成的界面（本次任务）

主窗口（`MainWindow.h/.cpp`）按原工程主对话框（`TESTGravMagnDataProcessing.rc` 的
`IDD_TESTGravMagnDataProcessing`，即 `example_img/MainWindow.png`）实现：
“重磁数据预处理和处理”窗口，五个功能分组（标题居中）、每组 8 列按钮共 80 个功能，
右下角“退出”按钮；已实现的功能打开真实对话框，其余按钮提示“尚未开发”。

| 对话框 | 对应 MFC 工程 | RC 对话框 | 说明 |
|--------|---------------|-----------|------|
| `CFreqDomainCmpsFilterDlg` | `CFreqDomainCmpsFilterDlg` | `IDD_FreqDomainCmpsFilter` | 频率域组合滤波（补偿圆滑滤波）：输入文件、网格信息、扩边行/列数（2 的幂微调）、扩边方法单选（余弦函数衰减/平均值差分/反距离加权/最小曲率）、补偿因子、指数因子、区域场/剩余场双输出 |
| `CFreqDomainDownwardDlg` | `CFreqDomainDownwardDlg` | `IDD_FreqDomainDownward` | 频率域向下延拓：输入文件、网格信息、扩边行/列数、扩边方法单选、延拓高度（默认 2 倍列距）、结果文件输出 |
| `CFreqDomainGradientDlg` | `CFreqDomainGradientDlg` | `IDD_FreqDomainGradient` | 频率域梯度（梯度计算）：输入文件、网格信息（2×2）、扩边行/列数、扩边方法单选（3 种）、水平梯度/总梯度/Theta 图/Tilt(斜)导数/斜导数水平梯度 5 个结果文件输出（默认 _HD/_FD/_ZT/_TD/_TH.grd） |
| `CFreqDomainGradwardDlg` | `CFreqDomainGradwardDlg` | `IDD_FreqDomainGradward` | 频率域逐次向下延拓（正则化滤波）：输入文件、网格信息、扩边行/列数、扩边方法单选（3 种）、延拓高度（默认 10 倍列距）、延拓次数、逐次延拓参数表格（序号/延拓高度/水平几何尺度，对应原 MSFlexGrid）、结果文件输出（默认 基准名+OutN.grd） |
| `CFreqDomainHorzGradDlg` | `CFreqDomainHorzGradDlg` | `IDD_FreqDomainHorzGrad` | 频率域总水平方向导数：输入文件、网格信息（2×2）、扩边行/列数、扩边方法单选（4 种）、结果文件输出（默认 _THDR.grd） |
| `CFreqDomainIterDrapeDlg` | `CFreqDomainIterDrapeDlg` | `IDD_FreqDomainIterDrape` | 频率域迭代曲化平（迭代下延计算）：重磁场值/地形高程双输入文件（行列数、坐标一致性校验）、网格信息与高程最小/最大值、扩边行/列数、扩边方法单选（3 种）、延拓高度/切割平面数/迭代因子/迭代均方差、切割平面高程表格（平面序号/平面高程，对应原 MSFlexGrid）、曲化平结果输出（默认 _Pro.grd） |
| `CFreqDomainIterwardDlg` | `CFreqDomainIterwardDlg` | `IDD_FreqDomainIterward` | 频率域迭代向下延拓：输入文件、网格信息、扩边行/列数、扩边方法单选（4 种）、延拓高度（默认 2 倍列距）/迭代方差（读文件后默认 (zmax-zmin)×0.01）/迭代因子/迭代次数（上限 100）、结果文件输出（默认 基准名+Pro.grd，无下划线）；完成后提示“迭代次数为 X , 迭代均方差为: Y” |
| `CFreqDomainMaxiGradDlg` | `CFreqDomainMaxiGradDlg` | `IDD_FreqDomainMaxiGrad` | 频率域最大水平方向导数（窄对话框）：输入文件、网格信息（2×2）、扩边行/列数、扩边方法单选（3 种，无最小曲率）、结果文件输出（默认 _Pro.grd）；方向步长 5 度/36 方向为原工程硬编码 |
| `CFreqDomainMultiCpnDlg` | `CFreqDomainMultiCpnDlg` | `IDD_FreqDomainMultiCpn` | 频率域任意方向分量转换（多分量）：输入文件、网格信息（2×2）、扩边行/列数、扩边方法单选（4 种）、测线方向/基线方向/磁化倾角/磁化偏角/转换倾角/转换偏角（默认 90/0/90/0/90/0，取值范围与原工程 DDV 一致）、结果文件输出（默认 _Pro.grd） |
| `CFreqDomainNormFilterDlg` | `CFreqDomainNormFilterDlg` | `IDD_FreqDomainNormFilter` | 频率域正则化滤波：输入文件、网格信息（2×2）、扩边行/列数、扩边方法单选（4 种）、水平几何尺度（默认 10，范围 1~999999）、区域场/剩余场双输出（默认 _Reg.grd / _Res.grd） |

对话框样式对照 `example_img/SubWindow.png`：分组框标题居中、只读信息框、
“...”/“显示”按钮、确定在左下角、取消在右下角、固定尺寸不可缩放。

行为与 MFC 原工程保持一致（已核对源码）：

* 默认输出文件名：输入文件 `xxx.grd` → `xxx_Reg.grd` / `xxx_Res.grd`（组合滤波）、`xxx_Pro.grd`（向下延拓）；
* 默认扩边尺寸：读文件后按原工程规则取 2 的幂（`ExtendGridSize.h`），微调框向上翻倍（上限 65536）、向下减半（不得小于该最小尺寸）；
* 默认扩边方法：余弦函数衰减；组合滤波默认补偿因子 20、指数因子 150；向下延拓默认高度 2 倍列距；
* “确定”后同步调用后端处理（等待光标），完成后提示并**保持对话框打开**（与原工程一致，点“取消”退出）；
* “显示”按钮：检查文件存在后发出 `viewGridFileRequested` 信号，由宿主处理（等值线显示窗口属前端后续版本实现）；
* 输入文件选择：下拉框列出宿主传入的候选文件（分号分隔），也可“...”浏览或手工输入。

## 三、后端对接说明（如何“留好接口”）

### 1. 接口定义（`src/backend/RgisBackend.h`，无 Qt 依赖）

```cpp
class IRgisBackend
{
public:
    // 读取 .grd 文件头（界面显示网格信息、计算默认扩边尺寸）
    virtual bool readGridFileHead(const std::string& filePath, GridFileHead& head,
                                  BackendError& error) = 0;

    // 频率域组合滤波（补偿圆滑滤波）
    virtual bool processCmpsFilter(const CmpsFilterParams& params, BackendError& error) = 0;

    // 频率域向下延拓
    virtual bool processDownward(const DownwardParams& params, BackendError& error) = 0;

    // 频率域梯度（梯度计算）
    virtual bool processGradient(const GradientParams& params, BackendError& error) = 0;

    // 频率域逐次向下延拓（正则化滤波）
    virtual bool processGradward(const GradwardParams& params, BackendError& error) = 0;

    // 频率域任意方向分量转换（多分量）
    virtual bool processMultiCpn(const MultiCpnParams& params, BackendError& error) = 0;

    // 频率域正则化滤波
    virtual bool processNormFilter(const NormFilterParams& params, BackendError& error) = 0;
};
```

* 所有路径为 **UTF-8 编码的 `std::string`**；
* 所有方法**同步调用、不许抛异常**，结果一律通过 `bool` + `BackendError`（错误码 + 错误描述）返回；
* 各 `Params` 结构体字段即界面输入项，含义见 `RgisBackend.h` 内注释；
* 各 process 方法的后端处理流程（与原 MFC 工程 `OnOK` 一致）已写在接口注释中：
  读数据 → 缺失数据插值 → 按方法扩边到 `exRows×exCols` → 二维 FFT → 算子
  （补偿圆滑滤波 / 向下延拓 / 梯度算子 / 正则化滤波+逐次延拓）→ 逆 FFT →
  截取原网格 → 缺失数据还原 → 写出结果文件。

### 2. 接入方式

* 新建一个类实现 `IRgisBackend`（可编译为 DLL / 静态库 / 直接并入）；
* 在 `src/main.cpp` 中注入：

```cpp
CBackendService::setRgisBackend(new CYourRgisBackend());
```

* 未注入时界面自动使用占位后端 `CNullRgisBackend`：`.grd` 文件头**真实可读**
  （DSAA / DSBB / DSRB / #GMD / #GEX，与原 MFC 工程 `CContourFile` 读取逻辑逐字段一致，
  已用 `data/` 下四种格式的样例文件验证通过），两个处理接口返回“未接入”错误，
  界面仍可完整联调（选文件、看网格信息、试参数、点确定看到明确提示）。

### 3. 前后端接口与 MFC 原工程对照

| 前端调用 | MFC 原工程实现 |
|----------|----------------|
| `IRgisBackend::readGridFileHead` | `CContourFile::Read` + `Get_FileHead` |
| `IRgisBackend::processCmpsFilter` | `CFreqDomainCmpsFilterDlg::OnOK`（含 `MissingDataIntrepolation`、扩边、`FFT2D`、`FreqDomainCmpsFilterProcess`、写文件） |
| `IRgisBackend::processDownward` | `CFreqDomainDownwardDlg::OnOK`（含 `FreqDomainDownwardProcess`） |
| `IRgisBackend::processGradient` | `CFreqDomainGradientDlg::OnOK`（含 `FreqDomainOneDerivCom` 三个方向求导、`TwoArraySquare`/`TriArraySquare`/`TwoArrayDivide`/`TwoArrayAtangle` 组合、写 5 个结果文件） |
| `IRgisBackend::processGradward` | `CFreqDomainGradwardDlg::OnOK`（含 `FreqDomainNormFilterCom` 正则化滤波 + `FreqDomainDownwardCom` 逐次延拓、中间结果 Out1..n-1.grd、写结果文件） |
| `IRgisBackend::processMultiCpn` | `CFreqDomainMultiCpnDlg::OnOK`（含 `FreqDomainMultiCmpProcess` 任意磁化方向分量转换、写结果文件） |
| `IRgisBackend::processNormFilter` | `CFreqDomainNormFilterDlg::OnOK`（含 `FreqDomainNormFilterProcess` 正则化滤波、区域场/剩余场写文件） |
| `viewGridFileRequested` 信号 | `CDrawOpenGLContourDlg` 等值线显示（前端后续版本实现） |

## 四、命名与代码风格

* 类名保留原工程 `C` 前缀（如 `CFreqDomainCmpsFilterDlg`），便于与 MFC 源码逐条对照；
* 函数与变量一律**驼峰命名**（`onOpenFileClicked`、`mInputFilePath`）；
* 所有回调均为**成员函数引用**（`connect(..., &类名::槽函数)`），**不使用 lambda**；
* **不使用 try/catch**，错误通过返回值 + `BackendError` / 消息框反馈；
* 成员函数与成员变量声明处均写明用途注释；
* 界面布局与文本直接对照 `RGISGravMagnDataProcessing.rc`。

## 五、编译方法

环境：Windows，Qt 5.15.2（`D:\Qt\5.15.2`），Visual Studio 2019/2022 或 MinGW 8.1。

* **Qt Creator**：直接打开 `CMakeLists.txt`，选择 5.15.2 套件，构建运行；
* **命令行（MSVC2019 x64）**：

```
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=D:/Qt/5.15.2/msvc2019_64
cmake --build build --config Release
```

* **命令行（MinGW 8.1）**：

```
cmake -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=D:/Qt/5.15.2/mingw81_64 ^
      -DCMAKE_MAKE_PROGRAM=D:/Qt/Tools/mingw810_64/bin/mingw32-make.exe
cmake --build build
```

* MSVC 编译选项已含 `/utf-8`（源码 UTF-8，中文注释/字符串不乱码）。

## 六、运行

```
GravMagnFrontend.exe [候选文件1;候选文件2;...]
```

* 主窗口“重磁数据预处理和处理”中，点击“频率域重磁数据处理”分组的
  “补偿圆滑滤波”“向下延拓”“逐次向下延拓”“迭代曲化平”“总水平方向导数”
  “任意方向分量转换”“正则化滤波”按钮打开对应对话框（其余功能提示“尚未开发”；
  “梯度计算”等对话框已完成前端但原工程主窗口无对应按钮，未接入）；
* 可先用 `data\sample_dsbb.grd` 等样例数据联调界面（选文件 → 网格信息自动填充）。

## 七、当前状态与后续任务

* ✅ 10 个频率域对话框前端（布局/交互/校验/默认值，与 .rc 对照）；
* ✅ 后端接口 `IRgisBackend`（10 个处理接口）+ 注入机制 + 占位后端（.grd 文件头四种格式已实现并验证）；
* ⬜ 后端算法实现（按 `RgisBackend.h` 契约实现 10 个 process 接口）；
* ⬜ 等值线显示窗口（对应 `CDrawOpenGLContourDlg`，“显示”按钮待实现，信号已留好）；
* ⬜ 其余 70 个功能对话框的同类前端迁移（按 `PfProcesses_DEPENDENCY.md` 推进）。

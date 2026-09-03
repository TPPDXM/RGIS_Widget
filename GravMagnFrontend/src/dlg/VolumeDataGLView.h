// VolumeDataGLView.h : 三维体数据 OpenGL 视图控件（Qt 前端）
//
// 功能：以三维体数据（VolumeData）为载体，用 OpenGL 渲染"三方向正交切片"。
//   对应原 MFC 工程中嵌在 CVolumeDataViewDlg 内的 ActiveX 体渲染控件
//   （VolumeDataViewCtl）的简化 Qt 重写。支持：
//     * 鼠标左键拖拽旋转、滚轮缩放；
//     * 体数据三方向（X/Y/Z）正交切片纹理透明叠合显示；
//     * 显示/隐藏体包围盒与坐标轴、线框/填充、六个面（三方向切片组）显隐、
//       Z 轴放大/缩小、恢复视角、自动旋转、随机方向切片、仅显示立方体等。
//
// 说明：数据由后端接口 readVolumeData 提供（见 RgisBackend.h / 调用方对话框），
//       本控件只负责渲染与交互，不含数据读取。

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QTimer>

#include "backend/RgisBackend.h"

class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLTexture;

// 三维体数据 OpenGL 视图控件
class CVolumeDataGLView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    // 构造函数
    explicit CVolumeDataGLView(QWidget* pParent = nullptr);
    // 析构函数
    ~CVolumeDataGLView() override;

    // 功能：加载体数据（由对话框在读体数据成功后调用），生成切片并刷新显示
    // 参数：vol —— 体数据（尺寸/范围/属性值/数据体）
    // 返回：true 成功；false 失败（数据为空或尺寸非法）
    bool setVolumeData(const VolumeData& vol);

    // 功能：清除当前体数据（显示空场景）
    void clearVolumeData();

    // ===== 显示开关（对应原 ActiveX 控件的各 OnDraw* 动作）=====
    void setLineMode(bool bLine);           // 功能：true=线框模式（切片只画边界/线框）
    void setFillMode(bool bFill);           // 功能：true=填充模式（半透明切片叠合，默认）
    void setShowTop(bool bShow);            // 功能：显示/隐藏 Z 方向（顶/底）切片组
    void setShowBottom(bool bShow);         // 功能：与 setShowTop 同组（顶/底）
    void setShowLeft(bool bShow);           // 功能：显示/隐藏 X 方向（左/右）切片组
    void setShowRight(bool bShow);          // 功能：与 setShowLeft 同组（左/右）
    void setShowFront(bool bShow);          // 功能：显示/隐藏 Y 方向（前/后）切片组
    void setShowBack(bool bShow);           // 功能：与 setShowFront 同组（前/后）
    void setShowAxis(bool bShow);           // 功能：显示/隐藏坐标轴
    void setLighting(bool bOn);             // 功能：光照/透明开关（true=半透明体，false=简化为恒亮）
    void setModelRotate(bool bRotate);      // 功能：true=自动旋转（定时器驱动）
    void setSliceMode(int nMode);           // 功能：切片模式 0=正交切片 1=随机方向 2=仅显示立方体
    void setViewSliceOrAll(bool bAll);      // 功能：true=显示全部方向切片，false=只显示 Z 方向切片
    void setZoomScaleByZ(float fScale);     // 功能：Z 轴方向缩放系数（0.2~5.0）
    void zoomIn();                          // 功能：整体放大
    void zoomOut();                         // 功能：整体缩小
    void upZScale();                        // 功能：Z 轴放大
    void downZScale();                      // 功能：Z 轴缩小
    void restoreView();                     // 功能：恢复默认视角（不旋转、居中、默认缩放）

    // ===== 显示状态查询（供对话框菜单做互斥/切换）=====
    bool lightingEnabled() const { return mLighting; }       // 功能：光照/透明开关当前值
    bool sliceOrAllEnabled() const { return mShowAllAxis; }  // 功能：是否显示全部方向切片
    bool rotateEnabled() const { return mRotateTimer != NULL && mRotateTimer->isActive(); }  // 功能：自动旋转是否开启
    bool topEnabled() const { return mShowTop; }             // 功能：顶（Z）切片组是否显示
    bool bottomEnabled() const { return mShowBottom; }       // 功能：底（Z）切片组是否显示
    bool leftEnabled() const { return mShowLeft; }           // 功能：左（X）切片组是否显示
    bool rightEnabled() const { return mShowRight; }         // 功能：右（X）切片组是否显示
    bool frontEnabled() const { return mShowFront; }         // 功能：前（Y）切片组是否显示
    bool backEnabled() const { return mShowBack; }           // 功能：后（Y）切片组是否显示

protected:
    // ===== QOpenGLWidget 覆写 =====
    void initializeGL() override;           // 功能：初始化 OpenGL（编译 shader、创建缓存）
    void resizeGL(int nWidth, int nHeight) override;  // 功能：窗口尺寸变化（更新投影纵横比）
    void paintGL() override;                // 功能：绘制体数据切片/包围盒/坐标轴
    void mousePressEvent(QMouseEvent* pEvent) override;   // 功能：记录拖拽起点
    void mouseMoveEvent(QMouseEvent* pEvent) override;    // 功能：拖拽旋转模型
    void wheelEvent(QWheelEvent* pEvent) override;        // 功能：滚轮缩放

private:
    // 功能：释放所有 OpenGL 资源（纹理/缓存/程序）
    void releaseGpuResources();
    // 功能：由体数据生成三方向切片纹理（在 initializeGL 后调用）
    void buildSliceTextures();
    // 功能：绘制体包围盒与坐标轴
    void drawBoxAndAxis(const QMatrix4x4& mvp);

    // 功能：取属性值 -> 伪彩色（RGBA 0~255），用于切片纹理着色
    static void valueToColor(float fValue, float fMin, float fMax, unsigned char* pRgba);

    // ===== OpenGL 对象 =====
    QOpenGLShaderProgram* mProgram;         // 着色器程序
    QOpenGLBuffer* mSliceBuffer;            // 切片 quad 顶点缓存（全部切片合并）
    QOpenGLBuffer* mBoxBuffer;              // 包围盒/坐标轴 线段顶点缓存
    QOpenGLVertexArrayObject* mSliceVao;    // 切片 VAO
    QOpenGLVertexArrayObject* mBoxVao;      // 线框 VAO
    std::vector<QOpenGLTexture*> mSliceTextures;  // 各切片纹理（顺序与 mSliceRanges 对齐）

    // 顶点大小与属性偏移（pos3 + uv2 + color3）
    static const int sVertStride = 8 * sizeof(float);

    // ===== 渲染切片描述（每切片：纹素从属 axis/索引）=====
    struct SSliceRange { int axis; int first; int count; };  // 某方向切片组在合并 VBO 中的范围
    std::vector<SSliceRange> mSliceRanges;  // 三个方向各自的范围
    int  mSliceCount;                       // 当前切片总数（=每方向切片数*方向数）

    // ===== 体数据与切片 =====
    VolumeData mVolume;                     // 加载体数据（拷贝的原始数据）
    bool   mHasData;                        // 是否已加载体数据
    bool   mTexturesBuilt;                  // 切片纹理是否已构建（GL 上下文就绪后延迟构建）
    int    mSlicesPerAxis;                  // 每方向切片数
    bool   mShowTop, mShowBottom;           // Z 方向切片组显隐（顶/底）
    bool   mShowLeft, mShowRight;           // X 方向切片组显隐（左/右）
    bool   mShowFront, mShowBack;           // Y 方向切片组显隐（前/后）
    bool   mShowAxis;                       // 坐标轴显隐
    bool   mLineMode;                       // 线框模式
    bool   mFillMode;                       // 填充模式
    bool   mLighting;                       // 光照/透明开关
    int    mSliceMode;                       // 切片模式（0=正交 1=随机 2=仅立方体）
    bool   mShowAllAxis;                    // 显示全部方向切片（否则仅 Z）

    // ===== 视图状态 =====
    float  mYaw;                            // 绕 Y 轴旋转角（度）
    float  mPitch;                          // 绕 X 轴旋转角（度）
    float  mZoom;                           // 整体缩放系数
    float  mZScale;                         // Z 轴方向缩放系数
    QPoint mLastPos;                        // 上次鼠标位置（拖拽）
    QTimer* mRotateTimer;                   // 自动旋转定时器
};

// VolumeDataGLView.cpp : 三维体数据 OpenGL 视图控件实现
//
// 说明：
//   * 现代 OpenGL 管线（version 3.3 core），用着色器 + VAO/VBO + 2D 纹理切片；
//   * 渲染"三方向正交切片"（沿 X/Y/Z 各自若干层，按属性值伪彩色映射为半透明纹理叠合）；
//   * 交互：左键拖拽旋转、滚轮缩放、Z 轴缩放、显示/隐藏片组、线框/填充、坐标轴、恢复视角等；
//   * 数据由后端接口 readVolumeData 提供，本控件不做数据读取。

#include "VolumeDataGLView.h"

#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QTimer>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <cstring>

// 着色器（GLSL 3.30 core；属性槽 0=位置 1=纹理坐标 2=颜色）
static const char sVertexShader[] =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec2 aUV;\n"
    "layout(location = 2) in vec3 aColor;\n"
    "uniform mat4 uMVP;\n"
    "out vec2 vUV;\n"
    "out vec3 vColor;\n"
    "void main() {\n"
    "    vUV = aUV;\n"
    "    vColor = aColor;\n"
    "    gl_Position = uMVP * vec4(aPos, 1.0);\n"
    "}\n";

static const char sFragmentShader[] =
    "#version 330 core\n"
    "in vec2 vUV;\n"
    "in vec3 vColor;\n"
    "uniform sampler2D uTex;\n"
    "uniform int uUseTex;\n"
    "uniform float uAlpha;\n"
    "uniform vec3 uColor;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    if (uUseTex == 1) {\n"
    "        vec4 c = texture(uTex, vUV);\n"
    "        fragColor = vec4(c.rgb, c.a * uAlpha);\n"
    "    } else {\n"
    "        fragColor = vec4(uColor, 1.0);\n"
    "    }\n"
    "}\n";

// 每方向切片数
static const int sSlicesPerAxis = 24;

// 构造函数：初始化显示开关与视图状态
CVolumeDataGLView::CVolumeDataGLView(QWidget* pParent)
    : QOpenGLWidget(pParent)
    , mProgram(NULL)
    , mSliceBuffer(NULL)
    , mBoxBuffer(NULL)
    , mSliceVao(NULL)
    , mBoxVao(NULL)
    , mSliceCount(0)
    , mHasData(false)
    , mTexturesBuilt(false)
    , mSlicesPerAxis(sSlicesPerAxis)
    , mShowTop(true)
    , mShowBottom(true)
    , mShowLeft(true)
    , mShowRight(true)
    , mShowFront(true)
    , mShowBack(true)
    , mShowAxis(true)
    , mLineMode(false)
    , mFillMode(true)
    , mLighting(true)
    , mSliceMode(0)
    , mShowAllAxis(true)
    , mYaw(45.0f)
    , mPitch(25.0f)
    , mZoom(1.0f)
    , mZScale(1.0f)
    , mRotateTimer(NULL)
{
    // 请求较新的 OpenGL 上下文（现代管线需 3.3 core；现代显卡驱动均支持）
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(fmt);

    // 自动旋转定时器（每 33ms 加 0.6 度）
    mRotateTimer = new QTimer(this);
    connect(mRotateTimer, &QTimer::timeout, this, [this]() { mYaw += 0.6f; update(); });
}

// 析构函数：释放资源
CVolumeDataGLView::~CVolumeDataGLView()
{
    makeCurrent();
    releaseGpuResources();
    doneCurrent();
}

// 功能：加载体数据，生成切片并刷新显示
bool CVolumeDataGLView::setVolumeData(const VolumeData& vol)
{
    if (vol.layers <= 0 || vol.rows <= 0 || vol.cols <= 0)
    {
        return false;
    }
    if ((int)vol.data.size() < vol.layers * vol.rows * vol.cols)
    {
        return false;
    }

    mVolume = vol;
    mHasData = true;
    mTexturesBuilt = false;     // 数据变化，需在 GL 就绪后重建切片纹理
    update();
    return true;
}

// 功能：清除当前体数据
void CVolumeDataGLView::clearVolumeData()
{
    mHasData = false;
    mTexturesBuilt = false;
    update();
}

// 功能：设置线框模式（true=切片只绘制边界线框）
void CVolumeDataGLView::setLineMode(bool bLine)
{
    mLineMode = bLine;
    mFillMode = !bLine;
    update();
}

// 功能：设置填充模式（默认）
void CVolumeDataGLView::setFillMode(bool bFill)
{
    mFillMode = bFill;
    mLineMode = !bFill;
    update();
}

// 功能：Z 方向（顶/底）切片组显隐
void CVolumeDataGLView::setShowTop(bool bShow) { mShowTop = bShow; update(); }
void CVolumeDataGLView::setShowBottom(bool bShow) { mShowBottom = bShow; update(); }
// 功能：X 方向（左/右）切片组显隐
void CVolumeDataGLView::setShowLeft(bool bShow) { mShowLeft = bShow; update(); }
void CVolumeDataGLView::setShowRight(bool bShow) { mShowRight = bShow; update(); }
// 功能：Y 方向（前/后）切片组显隐
void CVolumeDataGLView::setShowFront(bool bShow) { mShowFront = bShow; update(); }
void CVolumeDataGLView::setShowBack(bool bShow) { mShowBack = bShow; update(); }

// 功能：坐标轴显隐
void CVolumeDataGLView::setShowAxis(bool bShow) { mShowAxis = bShow; update(); }

// 功能：光照/透明开关（true=半透明体）
void CVolumeDataGLView::setLighting(bool bOn) { mLighting = bOn; update(); }

// 功能：自动旋转开关
void CVolumeDataGLView::setModelRotate(bool bRotate)
{
    if (bRotate)
    {
        mRotateTimer->start(33);
    }
    else
    {
        mRotateTimer->stop();
    }
    update();
}

// 功能：切片模式（0=正交 1=随机 2=仅立方体）
void CVolumeDataGLView::setSliceMode(int nMode)
{
    mSliceMode = nMode % 3;
    update();
}

// 功能：显示全部方向切片 / 仅 Z 方向
void CVolumeDataGLView::setViewSliceOrAll(bool bAll)
{
    mShowAllAxis = bAll;
    update();
}

// 功能：设置 Z 轴方向缩放系数
void CVolumeDataGLView::setZoomScaleByZ(float fScale)
{
    mZScale = fScale;
    if (mZScale < 0.2f) { mZScale = 0.2f; }
    if (mZScale > 5.0f) { mZScale = 5.0f; }
    update();
}

// 功能：整体放大
void CVolumeDataGLView::zoomIn()
{
    mZoom *= 1.15f;
    if (mZoom > 10.0f) { mZoom = 10.0f; }
    update();
}

// 功能：整体缩小
void CVolumeDataGLView::zoomOut()
{
    mZoom /= 1.15f;
    if (mZoom < 0.1f) { mZoom = 0.1f; }
    update();
}

// 功能：Z 轴放大
void CVolumeDataGLView::upZScale()
{
    setZoomScaleByZ(mZScale * 1.2f);
}

// 功能：Z 轴缩小
void CVolumeDataGLView::downZScale()
{
    setZoomScaleByZ(mZScale / 1.2f);
}

// 功能：恢复默认视角
void CVolumeDataGLView::restoreView()
{
    mYaw = 45.0f;
    mPitch = 25.0f;
    mZoom = 1.0f;
    mZScale = 1.0f;
    setModelRotate(false);
    update();
}

// 功能：释放所有 OpenGL 资源
void CVolumeDataGLView::releaseGpuResources()
{
    for (size_t i = 0; i < mSliceTextures.size(); i++)
    {
        if (mSliceTextures[i] != NULL)
        {
            mSliceTextures[i]->destroy();
            delete mSliceTextures[i];
        }
    }
    mSliceTextures.clear();
    mSliceRanges.clear();
    mSliceCount = 0;

    if (mSliceBuffer != NULL) { mSliceBuffer->destroy(); delete mSliceBuffer; mSliceBuffer = NULL; }
    if (mBoxBuffer != NULL) { mBoxBuffer->destroy(); delete mBoxBuffer; mBoxBuffer = NULL; }
    if (mSliceVao != NULL) { mSliceVao->destroy(); delete mSliceVao; mSliceVao = NULL; }
    if (mBoxVao != NULL) { mBoxVao->destroy(); delete mBoxVao; mBoxVao = NULL; }
    if (mProgram != NULL) { delete mProgram; mProgram = NULL; }
}

// 功能：OpenGL 初始化（编译着色器、创建缓存）
void CVolumeDataGLView::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.92f, 0.92f, 0.92f, 1.0f);

    // 编译着色器
    mProgram = new QOpenGLShaderProgram(this);
    mProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, sVertexShader);
    mProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, sFragmentShader);
    mProgram->link();
    mProgram->bind();

    // 切片顶点缓存 VAO
    mSliceVao = new QOpenGLVertexArrayObject(this);
    mSliceVao->create();
    mSliceVao->bind();
    mSliceBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    mSliceBuffer->create();
    mSliceBuffer->bind();
    mSliceBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    mProgram->enableAttributeArray(0);
    mProgram->enableAttributeArray(1);
    mProgram->enableAttributeArray(2);
    mProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sVertStride);
    mProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 2, sVertStride);
    mProgram->setAttributeBuffer(2, GL_FLOAT, 5 * sizeof(float), 3, sVertStride);
    mSliceVao->release();

    // 包围盒/坐标轴 线段缓存 VAO
    mBoxVao = new QOpenGLVertexArrayObject(this);
    mBoxVao->create();
    mBoxVao->bind();
    mBoxBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    mBoxBuffer->create();
    mBoxBuffer->bind();
    mBoxBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    mProgram->enableAttributeArray(0);
    mProgram->enableAttributeArray(1);
    mProgram->enableAttributeArray(2);
    mProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sVertStride);
    mProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 2, sVertStride);
    mProgram->setAttributeBuffer(2, GL_FLOAT, 5 * sizeof(float), 3, sVertStride);
    mBoxVao->release();

    // 构建包围盒/坐标轴顶点（24 + 6 顶点，含颜色）
    {
        float f = 1.0f;
        // 12 条立方体棱（每条两端点）
        float boxPos[24][3] = {
            {-f,-f,-f},{ f,-f,-f},{ f,-f,-f},{ f,-f, f},{ f,-f, f},{-f,-f, f},{-f,-f, f},{-f,-f,-f},
            {-f, f,-f},{ f, f,-f},{ f, f,-f},{ f, f, f},{ f, f, f},{-f, f, f},{-f, f, f},{-f, f,-f},
            {-f,-f,-f},{-f, f,-f},{ f,-f,-f},{ f, f,-f},{ f,-f, f},{ f, f, f},{-f,-f, f},{-f, f, f}
        };
        std::vector<float> boxVert;
        for (int i = 0; i < 24; i++)
        {
            boxVert.push_back(boxPos[i][0]); boxVert.push_back(boxPos[i][1]); boxVert.push_back(boxPos[i][2]);
            boxVert.push_back(0.0f); boxVert.push_back(0.0f);            // uv
            boxVert.push_back(0.35f); boxVert.push_back(0.35f); boxVert.push_back(0.35f); // 深灰
        }
        // 坐标轴 3 条线（X红 Y绿 Z蓝）
        float axis[6][3] = { {0,0,0},{ f,0,0}, {0,0,0},{0, f,0}, {0,0,0},{0,0, f} };
        float axisCol[3][3] = { {1.f,0,0}, {0,1.f,0}, {0,0,1.f} };
        for (int i = 0; i < 6; i++)
        {
            boxVert.push_back(axis[i][0]); boxVert.push_back(axis[i][1]); boxVert.push_back(axis[i][2]);
            boxVert.push_back(0.0f); boxVert.push_back(0.0f);
            boxVert.push_back(axisCol[i / 2][0]); boxVert.push_back(axisCol[i / 2][1]); boxVert.push_back(axisCol[i / 2][2]);
        }
        mBoxBuffer->allocate(boxVert.data(), boxVert.size() * sizeof(float));
        mBoxVao->release();
    }
}

// 功能：窗口尺寸变化（更新投影纵横比）
void CVolumeDataGLView::resizeGL(int nWidth, int nHeight)
{
    glViewport(0, 0, nWidth, nHeight);
}

// 功能：属性值 -> 伪彩色（RGBA 0~255）
void CVolumeDataGLView::valueToColor(float fValue, float fMin, float fMax, unsigned char* pRgba)
{
    float t = 0.0f;
    if (fMax > fMin)
    {
        t = (fValue - fMin) / (fMax - fMin);
    }
    if (t < 0.0f) { t = 0.0f; }
    if (t > 1.0f) { t = 1.0f; }

    // 蓝->青->绿->黄->红 的简单彩虹映射
    float r = 0.0f, g = 0.0f, b = 0.0f;
    if (t < 0.25f) { r = 0.0f; g = t * 4.0f; b = 1.0f; }
    else if (t < 0.5f) { r = 0.0f; g = 1.0f; b = 1.0f - (t - 0.25f) * 4.0f; }
    else if (t < 0.75f) { r = (t - 0.5f) * 4.0f; g = 1.0f; b = 0.0f; }
    else { r = 1.0f; g = 1.0f - (t - 0.75f) * 4.0f; b = 0.0f; }

    pRgba[0] = (unsigned char)(r * 255.0f);
    pRgba[1] = (unsigned char)(g * 255.0f);
    pRgba[2] = (unsigned char)(b * 255.0f);
    pRgba[3] = 200;    // 不透明度
}

// 功能：由体数据生成三方向切片纹理 + 合并顶点缓存（需 GL 上下文就绪后调用）
void CVolumeDataGLView::buildSliceTextures()
{
    // 清空旧资源（重复构建时）
    for (size_t i = 0; i < mSliceTextures.size(); i++)
    {
        if (mSliceTextures[i] != NULL) { mSliceTextures[i]->destroy(); delete mSliceTextures[i]; }
    }
    mSliceTextures.clear();
    mSliceRanges.clear();

    std::vector<float> verts;      // 合并到切片 VBO
    int firstVertex = 0;

    int layers = mVolume.layers;
    int rows = mVolume.rows;
    int cols = mVolume.cols;
    float valMin = (float)mVolume.valueMin;
    float valMax = (float)mVolume.valueMax;

    // 三个方向：0=Z(顶/底) 1=X(左/右) 2=Y(前/后)
    for (int axis = 0; axis < 3; axis++)
    {
        int n = mSlicesPerAxis;
        int dimSize = (axis == 0) ? layers : ((axis == 1) ? cols : rows);
        int sliceCount = (n > dimSize) ? dimSize : n;   // 不超过该维度尺寸
        if (sliceCount < 1) { sliceCount = 1; }

        int w = 0, h = 0;   // 切片纹理宽高
        if (axis == 0) { w = cols; h = rows; }        // Z 切片：x-y 平面
        else if (axis == 1) { w = rows; h = layers; } // X 切片：y-z 平面
        else { w = cols; h = layers; }                // Y 切片：x-z 平面

        std::vector<unsigned char> image((size_t)w * (size_t)h * 4);

        for (int s = 0; s < sliceCount; s++)
        {
            int idx = (sliceCount > 1) ? (s * (dimSize - 1) / (sliceCount - 1)) : 0;
            if (idx < 0) { idx = 0; }
            if (idx >= dimSize) { idx = dimSize - 1; }

            // 生成该切片 RGBA 图像
            for (int v = 0; v < h; v++)
            {
                for (int u = 0; u < w; u++)
                {
                    float val = 0.0f;
                    if (axis == 0)
                    {
                        val = mVolume.data[((size_t)idx * rows + v) * cols + u];
                    }
                    else if (axis == 1)
                    {
                        // X 方向：w=rows, h=layers；u=row, v=layer
                        val = mVolume.data[((size_t)v * rows + u) * cols + idx];
                    }
                    else
                    {
                        // Y 方向：w=cols, h=layers；u=col, v=layer
                        val = mVolume.data[((size_t)v * rows + idx) * cols + u];
                    }
                    unsigned char rgba[4];
                    valueToColor(val, valMin, valMax, rgba);
                    size_t off = ((size_t)v * w + u) * 4;
                    image[off + 0] = rgba[0];
                    image[off + 1] = rgba[1];
                    image[off + 2] = rgba[2];
                    image[off + 3] = rgba[3];
                }
            }

            // 创建 2D 纹理
            QOpenGLTexture* pTex = new QOpenGLTexture(QOpenGLTexture::Target2D);
            pTex->create();
            pTex->setFormat(QOpenGLTexture::RGBA8_UNorm);
            pTex->setSize(w, h);
            pTex->setMinificationFilter(QOpenGLTexture::Linear);
            pTex->setMagnificationFilter(QOpenGLTexture::Linear);
            pTex->setWrapMode(QOpenGLTexture::ClampToEdge);
            pTex->allocateStorage();
            pTex->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, static_cast<const void*>(image.data()));
            mSliceTextures.push_back(pTex);

            // 生成该切片 quad 顶点（[-1,1]^3 体空间），位置取决于轴与切片位置
            float t = (sliceCount > 1) ? (2.0f * s / (sliceCount - 1) - 1.0f) : 0.0f; // -1..1
            std::vector<float> quad;
            auto addVert = [&](float x, float y, float z, float uu, float vv) {
                quad.push_back(x); quad.push_back(y); quad.push_back(z);
                quad.push_back(uu); quad.push_back(vv);
                quad.push_back(1.0f); quad.push_back(1.0f); quad.push_back(1.0f);
            };
            if (axis == 0)          // Z 切片：x-y 平面 @ z=t
            {
                addVert(-1,-1,t, 0,0); addVert(1,-1,t, 1,0); addVert(1,1,t, 1,1);
                addVert(-1,-1,t, 0,0); addVert(1,1,t, 1,1); addVert(-1,1,t, 0,1);
            }
            else if (axis == 1)     // X 切片：y-z 平面 @ x=t；uv=(row归一化, layer归一化) -> (y, z)
            {
                addVert(t,-1,-1, 0,0); addVert(t,1,-1, 1,0); addVert(t,1,1, 1,1);
                addVert(t,-1,-1, 0,0); addVert(t,1,1, 1,1); addVert(t,-1,1, 0,1);
            }
            else                    // Y 切片：x-z 平面 @ y=t；uv=(col归一化, layer归一化) -> (x, z)
            {
                addVert(-1,t,-1, 0,0); addVert(1,t,-1, 1,0); addVert(1,t,1, 1,1);
                addVert(-1,t,-1, 0,0); addVert(1,t,1, 1,1); addVert(-1,t,1, 0,1);
            }

            for (size_t q = 0; q < quad.size(); q++) { verts.push_back(quad[q]); }

            // 记录该切片 (axis, firstVertex, 6 顶点, 纹理索引)
            SSliceRange rng;
            rng.axis = axis;
            rng.first = firstVertex;
            rng.count = 6;
            mSliceRanges.push_back(rng);
            firstVertex += 6;
        }
    }

    // 上传合并顶点到切片 VBO
    mSliceBuffer->bind();
    mSliceBuffer->allocate(verts.data(), verts.size() * sizeof(float));
    mSliceBuffer->release();

    mTexturesBuilt = true;
}

// 功能：绘制体包围盒与坐标轴
void CVolumeDataGLView::drawBoxAndAxis(const QMatrix4x4& mvp)
{
    mProgram->bind();
    mProgram->setUniformValue("uMVP", mvp);
    mProgram->setUniformValue("uUseTex", 0);
    mProgram->setUniformValue("uAlpha", 1.0f);
    mProgram->setUniformValue("uColor", QVector3D(0.35f, 0.35f, 0.35f));

    mBoxVao->bind();
    if (mLineMode || !mFillMode)
    {
        // 线框模式：只画包围盒线框
        glLineWidth(1.5f);
        mProgram->setUniformValue("uColor", QVector3D(0.35f, 0.35f, 0.35f));
        glDrawArrays(GL_LINES, 0, 24);
    }
    mBoxVao->release();

    if (mShowAxis)
    {
        mProgram->bind();
        mProgram->setUniformValue("uUseTex", 0);
        mProgram->setUniformValue("uAlpha", 1.0f);
        mBoxVao->bind();
        glLineWidth(2.0f);
        mProgram->setUniformValue("uColor", QVector3D(1.0f, 0.0f, 0.0f));
        glDrawArrays(GL_LINES, 24, 2);
        mProgram->setUniformValue("uColor", QVector3D(0.0f, 1.0f, 0.0f));
        glDrawArrays(GL_LINES, 26, 2);
        mProgram->setUniformValue("uColor", QVector3D(0.0f, 0.0f, 1.0f));
        glDrawArrays(GL_LINES, 28, 2);
        mBoxVao->release();
    }
}

// 功能：绘制场景
void CVolumeDataGLView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 投影：透视
    QMatrix4x4 projection;
    projection.perspective(45.0f, (width() > 0) ? (float)width() / (float)height() : 1.0f, 0.05f, 10.0f);

    // 视图：先绕 X(俯仰) 再绕 Y(偏航)
    QMatrix4x4 view;
    view.translate(0.0f, 0.0f, -3.2f);
    view.rotate(mPitch, 1.0f, 0.0f, 0.0f);
    view.rotate(mYaw, 0.0f, 1.0f, 0.0f);

    // 模型：整体/ Z 轴缩放
    QMatrix4x4 model;
    model.scale(mZoom, mZoom, mZoom * mZScale);

    QMatrix4x4 mvp = projection * view * model;

    // 若已加载体数据但切片纹理尚未构建，则在本次绘制前构建（需 GL 上下文就绪）
    if (mHasData && !mTexturesBuilt)
    {
        buildSliceTextures();
    }

    // 体数据切片
    if (mHasData && mTexturesBuilt)
    {
        mProgram->bind();
        // 填充模式：半透明混合 + 关闭深度写入（近似体叠合）
        if (mFillMode && !mLineMode)
        {
            glDepthMask(GL_FALSE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        mSliceVao->bind();
        for (size_t i = 0; i < mSliceRanges.size(); i++)
        {
            const SSliceRange& rng = mSliceRanges[i];
            // 按方向显隐过滤
            if (rng.axis == 0 && !(mShowTop && mShowBottom)) { continue; }
            if (rng.axis == 1 && !(mShowLeft && mShowRight)) { continue; }
            if (rng.axis == 2 && !(mShowFront && mShowBack)) { continue; }
            // 默认只显示全部方向；若仅 Z 方向，则只画 axis==0
            if (!mShowAllAxis && rng.axis != 0) { continue; }

            // 切片模式：仅立方体时不画切片（只画包围盒）
            if (mSliceMode == 2) { continue; }

            if ((int)i < (int)mSliceTextures.size() && mSliceTextures[i] != NULL)
            {
                mSliceTextures[i]->bind(0);
                mProgram->setUniformValue("uTex", 0);
            }
            mProgram->setUniformValue("uUseTex", 1);
            mProgram->setUniformValue("uAlpha", mLighting ? 0.55f : 0.85f);
            mProgram->setUniformValue("uMVP", mvp);
            mProgram->setUniformValue("uColor", QVector3D(1.0f, 1.0f, 1.0f));
            glDrawArrays(GL_TRIANGLES, rng.first, rng.count);
        }
        mSliceVao->release();

        glDepthMask(GL_TRUE);
        glEnable(GL_BLEND);
    }

    // 包围盒与坐标轴
    drawBoxAndAxis(mvp);

    // 没有数据时提示（用 OpenGL 绘制文字较复杂，省略；由对话框状态栏给出提示）
}

// 功能：鼠标按下（记录拖拽起点）
void CVolumeDataGLView::mousePressEvent(QMouseEvent* pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        mLastPos = pEvent->pos();
        setModelRotate(false);  // 手动拖拽时停止自动旋转
    }
    QOpenGLWidget::mousePressEvent(pEvent);
}

// 功能：鼠标移动（拖拽旋转）
void CVolumeDataGLView::mouseMoveEvent(QMouseEvent* pEvent)
{
    if (pEvent->buttons() & Qt::LeftButton)
    {
        int dx = pEvent->pos().x() - mLastPos.x();
        int dy = pEvent->pos().y() - mLastPos.y();
        mYaw += dx * 0.5f;
        mPitch += dy * 0.5f;
        if (mPitch > 89.0f) { mPitch = 89.0f; }
        if (mPitch < -89.0f) { mPitch = -89.0f; }
        mLastPos = pEvent->pos();
        update();
    }
    QOpenGLWidget::mouseMoveEvent(pEvent);
}

// 功能：滚轮缩放
void CVolumeDataGLView::wheelEvent(QWheelEvent* pEvent)
{
    if (pEvent->angleDelta().y() > 0)
    {
        zoomIn();
    }
    else
    {
        zoomOut();
    }
    pEvent->accept();
}

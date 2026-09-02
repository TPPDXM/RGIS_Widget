// ContourGLWidget.h : OpenGL 等值线绘制控件（Qt 前端）
//
// 对应原 MFC 工程：CContourGLCtl（ActiveX 控件，源码不可得，此处从零重写实现）。
// 功能：读取 .grd 网格（头+数据），用 Marching Squares 生成等值线段并按色带着色，
//       通过 OpenGL 绘制，支持缩放（滚轮）/平移（左键拖拽）/还原（双击）。
// 说明：等值线生成与 OpenGL 渲染为前端自实现，数据读取亦在前端进行（不涉及后端算法接口）。

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector>
#include <QPoint>

// OpenGL 等值线绘制控件
class CContourGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    // 构造函数
    explicit CContourGLWidget(QWidget* pParent = nullptr);

    // 功能：读取 .grd 网格文件并重建等值线
    // 参数：strFilePath —— .grd 文件路径
    // 参数：strError —— 失败时输出的错误描述
    // 返回：true 成功；false 失败
    bool loadGridFile(const QString& strFilePath, QString& strError);

    // 功能：设置等值线分层数（等间隔分层，默认 12）
    void setContourLevels(int nLevels);

    // 功能：还原视图（全图显示）
    void resetView();

signals:
    // 功能：交互/状态信息变化（如缩放比例），供宿主在状态栏显示
    void infoTextChanged(const QString& strInfo);

protected:
    // ===== QOpenGLWidget 虚函数 =====
    void initializeGL() override;               // 功能：初始化 OpenGL
    void resizeGL(int w, int h) override;       // 功能：视口尺寸变化
    void paintGL() override;                    // 功能：绘制等值线
    void wheelEvent(QWheelEvent* pEvent) override;  // 功能：滚轮缩放
    void mousePressEvent(QMouseEvent* pEvent) override;  // 功能：左键按下（开始平移）
    void mouseMoveEvent(QMouseEvent* pEvent) override;   // 功能：左键拖动（平移）
    void mouseDoubleClickEvent(QMouseEvent* pEvent) override;  // 功能：双击还原

private:
    // ===== .grd 文件读取（前端自实现，支持 DSAA/DSBB/DSRB/#GMD/#GEX）=====
    // 功能：按文件头魔数判断类型（1~5，0 无法识别）
    int detectFileType(const wchar_t* pWidePath);
    // 功能：读取 DSAA(文本) 网格头+数据
    bool readDsaa(const wchar_t* pWidePath, QString& strError);
    // 功能：读取 DSBB(二进制) 网格头+数据
    bool readDsbb(const wchar_t* pWidePath, QString& strError);
    // 功能：读取 DSRB(二进制) 网格头+数据
    bool readDsrb(const wchar_t* pWidePath, QString& strError);
    // 功能：读取 #GMD/#GEX(二进制) 网格头+数据
    bool readGmd(const wchar_t* pWidePath, QString& strError);
    // 功能：UTF-8 路径转宽字符路径
    static wchar_t* utf8ToWide(const std::string& utf8Path, size_t& bufferSize);

    // ===== 等值线生成（Marching Squares）=====
    void recomputeContours();                   // 功能：按当前数据与分层数生成等值线段集合
    void addCellSegment(int i, int j, double lv);  // 功能：对单个网格单元生成等值线段（level = lv）

    // ===== 颜色色带 =====
    // 功能：按归一化值 t(0~1) 取 jet 色带颜色（R,G,B 0~1 输出）
    void jetColor(double t, float& r, float& g, float& b) const;

    // ===== 网格数据 =====
    int     mCols;              // 网格列数
    int     mRows;              // 网格行数
    double  mXMin;              // X 最小值
    double  mXMax;              // X 最大值
    double  mYMin;              // Y 最小值
    double  mYMax;              // Y 最大值
    double  mZMin;              // 数据最小值
    double  mZMax;              // 数据最大值
    QString mFilePath;          // 当前文件路径
    QVector<float> mGridData;   // 网格数据（rows*cols，行优先）

    // ===== 等值线（每段 = QPointF 两个数据坐标端点）=====
    struct Segment
    {
        double x0; double y0; double x1; double y1;   // 线段两端点（数据坐标）
        double colorT;                                // 归一化配色值（0~1）
    };
    QVector<Segment> mSegments;     // 生成的等值线段集合
    int     mLayerCount;            // 等值线分层数

    // ===== 视图状态 =====
    double  mZoom;                  // 缩放因子（1=全图）
    double  mPanX;                  // 水平平移（相对半宽的倍数）
    double  mPanY;                  // 垂直平移（相对半宽的倍数）
    int     mPanStartX;             // 平移起始像素坐标（拖拽起点 X）
    int     mPanStartY;             // 平移起始像素坐标（拖拽起点 Y）
    double  mPanBaseX;              // 平移起始时的 mPanX 快照
    double  mPanBaseY;              // 平移起始时的 mPanY 快照
};

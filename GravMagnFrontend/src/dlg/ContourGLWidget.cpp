// ContourGLWidget.cpp : OpenGL 等值线绘制控件实现

#include "ContourGLWidget.h"

#include <QMouseEvent>
#include <QPoint>
#include <QWheelEvent>

#include <cstdio>
#include <cstring>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

// 构造函数：初始化网格与视图状态
CContourGLWidget::CContourGLWidget(QWidget* pParent)
    : QOpenGLWidget(pParent)
    , mCols(0)
    , mRows(0)
    , mXMin(0.0)
    , mXMax(0.0)
    , mYMin(0.0)
    , mYMax(0.0)
    , mZMin(0.0)
    , mZMax(0.0)
    , mLayerCount(12)
    , mZoom(1.0)
    , mPanX(0.0)
    , mPanY(0.0)
{
    setMinimumSize(400, 300);
    // 允许滚轮/鼠标事件
    setFocusPolicy(Qt::StrongFocus);
}

// 功能：设置等值线分层数（等间隔分层）
void CContourGLWidget::setContourLevels(int nLevels)
{
    if (nLevels < 2)
    {
        nLevels = 2;
    }
    mLayerCount = nLevels;
    recomputeContours();
    update();
}

// 功能：还原视图（全图显示）
void CContourGLWidget::resetView()
{
    mZoom = 1.0;
    mPanX = 0.0;
    mPanY = 0.0;
    update();
    emit infoTextChanged(QString());
}

// 功能：UTF-8 路径转宽字符路径（Windows 下打开含中文路径）
wchar_t* CContourGLWidget::utf8ToWide(const std::string& utf8Path, size_t& bufferSize)
{
#ifdef _WIN32
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
    (void)bufferSize;
    return NULL;
#endif
}

// 功能：按文件头魔数判断类型（1~5，0 无法识别）
int CContourGLWidget::detectFileType(const wchar_t* pWidePath)
{
    int nType = 0;
    FILE* fp = _wfopen(pWidePath, L"rb");
    if (fp == NULL)
    {
        return 0;
    }
    char head[4];
    if (fread(head, sizeof(char), 4, fp) == 4)
    {
        if (head[0] == 'D' && head[1] == 'S' && head[2] == 'A' && head[3] == 'A') { nType = 1; }
        else if (head[0] == 'D' && head[1] == 'S' && head[2] == 'B' && head[3] == 'B') { nType = 2; }
        else if (head[0] == 'D' && head[1] == 'S' && head[2] == 'R' && head[3] == 'B') { nType = 3; }
        else if (head[0] == '#' && head[1] == 'G' && head[2] == 'M' && head[3] == 'D') { nType = 4; }
        else if (head[0] == '#' && head[1] == 'G' && head[2] == 'E' && head[3] == 'X') { nType = 5; }
    }
    fclose(fp);
    return nType;
}

// 功能：读取 DSAA(Surfer ASCII) 网格头+数据
bool CContourGLWidget::readDsaa(const wchar_t* pWidePath, QString& strError)
{
    FILE* fp = _wfopen(pWidePath, L"rt");
    if (fp == NULL)
    {
        strError = QStringLiteral("打开网格文件失败。");
        return false;
    }
    char headToken[64] = { 0 };
    int cols = 0, rows = 0;
    float xMin = 0, xMax = 0, yMin = 0, yMax = 0, zMin = 0, zMax = 0;
    if (fscanf(fp, "%63s %d %d %f %f %f %f %f %f",
            headToken, &cols, &rows, &xMin, &xMax, &yMin, &yMax, &zMin, &zMax) != 9)
    {
        fclose(fp);
        strError = QStringLiteral("网格文件头读取失败。");
        return false;
    }
    // 读取数据（文本浮点，空格/换行分隔）
    std::vector<float> data((size_t)rows * cols, 0.0f);
    for (int i = 0; i < rows * cols; i++)
    {
        if (fscanf(fp, "%f", &data[i]) != 1)
        {
            break;
        }
    }
    fclose(fp);

    mCols = cols;
    mRows = rows;
    mXMin = xMin; mXMax = xMax;
    mYMin = yMin; mYMax = yMax;
    mZMin = zMin; mZMax = zMax;
    mGridData = QVector<float>(data.begin(), data.end());
    mFilePath = QString::fromWCharArray(pWidePath);
    return true;
}

// 功能：读取 DSBB(Surfer Binary) 网格头+数据
bool CContourGLWidget::readDsbb(const wchar_t* pWidePath, QString& strError)
{
    FILE* fp = _wfopen(pWidePath, L"rb");
    if (fp == NULL)
    {
        strError = QStringLiteral("打开网格文件失败。");
        return false;
    }
    char magic[4];
    unsigned short nCols = 0, nRows = 0;
    double coords[6] = { 0, 0, 0, 0, 0, 0 };
    bool bOk = true;
    bOk = bOk && (fread(magic, sizeof(char), 4, fp) == 4);
    bOk = bOk && (fread(&nCols, sizeof(unsigned short), 1, fp) == 1);
    bOk = bOk && (fread(&nRows, sizeof(unsigned short), 1, fp) == 1);
    bOk = bOk && (fread(coords, sizeof(double), 6, fp) == 6);
    if (!bOk)
    {
        fclose(fp);
        strError = QStringLiteral("网格文件头读取失败。");
        return false;
    }
    std::vector<float> data((size_t)nRows * nCols, 0.0f);
    if (fread(&data[0], sizeof(float), data.size(), fp) != data.size())
    {
        fclose(fp);
        strError = QStringLiteral("网格数据读取失败。");
        return false;
    }
    fclose(fp);

    mCols = nCols;
    mRows = nRows;
    mXMin = coords[0]; mXMax = coords[1];
    mYMin = coords[2]; mYMax = coords[3];
    mZMin = coords[4]; mZMax = coords[5];
    mGridData = QVector<float>(data.begin(), data.end());
    mFilePath = QString::fromWCharArray(pWidePath);
    return true;
}

// 功能：读取 DSRB 二进制网格头+数据（数据为 double）
bool CContourGLWidget::readDsrb(const wchar_t* pWidePath, QString& strError)
{
    FILE* fp = _wfopen(pWidePath, L"rb");
    if (fp == NULL)
    {
        strError = QStringLiteral("打开网格文件失败。");
        return false;
    }
    char magic[4];
    int tmpInt = 0;
    char tmpChars[5] = { 0 };
    int rows = 0, cols = 0;
    double xMin = 0, yMin = 0, fDx = 0, fDy = 0, zMin = 0, zMax = 0;
    double tmpD1 = 0, tmpD2 = 0;
    bool bOk = true;
    bOk = bOk && (fread(magic, sizeof(char), 4, fp) == 4);
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
    if (!bOk)
    {
        fclose(fp);
        strError = QStringLiteral("网格文件头读取失败。");
        return false;
    }
    std::vector<double> dataDouble((size_t)rows * cols, 0.0);
    if (fread(&dataDouble[0], sizeof(double), dataDouble.size(), fp) != dataDouble.size())
    {
        fclose(fp);
        strError = QStringLiteral("网格数据读取失败。");
        return false;
    }
    fclose(fp);

    mCols = cols;
    mRows = rows;
    mXMin = xMin;
    mXMax = xMin + (cols - 1) * fDx;
    mYMin = yMin;
    mYMax = yMin + (rows - 1) * fDy;
    mZMin = zMin; mZMax = zMax;
    mGridData.resize(dataDouble.size());
    for (size_t i = 0; i < dataDouble.size(); i++)
    {
        mGridData[i] = (float)dataDouble[i];
    }
    mFilePath = QString::fromWCharArray(pWidePath);
    return true;
}

// 功能：读取 #GMD / #GEX 二进制网格头+数据
bool CContourGLWidget::readGmd(const wchar_t* pWidePath, QString& strError)
{
    FILE* fp = _wfopen(pWidePath, L"rb");
    if (fp == NULL)
    {
        strError = QStringLiteral("打开网格文件失败。");
        return false;
    }
    char headChars[32] = { 0 };
    char descChars[32] = { 0 };
    float xMin = 0, yMin = 0, tmpF = 0;
    short nCols = 0, nRows = 0;
    float fDx = 0, fDy = 0, zMin = 0, zMax = 0;
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
    if (!bOk)
    {
        fclose(fp);
        strError = QStringLiteral("网格文件头读取失败。");
        return false;
    }
    int rows = nRows;
    int cols = nCols;
    std::vector<float> data((size_t)rows * cols, 0.0f);
    if (fread(&data[0], sizeof(float), data.size(), fp) != data.size())
    {
        fclose(fp);
        strError = QStringLiteral("网格数据读取失败。");
        return false;
    }
    fclose(fp);

    mCols = cols;
    mRows = rows;
    mXMin = xMin;
    mXMax = xMin + fDx * (cols - 1);
    mYMin = yMin;
    mYMax = yMin + fDy * (rows - 1);
    mZMin = zMin; mZMax = zMax;
    mGridData = QVector<float>(data.begin(), data.end());
    mFilePath = QString::fromWCharArray(pWidePath);
    return true;
}

// 功能：读取 .grd 网格文件并重建等值线
bool CContourGLWidget::loadGridFile(const QString& strFilePath, QString& strError)
{
    if (strFilePath.isEmpty())
    {
        strError = QStringLiteral("网格文件路径为空。");
        return false;
    }

    size_t bufferSize = 0;
    wchar_t* pWidePath = utf8ToWide(strFilePath.toUtf8().constData(), bufferSize);
    if (pWidePath == NULL)
    {
        strError = QStringLiteral("网格文件路径转换失败。");
        return false;
    }

    int nType = detectFileType(pWidePath);
    bool bOk = false;
    if (nType == 1)
    {
        bOk = readDsaa(pWidePath, strError);
    }
    else if (nType == 2)
    {
        bOk = readDsbb(pWidePath, strError);
    }
    else if (nType == 3)
    {
        bOk = readDsrb(pWidePath, strError);
    }
    else if (nType == 4 || nType == 5)
    {
        bOk = readGmd(pWidePath, strError);
    }
    else
    {
        strError = QStringLiteral("无法识别的网格文件格式（支持 DSAA/DSBB/DSRB/#GMD/#GEX）。");
    }
    delete[] pWidePath;

    if (!bOk)
    {
        return false;
    }

    // 重置视图并生成等值线
    mZoom = 1.0;
    mPanX = 0.0;
    mPanY = 0.0;
    recomputeContours();
    update();
    emit infoTextChanged(QStringLiteral("等值线显示: ") + strFilePath);
    return true;
}

// 功能：按 jet 色带取颜色（蓝→青→绿→黄→橙→红）
void CContourGLWidget::jetColor(double t, float& r, float& g, float& b) const
{
    // 采用简化 jet 色带分段线性插值
    double tt = t;
    if (tt < 0.0) { tt = 0.0; }
    if (tt > 1.0) { tt = 1.0; }
    const double n = 4.0;
    double idx = tt * n;
    int seg = (int)(idx);
    double f = idx - seg;
    if (seg > 3) { seg = 3; f = 1.0; }
    // 断点（R,G,B）
    const double cols[5][3] = {
        { 0.0, 0.0, 1.0 },   // 蓝
        { 0.0, 1.0, 1.0 },   // 青
        { 0.0, 1.0, 0.0 },   // 绿
        { 1.0, 1.0, 0.0 },   // 黄
        { 1.0, 0.0, 0.0 }    // 红
    };
    r = (float)(cols[seg][0] + (cols[seg + 1][0] - cols[seg][0]) * f);
    g = (float)(cols[seg][1] + (cols[seg + 1][1] - cols[seg][1]) * f);
    b = (float)(cols[seg][2] + (cols[seg + 1][2] - cols[seg][2]) * f);
}

// 功能：生成等值线段（对单个网格单元，level = lv）
void CContourGLWidget::addCellSegment(int i, int j, double lv)
{
    // 网格单元 (i,j) 的 4 个角：v00(左下) v10(右下) v11(右上) v01(左上)
    // 数据行优先：grid[r*cols+c]，网格数据 i 行、j 列
    double x0 = mXMin + j * (mXMax - mXMin) / (mCols - 1);
    double x1 = mXMin + (j + 1) * (mXMax - mXMin) / (mCols - 1);
    double y0 = mYMin + i * (mYMax - mYMin) / (mRows - 1);
    double y1 = mYMin + (i + 1) * (mYMax - mYMin) / (mRows - 1);

    double v00 = mGridData.at(i * mCols + j);
    double v10 = mGridData.at(i * mCols + (j + 1));
    double v11 = mGridData.at((i + 1) * mCols + (j + 1));
    double v01 = mGridData.at((i + 1) * mCols + j);

    // 每条边两端点值是否跨越 lv 的辅助取交点函数（此处直接内联计算，仅保留说明）
    // 收集本单元与 lv 相交的边交点
    // 边顺序：0=底(00->10) 1=右(10->11) 2=顶(11->01) 3=左(01->00)
    QVector<QPointF> pts;
    // 底边 y=y0, x0..x1
    if ((v00 >= lv) != (v10 >= lv))
    {
        pts.append(QPointF(x0 + (x1 - x0) * (lv - v00) / (v10 - v00), y0));
    }
    // 右边 x=x1, y0..y1
    if ((v10 >= lv) != (v11 >= lv))
    {
        pts.append(QPointF(x1, y0 + (y1 - y0) * (lv - v10) / (v11 - v10)));
    }
    // 顶边 y=y1, x1..x0
    if ((v11 >= lv) != (v01 >= lv))
    {
        pts.append(QPointF(x0 + (x1 - x0) * (lv - v01) / (v11 - v01), y1));
    }
    // 左边 x=x0, y1..y0
    if ((v01 >= lv) != (v00 >= lv))
    {
        pts.append(QPointF(x0, y0 + (y1 - y0) * (lv - v01) / (v00 - v01)));
    }

    if (pts.size() == 2)
    {
        Segment seg;
        seg.x0 = pts.at(0).x(); seg.y0 = pts.at(0).y();
        seg.x1 = pts.at(1).x(); seg.y1 = pts.at(1).y();
        seg.colorT = (lv - mZMin) / (mZMax - mZMin);
        mSegments.append(seg);
    }
    else if (pts.size() == 4)
    {
        // 歧义单元：连接 (0,1) 与 (2,3)（简化处理）
        for (int k = 0; k < 2; k++)
        {
            Segment seg;
            seg.x0 = pts.at(k * 2).x(); seg.y0 = pts.at(k * 2).y();
            seg.x1 = pts.at(k * 2 + 1).x(); seg.y1 = pts.at(k * 2 + 1).y();
            seg.colorT = (lv - mZMin) / (mZMax - mZMin);
            mSegments.append(seg);
        }
    }
}

// 功能：按当前数据与分层数生成等值线段集合
void CContourGLWidget::recomputeContours()
{
    mSegments.clear();
    if (mCols < 2 || mRows < 2 || mGridData.size() < mCols * mRows)
    {
        return;
    }
    if (mZMax <= mZMin)
    {
        return;
    }

    // 等间隔分层
    for (int l = 1; l < mLayerCount; l++)
    {
        double lv = mZMin + (mZMax - mZMin) * l / mLayerCount;
        for (int i = 0; i < mRows - 1; i++)
        {
            for (int j = 0; j < mCols - 1; j++)
            {
                addCellSegment(i, j, lv);
            }
        }
    }
}

// 功能：初始化 OpenGL
void CContourGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.94f, 0.94f, 0.94f, 1.0f);   // 浅灰背景
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

// 功能：视口尺寸变化
void CContourGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    update();
}

// 功能：绘制等值线
void CContourGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if (mSegments.size() <= 0)
    {
        return;
    }

    // 计算显示视口（保持数据纵横比，支持缩放/平移）
    double dataAspect = (mXMax - mXMin) / (mYMax - mYMin);
    double winAspect = (width() > 0 && height() > 0) ? (double)width() / height() : 1.0;
    double spanX = (mXMax - mXMin) / 2.0;
    double spanY = (mYMax - mYMin) / 2.0;
    // 全图 fit：按较窄方向适配
    if (dataAspect > winAspect)
    {
        spanY = spanX / winAspect;
    }
    else
    {
        spanX = spanY * winAspect;
    }
    // 缩放
    spanX /= mZoom;
    spanY /= mZoom;

    double centerX = (mXMin + mXMax) / 2.0 + mPanX * spanX * 2.0;
    double centerY = (mYMin + mYMax) / 2.0 + mPanY * spanY * 2.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(centerX - spanX, centerX + spanX, centerY - spanY, centerY + spanY, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 绘制每条等值线段
    glBegin(GL_LINES);
    for (int i = 0; i < mSegments.size(); i++)
    {
        const Segment& seg = mSegments.at(i);
        float r, g, b;
        jetColor(seg.colorT, r, g, b);
        glColor3f(r, g, b);
        glVertex2d(seg.x0, seg.y0);
        glVertex2d(seg.x1, seg.y1);
    }
    glEnd();
}

// 功能：滚轮缩放（以鼠标为中心）
void CContourGLWidget::wheelEvent(QWheelEvent* pEvent)
{
    double factor = (pEvent->angleDelta().y() > 0) ? 1.1 : 1.0 / 1.1;
    mZoom *= factor;
    if (mZoom < 0.2) { mZoom = 0.2; }
    if (mZoom > 20.0) { mZoom = 20.0; }
    update();
    emit infoTextChanged(QStringLiteral("缩放: ") + QString::number(mZoom, 'f', 2));
    pEvent->accept();
}

// 功能：左键按下（记录平移起点）
void CContourGLWidget::mousePressEvent(QMouseEvent* pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        mPanStartX = pEvent->x();
        mPanStartY = pEvent->y();
        mPanBaseX = mPanX;
        mPanBaseY = mPanY;
    }
    pEvent->accept();
}

// 功能：左键拖动（平移视图）
void CContourGLWidget::mouseMoveEvent(QMouseEvent* pEvent)
{
    if (pEvent->buttons() & Qt::LeftButton)
    {
        double dx = pEvent->x() - mPanStartX;
        double dy = pEvent->y() - mPanStartY;
        // 以像素折算为相对半宽/半高的平移
        mPanX = mPanBaseX - dx / (double)width();
        mPanY = mPanBaseY + dy / (double)height();
        update();
        pEvent->accept();
    }
}

// 功能：双击还原（全图显示）
void CContourGLWidget::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
    resetView();
    pEvent->accept();
}

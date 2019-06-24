#include "UiMetalPaintEngine.h"

#include "source/Rectangle/Rect.h"

UiMetalPaintEngine::UiMetalPaintEngine()
    : QPaintEngine(QPaintEngine::AllFeatures)
    , m_Scene(NULL)
{
}

UiMetalPaintEngine::~UiMetalPaintEngine()
{
}

bool UiMetalPaintEngine::Init(Scene* p_Scene)
{
    m_Scene = p_Scene;

    return (m_Scene != NULL);
}

void UiMetalPaintEngine::Render(QPainter* p_Painter)
{
}

bool UiMetalPaintEngine::begin(QPaintDevice* p_Device)
{
    return false;
}

bool UiMetalPaintEngine::end()
{
    return true;
}

void UiMetalPaintEngine::updateState(const QPaintEngineState& p_State)
{
    //m_State = p_State;
    qDebug("UiMetalRenderEngine::updateState");
}

void UiMetalPaintEngine::drawRects(const QRect* p_Rects, int p_RectCount)
{
    qDebug("UiMetalRenderEngine::drawRects QRect");
}

void UiMetalPaintEngine::drawLines(const QLine* p_Lines, int p_LineCount)
{
    qDebug("UiMetalRenderEngine::drawLines QLine");
}

void UiMetalPaintEngine::drawLines(const QLineF* p_Lines, int p_LineCount)
{
    qDebug("UiMetalRenderEngine::drawLines QLineF");
}

void UiMetalPaintEngine::drawEllipse(const QRectF& p_Rect)
{
    qDebug("UiMetalRenderEngine::drawEllipse QRectF");
}

void UiMetalPaintEngine::drawEllipse(const QRect& p_Rect)
{
    qDebug("UiMetalRenderEngine::drawEllipse QRect");
}

void UiMetalPaintEngine::drawPath(const QPainterPath& p_Path)
{
    qDebug("UiMetalRenderEngine::drawPath QPainterPath");
}

void UiMetalPaintEngine::drawPoints(const QPointF* p_Points, int p_PointCount)
{
    qDebug("UiMetalRenderEngine::drawPoints QPointF");
}

void UiMetalPaintEngine::drawPoints(const QPoint* p_Points, int p_PointCount)
{
    qDebug("UiMetalRenderEngine::drawPoints QPoint");
}

void UiMetalPaintEngine::drawPolygon(const QPointF* p_Points, int p_PointCount, QPaintEngine::PolygonDrawMode p_Mode)
{

}

void UiMetalPaintEngine::drawPolygon(const QPoint* p_Points, int p_PointCount, QPaintEngine::PolygonDrawMode p_Mode)
{

}

void UiMetalPaintEngine::drawTextItem(const QPointF& p_DestPos, const QTextItem& p_TextItem)
{

}

void UiMetalPaintEngine::drawTiledPixmap(const QRectF& p_DestRect, const QPixmap& p_Pixmap, const QPointF& p_SrcRect)
{

}

void UiMetalPaintEngine::drawImage(const QRectF& p_DestRect, const QImage& p_Image, const QRectF& p_SrcRect, Qt::ImageConversionFlags p_Flags)
{

}

void UiMetalPaintEngine::drawRects(const QRectF* pRects, int pRectCount)
{
//    // br, bl, tr, tl
//    // l, r, t, b
//    float rectLFTB[4] = {static_cast<float>(pRects[0].left()), static_cast<float>(pRects[0].right()), static_cast<float>(pRects[0].top()), static_cast<float>(pRects[0].bottom())};

//    [reinterpret_cast<MetalRenderer*>(m_Renderer) queueRect:rectLFTB andSize: pRectCount];

    static Node* rect1 = NULL;
    static Node* rect2 = NULL;
    bool updated = false;
    if (rect1)
    {
        m_Scene->RemoveItem(rect1);
        rect1 = NULL;
        updated = true;
    }

    if (rect2)
    {
        m_Scene->RemoveItem(rect2);
        rect2 = NULL;
        updated = true;
    }

    if (!rect1)
    {
        rect1 = new Rectangl(m_Scene, NULL, BoundingRegion(800, 400, 200, 200), "Node 1", SHAPE::SHAPE_RECTANGLE_INSTANCED);
    }

    rect1->SetColor(glm::vec4(1.0, 0.0, 0.0, 1.0)); // RED
    rect1->SetDefaultColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
    rect1->SetZOrder(15);

    if (!rect2)
    {
        rect2 = new Rectangl(m_Scene, NULL, BoundingRegion(400, 400, 200, 200), "Node 2", SHAPE::SHAPE_RECTANGLE_INSTANCED);
        m_Scene->Setup();
        m_Scene->SetEarlyDepthTest(true);
    }

    rect2->SetColor(glm::vec4(0.0, 1.0, 0.0, 1.0)); // GREEN
    rect2->SetDefaultColor(glm::vec4(0.0, 1.0, 0.0, 1.0));
    //rect2->SetZOrder(0);

    if (updated)
    {
        m_Scene->Update();
    }
}

void UiMetalPaintEngine::drawPixmap(const QRectF& pRect, const QPixmap& p_Pixmap, const QRectF& sr)
{
//    [reinterpret_cast<MetalRenderer*>(m_Renderer) drawImage];
}

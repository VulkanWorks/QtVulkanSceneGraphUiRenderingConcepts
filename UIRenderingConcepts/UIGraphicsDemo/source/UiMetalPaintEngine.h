#pragma once

#include <QPaintEngine>

class UiMetalWidget;
class Scene;
class UiMetalPaintEngine : public QPaintEngine
{
public:
    UiMetalPaintEngine();
    virtual ~UiMetalPaintEngine();

    bool Init(Scene* p_Scene);

    void Render(QPainter* p_Painter);

    virtual Type type() const { return static_cast<Type>(User + 1); }

    virtual bool begin(QPaintDevice* p_Device) override;
    virtual bool end() override;

    virtual void updateState(const QPaintEngineState& p_State) override;

    virtual void drawRects(const QRect* p_Rects, int p_RectCount) override;
    virtual void drawRects(const QRectF* p_Rects, int p_RectCount) override;

    virtual void drawLines(const QLine* p_Lines, int p_LineCount) override;
    virtual void drawLines(const QLineF* p_Lines, int p_LineCount) override;

    virtual void drawEllipse(const QRectF& p_Rect) override;
    virtual void drawEllipse(const QRect& p_Rect) override;

    virtual void drawPath(const QPainterPath& p_Path) override;

    virtual void drawPoints(const QPointF* p_Points, int p_PointCount) override;
    virtual void drawPoints(const QPoint* p_Points, int p_PointCount) override;

    virtual void drawPolygon(const QPointF* p_Points, int p_PointCount, PolygonDrawMode p_Mode) override;
    virtual void drawPolygon(const QPoint* p_Points, int p_PointCount, PolygonDrawMode p_Mode) override;

    virtual void drawPixmap(const QRectF& p_DestRect, const QPixmap& p_Pm, const QRectF& p_SrcRect) override;
    virtual void drawTextItem(const QPointF& p_DestPos, const QTextItem& p_TextItem) override;
    virtual void drawTiledPixmap(const QRectF& p_DestRect, const QPixmap& p_Pixmap, const QPointF& p_SrcRect) override;
    virtual void drawImage(const QRectF& p_DestRect, const QImage& p_Image, const QRectF& p_SrcRect, Qt::ImageConversionFlags p_Flags = Qt::AutoColor) override;

private:
//    void* m_Renderer;
//    UiMetalWidget* m_MetalWidget;
//    QPaintEngineState m_State;
    Scene* m_Scene;
};

#pragma once
#include "../../common/SceneGraph/Scene.h"
#include "../../common/SceneGraph/Node.h"
#include "../../common/SceneGraph/PWidget.h"

#define CIRCLE_DEFINED 1

#if CIRCLE_DEFINED == 1
class Circle;
#endif

class Rectangl;
class TransformationConformTest;
class QMouseEvent;

class SliderWidget : public CustomWidget
{
    Q_OBJECT

public:
    SliderWidget(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "");
    virtual ~SliderWidget() {}

    void SetHorizontalBarScale(float p_Scale);
    void SetIndicatorWidth(int p_IndicatorWidth);

    void BuildSlider();

signals:
    void ValueChanged(int p_NewValue);

protected:
    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual void EmitValueChanged(int p_NewValue);

public slots:
    void ShowLink(QString p_Link);

private:
    float MapSliderToWidgetRange(float p_Value);
    float MapWidgetToSliderRange(float p_Value);

public:
    void SetRange(int p_Min, int p_Max);
    void SetValue(int p_Value);

    inline int GetValue() const { return m_Value; }
    inline int GetMinimum() const { return m_Min; }
    inline int GetMaximum() const { return m_Max; }

    Rectangl* m_Indicator       = NULL;
    Rectangl* m_HorizontalBar   = NULL;

    float m_Min   = 0;
    float m_Max   = 100;
    float m_Value = 0;

    float m_HorizontalBarScale      = 0.25;
    int m_IndicatorWidth            = 20;
    Scene* m_Scene                  = NULL;
    bool m_IsMousePressed           = false;
    float m_LastIndicatorPosition   = 0.0;
};

class AudioMixerItem : public CustomWidget
{
public:
    AudioMixerItem(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name);
    virtual ~AudioMixerItem() {}

    //virtual void mouseMoveEvent(QMouseEvent* p_Event);
};

class MixerWidget : public CustomWidget
{
    Q_OBJECT

public:
    MixerWidget(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "");

    virtual ~MixerWidget() {}

    virtual void Update(Node* p_Item = NULL){}

protected:
    virtual void mousePressEvent(QMouseEvent* p_Event){}
    virtual void mouseMoveEvent(QMouseEvent* p_Event){}

private:
    Scene* m_Scene = NULL;
};

class TransformationConformTestWidget : public CustomWidget
{
    Q_OBJECT

public:
    TransformationConformTestWidget(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "");

    virtual ~TransformationConformTestWidget() {}

    virtual void Update(Node* p_Item = NULL);

protected:
    virtual void mousePressEvent(QMouseEvent* p_Event){}
    virtual void mouseMoveEvent(QMouseEvent* p_Event){}

private:
    Scene* m_Scene = NULL;

    Rectangl* m_RectTr1 = NULL;
    Rectangl* m_RectTr2 = NULL;
    Rectangl* m_RectTr3 = NULL;
    Rectangl* m_RectTr4 = NULL;

#if CIRCLE_DEFINED == 1
    Circle*   m_CircleTr5 = NULL;
#endif

    Rectangl* m_Rect1 = NULL;
    Rectangl* m_Rect2 = NULL;
    Rectangl* m_Rect3 = NULL;
    Rectangl* m_Rect4 = NULL;
};

class NodeGridWidget : public CustomWidget
{
    Q_OBJECT

public:
    NodeGridWidget(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "");
    virtual ~NodeGridWidget() {}

    virtual void Update(Node* p_Item = NULL);
    //virtual void UpdateNoTransformation();

protected:
    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);
    //virtual void mouseReleaseEvent(QMouseEvent* p_Event) {}

private:
    bool HitTest(int p_X, int p_Y, Node* p_Node, std::vector<Node *> &p_HitList, bool p_DepthSort = true);
    void SortHitTestList(std::vector<Node*>& p_HitList);
    void HandleHitTest(Node* p_HitItem);

private:
    Node* m_Root = NULL;
    Scene* m_Scene = NULL;
    std::vector<Node*> m_FlatList;
};

class UIDemo
{
public:
    UIDemo();
    virtual ~UIDemo();

    void Grid(Scene* p_Scene, int p_Width, int p_Height);
    void MixerView(Scene* p_Scene, int p_Width, int p_Height);
    void SliderFunc(Scene* p_Scene);
    void TransformationConformTest(Scene* p_Scene);
};

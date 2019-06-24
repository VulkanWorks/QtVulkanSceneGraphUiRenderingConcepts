#include "UIDemo.h"

#include "source/Rectangle/Rect.h"

#if CIRCLE_DEFINED == 1
#include "source/Circle/Circle.h"
#endif

#include <QMessageBox>
#include <QDebug>

// Todo: Parminder
// The below flag is temporary removed once multi-draw
const bool useMultiDraw = true;

UIDemo::UIDemo()
{
}

UIDemo::~UIDemo()
{
}

void UIDemo::Grid(Scene* p_Scene, int p_Width, int p_Height)
{
    BoundingRegion boundedRegion(0, 0, p_Width, p_Height);
    new NodeGridWidget(p_Scene, NULL, boundedRegion, "NodeGridClsObject");
}

void UIDemo::MixerView(Scene* p_Scene, int p_Width, int p_Height)
{
    BoundingRegion boundedRegion(0, 0, p_Width/2, p_Height);
    new MixerWidget(p_Scene, NULL, boundedRegion, "MixerWidgetClsObject");
    return;
}

void UIDemo::SliderFunc(Scene* p_Scene)
{
    new SliderWidget(p_Scene, NULL, BoundingRegion(700, 300, 400, 100), "Slider");
}

void UIDemo::TransformationConformTest(Scene* p_Scene)
{
    new TransformationConformTestWidget(p_Scene, NULL, BoundingRegion(700, 300, 400, 100), "TransformationConformTest");
}

///////////////////////////////////////////////////////////////////////////////////////////////

SliderWidget::SliderWidget(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name)
    : CustomWidget(p_Scene, p_Parent, p_BoundedRegion, p_Name)
    , m_Scene(p_Scene)
{
    BuildSlider();

//    LinkFilter* linkFilter = new LinkFilter(this);
//    this->installEventFilter(linkFilter);
//    connect(linkFilter, SIGNAL(linkClicked(QString)), this, SLOT(ShowLink(QString)));
}

void SliderWidget::SetHorizontalBarScale(float p_Scale)
{
    if (m_HorizontalBarScale != p_Scale)
    {
        m_HorizontalBarScale = p_Scale;
        BuildSlider();
    }
}

void SliderWidget::SetIndicatorWidth(int p_IndicatorWidth)
{
    if (m_IndicatorWidth != p_IndicatorWidth)
    {
        m_IndicatorWidth = p_IndicatorWidth;
        BuildSlider();
    }
}

void SliderWidget::BuildSlider()
{
    BoundingRegion bgDim(0, 0, m_BoundedRegion.m_Dimension.x, m_BoundedRegion.m_Dimension.y);

    // View the Slider rectangular region for debugging purpose - like hit test
    bool showSliderRegion = true;
    Rectangl* debugBackground = new Rectangl(m_Scene, this, bgDim, "", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    debugBackground->SetColor(glm::vec4(0.6, 1.0, 0.0, 0.5));
    debugBackground->SetDefaultColor(glm::vec4(0.42, 0.65, 0.60, 1.0));
    debugBackground->SetVisible(showSliderRegion);
    //debugBackground->SetZOrder(1.0);

    BoundingRegion horizontalBarDim(0, ((m_BoundedRegion.m_Dimension.y - 25) / 2), m_BoundedRegion.m_Dimension.x, (m_BoundedRegion.m_Dimension.y * m_HorizontalBarScale));
    m_HorizontalBar = new Rectangl(m_Scene, this, horizontalBarDim, "SliderHorizontal", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    m_HorizontalBar->SetColor(glm::vec4(0.6, 0.52, 0.320, 1.0));
    m_HorizontalBar->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));
    //m_HorizontalBar->SetZOrder(2.0);

    BoundingRegion indicatorDim(0, 0, m_IndicatorWidth, m_BoundedRegion.m_Dimension.y);
    m_Indicator = new Rectangl(m_Scene, this, indicatorDim, "SliderIndicator", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    m_Indicator->SetColor(glm::vec4(0.1, 0.52, 0.320, 1.0));
    m_Indicator->SetDefaultColor(glm::vec4(0.2, 0.15, 0.60, 1.0));
    //m_Indicator->SetZOrder(3.0);

    //m_Indicator->SetZOrder(-1.0);

    SetValue(0);
}

void SliderWidget::mousePressEvent(QMouseEvent* p_Event)
{
    float objX, objY;
    if (m_Indicator->Contains(p_Event->x(), p_Event->y(), objX, objY))
    {
        glm::vec2 mapMovePos = MapScreenToObjectCoord(p_Event->x(), p_Event->y());
        m_LastIndicatorPosition = MapWidgetToSliderRange(mapMovePos.x);
        m_IsMousePressed = true;
        return;
    }
}

void SliderWidget::mouseMoveEvent(QMouseEvent* p_Event)
{
    if (!m_IsMousePressed) return;

    float objX, objY;
    if (Contains(p_Event->x(), p_Event->y(), objX, objY))
    {
        const int value = MapWidgetToSliderRange(objX);
        SetValue(m_Value + value - m_LastIndicatorPosition);
        m_LastIndicatorPosition = value;
    }
}

void SliderWidget::mouseReleaseEvent(QMouseEvent *p_Event)
{
    m_IsMousePressed = false;
}

void SliderWidget::EmitValueChanged(int p_NewValue)
{
}

void SliderWidget::ShowLink(QString p_Link)
{
    QMessageBox msgBox;
    msgBox.setText(QString("Link Clicked: %1").arg(p_Link));
    msgBox.exec();
}

float SliderWidget::MapSliderToWidgetRange(float p_Value)
{
    int value = (/*m_Value*/p_Value - m_Min) * ((m_BoundedRegion.m_Dimension.x - 0) / (m_Max - m_Min)) + 0;
    return value;
}

float SliderWidget::MapWidgetToSliderRange(float p_Value)
{
    return (p_Value - 0) * ((m_Max - m_Min) / (m_BoundedRegion.m_Dimension.x - 0)) + m_Min;
}

void SliderWidget::SetValue(int p_Value)
{
    m_Value = p_Value;

    m_Indicator->SetPosition(MapSliderToWidgetRange(m_Value), 0.0);
}

////////////////////////////////////////////////////////////////////////////////////////////

AudioMixerItem::AudioMixerItem(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name)
    : CustomWidget(p_Scene, p_Parent, p_BoundedRegion, p_Name)
{
    Node* background = new Rectangl(m_Scene, this, BoundingRegion(0, 0, p_BoundedRegion.m_Dimension.x, p_BoundedRegion.m_Dimension.y), "Audio Mixer Background", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    background->SetColor(glm::vec4(47.0f / 255.0f, 48.0f / 255.0f, 44.0f / 255.0f, 1.0));
    background->SetDefaultColor(glm::vec4(47.0f / 255.0f, 48.0f / 255.0f, 44.0f / 255.0f, 1.0));

    const int activeIndicatorWidth = 7;
    const int activeTrackIndicatorTopMargin = 5.0;
    const int activeTrackIndicatorTopMarginLeftMargin = 4.0;
    Node* activeTrackIndicator = new Rectangl(m_Scene, background, BoundingRegion(activeTrackIndicatorTopMarginLeftMargin, activeTrackIndicatorTopMargin, p_BoundedRegion.m_Dimension.x - (5 * activeTrackIndicatorTopMarginLeftMargin), activeIndicatorWidth), "Active Track Indicator", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    activeTrackIndicator->SetColor(glm::vec4(67.0f / 255.0f, 139.0f / 255.0f, 98.0f / 255.0f, 1.0));
    activeTrackIndicator->SetDefaultColor(glm::vec4(67.0f / 255.0f, 139.0f / 255.0f, 98.0f / 255.0f, 1.0));

    static int cnt = 0;
    const int formatType = 7;
    const int channelTopMargin = activeTrackIndicatorTopMargin + 15.0;
    const int channelLeftMargin = 4.0;
    const int channelWidth = (p_BoundedRegion.m_Dimension.x / formatType) / 2;
    for (int i = 0; i < formatType; i++)
    {
        Node* channelBackground = new Rectangl(m_Scene, background, BoundingRegion((i * channelWidth) + channelLeftMargin, channelTopMargin, ((i == (formatType - 1)) ? 2 : 0) + channelWidth, p_BoundedRegion.m_Dimension.y - channelTopMargin - 5.0), "Channel", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
        channelBackground->SetColor(glm::vec4(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0));
        channelBackground->SetDefaultColor(glm::vec4(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0));

        Node* channel = new Rectangl(m_Scene, channelBackground, BoundingRegion(2, 2, channelWidth - 2, p_BoundedRegion.m_Dimension.y - channelTopMargin - 5.0 - 4), "Channel", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
        channel->SetColor(glm::vec4(47.0f / 255.0f, 48.0f / 255.0f, 44.0f / 255.0f, 1.0));
        channel->SetDefaultColor(glm::vec4(47.0f / 255.0f, 48.0f / 255.0f, 44.0f / 255.0f, 1.0));

        glm::vec4 red(246.0 / 255.0f, 24.0 / 255.0f, 39.0f / 255.0f, 1.0);
        glm::vec4 yellow(226.0 / 255.0f, 208.0 / 255.0f, 4.0f / 255.0f, 1.0);
        glm::vec4 green(29.0 / 255.0f, 148.0 / 255.0f, 56.0f / 255.0f, 1.0);
        const int totalRangeIndicator = channel->GetBoundedRegion().m_Dimension.y / 4;
        const int redIndicatorRange = totalRangeIndicator * 0.05;
        const int yellowIndicatorRange = totalRangeIndicator * 0.20;
        for (int j = 0; j < totalRangeIndicator; j++)
        {
            Node* levelIndicator = new Rectangl(m_Scene, channel, BoundingRegion(2, j * 4, channelWidth - 4.0, 2.0), "Channel", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));

            const glm::vec4 color = (j <= redIndicatorRange) ? red : ((j <= yellowIndicatorRange) ? yellow : green);
            levelIndicator->SetColor(color);
            levelIndicator->SetDefaultColor(color);
            cnt++;
        }
    }
}

NodeGridWidget::NodeGridWidget(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name)
    : CustomWidget(p_Scene, p_Parent, p_BoundedRegion, p_Name)
    , m_Scene(p_Scene)
{
    assert (m_Scene);

    float rootLevelParentColWidth = m_BoundedRegion.m_Dimension.x;
    float rootLevelParentColHeight = m_BoundedRegion.m_Dimension.y;

    float secondLevelParentCol = 20;
    float secondLevelParentRow = 20;
    float secondLevelParentColWidth = m_BoundedRegion.m_Dimension.x / secondLevelParentCol;
    float secondLevelParentColHeight = m_BoundedRegion.m_Dimension.y / secondLevelParentRow;

    const float thirdLevelParentCol = 20;
    const float thirdLevelParentRow = 20;
    float colWidth = secondLevelParentColWidth / thirdLevelParentCol;
    float colHeight = secondLevelParentColHeight / thirdLevelParentRow;

    QString name;
    static long itemNum = 0;

    // ParentParamNote: if m_Root's parent is specificed as NULL, the m_Root is added to Scene Root element
    // This will allow the scene to execute mouse events which is unnecessary because this class
    // override the mouse event and traverse nested children which similar to how m_Root traverse it children
    // when traversed by the scene.
    // This will not cause any bug but the performance got detireorate as the scene unconditinally traverse all
    // nest parent child relationship. BE AWARE OF SUCH MISTAKES
    m_Root = new Rectangl(p_Scene, this/*ParentParamNote*/, BoundingRegion(0, 0, rootLevelParentColWidth, rootLevelParentColHeight), name, (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
//    m_Root->SetZOrder(1);

    for (int i = 0; i < secondLevelParentCol; i++)
    {
        for (int j = 0; j < secondLevelParentRow; j++)
        {
            name = QString::number(++itemNum);
            Node* m_Parent = new Rectangl(p_Scene, m_Root, BoundingRegion((i * secondLevelParentColWidth), (j * secondLevelParentColHeight), secondLevelParentColWidth - 2, secondLevelParentColHeight), name, (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
            m_Parent->SetColor(glm::vec4(0.6, 0.2, 0.20, 1.0));
            m_Parent->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));
            //m_Parent->SetZOrder(-1);
            //m_Parent->SetMemPoolIdx(itemNum);

            for (int k = 0; k < thirdLevelParentCol; k++)
            {
                for (int l = 0; l < thirdLevelParentRow; l++)
                {
                    name = QString::number(++itemNum);
                    Rectangl* rect = new Rectangl(p_Scene, m_Parent, BoundingRegion((k * colWidth), (l * colHeight), colWidth, colHeight), name, (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
                    rect->SetColor(glm::vec4(l / thirdLevelParentCol, k / thirdLevelParentRow, 0.50, 1.0));
                    rect->SetDefaultColor(glm::vec4(0.2, 0.5, 0.50, 1.0));
                    //rect->SetVisible(!(k == l));
                    //rect->SetZOrder(itemNum);
                }
            }
        }
    }

    GatherFlatNodeList(m_FlatList);
}

void NodeGridWidget::Update(Node* p_Item)
{
    const int flatListSize = m_FlatList.size();
    int counter = 0;
    int limit = 100;
    static float rot = 0.01;
    rot += 0.01;
    foreach (Node* node, m_FlatList)
    {
        if (counter++ < 5) continue;
        //node->Rotate(rot, 0.0f, 0.0, 1.0f);
        //node->SetColor(glm::vec4(rot, 0.0, 0.0, 1.0));
        node->Rotate(rot, 0.0f, 0.0, 1.0f);
        node->SetColor(glm::vec4(rot, 0.0, 0.0, 1.0));
   //     qDebug() << "...." << counter;

        if (limit == counter++) break;
    }

    Node::Update(p_Item);
}

void NodeGridWidget::mousePressEvent(QMouseEvent* p_Event)
{
    std::vector<Node*> hitList;
    if (HitTest(p_Event->x(), p_Event->y(), m_Root, hitList))
    {
        HandleHitTest(hitList.back());
        //p_Event->accept();

//        QMessageBox msgBox;
//        msgBox.setText(QString("Item: %1 Clicked").arg(hitList.back()->GetName()));
//        msgBox.exec();

        return;
    }
}

void NodeGridWidget::mouseMoveEvent(QMouseEvent *p_Event)
{
    std::vector<Node*> hitList;
    if (HitTest(p_Event->x(), p_Event->y(), m_Root, hitList))
    {
        HandleHitTest(hitList.back());

        //p_Event->accept();
        return;
    }
}

bool NodeGridWidget::HitTest(int p_X, int p_Y, Node* p_Node, std::vector<Node*>& p_HitList, bool p_DepthSort/*=true*/)
{
    float objX, objY;
    if (p_Node->Contains(p_X, p_Y, objX, objY))
    {
        p_HitList.push_back(p_Node);
        QList<Node*> childList = p_Node->GetChildList();
        for (int i = 0; i < childList.size(); ++i)
        {
            Node* childNodeItem = childList.at(i);
            if (!childNodeItem) continue;

            HitTest(p_X, p_Y, childNodeItem, p_HitList);
        }
    }

    if (p_DepthSort)
    {
        SortHitTestList(p_HitList);
    }

    return !p_HitList.empty();
}

void NodeGridWidget::SortHitTestList(std::vector<Node *>& p_HitList)
{
    std::sort(p_HitList.begin(), p_HitList.end(),
        [](const Node* p_Item1, const Node* p_Item2) -> bool
    {
        if (!p_Item1 || !p_Item2) return true;

        return (p_Item1->GetBoundedRegion().m_Position.z < p_Item2->GetBoundedRegion().m_Position.z);
    });
}

void NodeGridWidget::HandleHitTest(Node* p_HitItem)
{
    static Node* oldHitItem = NULL;
    oldHitItem = m_Scene->GetCurrentHoverItem();
    m_Scene->SetCurrentHoverItem(p_HitItem);

    if (oldHitItem && oldHitItem != p_HitItem)
    {
        oldHitItem->SetColor(oldHitItem->GetDefaultColor());
    }

    p_HitItem->SetColor(glm::vec4(1.0, 1.0, 0.3, 0.5));
}

MixerWidget::MixerWidget(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name)
    : CustomWidget(p_Scene, p_Parent, p_BoundedRegion, p_Name)
    , m_Scene(p_Scene)
{
    const float mixerPanelWidth = p_BoundedRegion.m_Dimension.x;
    const float mixerPanelHeight = p_BoundedRegion.m_Dimension.y;

    const float mixerWidth = 100;
    const int numberOfMixers = mixerPanelWidth / mixerWidth;

    for (int i = 0; i < numberOfMixers; i++)
    {
        BoundingRegion boundedRegion((i * mixerWidth), 0, mixerWidth, mixerPanelHeight);
        AudioMixerItem* m_MixerItem = new AudioMixerItem(p_Scene, p_Parent, boundedRegion, "Mixer Item 1");
        m_MixerItem->SetColor(glm::vec4(1.1, 0.2, 0.20, 1.0));
        m_MixerItem->SetDefaultColor(glm::vec4(1.0, 0.15, 0.60, 1.0));
    }
}

TransformationConformTestWidget::TransformationConformTestWidget(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name)
    : CustomWidget(p_Scene, p_Parent, p_BoundedRegion, p_Name)
    , m_Scene(p_Scene)
{
    const unsigned int memppoolId = 5;
    m_RectTr1 = new Rectangl(p_Scene, NULL, BoundingRegion(200, 200, 100, 100), "", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    m_RectTr1->SetColor(glm::vec4(0.6, 0.2, 0.20, 1.0));
    m_RectTr1->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));
    m_RectTr1->SetZOrder(1.1);
    m_RectTr1->SetMemPoolIdx(memppoolId);

    m_RectTr2 = new Rectangl(p_Scene, m_RectTr1, BoundingRegion(100, 100, 50, 50), "", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    m_RectTr2->SetColor(glm::vec4(0.0, 0.0, 1.0, 1.0));
    m_RectTr2->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));
    m_RectTr2->SetMemPoolIdx(memppoolId);

    m_RectTr3 = new Rectangl(p_Scene, m_RectTr1, BoundingRegion(0, 0, 50, 50), "", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    m_RectTr3->SetColor(glm::vec4(0.6, 0.0, 1.0, 1.0));
    m_RectTr3->SetDefaultColor(glm::vec4(0.2, 0.55, 0.20, 1.0));
    m_RectTr3->SetMemPoolIdx(memppoolId);

    m_RectTr4 = new Rectangl(p_Scene, m_RectTr1, BoundingRegion(75, -25, 50, 50), "", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
    m_RectTr4->SetZOrder(-10.1);
    m_RectTr4->SetOriginOffset(glm::vec3(25, 25, 0));
    m_RectTr4->SetColor(glm::vec4(0.0, 0.2, 1.0, 1.0));
    m_RectTr4->SetDefaultColor(glm::vec4(0.2, 0.35, 0.30, 1.0));
    m_RectTr4->SetMemPoolIdx(memppoolId);

#if CIRCLE_DEFINED == 1
    m_CircleTr5 = new Circle(p_Scene, m_RectTr1, glm::vec2(0, 0), 50.0f);
    m_CircleTr5->SetOriginOffset(glm::vec3(25, 25, 0));
    m_CircleTr5->SetColor(glm::vec4(0.0, 0.5, 1.0, 1.0));
    m_CircleTr5->SetDefaultColor(glm::vec4(0.62, 0.25, 0.60, 1.0));
    m_CircleTr5->SetZOrder(10.1);
    m_CircleTr5->SetVisible(!false);
    m_CircleTr5->SetName("m_CircleTr5");
    m_CircleTr5->SetMemPoolIdx(memppoolId);
#endif

    {
        float x = 0;
        float y = 0;
        m_Rect1 = new Rectangl(p_Scene, NULL, BoundingRegion(x, y, 100, 100, -1), "Item1", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
        m_Rect1->SetColor(glm::vec4(0.6, 0.2, 0.20, 1.0));
        m_Rect1->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));
        m_Rect1->SetMemPoolIdx(memppoolId);
        x += 50;

        m_Rect2 = new Rectangl(p_Scene, m_Rect1, BoundingRegion(x, y, 100, 100, -1), "Item2", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
        m_Rect2->SetColor(glm::vec4(1.0, 0.2, 0.20, 1.0));
        m_Rect2->SetDefaultColor(glm::vec4(1.42, 0.15, 0.60, 1.0));
        m_Rect2->SetMemPoolIdx(memppoolId);
        x += 50;

        m_Rect3 = new Rectangl(p_Scene, m_Rect1, BoundingRegion(x, y, 100, 100, 10), "Item3", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
        m_Rect3->SetColor(glm::vec4(1.0, 1.2, 0.20, 1.0));
        m_Rect3->SetDefaultColor(glm::vec4(1.42, 1.15, 0.60, 1.0));
        m_Rect3->SetMemPoolIdx(memppoolId);
        x += 50;

        m_Rect4 = new Rectangl(p_Scene, m_Rect1, BoundingRegion(x, y, 100, 100, -1000), "Item4", (useMultiDraw ? SHAPE::SHAPE_RECTANGLE_MULTIDRAW : SHAPE::SHAPE_RECTANGLE_INSTANCED));
        m_Rect4->SetColor(glm::vec4(1.0, 1.2, 1.0, 1.0));
        m_Rect4->SetDefaultColor(glm::vec4(1., 0.5, 0.60, 1.0));
        m_Rect4->SetMemPoolIdx(memppoolId);
        x += 50;
    }
}

void TransformationConformTestWidget::Update(Node* p_Item)
{
    if (!m_RectTr1) return;

    static float rot = 0.0;
    {
        m_RectTr1->ResetPosition();
        m_RectTr1->Rotate(.001, 0.0, 0.0, 1.0);

        m_RectTr2->ResetPosition();
        m_RectTr2->Rotate(rot += .1, 0.0, 0.0, 1.0);

        m_RectTr3->Rotate(.003, 0.0, 0.0, 1.0);
        m_RectTr4->Rotate(.003, 0.0, 0.0, 1.0);
    }
    {
        m_Rect1->Rotate(.003, 0.0, 0.0, 1.0);
        m_Rect2->Rotate(.003, 0.0, 0.0, 1.0);
        m_Rect3->Rotate(.003, 0.0, 0.0, 1.0);
        m_Rect4->Rotate(.003, 0.0, 0.0, 1.0);
    }

    Node::Update(p_Item);
}

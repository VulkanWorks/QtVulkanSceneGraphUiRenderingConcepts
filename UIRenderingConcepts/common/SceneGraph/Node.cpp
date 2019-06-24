#include "Node.h"
#include<QMouseEvent>
#include<glm/gtx/string_cast.hpp>
#include "EngineInstance.h"

Node::Node(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name, SHAPE p_ShapeType)
    : m_Scene(p_Scene)
    , m_Parent(p_Parent)
    , m_Name(p_Name)
    , m_ShapeType(p_ShapeType)
    , m_BoundedRegion(p_BoundedRegion)
    , m_OriginOffset(glm::vec3(0.0f, 0.0f, 0.0f))
    , m_Visible(true)
    , m_DirtyType(DIRTY_TYPE::ALL)
    , m_MemPoolIdx(0)
    , m_RenderSchemeObject(NULL)
{
    m_Parent ? m_Parent->m_ChildList.append(this) : p_Scene->AddItem(this);
    
    // Todo: We can directly use the translate as the m_BoundedRegion is already set
    SetGeometry(m_BoundedRegion.m_Position.x, m_BoundedRegion.m_Position.y, m_BoundedRegion.m_Dimension.x, m_BoundedRegion.m_Dimension.y, m_BoundedRegion.m_Position.z);
}

Node::~Node()
{
    if (m_Scene)
    {
        m_Scene->RemoveItem(this);
    }
}

//void Node::Setup()
//{
//    foreach(Node* childItem, m_ChildList)
//    {
//        if (!childItem) continue;

//        childItem->Setup();
//    }
//}

// p_Item != NULL => Update is performed w.r.t. root parent
// p_Item == NULL => Update is performed w.r.t. p_Item's parent
void Node::Update(Node* p_Item)
{
//    UpdateTransform(p_Item);
    {
        QList<Node*>& childList = (p_Item ? p_Item->m_ChildList : m_ChildList);
        Q_FOREACH(Node* childItem, childList)
        {
            if (!childItem) continue;

            childItem->Update();
        }
    }
    return;
//    m_Scene->PushMatrix();
//    if (p_Item)
//    {
//        m_Scene->ApplyTransformation(p_Item->GetParentsTransformation(GetParent()) * m_ModelTransformation); // This retrives all the transformation from the parent
//    }
//    else
//    {
//        m_Scene->ApplyTransformation(m_ModelTransformation);
//    }

//    m_AbsoluteTransformation = *m_Scene->GetRefTransform().GetModelMatrix();

//    QList<Node*>& childList = (p_Item ? p_Item->m_ChildList : m_ChildList);
//    Q_FOREACH(Node* childItem, childList)
//    {
//        if (!childItem) continue;

//        childItem->Update();
//    }

//    m_Scene->PopMatrix();
}

void Node::UpdateTransformGraph(Node* p_Item)
{
    m_Scene->PushMatrix();
    if (p_Item)
    {
        m_Scene->ApplyTransformation(p_Item->GetParentsTransformation(GetParent()) * m_ModelTransformation); // This retrives all the transformation from the parent
    }
    else
    {
        m_Scene->ApplyTransformation(m_ModelTransformation);
    }

    m_AbsoluteTransformation = *m_Scene->GetRefTransform().GetModelMatrix();

    QList<Node*>& childList = (p_Item ? p_Item->m_ChildList : m_ChildList);
    Q_FOREACH(Node* childItem, childList)
    {
        if (!childItem) continue;

        childItem->UpdateTransformGraph();
    }

    m_Scene->PopMatrix();
}

void Node::Rotate(float p_Angle, float p_X, float p_Y, float p_Z)
{
    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_ModelTransformation = glm::translate(m_ModelTransformation, m_OriginOffset);
    }

    m_ModelTransformation = glm::rotate(m_ModelTransformation, p_Angle, glm::vec3(p_X, p_Y, p_Z));

    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_ModelTransformation = glm::translate(m_ModelTransformation, -m_OriginOffset);
    }

//    SetDirtyType(DIRTY_TYPE::POSITION);
    SetDirtyType(DIRTY_TYPE::ATTRIBUTES);
}

void Node::Translate(float p_X, float p_Y, float p_Z)
{
    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_ModelTransformation = glm::translate(m_ModelTransformation, m_OriginOffset);
    }

    m_ModelTransformation = glm::translate(m_ModelTransformation, glm::vec3(p_X, p_Y, p_Z));

    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_ModelTransformation = glm::translate(m_ModelTransformation, -m_OriginOffset);
    }

    SetDirtyType(DIRTY_TYPE::POSITION);
}

void Node::Scale(float p_X, float p_Y, float p_Z)
{
    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_ModelTransformation = glm::translate(m_ModelTransformation, m_OriginOffset);
    }

    m_ModelTransformation = glm::scale(m_ModelTransformation, glm::vec3(p_X, p_Y, p_Z));

    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_ModelTransformation = glm::translate(m_ModelTransformation, -m_OriginOffset);
    }

    SetDirtyType(DIRTY_TYPE::POSITION);
}

void Node::ResetPosition()
{
    Reset();
    Translate(m_BoundedRegion.m_Position.x, m_BoundedRegion.m_Position.y, m_BoundedRegion.m_Position.z);
    m_AbsoluteTransformation = m_ModelTransformation * GetParentsTransformation(GetParent());

    SetDirtyType(DIRTY_TYPE::POSITION);
}

void Node::SetZOrder(float p_ZOrder)
{
    m_BoundedRegion.m_Position.z = p_ZOrder;

    Reset();
    Translate(m_BoundedRegion.m_Position.x, m_BoundedRegion.m_Position.y, m_BoundedRegion.m_Position.z);
}

float Node::GetZOrder()
{
    return m_BoundedRegion.m_Position.z;
}

void Node::SetPosition(float p_X, float p_Y)
{
//    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
//    glm::vec4 posStartResult = m_AbsoluteTransformation * posStart;

//    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
//    glm::vec4 posEndResult = m_AbsoluteTransformation * posEnd;

    if ((p_X == 0.0f) && (p_Y == 0.0f)) return;

    m_BoundedRegion.m_Position.x = p_X;
    m_BoundedRegion.m_Position.y = p_Y;

    ResetPosition();
}

void Node::SetGeometry(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder/*=0*/)
{
    Translate(p_X, p_Y, p_ZOrder);

    m_BoundedRegion.m_Position.x = p_X;
    m_BoundedRegion.m_Position.y = p_Y;
    m_BoundedRegion.m_Position.z = p_ZOrder;

    m_BoundedRegion.m_Dimension.x = p_Width;
    m_BoundedRegion.m_Dimension.y = p_Height;
}

void Node::mousePressEvent(QMouseEvent* p_Event)
{
//    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
//    glm::vec4 posStartResult = m_AbsoluteTransformation * posStart;

//    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
//    glm::vec4 posEndResult = m_AbsoluteTransformation * posEnd;

//    cout << "\n##### mousePressEventS" << glm::to_string(posStartResult);// << posEndResult;
//    cout << "\n##### mousePressEventE" << glm::to_string(posEndResult);// << posEndResult;

//    QRect rect(QPoint(posStartResult.x, posStartResult.y), QPoint(posEndResult.x, posEndResult.y));
//    if (rect.contains(p_Event->x(), p_Event->y()))
//        cout << "\n***************";
    if (p_Event->isAccepted()) return;

    foreach(Node* childItem, m_ChildList)
    {
        assert(childItem);

        childItem->mousePressEvent(p_Event);
    }
return;

//    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
//    glm::vec4 posStartResult = m_AbsoluteTransformation * posStart;

//    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
//    glm::vec4 posEndResult = m_AbsoluteTransformation * posEnd;

//    QRect rect(QPoint(posStartResult.x, posStartResult.y), QPoint(posEndResult.x, posEndResult.y));
//    if (rect.contains(p_Event->x(), p_Event->y()))
//    {
//        m_Scene->SetCurrentHoverItem(this);

//        for (int i = m_ChildList.size() - 1; i >= 0; i--)
//        {
//            Node* childItem = m_ChildList.at(i);
//            assert(childItem);

//            childItem->mousePressEvent(p_Event);
//        }

//        p_Event->accept();
//        return;
//    }

//    p_Event->ignore();
}

void Node::mouseReleaseEvent(QMouseEvent* p_Event)
{
    if (p_Event->isAccepted()) return;

    foreach(Node* childItem, m_ChildList)
    {
        assert(childItem);

        childItem->mouseReleaseEvent(p_Event);
    }
}

void Node::mouseMoveEvent(QMouseEvent* p_Event)
{
    if (p_Event->isAccepted()) return;

    foreach(Node* childItem, m_ChildList)
    {
        assert(childItem);

        childItem->mouseMoveEvent(p_Event);
    }
return;

//    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
//    glm::vec4 posStartResult = /*GetParentsTransformation(GetParent()) **/ m_AbsoluteTransformation * posStart;

//    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
//    glm::vec4 posEndResult = /*GetParentsTransformation(GetParent()) **/ m_AbsoluteTransformation * posEnd;

//    QRect rect(QPoint(posStartResult.x, posStartResult.y), QPoint(posEndResult.x, posEndResult.y));
//    if (rect.contains(p_Event->x(), p_Event->y()))
//    {
//        m_Scene->SetCurrentHoverItem(this);

//        for (int i = m_ChildList.size() - 1; i >= 0; i--)
//        {
//            Node* childItem = m_ChildList.at(i);
//            assert(childItem);

//            childItem->mouseMoveEvent(p_Event);
//        }

//        p_Event->accept();
//        return;
//    }

//    p_Event->ignore();
}

void Node::mouseDoubleClickEvent(QMouseEvent* p_Event)
{
    Q_UNUSED(p_Event);
}

void Node::ResizeWindow(int p_Width, int p_Height)
{
    Q_UNUSED(p_Width);
    Q_UNUSED(p_Height);
}

void Node::ApplyTransformation()
{
    *m_Scene->Transform().GetModelMatrix() *= m_ModelTransformation;
}

glm::mat4 Node::GetAbsoluteTransformationComputation() const
{
    return GetParentsTransformation(GetParent()) * m_ModelTransformation;
}

glm::mat4 Node::GetParentsTransformation(Node *p_Parent) const
{
    return p_Parent ? (GetParentsTransformation(p_Parent->GetParent()) * p_Parent->m_ModelTransformation) : glm::mat4();
}

glm::vec2 Node::MapScreenToObjectCoord(float p_X, float p_Y)
{
    glm::vec3 winCoord(p_X, p_Y, 0.0f);
    glm::mat4 modelView = *m_Scene->Transform().GetViewMatrix() * GetAbsoluteTransformationComputation();
    glm::mat4 projectionMatrix = *m_Scene->Transform().GetProjectionMatrix();
    glm::vec4 viewport(0.0f, 0.0f, float(m_Scene->GetApplication()->GetWindowDimension().x), float(m_Scene->GetApplication()->GetWindowDimension().y));
    glm::vec3 objCoord = glm::unProject(winCoord, modelView, projectionMatrix, viewport);

    return glm::vec2(objCoord.x, objCoord.y);
}

bool Node::Contains(int p_X, int p_Y, float& p_XObj, float& p_YObj)
{
    glm::vec2 objCoord = MapScreenToObjectCoord(p_X, p_Y);
    QRect rect(QPoint(0, 0), QPoint(m_BoundedRegion.m_Dimension.x, m_BoundedRegion.m_Dimension.y));
    if (rect.contains(objCoord.x, objCoord.y))
    {
        p_XObj = objCoord.x;
        p_YObj = objCoord.y;
        return true;
    }

    p_XObj = 0.0f;
    p_YObj = 0.0f;

    return false;
}

Node* Node::GetParent() const
{
    return m_Parent;
}

Node* Node::GetRoot() const
{
    Node* currentNode = const_cast<Node*>(this);
    Node* parent = NULL;
    while (currentNode)
    {
        parent = currentNode->GetParent();
        if (parent == NULL)
        {
            return currentNode;
        }

        currentNode = parent;
    }

    return NULL;
}

void Node::GatherFlatNodeList()
{
    if (!m_Scene) return;

    m_Scene->AppendToFlatNodeList(this);

    Q_FOREACH(Node* childItem, m_ChildList)
    {
        assert(childItem);
        childItem->GatherFlatNodeList();
    }
}

void Node::GatherFlatNodeList(std::vector<Node*>& p_FlatList)
{
    p_FlatList.push_back(this);

    Q_FOREACH(Node* childItem, m_ChildList)
    {
        assert(childItem);
        childItem->GatherFlatNodeList(p_FlatList);
    }
}

// When a Model is updated it may need recomputation of the transformation
void Node::SetDirtyType(DIRTY_TYPE p_InvalidateType)
{
    m_DirtyType = p_InvalidateType;

    if (m_Scene && IsDirty())
    {
        const DIRTY_TYPE isPositionUpdated = static_cast<DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(DIRTY_TYPE::POSITION));
        if (isPositionUpdated == DIRTY_TYPE::POSITION)
        {
            m_Scene->SetDirtyType(static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_Scene->GetDirtyType()) | static_cast<int>(SCENE_DIRTY_TYPE::TRANSFORMATION)));
        }

        const DIRTY_TYPE isAttributeUpdated = static_cast<DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(DIRTY_TYPE::ATTRIBUTES));
        if (isAttributeUpdated == DIRTY_TYPE::ATTRIBUTES)
        {
            m_Scene->SetDirtyType(static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_Scene->GetDirtyType()) | static_cast<int>(SCENE_DIRTY_TYPE::DIRTY_ITEMS)));
        }
    }
}

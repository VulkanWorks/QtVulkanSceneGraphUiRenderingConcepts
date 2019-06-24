#include "Scene.h"
#include "Node.h"

#include "../common/SceneGraph/EngineInstance.h"
#include <QDebug>
#include <QMouseEvent>
#include <QMessageBox>

#define DIRTY_UPDATE_IMPLEMENTED 1
const float PainterAlgoDepthDiff = 0.0000001f;

Scene::Scene(BaseRenderer* p_Application, const QString& p_Name)
    : m_Application(p_Application)
    , m_Name(p_Name)
    , m_ScreenWidth(1200)
    , m_ScreenHeight(800)
    , m_Frame(0)
    , m_EarlyDepthTest(false)
    , m_CurrentHoverItem(NULL)
    , m_DirtyType(SCENE_DIRTY_TYPE::ALL)
{
    // By Default a scene is treated as 2D
    m_ViewInfo.SetViewType(ViewInfo::VIEW_TYPE_2D);
}

Scene::~Scene()
{
    qDebug() << "............DESTRUCTOR Scene";
//    std::map<SHAPE, RenderSchemeFactory*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();
    std::multimap<SHAPE, RenderSchemeFactory*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();

    while (itSRST != m_ShapeRenderSchemeTypeMap.end())
    {
        delete itSRST->second;

        ++itSRST;
    }

    foreach (Node* currentNode, m_RootDrawableList)
    {
        delete currentNode;
    }

    std::cout << "\n Scene " << m_Name.toStdString() << " Destructed.\n";
}

void Scene::AddToPipeline(Node* p_Node)
{
    RenderSchemeFactory* renderSchemeFactory = GetRenderSchemeFactory(p_Node); // Populate factories
    if (!renderSchemeFactory) return;

    renderSchemeFactory->UpdatePipelineNodeList(p_Node);

    m_RenderSchemeFactorySet.insert(renderSchemeFactory);
}

void Scene::RemoveFromPipeline(Node* p_Node)
{
    RenderSchemeFactory* renderSchemeFactory = GetRenderSchemeFactory(p_Node); // Populate factories
    if (!renderSchemeFactory) return;

    renderSchemeFactory->RemovePipelineNodeList(p_Node);

}

void Scene::Setup()
{
    GatherFlatNodeList(); // Assuming all nodes are added into the scenes by now

//    foreach (Node* currentModel, m_RootDrawableList)
//    {
//        currentModel->Setup();
//    }

    foreach (Node* currentModel, m_RootDrawableList)
    {
        currentModel->UpdateTransformGraph();
    }

//    foreach (Node* currentModel, m_FlatList)
//    {
//        RenderSchemeFactory* renderSchemeFactory = GetRenderSchemeFactory(currentModel); // Populate factories
//        if (!renderSchemeFactory) continue;

//        renderSchemeFactory->UpdatePipelineNodeList(currentModel);
//    }

//    std::multimap<SHAPE, RenderSchemeFactory*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();
//    while (itSRST != m_ShapeRenderSchemeTypeMap.end())
//    {
//        m_RenderSchemeFactorySet.insert(itSRST->second);

//        itSRST++;
//    }

    //assert(m_RenderSchemeFactorySet.size()); // Commented because painter engine adds the node on fly
    foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
    {
        currentModelFactory->Setup();
    }

    // Setup is the first time update() therefore update ALL
    m_DirtyType = SCENE_DIRTY_TYPE::ALL;
}

void Scene::Update()
{
//    for (int j = 0; j <10;)
//    {
//        qDebug() << ".";
//        j++;
//    }
//    foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
//    {
//        currentModelFactory->UpdateUniform();
//    }
    //if (!IsDirty()) return;

    foreach (Node* item, m_RootDrawableList)
    {
        assert(item);

        item->Update(); // Gather the update for dirty test
    }

    foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
    {
        glm::mat4 transformation = *m_Transform.GetProjectionMatrix() * *m_Transform.GetViewMatrix();
        currentModelFactory->SetRefProjectViewMatrix(transformation);
    }

    //if (!IsDirty()) return; // There should a dirty update for uniform as well and the factory should be update only at the time of update dirty uniform

    foreach (RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
    {
        if (!currentModelFactory) continue;

        currentModelFactory->Update();
    }

    m_DirtyType = SCENE_DIRTY_TYPE::NONE;
}

void Scene::Render(void* p_CommandBuffer)
{
    foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
    {
        currentModelFactory->Render(p_CommandBuffer);
    }
}

void Scene::SetSceneRect(float p_OriginX, float p_OriginY, float p_Width, float p_Height)
{
    m_ViewInfo.SetView2DParam(p_OriginX, p_Width, p_OriginY, p_Height);
}

void Scene::SetScenePrespective(float p_FOV, float p_AspectRatio, float p_Front, float p_Back, float p_CameraDistance)
{
    m_ViewInfo.SetView3DParam(p_FOV, p_AspectRatio, p_Front, p_Back, p_CameraDistance);
}

// The early depth testing
/*
1. Draw two rectangles(with partial alpha) in order RED(Depth 0) > GREEN(Depth 0)
Expect output: Draw Red(Below) and Green(Above)
Actual output: Draw Red(Below) and Green(Above)

2. Draw two rectangles(with partial alpha) in order RED(Depth 15) > GREEN(Depth 0)
Expect output: Draw Red(Above) and Green(Below)
Actual output: Draw Red(Above) and Green(Below), the alpha of Red above create artefact on the green rect.
               (The alpha region of red eats aways the overlap portion of Green Rectangle)

Fix: SetEarlyDepthTest(true);
Expect output: Draw Red(Above) and Green(Below)
Actual output: Draw Red(Above) and Green(Below), the alpha of Red rectangle appears correct on of top Green one.
*/
void Scene::GatherFlatNodeList()
{
    m_FlatList.clear();

    foreach (Node* item, m_RootDrawableList)
    {
        assert(item);

        item->GatherFlatNodeList();
    }

    m_PainterAlgoOrder = 0.0f;
    foreach (Node* currentModel, m_FlatList)
    {
        currentModel->SetZOrder(currentModel->GetZOrder() + m_PainterAlgoOrder);
        m_PainterAlgoOrder += PainterAlgoDepthDiff;
    }

    if (m_EarlyDepthTest)
    {
        // Sort Z-Order
        std::sort(m_FlatList.begin(), m_FlatList.end(),
            [](const Node* p_Item1, const Node* p_Item2) -> bool
        {
            if (!p_Item1 || !p_Item2) return true;

            return (p_Item1->GetBoundedRegion().m_Position.z < p_Item2->GetBoundedRegion().m_Position.z);
        });
    }
}

void Scene::AddItem(Node* p_Item)
{
    if (p_Item && !p_Item->GetParent())
    {
        m_RootDrawableList.push_back(p_Item);
    }
}

// While removing the model remove it from model list and flat list.
void Scene::RemoveItem(Node* p_Item)
{
    while (true)
    {
        auto result = std::find(std::begin(m_RootDrawableList), std::end(m_RootDrawableList), p_Item);
        if (result == std::end(m_RootDrawableList)) break;

        m_RootDrawableList.erase(result);
    }

    while (true)
    {
        auto result = std::find(std::begin(m_FlatList), std::end(m_FlatList), p_Item);
        if (result == std::end(m_FlatList)) break;

        m_FlatList.erase(result);
    }

    RenderSchemeFactory* factory = GetRenderSchemeFactory(p_Item); // Populate factories
    if (!factory) return;

    factory->RemovePipelineNodeList(p_Item);
}

void Scene::Resize(int p_Width, int p_Height)
{
    m_ScreenWidth = p_Width;
    m_ScreenHeight = p_Height;

    m_Transform.SetMatrixMode(Transformation::PROJECTION_MATRIX);
    m_Transform.LoadIdentity();

    if (m_ViewInfo.GetViewType() == ViewInfo::VIEW_TYPE_2D)
    {
#ifdef HAVE_METAL_GRAPHICS_API
        m_Transform.Ortho(m_ViewInfo.GetView2DParam().left,
                          (IsUsingDynamicSurfaceDimensions() ? m_ScreenWidth  : m_ViewInfo.GetView2DParam().right),
                          (IsUsingDynamicSurfaceDimensions() ? m_ScreenHeight : m_ViewInfo.GetView2DParam().top),
                          m_ViewInfo.GetView2DParam().bottom,
                          m_ViewInfo.GetView2DParam().front,
                          m_ViewInfo.GetView2DParam().back);
#else
        m_Transform.Ortho(m_ViewInfo.GetView2DParam().left,
                          (IsUsingDynamicSurfaceDimensions() ? m_ScreenWidth  : m_ViewInfo.GetView2DParam().right),
                          m_ViewInfo.GetView2DParam().bottom,
                          (IsUsingDynamicSurfaceDimensions() ? m_ScreenHeight : m_ViewInfo.GetView2DParam().top),
                          m_ViewInfo.GetView2DParam().front,
                          m_ViewInfo.GetView2DParam().back);
#endif
    }
    else
    {
        const float aspectRatio = IsUsingDynamicSurfaceDimensions() ? (static_cast<float>(m_ScreenWidth) / static_cast<float>(m_ScreenHeight)) : m_ViewInfo.GetView3DParam().aspectRatio;
        m_Transform.SetPerspective(m_ViewInfo.GetView3DParam().fov,
                                   aspectRatio,
                                   m_ViewInfo.GetView3DParam().nearPlane,
                                   m_ViewInfo.GetView3DParam().farPlane);
    }

    m_Transform.SetMatrixMode(Transformation::VIEW_MATRIX);
    m_Transform.LoadIdentity();

    if (m_ViewInfo.GetViewType() == ViewInfo::VIEW_TYPE_3D)
    {
        m_Transform.Translate(0.0f, 0.0f, m_ViewInfo.GetView3DParam().cameraDistance);
    }

    m_Transform.SetMatrixMode(Transformation::MODEL_MATRIX);

    foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
    {
        currentModelFactory->ResizeWindow();
    }
}

void Scene::mousePressEvent(QMouseEvent* p_Event)
{
    foreach (Node* item, m_RootDrawableList)
    {
        assert(item);

        //if (p_Event->isAccepted()) return;

        item->mousePressEvent(p_Event);
    }

    return;
    static Node* oldModelItem = NULL;
    for (int i = m_RootDrawableList.size() - 1; i >= 0; i--)
    {
        Node* item = m_RootDrawableList.at(i);
        assert(item);

        item->mousePressEvent(p_Event);
        if (p_Event->isAccepted())
        {
            Node* currentModel = GetCurrentHoverItem();
            if (oldModelItem && oldModelItem != currentModel)
            {
                oldModelItem->SetColor(oldModelItem->GetDefaultColor());
            }

            currentModel->SetColor(glm::vec4(1.0, 1.0, 0.3, 0.5));
            oldModelItem = GetCurrentHoverItem();

            QMessageBox msgBox;
            msgBox.setText(QString("Item: %1 Clicked").arg(currentModel->GetName()));
            msgBox.exec();

            return;
        }
    }

    if (oldModelItem)
    {
        oldModelItem->SetColor(oldModelItem->GetDefaultColor());
    }
}

void Scene::mouseReleaseEvent(QMouseEvent* p_Event)
{
    foreach (Node* item, m_RootDrawableList)
    {
        assert(item);

        item->mouseReleaseEvent(p_Event);
    }
}

void Scene::mouseMoveEvent(QMouseEvent* p_Event)
{
    foreach (Node* item, m_RootDrawableList)
    {
        assert(item);

        item->mouseMoveEvent(p_Event);
    }

return;
    static Node* oldModelItem = NULL;
    for (int i = m_RootDrawableList.size() - 1; i >= 0; i--)
    {
        Node* item = m_RootDrawableList.at(i);
        assert(item);

        item->mouseMoveEvent(p_Event);
        if (p_Event->isAccepted())
        {
            Node* currentModel = GetCurrentHoverItem();
            if (oldModelItem && oldModelItem != currentModel)
            {
                oldModelItem->SetColor(oldModelItem->GetDefaultColor());
            }

            currentModel->SetColor(glm::vec4(1.0, 1.0, 0.3, 0.5));
            oldModelItem = GetCurrentHoverItem();
            return;
        }
    }

    if (oldModelItem)
    {
        oldModelItem->SetColor(oldModelItem->GetDefaultColor());
    }

}

RenderSchemeFactory* Scene::GetRenderSchemeFactory(Node* p_Item)
{
    const SHAPE shapeType = p_Item->GetShapeType();
    if ((shapeType <= SHAPE::SHAPE_NONE) && (shapeType >= SHAPE::SHAPE_COUNT)) return NULL;

    typedef std::multimap<SHAPE, RenderSchemeFactory*>::iterator MMAPIterator;
    std::pair<MMAPIterator, MMAPIterator> result = m_ShapeRenderSchemeTypeMap.equal_range(shapeType);
    for (MMAPIterator it = result.first; it != result.second; it++)
    {
        if (it->second->GetMemPoolIdx() == p_Item->GetMemPoolIdx())
            return it->second;
    }

//    std::map<SHAPE, RenderSchemeFactory*>::iterator it = m_ShapeRenderSchemeTypeMap.find(shapeType);
//    if (it != m_ShapeRenderSchemeTypeMap.end())
//    {
//        return it->second;
//    }

    RenderSchemeFactory* renderSchemeFactoryItem = p_Item->GetRenderSchemeFactory();
    if (!renderSchemeFactoryItem) return NULL;

    renderSchemeFactoryItem->SetMemPoolIdx(p_Item->GetMemPoolIdx());
    m_ShapeRenderSchemeTypeMap.insert (std::make_pair(shapeType, renderSchemeFactoryItem));

    return renderSchemeFactoryItem;
}
/*
RenderSchemeFactory* Scene::GetRenderSchemeFactory(Node* p_Item)
{
    const SHAPE shapeType = p_Item->GetShapeType();
    if ((shapeType <= SHAPE::SHAPE_NONE) && (shapeType >= SHAPE::SHAPE_COUNT)) return NULL;

    std::map<SHAPE, RenderSchemeFactory*>::iterator it = m_ShapeRenderSchemeTypeMap.find(shapeType);
    if (it != m_ShapeRenderSchemeTypeMap.end())
    {
        return it->second;
    }

    RenderSchemeFactory* renderSchemeFactoryItem = p_Item->GetRenderSchemeFactory();
    if (renderSchemeFactoryItem)
    {
        (m_ShapeRenderSchemeTypeMap)[shapeType] = renderSchemeFactoryItem;
    }

    return renderSchemeFactoryItem;
}
*/
void Scene::AppendToFlatNodeList(Node* p_Item)
{
    m_FlatList.push_back(p_Item);
}

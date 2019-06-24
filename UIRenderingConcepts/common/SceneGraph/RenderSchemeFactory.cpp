#include "RenderSchemeFactory.h"
#include <QDebug>

#define UNDEFINED_IMPLEMENTATION qDebug() << "Unimplemented: " << Q_FUNC_INFO;

RenderSchemeFactory::RenderSchemeFactory(BaseRenderer* p_Renderer)
    : m_Renderer(p_Renderer)
    , m_MemPoolIdx(0)
{
}

RenderSchemeFactory::~RenderSchemeFactory() {}

/**
 * @brief RenderSchemeFactory::Setup - Setup represents a state of initilization where the scene does knows all
 * its nodes. This function is useful for instancig based scheme where allocation happens in burst and the life time
 * of the nodes are persistent. Immediate mode node are not good place to initialize here.
 */
void RenderSchemeFactory::Setup() {}

void RenderSchemeFactory::Update() {}

void RenderSchemeFactory::Render(void* p_CommandBuffer) {}

void RenderSchemeFactory::UpdatePipelineNodeList(Node* p_Item) { Q_UNUSED(p_Item) }

void RenderSchemeFactory::RemovePipelineNodeList(Node* p_Item) { Q_UNUSED(p_Item) }

/**
 * @brief RenderSchemeFactory::ResizeWindow - This function indicates the underlying implmentation about the
 * window resizing. The argument may be optional for a given implemenation like Metal. The underlaying implementation
 * can react to resizing - For example: in Vulkan implementation if the dynamic states are not configured then
 * the viewport or scisorring may endup by re-creation of the pipelines. On the otherhand the Metal API are more
 * explicitly sets the viewport parameter from the encoder itself and therefore the may not required to re-create the pipeline.
 */
void RenderSchemeFactory::ResizeWindow()
{
}

/**
 * @brief RenderSchemeFactory::GetPipeline:
 * @param p_PipelineType
 * @return Returns - MTLRenderPipelineState, VkPipeline as per specific implementation"
 */
void* RenderSchemeFactory::GetPipeline(int p_PipelineType)
{
    Q_UNUSED(p_PipelineType)

    UNDEFINED_IMPLEMENTATION
    qDebug(" // Returns - MTLRenderPipelineState, VkPipeline as per specific implementation");
    assert(false);
}

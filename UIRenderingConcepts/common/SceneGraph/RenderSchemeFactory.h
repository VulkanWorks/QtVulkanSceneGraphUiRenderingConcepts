#pragma once

#include "Scene.h"
#include "../common/SceneGraph/Common.h"

class BaseRenderer;

// Rename to RenderSchemeBase
class RenderSchemeFactory
{
public:
    RenderSchemeFactory(BaseRenderer* p_Renderer);
    virtual ~RenderSchemeFactory();

    virtual void Setup();
    virtual void Update();
    virtual void Render(void* p_CommandBuffer); // Command buffer Or Render encoder

    virtual void UpdatePipelineNodeList(Node* p_Item);
    virtual void RemovePipelineNodeList(Node* p_Item);

    virtual void ResizeWindow();
    virtual void* GetPipeline(int p_PipelineType);

    GETSET(glm::mat4x4, ProjectViewMatrix)
    GETSET(BaseRenderer*, Renderer)
    GETSET(unsigned int, MemPoolIdx)

protected:
    QMap<int, void*> m_GraphicsPipelineMap;
};

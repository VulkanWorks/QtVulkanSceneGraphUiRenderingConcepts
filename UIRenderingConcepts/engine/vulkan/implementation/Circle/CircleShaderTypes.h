#pragma once

#define VERTEX_BUFFER_BIND_IDX 0

//static char* PIPELINE_CIRCLE_FILLED = "CircleFilled";
//static char* PIPELINE_CIRCLE_OUTLINE = "CircleOutline";

struct CircleVertex
{
    glm::vec3 m_Position;       // Vertex Position => x, y, z
    glm::vec2 m_TexCoord;       // TexCoord format => u, v
    unsigned int m_DrawType;
};

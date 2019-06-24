#pragma once

#define VERTEX_BUFFER_BIND_IDX 0
#define INSTANCE_BUFFER_BIND_IDX 1

//static char* PIPELINE_RECT_FILLED = "RectFilled";
//static char* PIPELINE_RECT_OUTLINE = "RectOutline";

struct Vertex
{
    glm::vec3 m_Position;       // Vertex Position => x, y, z
    glm::vec3 m_Color;          // Color format => r, g, b
//    unsigned int m_DrawType;
};

#pragma once

/*********** GLM HEADER FILES ***********/
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

enum SCENE_GRAPH_STATES
{
    SG_STATE_NONE = -1,

    SG_STATE_SETUP,     // Initial setup
    SG_STATE_RESIZE,    // Resize of window
    SG_STATE_RENDER,    // Scene Rendering

    SG_STATE_CUSTOM
};

// 1. Rename SHAPE to SCHEME
// 2. Think over this: Can we reduce the enum by only keeping the scheme type lke enum SCHEME { MULTI_DRAW, INSTANCED }, with SHAPE there is a risk
//    that the user can create a RECTANGLE object by by Rectnagl class but pass the SHAPE_CIRCLE_MULTIDRAW/INSTANCED enum as scheme type.
//    This is not good.
enum /*class */SHAPE/* : int32_t*/
{
    SHAPE_NONE = -1,

    // BASIC SHAPES GOES HERE
    SHAPE_RECTANGLE_MULTIDRAW,
    SHAPE_RECTANGLE_INSTANCED,

    SHAPE_CIRCLE_MULTIDRAW,
    SHAPE_CIRCLE_INSTANCED,

    SHAPE_COUNT, // TOTAL SHAPE COUNTS

    // CUSTOM SHAPES GOES HERE
    SHAPE_CUSTOM // NOTE: Custom Node used as Widget this inherits QObject acts an interface by Qt and Graphics engine.
};

enum class DIRTY_TYPE : uint32_t
{
    NONE       = (0u),
    ATTRIBUTES = (1u << 0u),
    POSITION   = (1u << 1u),
    ALL        = (POSITION | ATTRIBUTES),
};

enum class SCENE_DIRTY_TYPE : uint32_t
{
    NONE            = (0u),
    TRANSFORMATION  = (1u << 0u),
    DIRTY_ITEMS     = (1u << 2u),
    ALL             = (TRANSFORMATION),
};

struct BoundingRegion
{
    BoundingRegion(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder = 0.0f) // For 2D Bounding Box
    {
        m_Position.x = p_X;         m_Position.y = p_Y;        m_Position.z = p_ZOrder;
        m_Dimension.x = p_Width;    m_Dimension.y = p_Height;  m_Dimension.z = p_ZOrder;
    }

    BoundingRegion(float p_X, float p_Y, float p_Z, float p_Width, float p_Height, float p_Depth)
    {
        m_Position.x = p_X;         m_Position.y = p_Y;        m_Position.z = p_Z;
        m_Dimension.x = p_Width;    m_Dimension.y = p_Height;  m_Dimension.z = p_Depth;
    }

    glm::vec3 m_Position;
    glm::vec3 m_Dimension;
};

using namespace std;
#define GETSET(type, var) \
    protected: type m_##var; \
    public: type Get##var() { return m_##var; } \
    void Set##var(type val) { m_##var = val; } \
    type& GetRef##var() { return m_##var; } \
    void SetRef##var(type& val) { m_##var = val; }

#define UNIMPLEMENTED_INTEFACE { printf("\n Attempting to use an unimplemented default interface: %s\n", __FUNCTION__); assert(0); }

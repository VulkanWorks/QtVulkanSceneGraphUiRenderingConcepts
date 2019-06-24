#pragma once

#define HAVE_METAL 0

#if HAVE_METAL == 1
    @import MetalKit;

    #define vec3 float3;
    #define vec4 float4
    #define mat3 matrix_float3x3
    #define mat4 matrix_float4x4
#else
    #define GLM_FORCE_RADIANS
    #define GLM_FORCE_DEPTH_ZERO_TO_ONE
    #include "glm/glm.hpp"
    #include <glm/gtc/matrix_transform.hpp>
    using namespace glm;

    #define TransformScale glm::scale
    #define TransformOrtho glm::ortho
#endif

//! The depth of the projection matrix stack.
#define MAX_PROJECTION_MATRIX	2

//! The depth of the model matrix stack.
#define MAX_MODEL_MATRIX	8

//! The depth of the view matrix stack.
#define MAX_VIEW_MATRIX	8

//! The depth of the texture matrix stack.
#define MAX_TEXTURE_MATRIX		2

//! The definition of the global  structure. This structure maintain the matrix stacks and current indexes. 
typedef struct
{
    //! The current matrix mode (either MODEL_MATRIX, VIEW_MATRIX, PROJECTION_MATRIX, TEXTURE_MATRIX).
    unsigned char	matrix_mode;

    //! The current modelview matrix index in the stack.
    unsigned char   modelMatrixIndex;

    //! The current modelview matrix index in the stack.
    unsigned char   viewMatrixIndex;

    //! The current projection matrix index in the stack.
    unsigned char   projectionMatrixIndex;

    //! The current texture matrix index in the stack.
    unsigned char   textureMatrixIndex;

    //! Array of 4x4 matrix that represent the model matrix stack.
    mat4            model_matrix[ MAX_MODEL_MATRIX ];

    //! Array of 4x4 matrix that represent the view matrix stack.
    mat4            view_matrix[ MAX_VIEW_MATRIX ];

    //! Array of 4x4 matrix that represent the projection matrix stack.
    mat4            projection_matrix[ MAX_PROJECTION_MATRIX ];

    //! Array of 4x4 matrix that represent the texture matrix stack.
    mat4            texture_matrix[ MAX_TEXTURE_MATRIX ];

    //! Used to store the result of the modelview matrix multiply by the projection matrix. \sa _get_modelview_projection_matrix
    mat4            modelview_projection_matrix;

    //! Used to store the result of the modelview matrix multiply by the projection matrix. \sa _get_modelview_projection_matrix
    mat4            modelview_matrix;

    //! Used to store the result of the inverse, tranposed modelview matrix. \sa _get_normal_matrix
    mat3            normal_matrix;
} TransformData;

class Transformation
{
public:
    enum
    {
        //! The model matrix identifier.
        MODEL_MATRIX = 0,

        //! The view matrix identifier.
        VIEW_MATRIX = 1,

        //! The projection matrix identifier.
        PROJECTION_MATRIX = 2,

        //! The texture matrix identifier.
        TEXTURE_MATRIX = 3
    };

    Transformation(void);
    ~Transformation(void);

    void Init( void );

    void SetMatrixMode( unsigned int mode );

    void LoadIdentity( void );

    void PushMatrix( void );

    void PopMatrix( void );

    void LoadMatrix( mat4 *m );

    void MultiplyMatrix( mat4 *m );

    void Translate( float x, float y, float z );

    void Rotate( float angle, float x, float y, float z );

    void Scale( float x, float y, float z );

    mat4* GetModelMatrix( void );
    
    mat4* GetViewMatrix( void );
    
    mat4* GetProjectionMatrix( void );

    mat4* GetTextureMatrix( void );

    mat4* GetModelViewProjectionMatrix( void );

    mat4* GetModelViewMatrix( void );

    void GetNormalMatrix( mat3* );

    void Ortho( float left, float right, float bottom, float top, float clip_start, float clip_end );

    void OrthoGrahpic( float screen_ratio, float scale, float aspect_ratio, float clip_start, float clip_end, float screen_orientation );

    void SetPerspective( float fovy, float aspect_ratio, float clip_start, float clip_end);

    void LookAt( vec3 *eye, vec3 *center, vec3 *up );

    void SetView(mat4 mat);

    int TransformProject( float objx, float objy, float objz, mat4 *modelview_matrix, mat4 *projection_matrix, int *viewport_matrix, float *winx, float *winy, float *winz );

    int TransformUnproject( float winx, float winy, float winz, mat4 *modelview_matrix, mat4 *projection_matrix, int *viewport_matrix, float *objx, float *objy, float *objz );

    void Vec4MultiplyMat4( vec4* dst, vec4 *v0, mat4 *v1 );

    TransformData TransformMemData;
};

#include "Transformation.h"

#include <cstring> // Added for memcpy(required by MaxOS)

Transformation::Transformation(void)
{
    TransformMemData.modelMatrixIndex       = 0;
    TransformMemData.viewMatrixIndex        = 0;
    TransformMemData.projectionMatrixIndex  = 0;
    TransformMemData.textureMatrixIndex     = 0;
}

Transformation::~Transformation(void)
{
}

void Transformation::Init( void )
{
    memset( &TransformMemData, 0, sizeof( TransformData ) );

    SetMatrixMode( TEXTURE_MATRIX );
    LoadIdentity();

    SetMatrixMode( PROJECTION_MATRIX );
    LoadIdentity();

    SetMatrixMode( MODEL_MATRIX );
    LoadIdentity();

    SetMatrixMode( VIEW_MATRIX );
    LoadIdentity();
}

void Transformation::SetMatrixMode( unsigned int mode )
{
    TransformMemData.matrix_mode = mode;
}

void Transformation::LoadIdentity( void )
{
    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            memset(GetModelMatrix(), 0, sizeof(mat4));
            mat4* mv = GetModelMatrix();
            (*mv)[0][0] = (*mv)[1][1] = (*mv)[2][2] = (*mv)[3][3] = 1.0;
            break;
        }

        case VIEW_MATRIX:
        {
            memset(GetViewMatrix(), 0, sizeof(mat4));
            mat4* mv = GetViewMatrix();
            (*mv)[0][0] = (*mv)[1][1] = (*mv)[2][2] = (*mv)[3][3] = 1.0;
            break;
        }

        case PROJECTION_MATRIX:
        {
            memset(GetProjectionMatrix(), 0, sizeof(mat4));
            mat4* pm = GetProjectionMatrix();
            (*pm)[0][0] = (*pm)[1][1] = (*pm)[2][2] = (*pm)[3][3] = 1.0;
            break;
        }

        case TEXTURE_MATRIX:
        {
            memset(GetTextureMatrix(), 0, sizeof(mat4));
            mat4* tm = GetTextureMatrix();
            (*tm)[0][0] = (*tm)[1][1] = (*tm)[2][2] = (*tm)[3][3] = 1.0;
            break;
        }
    }
}

mat4* Transformation::GetModelMatrix( void )
{
    return &TransformMemData.model_matrix[ TransformMemData.modelMatrixIndex ];
}


mat4* Transformation::GetViewMatrix( void )
{
    return &TransformMemData.view_matrix[ TransformMemData.viewMatrixIndex ];
}

mat4* Transformation::GetProjectionMatrix( void )
{
    return &TransformMemData.projection_matrix[ TransformMemData.projectionMatrixIndex ];
}

mat4* Transformation::GetTextureMatrix( void )
{
    return &TransformMemData.texture_matrix[ TransformMemData.textureMatrixIndex ];
}

mat4* Transformation::GetModelViewProjectionMatrix( void )
{
    TransformMemData.modelview_projection_matrix = *GetProjectionMatrix() *
    *GetViewMatrix() * *GetModelMatrix();
    return &TransformMemData.modelview_projection_matrix;
}

mat4* Transformation::GetModelViewMatrix( void )
{
    TransformMemData.modelview_matrix =
    *GetViewMatrix() * *GetModelMatrix();
    return &TransformMemData.modelview_matrix;
}

void Transformation::PushMatrix( void )
{
    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            if (TransformMemData.modelMatrixIndex >= MAX_MODEL_MATRIX){
                return;
            }

            memcpy( &TransformMemData.model_matrix[ TransformMemData.modelMatrixIndex + 1 ],
                   &TransformMemData.model_matrix[ TransformMemData.modelMatrixIndex     ],
                   sizeof( mat4 ) );

            ++TransformMemData.modelMatrixIndex;

            break;
        }

        case VIEW_MATRIX:
        {
            if (TransformMemData.viewMatrixIndex >= MAX_VIEW_MATRIX){
                return;
            }

            memcpy( &TransformMemData.view_matrix[ TransformMemData.viewMatrixIndex + 1 ],
                   &TransformMemData.view_matrix[ TransformMemData.viewMatrixIndex     ],
                   sizeof( mat4 ) );

            ++TransformMemData.viewMatrixIndex;

            break;
        }

        case PROJECTION_MATRIX:
        {
            if (TransformMemData.projectionMatrixIndex >= MAX_PROJECTION_MATRIX){
                return;
            }

            memcpy( &TransformMemData.projection_matrix[ TransformMemData.projectionMatrixIndex + 1 ],
                    &TransformMemData.projection_matrix[ TransformMemData.projectionMatrixIndex     ],
                    sizeof( mat4 ) );

            ++TransformMemData.projectionMatrixIndex;

            break;
        }

        case TEXTURE_MATRIX:
        {
            if (TransformMemData.textureMatrixIndex >= MAX_TEXTURE_MATRIX){
                return;
            }

            memcpy( &TransformMemData.texture_matrix[ TransformMemData.textureMatrixIndex + 1 ],
                    &TransformMemData.texture_matrix[ TransformMemData.textureMatrixIndex     ],
                    sizeof( mat4 ) );

            ++TransformMemData.textureMatrixIndex;
            break;
        }
    }
}

void Transformation::PopMatrix( void )
{
    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            if (TransformMemData.modelMatrixIndex == 0){
                return;
            }
            --TransformMemData.modelMatrixIndex;

            break;
        }

        case VIEW_MATRIX:
        {
            if (TransformMemData.viewMatrixIndex == 0){
                return;
            }
            --TransformMemData.viewMatrixIndex;

            break;
        }

        case PROJECTION_MATRIX:
        {
            if (TransformMemData.projectionMatrixIndex == 0){
                return;
            }
            --TransformMemData.projectionMatrixIndex;

            break;
        }

        case TEXTURE_MATRIX:
        {
            if (TransformMemData.textureMatrixIndex == 0){
                return;
            }
            --TransformMemData.textureMatrixIndex;

            break;
        }
    }
}

void Transformation::LoadMatrix( mat4 *m )
{
    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            memcpy(GetModelMatrix(), m, sizeof(mat4));

            break;
        }

        case VIEW_MATRIX:
        {
            memcpy(GetViewMatrix(), m, sizeof(mat4));

            break;
        }

        case PROJECTION_MATRIX:
        {
            memcpy(GetProjectionMatrix(), m, sizeof(mat4));

            break;
        }

        case TEXTURE_MATRIX:
        {
            memcpy(GetTextureMatrix(), m, sizeof(mat4));

            break;
        }
    }
}

void Transformation::MultiplyMatrix( mat4 *m )
{
    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            *GetModelMatrix() *=  (*m);

            break;
        }

        case VIEW_MATRIX:
        {
            *GetViewMatrix() *=  (*m);

            break;
        }

        case PROJECTION_MATRIX:
        {
            *GetProjectionMatrix() *=  (*m);

            break;
        }

        case TEXTURE_MATRIX:
        {
            *GetTextureMatrix() *=  (*m);

            break;
        }
    }
}

void Transformation::Translate( float x, float y, float z )
{
    vec3 v( x, y, z );

    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            *GetModelMatrix() = translate( *GetModelMatrix(), v);

            break;
        }

        case VIEW_MATRIX:
        {
            *GetViewMatrix() = translate( *GetViewMatrix(), v);
            break;
        }

        case PROJECTION_MATRIX:
        {
            *GetProjectionMatrix() = translate( *GetProjectionMatrix(), v);

            break;
        }

        case TEXTURE_MATRIX:
        {
            *GetTextureMatrix() = translate( *GetTextureMatrix(), v);

            break;
        }
    }
}

void Transformation::Rotate( float angle, float x, float y, float z )
{
    if( !angle ) return;

    vec3 v( x, y, z);

    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            *GetModelMatrix() = rotate( *GetModelMatrix(), angle, v );

            break;
        }

        case VIEW_MATRIX:
        {
            *GetViewMatrix() = rotate( *GetViewMatrix(), angle, v );

            break;
        }

        case PROJECTION_MATRIX:
        {
            *GetProjectionMatrix() = rotate( *GetProjectionMatrix(), angle, v );

            break;
        }

        case TEXTURE_MATRIX:
        {
            *GetTextureMatrix() = rotate( *GetTextureMatrix(), angle, v );

            break;
        }
    }
}

void Transformation::Scale( float x, float y, float z )
{
    static vec3 scale( 1.0f, 1.0f, 1.0f );

    vec3 v( x, y, z );

    if( !memcmp( &v, &scale, sizeof( vec3 ) ) ) return;


    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            *GetModelMatrix() = TransformScale(*GetModelMatrix(), v);

            break;
        }

        case VIEW_MATRIX:
        {
            *GetViewMatrix() = TransformScale(*GetViewMatrix(), v);

            break;
        }

        case PROJECTION_MATRIX:
        {
            *GetProjectionMatrix() = TransformScale(*GetProjectionMatrix(), v);

            break;
        }

        case TEXTURE_MATRIX:
        {
            *GetTextureMatrix() = TransformScale(*GetTextureMatrix(), v);

            break;
        }
    }
}

void Transformation::GetNormalMatrix( mat3* mat3Obj )
{
    mat4 mat4Obj;
    mat4Obj = inverse(transpose(*GetModelViewMatrix()));

    memcpy(&(*mat3Obj)[0], &mat4Obj[0], sizeof(mat3));
    memcpy(&(*mat3Obj)[1], &mat4Obj[0], sizeof(mat3));
    memcpy(&(*mat3Obj)[2], &mat4Obj[0], sizeof(mat3));
}

void Transformation::Ortho(float left, float right, float bottom, float top, float clip_start, float clip_end )
{
    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            *GetModelMatrix() = TransformOrtho(left, right, bottom, top, clip_start, clip_end);
            break;
        }

        case VIEW_MATRIX:
        {
            *GetViewMatrix() = TransformOrtho(left, right, bottom, top, clip_start, clip_end);
            break;
        }

        case PROJECTION_MATRIX:
        {
            //mat4	*matProjection = GetProjectionMatrix();
            *GetProjectionMatrix() = TransformOrtho(left, right, bottom, top, clip_start, clip_end);
            //*matProjection = ortho(left, right, bottom, top, clip_start, clip_end);

            break;
        }

        case TEXTURE_MATRIX:
        {
            *GetTextureMatrix() = TransformOrtho(left, right, bottom, top, clip_start, clip_end);
            break;
        }
    }
}

void Transformation::OrthoGrahpic( float screen_ratio, float scale, float aspect_ratio, float clip_start, float clip_end, float orientation )
{
    scale = ( scale * 0.5f ) * aspect_ratio;

    Ortho( -1.0f, 1.0f, -screen_ratio, screen_ratio, clip_start, clip_end );
    Scale( 1.0f / scale, 1.0f / scale, 1.0f );

    if ( orientation )
    {
        Rotate( orientation, 0.0f, 0.0f, 1.0f );
    }
}

void Transformation::SetPerspective( float fovy, float aspect_ratio, float clip_start, float clip_end )
{
    mat4 mat;

    switch( TransformMemData.matrix_mode )
    {
        case MODEL_MATRIX:
        {
            *GetModelMatrix() = perspective(fovy, aspect_ratio, clip_start, clip_end);
            break;
        }

        case VIEW_MATRIX:
        {
            *GetViewMatrix() = perspective(fovy, aspect_ratio, clip_start, clip_end);
            break;
        }

        case PROJECTION_MATRIX:
        {
            *GetProjectionMatrix() = perspective(fovy, aspect_ratio, clip_start, clip_end);
            break;
        }

        case TEXTURE_MATRIX:
        {
            *GetTextureMatrix() = perspective(fovy, aspect_ratio, clip_start, clip_end);
            break;
        }
    }
}

void Transformation::SetView(mat4 mat)
{
    TransformMemData.view_matrix[ TransformMemData.viewMatrixIndex ] = mat;
}

void Transformation::LookAt( vec3 *eye, vec3 *center, vec3 *up )
{
    *GetViewMatrix() = lookAt(*eye, *center, *up);
}

int Transformation::TransformProject( float objextx, float objecty, float objectz, mat4 *modelview_matrix, mat4 *projection_matrix, int *viewport_matrix, float *windowx, float *windowy, float *windowz )
{
    vec4 vin, vout;

    vin.x = objextx;
    vin.y = objecty;
    vin.z = objectz;
    vin.w = 1.0f;

    Vec4MultiplyMat4( &vout, &vin, modelview_matrix );

    Vec4MultiplyMat4( &vin, &vout, projection_matrix );

    if( !vin.w ) return 0;

    vin.x /= vin.w;
    vin.y /= vin.w;
    vin.z /= vin.w;

    vin.x = vin.x * 0.5f + 0.5f;
    vin.y = vin.y * 0.5f + 0.5f;
    vin.z = vin.z * 0.5f + 0.5f;

    vin.x = vin.x * viewport_matrix[ 2 ] + viewport_matrix[ 0 ];
    vin.y = vin.y * viewport_matrix[ 3 ] + viewport_matrix[ 1 ];

    *windowx = vin.x;
    *windowy = vin.y;
    *windowz = vin.z;

    return 1;
}

void Transformation::Vec4MultiplyMat4( vec4 *dst, vec4 *v, mat4 *m )
{
    dst->x = ( v->x * (*m)[ 0 ].x ) +
             ( v->y * (*m)[ 1 ].x ) +
             ( v->z * (*m)[ 2 ].x ) +
             ( v->w * (*m)[ 3 ].x );

    dst->y = ( v->x * (*m)[ 0 ].y ) +
             ( v->y * (*m)[ 1 ].y ) +
             ( v->z * (*m)[ 2 ].y ) +
             ( v->w * (*m)[ 3 ].y );

    dst->z = ( v->x * (*m)[ 0 ].z ) +
             ( v->y * (*m)[ 1 ].z ) +
             ( v->z * (*m)[ 2 ].z ) +
             ( v->w * (*m)[ 3 ].z );

    dst->w = ( v->x * (*m)[ 0 ].w ) +
             ( v->y * (*m)[ 1 ].w ) +
             ( v->z * (*m)[ 2 ].w ) +
             ( v->w * (*m)[ 3 ].w );
}

int Transformation::TransformUnproject( float windowx, float windowy, float windowz, mat4 *modelview_matrix, mat4 *projection_matrix, int *viewport_matrix, float *objextx, float *objecty, float *objectz )
{
    mat4 final;

    vec4 vin,
         vout;

    final = *projection_matrix**modelview_matrix;

    final = inverse(final);
    vin.x = windowx;
    vin.y = windowy;
    vin.z = windowz;
    vin.w = 1.0f;

    vin.x = ( vin.x - viewport_matrix[ 0 ] ) / viewport_matrix[ 2 ];
    vin.y = ( vin.y - viewport_matrix[ 1 ] ) / viewport_matrix[ 3 ];

    vin.x = vin.x * 2.0f - 1.0f;
    vin.y = vin.y * 2.0f - 1.0f;
    vin.z = vin.z * 2.0f - 1.0f;

    Vec4MultiplyMat4( &vout, &vin, &final );

    if( !vout.w ) return 0;

    vout.x /= vout.w;
    vout.y /= vout.w;
    vout.z /= vout.w;

    *objextx = vout.x;
    *objecty = vout.y;
    *objectz = vout.z;

    return 1;
}

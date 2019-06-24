// Filename: RectInstance.vert
#version 450
#extension GL_ARB_separate_shader_objects : enable
layout (std140, binding = 0) uniform TransformBufferStruct {
	mat4 mvp;
} TransformBuffer;

layout(push_constant) uniform colorBlock {
    vec4 inNewColor;
    mat4 modelMatrix;
} pushConstantsColorBlock;

// Vextex attributes
layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec2 inColor;
//layout (location = 2) in uint DrawType;
layout (location = 0) out vec4 fragColor;
layout(location = 1) out vec2 texCoord;

out gl_PerVertex { 
    vec4 gl_Position;
};

void main() 
{
	gl_Position   = TransformBuffer.mvp * pushConstantsColorBlock.modelMatrix * inPosition;
    fragColor = pushConstantsColorBlock.inNewColor;
	texCoord = inColor;
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
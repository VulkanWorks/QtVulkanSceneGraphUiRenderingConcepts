// Filename: Triangle.vert
#version 450
#extension GL_ARB_separate_shader_objects : enable
layout (std140, binding = 0) uniform bufferVals {
mat4 mvp;
float time;
int dirtyTest;
} myBufferVals;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inColor; // Not being in use

// Instanced attributes
layout (location = 2) in mat4 instancePos;
layout (location = 6) in vec4 dimension;
layout (location = 7) in vec4 color;
layout (location = 8) in uint isVisible;
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 texCoord;
layout(location = 2) flat out uint visibilityFlag;
layout(location = 3) flat out float time;
layout(location = 4) flat out float dirtyTest;

out gl_PerVertex { 
    vec4 gl_Position;
};

void main() 
{
    vec4 inPositionNew = inPosition;
//	fragColor = inColor;
	fragColor = color;
	texCoord = inPosition;
	visibilityFlag = isVisible;
	time = myBufferVals.time;
	dirtyTest = myBufferVals.dirtyTest;

	inPositionNew.x = inPosition.x * dimension.x;
	inPositionNew.y = inPosition.y * dimension.y;
    gl_Position   = myBufferVals.mvp * instancePos * (inPositionNew /*+ dimension*/);
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;

}
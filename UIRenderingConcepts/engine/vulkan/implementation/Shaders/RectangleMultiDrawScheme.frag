// Filename: RectInstance.frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 texCoord;

layout(location = 0) out vec4 outColor;

float roundRect(in vec2 distFromCenter, in vec2 halfSize, in float radius)
{
    float t = length(max(abs(distFromCenter) - (halfSize - radius), vec2(0.0))) - radius;
    return smoothstep(-1.0, 1.0,t);

}

void main() 
{
    // Pass through fragment color input as output
    outColor = fragColor;
	return;
	///////////////////////////////

	    // Pass through fragment color input as output
    outColor = texCoord;
	///////////////////////////
	float weight = 0.0f;
    float dx     = texCoord.x - 0.5;
    float dy     = texCoord.y - 0.5;
    float len = sqrt(dx * dx + dy * dy);
    
    // Calculate the weights
    weight = smoothstep( 0.4, 0.45, len );

    outColor = mix( vec4(fragColor.rgb, 1.0), vec4(1.0, 0.0, 0.0, 0.0), weight);
	return;
    float p = roundRect(vec2(dx, dy), vec2(0.0, 0.0), 0.10);
	outColor = mix(vec4(1.0,0.0,0.0,1.0), vec4(0.0,1.0,0.0,1.0), p);
//		1/500
    float margin = 0.02;
    float radius = 0.010;
	
	vec2 offset = abs(vec2(dx, dy)) - (0.5 - margin - radius);
    float distance = length(max(offset, 0.0)) - radius;
	
	weight = smoothstep(0.0, margin, distance);
    outColor = mix( vec4(fragColor.rgb, 1.0), vec4(0.0, 1.0, 0.0, 0.0), weight);
 //   outColor = vec4(fragColor.rgb, weight);//, vec4(0.0, 1.0, 0.0, 0.0), weight);

}
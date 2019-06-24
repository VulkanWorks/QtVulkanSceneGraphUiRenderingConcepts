// Filename: Triangle.frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 texCoord;
layout(location = 2) flat in uint visibilityFlag;
layout(location = 3) flat in float time;
layout(location = 4) flat in float dirtyTest;

layout(location = 0) out vec4 outColor;

float roundRect(in vec2 distFromCenter, in vec2 halfSize, in float radius)
{
    float t = length(max(abs(distFromCenter) - (halfSize - radius), vec2(0.0))) - radius;
    return smoothstep(-1.0, 1.0,t);
}

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

void main() 
{
    if (visibilityFlag == 0) discard;

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
//	return;
    float p = roundRect(vec2(dx, dy), vec2(0.0, 0.0), 0.10);
	outColor = mix(vec4(1.0,0.0,0.0,1.0), vec4(0.0,1.0,0.0,1.0), p);
//		1/500
    float margin = 0.02;
    float radius = 0.010;
	
	vec2 offset = abs(vec2(dx, dy)) - (0.5 - margin - radius);
    float distance = length(max(offset, 0.0)) - radius;
	
	weight = smoothstep(0.0, margin, distance);
    if (dirtyTest == 1)
    {
        outColor = vec4(noise(texCoord.xy), noise(vec2(time, 1.0 - time)), noise(vec2(1.0 - time, time)), 1.0);
    }
    else
    {
        outColor = mix( vec4(fragColor.rgb, 1.0), vec4(0.0, 1.0, 0.0, 0.0), weight);
    }
    //outColor = mix( vec4(/*fragColor.rg*/cos(time), 1.0, sin(time), 1.0), vec4(0.0, 1.0, 0.0, 0.0), weight);
 //   outColor = vec4(fragColor.rgb, weight);//, vec4(0.0, 1.0, 0.0, 0.0), weight);
}
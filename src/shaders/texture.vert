#version 460 core 
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec2 aTexCoord;
 
out vec2 vTexCoord;
out vec3 vColor;
out vec3 vLDirVS;
out vec3 vPosWS;
out vec3 vNormalVS;
out vec4 vSunShadowCoords;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec3 uLDir;

uniform mat4 uSunProj;
uniform mat4 uSunView;

struct SpotLight
{
    vec4 positionVS; // xyz = pos, w = padding
    vec4 directionVS; // xyz = dir, w = padding
    float cutoff, inner_cutoff, pad1, pad2;
    vec4 color; // rgb = color, a = padding
    mat4 light_matrix; // for light space
};

// Shader Storage Buffer Object
layout(std430, binding = 1) readonly buffer SpotLights {
    SpotLight spot_lights[];
};

uniform int uNumSpotLights;

void main(void)
{
	vLDirVS   =  (uView*vec4(uLDir,0.f)).xyz;
	
	vNormalVS =  normalize((uView*uModel*vec4(aNormal, 0.0)).xyz);
	
	vPosWS = (uModel*vec4(aPosition, 1.0)).xyz; 

	vSunShadowCoords = uSunProj * uSunView * uModel * vec4(aPosition, 1.0);
		
    vTexCoord = aTexCoord;
    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0);
}

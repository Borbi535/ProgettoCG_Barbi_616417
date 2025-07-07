#version 460 core  

in vec2 vTexCoord;
in vec3 vColor; 
in vec3 vLDirVS;
in vec3 vPosVS;
in vec3 vNormalVS;
in vec4 vSunShadowCoords;

out vec4 color;

uniform sampler2D uColorImage;

uniform vec3 uDiffuseColor;
uniform vec3 uAmbientColor;
uniform vec3 uSpecularColor;
uniform vec3 uSunColor;
uniform float uShininess;

uniform int uTextureAvailable;
uniform vec4 uColor;


vec3 phong (vec3 L, vec3 V, vec3 N)
{
	float LN = max(0.0,dot(L,N));

	vec3 R = -L+2*dot(L,N)*N; // specular direction w.r.t L

	float spec = ((LN>0.f)?1.f:0.f) * max(0.0,pow(dot(V,R),uShininess));

	return (uAmbientColor+LN*uDiffuseColor + spec * uSpecularColor)*uSunColor;
}

void main(void) 
{
	color = vec4(phong(vLDirVS,normalize(-vPosVS),normalize(vNormalVS)),1.0); //all'height field manca la normale? (ie è 000?) in questo caso è normale che il contributo sia sempre lo stesso
	
	//color = vec4(255,255,255,1)*(color.x+color.y+color.z)/3.f;
	color = vec4(vec3((color.x+color.y+color.z)/3.f), 1);
}
#version 460 core  

in vec2 vTexCoord;
in vec3 vColor; 
in vec3 vLDirVS;
in vec3 vPosVS;
in vec3 vNormalVS;

out vec4 color;

uniform sampler2D uColorImage;

uniform vec3 uDiffuseColor;
uniform vec3 uAmbientColor;
uniform vec3 uSpecularColor;
uniform vec3 uLightColor;
uniform float uShininess;

uniform int uTextureAvailable;
uniform vec4 uColor;

/* phong lighting */
vec3 phong ( vec3 L, vec3 V, vec3 N){
	float LN = max(0.0,dot(L,N));

	vec3 R = -L+2*dot(L,N)*N;

	float spec = ((LN>0.f)?1.f:0.f) * max(0.0,pow(dot(V,R),uShininess));

	return (uAmbientColor+LN*uDiffuseColor + spec * uSpecularColor)*uLightColor;
}

void main(void) 
{
	color = vec4(phong(vLDirVS,normalize(-vPosVS),normalize(vNormalVS)),1.0);

    if (uTextureAvailable == 0) color = uColor*(color.x+color.y+color.z)/3.f;
	else color = texture2D(uColorImage,vTexCoord.xy)*(color.x+color.y+color.z)/3.f;
} 
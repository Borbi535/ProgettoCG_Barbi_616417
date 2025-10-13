#version 460 core  


in vec2 vTexCoord;
in vec3 vColor; 
in vec3 vLDirVS;
in vec3 vPosWS;
in vec3 vNormalVS;
in vec4 vSunShadowCoords;

out vec4 color;

uniform sampler2D uColorImage;

uniform mat4 uView;

uniform vec3 uDiffuseColor;
uniform vec3 uAmbientColor;
uniform vec3 uSpecularColor;
uniform float uShininess;
uniform vec3 uSunColor;
uniform float uSunIntensity;



uniform ivec2 uShadowMapSize;

uniform sampler2DArray uSpotLightsShadowMapArray;
uniform sampler2DArray uPositionalLightsShadowMapArray;

// PositionalLights
struct PositionalLight
{
    vec4 positionVS;
    float intensity, pad0, pad1, pad2;
    vec4 color;
};

// Shader Storage Buffer Object
layout(std430, binding = 0) readonly buffer PositionalLights { PositionalLight positional_lights[]; };

uniform int uNumPositionalLights;

// SpotLights
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

uniform int uTextureAvailable;
uniform vec4 uColor;

vec3 phong (vec3 L, vec3 V, vec3 N, vec3 color)
{
	float LN = max(0.0,dot(L,N));

	vec3 R = -L+2*dot(L,N)*N; // specular direction w.r.t. L

	float spec = ((LN>0.f)?1.f:0.f) * max(0.0,pow(dot(V,R),uShininess));

	return (uAmbientColor+LN*uDiffuseColor + spec * uSpecularColor)*color;
}

float linstep(float low, float high, float v)
{
    return clamp((v-low)/(high-low), 0.0, 1.0);
}

void main(void) 
{
    vec3 vPosVS = (uView * vec4(vPosWS, 1.0)).xyz;
    vec3 V = normalize(-vPosVS);
    vec3 N = normalize(vNormalVS);

	color = vec4(phong(vLDirVS, V, N, uSunColor * uSunIntensity), 1.0);
    //color = vec4(255,255,255,1.0);

    // TODO: ombre sole
    
    // positional lights
    for (int i = 0; i < uNumPositionalLights; i++)
    {      
        float visibility = 1.0;
        float bias = 0.005;

        vec4 vPosLS = spot_lights[i].light_matrix * vec4(vPosWS, 1.0);
        vec3 projCoords = (vPosLS.xyz / vPosLS.w) * 0.5 + 0.5;

        if (!(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z > 1.0))
        {
            vec2 depth = texture(uSpotLightsShadowMapArray, vec3(projCoords.xy, i)).xy;
            float variance = max(0.0001, depth.y - (depth.x * depth.x));

            float diff = projCoords.z - depth.x;
            if (diff > bias)
            {
                visibility = variance / (variance + diff * diff);
                visibility = linstep(0.001, 1.0, visibility);
            }
        }

        vec3 L = positional_lights[i].positionVS.xyz - vPosVS;
        
        float max_distance = positional_lights[i].intensity / 30.0;
        float distance = length(L);
        float attenuation = max_distance / distance;

        if (attenuation > 1)
        {
            float attenuation_mapped = clamp(1.0 - distance / max_distance, 0.0, 1.0);
            color += vec4(phong(L, V, N, positional_lights[i].color.rgb) * attenuation_mapped * visibility, 0);
        }
    }
    
    for (int i = 0; i < uNumSpotLights; i++)
    {
        float visibility = 1.0;
        float bias = 0.005;

        vec4 vPosLS = spot_lights[i].light_matrix * vec4(vPosWS, 1.0);
        vec3 projCoords = (vPosLS.xyz / vPosLS.w) * 0.5 + 0.5;

        if (!(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z > 1.0))
        {
            vec2 depth = texture(uSpotLightsShadowMapArray, vec3(projCoords.xy, i)).xy;
            float variance = max(0.0001, depth.y - (depth.x * depth.x));

            float diff = projCoords.z - depth.x;
            if (diff > bias)
            {
                visibility = variance / (variance + diff * diff);
                visibility = linstep(0.001, 1.0, visibility);
            }
        }

        vec3 L = normalize(spot_lights[i].positionVS.xyz - vPosVS);
        
        float theta = dot(L, -spot_lights[i].directionVS.xyz);

        float outer_cone_cos = cos(radians(spot_lights[i].cutoff)); // es. cos(45) = 0.707
        float inner_cone_cos = cos(radians(spot_lights[i].inner_cutoff)); // es. cos(2) = 0.999
        
        float epsilon = max(0.0001, inner_cone_cos - outer_cone_cos);
        float intensity = clamp((theta - outer_cone_cos) / epsilon, 0.0, 1.0);

        color += vec4(phong(L, V, N, spot_lights[i].color.rgb) * intensity * visibility, 0);        
    }

    if (uTextureAvailable == 0) color = uColor*(color.x+color.y+color.z)/3.f;
    else color = texture2D(uColorImage,vTexCoord.xy)*(color.x+color.y+color.z)/3.f;
}
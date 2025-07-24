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
uniform float uShininess;
uniform vec3 uSunColor;
uniform float uSunIntensity;

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
// definizione della struttura per compatibilità con SSBO
struct SpotLight
{
    vec4 positionVS; // xyz = pos, w = padding
    vec4 directionVS; // xyz = dir, w = padding
    float cutoff, inner_cutoff, pad1, pad2;
    vec4 color; // rgb = color, a = padding
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

void main(void) 
{
    vec3 V = normalize(-vPosVS);
    vec3 N = normalize(vNormalVS);

	color = vec4(phong(vLDirVS, V, N, uSunColor * uSunIntensity), 1.0);
    //color = vec4(255,255,255,1.0);

    // positional lights
    for (int i = 0; i < uNumPositionalLights; i++)
    {
        vec3 L = positional_lights[i].positionVS.xyz - vPosVS;
        //vec3 L = vPosVS - positional_lights[i].positionVS.xyz;
        
        float max_distance = positional_lights[i].intensity / 30.0;
        float distance = length(L);
        float attenuation = max_distance / distance;
        //color = vec4(positional_lights[i].positionVS.xyz / 100, 1.0); // costante
        //color = vec4(0.0, clamp(distance/200,0,1), 0.0, 1.0);
        //color = vec4((L + 30) / 50.0, 1.0);
        //color = vec4(vPosVS, 1.0); // varia (gradiente)

        //vec3 L_debug = positional_lights[i].positionVS.xyz - vPosVS;
        //color = vec4(step(0.0, L_debug.x), step(0.0, L_debug.y), step(0.0, L_debug.z), 1.0); // (255,0,255)
        //return; // Per isolare il debug

        //vec3 L_raw = positional_lights[i].positionVS.xyz - vPosVS;
        //float dist = length(L_raw);
        // Prova a mappare la distanza a un colore. Devi stimare un range ragionevole per 'dist'.
        // Ad esempio, se le distanze variano da 0 a 100, usa 100.0.
        //float max_possible_dist = 100.0; // Adatta questo valore alla tua scena!
        //float normalized_dist = clamp(dist / max_possible_dist, 0.0, 1.0);
        //color = vec4(normalized_dist, normalized_dist, normalized_dist, 1.0); // Scala di grigi // (137,137,137)
        //return; // Per isolare il debug
                
        //vec3 L_norm = normalize(positional_lights[i].positionVS.xyz - vPosVS);
        //// Mappa [-1, 1] a [0, 1] per la visualizzazione.
        //vec3 color_dir = (L_norm + 1.0) / 2.0;
        //color = vec4(color_dir, 1.0); // (148,69,239)
        //return; // Per isolare il debug

        if (attenuation > 1)
        {
            float attenuation_mapped = clamp(1.0 - distance / max_distance, 0.0, 1.0);
            color += vec4(phong(L, V, N, positional_lights[i].color.rgb) * attenuation_mapped, 0);
            //color = vec4(255,0,0,1.0);
        }
    }

    for (int i = 0; i < uNumSpotLights; i++)
    {
        vec3 L = normalize(spot_lights[i].positionVS.xyz - vPosVS);
        
        //float epsilon = cos(radians(positional_lights[i].inner_cutoff)) - cos(radians(positional_lights[i].cutoff));
        //float intensity = clamp((theta - (cos(radians(positional_lights[i].cutoff)))) / epsilon, 0.0, 1.0);
        //
        //color = vec4(0, max(0.0, theta), 0, 1);

        float theta = dot(L, -spot_lights[i].directionVS.xyz); // theta dovrebbe essere tra -1 e 1

        float outer_cone_cos = cos(radians(spot_lights[i].cutoff)); // es. cos(45) = 0.707
        float inner_cone_cos = cos(radians(spot_lights[i].inner_cutoff)); // es. cos(2) = 0.999
        
        // prevenire divisione per zero se gli angoli sono uguali
        float epsilon = max(0.0001, inner_cone_cos - outer_cone_cos);
        
        float intensity = clamp((theta - outer_cone_cos) / epsilon, 0.0, 1.0);
        
        //if(theta > cos(radians(spot_lights[i].cutoff))/* && !(depth + bias < lampCoords.z)*/)
             //color += vec4(phong(L, V, N, spot_lights[i].color.rgb) * intensity, 0);

    }

    if (uTextureAvailable == 0) color = uColor*(color.x+color.y+color.z)/3.f;
    else color = texture2D(uColorImage,vTexCoord.xy)*(color.x+color.y+color.z)/3.f;
}
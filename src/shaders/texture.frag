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
uniform vec3 uSunColor; // ???

// Positional Lights
// Definizione della struttura per compatibilità con SSBO
struct PositionalLight {
    vec4 position; // xyz = pos, w = padding
    vec4 direction; // xyz = dir, w = padding
    float cutoff, inner_cutoff;
    float pad1, pad2;
    vec4 color; // rgb = color, a = padding
};

// Shader Storage Buffer Object
layout(std430, binding = 0) readonly buffer PositionalLights {
    PositionalLight positional_lights[]; // Array dinamico
};

uniform int uNumPosLights;


uniform int uTextureAvailable;
uniform vec4 uColor;




vec3 phong (vec3 L, vec3 V, vec3 N, vec3 color)
{
	float LN = max(0.0,dot(L,N));

	vec3 R = -L+2*dot(L,N)*N; // specular direction w.r.t L

	float spec = ((LN>0.f)?1.f:0.f) * max(0.0,pow(dot(V,R),uShininess));

	return (uAmbientColor+LN*uDiffuseColor + spec * uSpecularColor)*color;
}

void main(void) 
{
    vec3 V = normalize(-vPosVS);
    vec3 N = normalize(vNormalVS);

	color = vec4(phong(vLDirVS, V, N, uSunColor), 1.0);
    //color = vec4(255,255,255,1.0);
    //color = vec4(0,0,0,1.0);

    for (int i = 0; i < uNumPosLights; i++)
    {
        vec3 L = normalize(positional_lights[i].position.xyz - vPosVS);
        //float theta = dot(L, -positional_lights[i].direction.xyz);
        float theta = dot(L, positional_lights[i].direction.xyz);

        float epsilon = cos(radians(positional_lights[i].inner_cutoff)) - cos(radians(positional_lights[i].cutoff));
        float intensity = clamp((theta - (cos(radians(positional_lights[i].cutoff)))) / epsilon, 0.0, 1.0);
        
        //color = vec4(max(0.0, theta),0,0,1);

        //color = vec4(L.x * 0.5 + 0.5, L.y * 0.5 + 0.5, L.z * 0.5 + 0.5, 1.0);
        //vec3 diff_vec = positional_lights[i].position.xyz - vPosVS;
        //color = vec4(vPosVS, 1.0);
        // DEBUG: Visualizza il vettore differenza
        // Scaliamo per visualizzare: Mappiamo un range atteso di [-X, X] a [0, 1]
        // Se i tuoi oggetti sono di dimensione circa 10x10x10, e la luce è a 0,5,0,
        // diff_vec potrebbe essere tra -10 e 10. Scaliamolo di 1/20 per mappare a [-0.5, 0.5] e poi aggiungere 0.5.
        // PROVA con una scala più aggressiva, es. 0.05
        //float scale = 0.05; // 1 / (max_expected_coord_diff * 2) ad es. 1 / (10 * 2) = 0.05
        // Adatta la scala in base al range atteso delle coordinate
        // Se le tue coordinate sono tra -10 e 10, il diff_vec può essere tra -20 e 20.
        // Quindi un range di [-30, 30] per la visualizzazione potrebbe essere utile.
        //float visualization_range = 30.0; // Adatta questo valore in base alle tue coordinate del mondo
        //color = vec4(diff_vec.x / (2.0 * visualization_range) + 0.5,
        //             diff_vec.y / (2.0 * visualization_range) + 0.5,
        //             diff_vec.z / (2.0 * visualization_range) + 0.5, 1.0);
        //color = vec4(max(0.0, theta), max(0.0, theta), max(0.0, theta), 1.0);
        //color = vec4(positional_lights[i].position.xyz, 1.0);
        //vec4 lampCoords = (lamp_shadowCoords[i]/lamp_shadowCoords[i].w)*0.5 + 0.5;
        //vec3 L2 = normalize(vec3(view_lamp[i][3]) - vPosVS);
        //float bias = clamp(uBias*tan(acos(dot(N,L2))),uBias,0.05);
	    //float depth = texture(shadowMap_texture_lamp[i],lampCoords.xy).x;

        if(theta > cos(radians(positional_lights[i].cutoff))/* && !(depth + bias < lampCoords.z)*/)
        {
            //color += vec4(phong(L, V, N, positional_lights[i].color.rgb) * intensity , 1.0);

            color = vec4(max(0.0, theta),0,0,1);
        }
    }


    if (uTextureAvailable == 0) color = uColor*(color.x+color.y+color.z)/3.f;
	else color = texture2D(uColorImage,vTexCoord.xy)*(color.x+color.y+color.z)/3.f;
	
	//color = vec4(vec3((color.x+color.y+color.z)/3.f), 1);
} 
//if(uShadingMode == 4 ) // pbrBaseTexture
//{
//		color = vec4(phong(vLDirVS,normalize(-vPosVS),normalize(vNormalVS)),1.0);
//		color = texture2D(uTexColor,vTexCoord0.xy)*(color.x+color.y+color.z)/3.f;
//}
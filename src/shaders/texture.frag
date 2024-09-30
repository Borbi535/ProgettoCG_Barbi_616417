#version 460 core  

in vec2 vTexCoord;
in vec3 vColor; 

out vec4 color;

uniform sampler2D uColorImage;


void main(void) 
{
    color = texture2D(uColorImage,vTexCoord);
} 
#version 460 core 
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec2 aTexCoord;
 
out vec2 vTexCoord;
out vec3 vColor;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;


void main(void)
{
	vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0);
}

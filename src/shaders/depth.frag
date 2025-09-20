#version 430 core  
out vec4 color; 

uniform float uPlaneApprox;


float  PlaneApprox() {   
	// Compute partial derivatives of depth.    
	float dx = dFdx(gl_FragCoord.z);   
	float dy = dFdy(gl_FragCoord.z);

	// Compute second moment over the pixel extents.   
	return  gl_FragCoord.z*gl_FragCoord.z + uPlaneApprox*0.5*(dx*dx + dy*dy);   
} 

void main(void) 
{ 
 	color = vec4(gl_FragCoord.z,PlaneApprox(),0.0,1.0);
} 
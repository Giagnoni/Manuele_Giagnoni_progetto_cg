#version 430 core
out vec4 color;

uniform float uPlaneApprox;

float  PlaneApprox() {  
	float dx = dFdx(gl_FragCoord.z);   
	float dy = dFdy(gl_FragCoord.z);

	return  gl_FragCoord.z*gl_FragCoord.z + uPlaneApprox*0.5*(dx*dx + dy*dy);   
} 

void main(void) 
{ 
//	color = vec4(vec3((gl_FragCoord.z)),1.0);
//	color = vec4(1.0,0.0,0.0,1.0);
	color = vec4(gl_FragCoord.z,PlaneApprox(),0.0,1.0);
} 
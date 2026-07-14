#version 430 core 
layout (location = 0) in vec3 aPosition; 

uniform mat4 model_matrix;
uniform mat4 uLightMatrix;

void main(void) 
{ 
    gl_Position = uLightMatrix*model_matrix*vec4(aPosition, 1.0); 
}
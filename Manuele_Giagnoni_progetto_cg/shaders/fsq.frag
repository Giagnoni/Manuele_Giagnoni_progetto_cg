#version 430 core  
out vec4 color; 

in vec2 vTexCoord;

uniform sampler2D uTexture;

void main(void) 
{ 
	color = texture(uTexture,vTexCoord);
} 
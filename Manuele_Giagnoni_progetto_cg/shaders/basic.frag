#version 460
in vec3 fColori;

out vec4 color;

void main(){
	color = vec4(fColori, 1.0);
}
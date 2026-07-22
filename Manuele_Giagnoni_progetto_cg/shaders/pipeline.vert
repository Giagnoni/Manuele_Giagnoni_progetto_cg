#version 460
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColori;
layout (location = 4) in vec2 aTexCoords;
layout (location = 10) in vec2 aProg;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat4 uLightMatrix;

uniform int oggetto_mappato;

uniform vec3 uSunlight;

out vec3 fColori;
out vec2 texCoords;
out vec3 sunlightDir;
out vec3 vPosVS;
out vec4 vCoordLS;
out vec3 FragPos;

void main(){
	fColori = aColori;
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(aPos, 1.0);
	sunlightDir = (view_matrix * vec4(uSunlight, 0.f)).xyz;
	vPosVS = (view_matrix * model_matrix * vec4(aPos, 1.0)).xyz;
	FragPos = vec3(model_matrix * vec4(aPos, 1.0));

	if(oggetto_mappato == 0){
		texCoords = vec2((aPos.x + 1) / 2, (aPos.z + 1) / 2);
	}else if(oggetto_mappato == 1){
		texCoords = vec2(aProg.x / 100, aProg.y);
	}else if(oggetto_mappato == 2){
		texCoords = aTexCoords;
	}
	
	vCoordLS =  uLightMatrix*model_matrix*vec4(aPos, 1.0);
}
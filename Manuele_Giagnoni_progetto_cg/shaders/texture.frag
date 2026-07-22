#version 460
in vec3 fColori;
in vec2 texCoords;
in vec3 sunlightDir;
in vec3 vPosVS;
in vec4 vCoordLS;
in vec3 FragPos;

struct SpotLight{
	vec3 spotPos;
	vec3 spotDir;
	float spotCutoff;
};

uniform sampler2D uColorImage;
uniform sampler2D uShadowMap;

uniform float uBias;
uniform ivec2 uShadowMapSize;
uniform vec3 uColor;

#define NUMERO_LAMPIONI 19
uniform vec3 sl[NUMERO_LAMPIONI];
uniform vec3 spotDir;
uniform float spotCutoff;

out vec4 color;

void main(){
	vec3 N = normalize(cross(dFdx(vPosVS),dFdy(vPosVS)));
	float diffuse = max(0.0,dot(sunlightDir,N));
	vec3 ambient_light = vec3(0.35,0.35,0.35);
	float lit = 1.0;
	int spotlit = 0;

	float storedDepth;
	vec4 pLS = (vCoordLS/vCoordLS.w)*0.5+0.5;
	for( float  x = 0.0; x < 5.0;x+=1.0)
		for( float y = 0.0; y < 5.0;y+=1.0)
			{
				storedDepth =  texture(uShadowMap,pLS.xy+vec2(-2.0+x,-2.0+y)/uShadowMapSize).x;
				if(storedDepth + uBias < pLS.z )
					lit  -= 1.0/25.0;
			}

	for(int i=0; i<NUMERO_LAMPIONI; i++){
		vec3 lightDir = normalize(sl[i] - FragPos);
		float theta = dot(lightDir, normalize(-spotDir));

		if(theta > spotCutoff){
			spotlit = 1;
			break;
		}
	}

	if(spotlit == 1){
		color = texture(uColorImage, texCoords);
	}else{
		color = vec4(vec3(lit*diffuse) + ambient_light ,1.0) * (uColor.x < 0 ? texture(uColorImage, texCoords) : vec4(uColor, 1.0));
	}
}

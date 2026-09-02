#version 460
#define NUMERO_AUTO 10
#define NUMERO_LAMPIONI 19

in vec3 fColori;
in vec2 texCoords;
in vec3 sunlightDir;
in vec3 vPosVS;
in vec4 vCoordLS;
in vec4 vCoordCL[NUMERO_AUTO][2];
in vec3 FragPos;

struct SpotLight{
	vec3 spotPos;
	vec3 spotDir;
	float spotCutoff;
};

uniform sampler2D uColorImage;
uniform sampler2D uShadowMap;
uniform sampler2D uCarlightShadowMap[NUMERO_AUTO][2];

uniform float uBias;
uniform ivec2 uShadowMapSize;
uniform vec3 uColor;

uniform vec3 sl[NUMERO_LAMPIONI];
uniform vec3 spotDir;
uniform float spotCutoff;

uniform vec3 cl[NUMERO_AUTO][2];
uniform vec3 clVS[NUMERO_AUTO][2];
uniform vec3 clDir[NUMERO_AUTO];
uniform float att_const;
uniform float att_lin;
uniform float att_quad;
uniform float tb_scaling;
uniform float uLunghezzaLuci;

out vec4 color;

void main(){
	vec3 N = normalize(cross(dFdx(vPosVS),dFdy(vPosVS)));
	float diffuse = max(0.0,dot(sunlightDir,N));
	vec3 ambient_light = vec3(0.35,0.35,0.65);
	float lit = 1.0;
	vec3 spotlit = vec3(0, 0, 0);

	float storedDepth, carStoredDepth1, carStoredDepth2;
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
			spotlit = vec3(0.9, 0.9, 0);
			break;
		}
	}
	
	vec4 pCL1, pCL2;
	for(int i=0; i<NUMERO_AUTO; i++){
		vec3 lightDir1 = normalize(cl[i][0] - FragPos);
		vec3 lightDir2 = normalize(cl[i][1] - FragPos);

		float distance1 = length(cl[i][0] - FragPos) / tb_scaling;
		float distance2 = length(cl[i][1] - FragPos) / tb_scaling;

		float attenuation = 1.0 / (att_const + att_lin * distance1 + att_quad * (distance1 * distance1)) / (att_const + att_lin * distance2 + att_quad * (distance2 * distance2));

		float theta1 = dot(lightDir1, normalize(-clDir[i]));
		float theta2 = dot(lightDir2, normalize(-clDir[i]));
		
		pCL1 = (vCoordCL[i][0]/vCoordCL[i][0].w)*0.5+0.5;
		pCL2 = (vCoordCL[i][1]/vCoordCL[i][1].w)*0.5+0.5;

		carStoredDepth1 = texture(uCarlightShadowMap[i][0], pCL1.xy).x;
		carStoredDepth2 = texture(uCarlightShadowMap[i][1], pCL2.xy).x;

		if(carStoredDepth1 + uBias > pCL1.z && carStoredDepth2 + uBias > pCL2.z)
			if(theta1 > spotCutoff && distance1 < uLunghezzaLuci || theta2 > spotCutoff && distance2 < uLunghezzaLuci){
				vec3 new_light = vec3(0.9, 0.9, 0) * attenuation;
				spotlit = vec3(max(new_light.x, spotlit.x), max(new_light.y, spotlit.y), max(new_light.z, spotlit.z));
		}
	}

	color = vec4(vec3(lit*diffuse) + ambient_light + spotlit,1.0) * (uColor.x < 0 ? texture(uColorImage, texCoords) : vec4(uColor, 1.0));
}

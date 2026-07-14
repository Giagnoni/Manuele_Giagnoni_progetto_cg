#version 460
in vec3 fColori;
in vec2 texCoords;
in vec3 sunlightDir;
in vec3 vPosVS;
in vec4 vCoordLS;

uniform sampler2D uColorImage;
uniform sampler2D uShadowMap;

uniform vec3 uDiffuseColor;
uniform vec3 uAmbientColor;
uniform vec3 uSpecularColor;
uniform vec3 uLightColor;
uniform float uShininess;
uniform float uBias;
uniform ivec2 uShadowMapSize;

out vec4 color;

vec3 phong ( vec3 L, vec3 V, vec3 N){
	float LN = max(0.0,dot(L,N));

	vec3 R = -L+2*dot(L,N)*N;

	float spec = ((LN>0.f)?1.f:0.f) * max(0.0,pow(dot(V,R),uShininess));

	return (uAmbientColor+LN*uDiffuseColor + spec * uSpecularColor)*uLightColor;
}

vec3 luminosita(vec3 L, vec3 N){
	float LN = max(0.0,dot(L,N));

	return vec3(LN, LN, LN);
}

void main(){
	vec3 N = normalize(cross(dFdx(vPosVS),dFdy(vPosVS)));
	//color = vec4(phong(sunlightDir,normalize(-vPosVS),N),1.0);

	//color = texture2D(uColorImage, texCoords);

	//color = texture2D(uColorImage, texCoords) * vec4(luminosita(sunlightDir, N), 1.0);
	
	float lit = 1.0;

	float storedDepth;
	vec4 pLS = (vCoordLS/vCoordLS.w)*0.5+0.5;
	/*for( float  x = 0.0; x < 5.0;x+=1.0)
		for( float y = 0.0; y < 5.0;y+=1.0)
			{
				storedDepth =  texture(uShadowMap,pLS.xy+vec2(-2.0+x,-2.0+y)/uShadowMapSize).x;
				if(storedDepth + uBias < pLS.z )
					lit  -= 1.0/25.0;
			}*/

	
	float depth = texture(uShadowMap,pLS.xy).x;
	if(depth < pLS.z)
		lit = 0.0;
	color = vec4(vec3(lit) ,1.0) * texture(uColorImage, texCoords);
}

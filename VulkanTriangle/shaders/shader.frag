#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D shadowMap;

layout(location = 0) out vec4 outColor;

layout (location = 4) in vec4 inShadowCoord;

vec3 ambLight = vec3(0.35, 0.35, 0.35);
layout(location = 2) in vec3 lightDir;
vec3 lightColour = vec3(1, 1, 1);

layout (constant_id = 0) const int enablePCF = 0;

//Project the shadwo texture to check if a fragment is visable from the lights perspective
float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.35;
		}
	}
	return shadow;
}

//Do multiple interation of the texture projections to filter the result, this helps feather edges and avoid artificating
float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main() {
	
	float shadow = filterPCF(inShadowCoord / inShadowCoord.w); //Calculate shadow value
	
	vec3 norm = normalize(fragNormal); //Normalize normalize
	float diff =  max(dot(norm, lightDir), 0.0); //Calulate lamberisan
	vec3 diffuse = lightColour * diff; //Calculate diffuse light
	vec4 col = texture(texSampler, fragTexCoord); //Get texture colour
	

		outColor = vec4(((ambLight+diffuse)*shadow)*col.xyz, 1.0) ;; //Output final lighting
	
	//outColor = vec4(col.r, col.g, col.b, 1);
}
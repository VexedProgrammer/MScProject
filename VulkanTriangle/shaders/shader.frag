#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D shadowMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D specMap;

layout(location = 0) out vec4 outColor;

layout (location = 4) in vec4 inShadowCoord;
layout (location = 5) in vec3 fragPos;

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
	
	
	
	vec4 normal = texture(normalMap, fragTexCoord); //Get texture colour
	// compute derivations of the world position
	vec3 p_dx = dFdx(fragPos);
	vec3 p_dy = dFdy(fragPos);
	// compute derivations of the texture coordinate
	vec2 tc_dx = dFdx(fragTexCoord);
	vec2 tc_dy = dFdy(fragTexCoord);
	// compute initial tangent and bi-tangent
	vec3 t = normalize( tc_dy.y * p_dx - tc_dx.y * p_dy );
	vec3 b = normalize( tc_dy.x * p_dx - tc_dx.x * p_dy ); // sign inversion
	// get new tangent from a given mesh normal
	vec3 n = normalize(fragNormal);
	vec3 x = cross(n, t);
	t = cross(x, n);
	t = normalize(t);
	// get updated bi-tangent
	x = cross(b, n);
	b = cross(n, x);
	b = normalize(b);
	mat3 tbn = mat3(t, b, n);
	normal = normalize(normal * 2.0 - 1.0);   
	vec3 normalNorm = normalize(tbn * normal.xyz); 
	
	vec3 norm = normalize(normalNorm); //Normalize normalize
	float diff =  max(dot(norm, lightDir), 0.0); //Calulate lamberisan
	vec3 diffuse = lightColour * diff; //Calculate diffuse light
	vec4 col = texture(texSampler, fragTexCoord); //Get texture colour
	
	vec3 viewDir = normalize(-vec3(0.0f, 0.01f, 0.1f));
	vec3 halfwayDir = normalize(normalize(lightDir) + viewDir);
	
	
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 64) * (1- texture(specMap, fragTexCoord).r);
	vec3 specular = vec3(1,1,1) * spec;
	
	float shadow =  filterPCF(inShadowCoord / inShadowCoord.w); //Calculate shadow value
	outColor = vec4(((ambLight+diffuse+specular)*shadow)*col.xyz, 1.0) ;; //Output final lighting
	
	//outColor = vec4(col.r, col.g, col.b, 1);
}
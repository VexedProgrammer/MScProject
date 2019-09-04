#version 450
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.14159265359

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D shadowMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D specMap;

layout(location = 2) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 0) out vec4 outPosition;

layout (location = 4) in vec4 inShadowCoord;
layout (location = 5) in vec3 fragPos;
layout (location = 6) in flat int enableLighting;

layout(location = 7) in vec4 AmbientColour;
layout(location = 8) in vec4 DirectionalColour;
layout(location = 9) in vec4 FragmentPosition;
layout(location = 10) in mat4 LightViewProj;

layout(location = 2) in vec3 lightDir;

layout (constant_id = 0) const int enablePCF = 0;

//Project the shadow texture to check if a fragment is visable from the lights perspective
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

//Itterate through surrounding pixel to get an avarage value
float filterPCF(vec4 shadowCoords)
{
	 ivec2 textureDimensions = textureSize(shadowMap, 0);
	 float scale = 0.5;
	 float deltaX = scale * 1.0 / float(textureDimensions.x);
	 float deltaY = scale * 1.0 / float(textureDimensions.y);

	 float shadowFactor = 0.0;
	 int count = 0;
	 int range = 1;
	 float bias = 0.005;

	 for (int x = -range; x <= range; x++)
	 {
		 for (int y = -range; y <= range; y++)
		 {
			 float factor = textureProj(shadowCoords, vec2(deltaX*x, deltaY*y));
			 shadowFactor += shadowCoords.z - bias > factor ? 0.0 : 1.0;
			 count++;
		 }
	 }
	 return (shadowFactor) / count;
}

vec3 CalculateNorm()
{
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
	return normalNorm;
}

float dist(vec3 posW, vec3 normalW) {
	 
	float distToLight = length(-lightDir - posW);
	vec4 posL = LightViewProj * vec4(posW, 1.0);
	vec3 shadowCoords = inShadowCoord.xyz/inShadowCoord.w;
	// Fetch depth from the shadow map:
	vec4 d1 = texture(shadowMap, shadowCoords.xy*0.5+0.5);
	vec3 Ni = texture(normalMap, d1.yz).xyz * vec3(2,2,2) - vec3(1,1,1);
	
	d1 = d1*0.95;
	
	float backFacingEst = clamp(-dot( Ni, normalW ), 0.0, 1.0);
	float thickness = distToLight - d1.x;
	float nDotL1 = dot(normalW, lightDir);
	if(nDotL1 > 0.0)
	{
		thickness = -50.0;
	}
	float correctThickness = clamp(-nDotL1, 0.0,1.0)*thickness;
	float finalThickness = mix(thickness, correctThickness, backFacingEst);
	float alpha = exp(finalThickness-20);
	return finalThickness; 
	
}

//Three-Layer Skin 
vec3 T(float scaledDist) {
	float dd = -scaledDist* scaledDist;

	return	vec3(0.233f, 0.455f, 0.649f) * exp(dd / 0.0064f) +
			vec3(0.1f,   0.336f, 0.344f) * exp(dd / 0.0484f) +
			vec3(0.118f, 0.198f, 0.0f)   * exp(dd / 0.187f)  +
			vec3(0.113f, 0.007f, 0.007f) * exp(dd / 0.567f)  +
			vec3(0.358f, 0.004f, 0.0f)   * exp(dd / 1.99f)   +
			vec3(0.078f, 0.0f,   0.0f)   * exp(dd / 7.41f);
}
void main() {
	vec4 col = texture(texSampler, fragTexCoord); //Get texture colour
	if(AmbientColour.a == 0)
	{
		outColor = vec4(col.r, col.g, col.b, 1);
		outNormal = vec4(0,0,0,0);
		outPosition = vec4(0,0,0,0);
		return;
	}
	  
	vec3 norm = normalize(CalculateNorm()); //Normalize normalize
	float s = dist(FragmentPosition.xyz, norm) * 2.0;
	float irradiance = clamp(0.3f + dot(lightDir, -norm), 0.f, 1.f);
	
	
	
	float diff =  max(dot(lightDir, norm), 0.0); //Calulate lamberisan
	vec3 diffuse = DirectionalColour.rgb * diff; //Calculate diffuse light
	
	vec3 viewDir = normalize(vec3(0.0f, 0.0f, 0.5f) - fragPos.xyz);
	vec3 halfwayDir = normalize(normalize(lightDir) + viewDir);
	float spec = pow(max(0.0, dot(norm, halfwayDir)), 16) * ((texture(specMap, fragTexCoord).r)*0.25);
	vec3 specular = vec3(1,1,1) * spec;
	diffuse += specular;
	
	float shadow =  filterPCF(inShadowCoord / inShadowCoord.w); //Calculate shadow value
	diffuse *= shadow;
	
	vec4 reflectance = vec4(((diffuse))*col.xyz, 1.0); //Output final lighting
	
	
	outColor.rgb =  (AmbientColour.rgb*col.rgb) + reflectance.rgb + clamp(s*(T(s) * DirectionalColour.rgb * col.rgb * irradiance),0,1);
	outColor.a = 1;
	outNormal = vec4(norm, gl_FragCoord.z);
	outPosition = vec4(FragmentPosition.xyz, 1.0);
	

}
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
	 float scale = 0.5;
	 float dx = scale * 1.0 / float(texDim.x);
	 float dy = scale * 1.0 / float(texDim.y);

	 float shadowFactor = 0.0;
	 int count = 0;
	 int range = 1;
	
	 for (int x = -range; x <= range; x++)
	 {
		 for (int y = -range; y <= range; y++)
		 {
			 float factor = textureProj(sc, vec2(dx*x, dy*y));
			 shadowFactor += sc.z - 0.005 > factor ? 0.0 : 1.0;
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
	 // Shrink the position to avoid artifacts on the silhouette:
 /* 	posW = (posW - 0.005 * normalW);

	float scale = 180.0 * (1.0-0.2)/0.4;
  
	// Transform to light space:
	vec4 posL = LightViewProj * vec4(posW, 1.0);
	vec3 shadowCoords = posL.xyz/posL.w;
	// Fetch depth from the shadow map:
	float d1 = texture(shadowMap, (inShadowCoord.xy/inShadowCoord.w)*0.5+0.5).r;
	d1=d1*0.2;
	float d2 = shadowCoords.z;
	float nDotL = dot(normalW, lightDir);
	// if(nDotL < 0)
	// {
		// return 0;
	// }
	// Calculate the difference:
	return abs(d1 - d2);  */
	
	float distToLight = length(-lightDir - posW);
	vec4 posL = LightViewProj * vec4(posW, 1.0);
	vec3 shadowCoords = inShadowCoord.xyz/inShadowCoord.w;
	// Fetch depth from the shadow map:
	vec4 d1 = texture(shadowMap, shadowCoords.xy*0.5+0.5);
	vec3 Ni = texture(normalMap, d1.yz).xyz * vec3(2,2,2) - vec3(1,1,1);
	
	d1 = d1*0.99;
	
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
// This function can be precomputed for efficiency

vec3 T(float scaledDist) {
	float dd = -scaledDist* scaledDist;

	return	vec3(0.233f, 0.455f, 0.649f) * exp(dd / 0.0064f) +
			vec3(0.1f,   0.336f, 0.344f) * exp(dd / 0.0484f) +
			vec3(0.118f, 0.198f, 0.0f)   * exp(dd / 0.187f)  +
			vec3(0.113f, 0.007f, 0.007f) * exp(dd / 0.567f)  +
			vec3(0.358f, 0.004f, 0.0f)   * exp(dd / 1.99f)   +
			vec3(0.078f, 0.0f,   0.0f)   * exp(dd / 7.41f);
}
vec3 W(float scaledDist) {
	float dd = -scaledDist* scaledDist;

	return	vec3(1, 1, 1) * exp(dd / 0.0064f) +
			vec3(0.8f,   0.8f, 0.8f) * exp(dd / 0.0484f) +
			vec3(0.5f, 0.5f, 0.5f)   * exp(dd / 0.187f)  +
			vec3(0.2f, 0.2f, 0.2f) * exp(dd / 0.567f)  +
			vec3(0.05f, 0.05f, 0.05f)   * exp(dd / 1.99f)   +
			vec3(0.005f, 0.005f,   0.005f)   * exp(dd / 7.41f);
}
void main() {
	vec4 col = texture(texSampler, fragTexCoord); //Get texture colour
	//col = texture( shadowMap, inShadowCoord.st);
	if(AmbientColour.a == 0)
	{
		outColor = vec4(col.r, col.g, col.b, 1);
		outNormal = vec4(0,0,0,0);
		outPosition = vec4(0,0,0,0);
		return;
	}
	  
	vec3 tangentNorm = texture(normalMap, fragTexCoord).rgb;
	vec3 normalNorm = CalculateNorm(); 
	float s = dist(FragmentPosition.xyz, normalNorm) * 7.0;
	float irradiance = clamp(0.3f + dot(lightDir, -normalNorm), 0.f, 1.f);
	
	vec3 norm = normalize(normalNorm); //Normalize normalize
	float diff =  max(dot(lightDir, norm), 0.0); //Calulate lamberisan
	vec3 diffuse = DirectionalColour.xyz * diff; //Calculate diffuse light
	
	
	vec3 viewDir = normalize(-vec3(0.0f, 0.0f, 0.1f));
	vec3 halfwayDir = normalize(normalize(lightDir) + viewDir);
	
	float shadow =  filterPCF(inShadowCoord / inShadowCoord.w); //Calculate shadow value
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 64) * (1- texture(specMap, fragTexCoord).r);
	vec3 specular = vec3(1,1,1) * spec;
	diffuse += specular;
	diffuse *= shadow;
	//diffuse *= ;
	
	
	
	vec4 reflectance = vec4(((diffuse))*col.xyz, 1.0) ;; //Output final lighting
	outColor = vec4(((AmbientColour.xyz+diffuse+specular)*shadow)*col.xyz,1) ;
	outColor = reflectance;
	vec3 colour = irradiance * col.xyz *DirectionalColour.xyz * T(s);
	//outColor += vec4(colour,1.0);
	//.xyz = colour;
	outColor.a = 1;
	//outColor = vec4(T(clamp(s,0,1)), 1);
	//outColor = vec4(shadow, shadow, shadow, 1);
	
	
	
	
	vec3 lightScale = diffuse;
	vec3 kt = (T(s)*(s*irradiance));
	vec3 lightMult = (col.rgb*kt);
	outColor.rgb =  (AmbientColour.rgb*col.rgb) + reflectance.rgb + clamp(s*(T(s) * DirectionalColour.rgb * col.rgb * irradiance),0,1);// * (irradiance*col.rgb));//*DirectionalColour.rgb*col.rgb;
	//outColor.rgb = (T(s)*s*irradiance)* DirectionalColour.rgb * col.rgb * 3;
	outColor.a = 1;
	outNormal = vec4(normalNorm, 1.0);
	outPosition = vec4(FragmentPosition.xyz, 1.0);
	//outColor.rgb += diffuse;
	//outColor.rgb = vec3(s);
	
	
	
}
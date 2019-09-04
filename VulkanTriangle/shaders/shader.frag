#version 450
#extension GL_ARB_separate_shader_objects : enable

#define NUM_SAMPLES	25
#define EDGE_LERP_SCALE 300.0f
#define FOVY 0.785398

layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D colourSampler;
layout(binding = 2) uniform sampler2D normSampler;

layout(location = 0) out vec4 outColor;

layout(location = 2) in vec2 blurDir;
layout(location = 3) in vec4 kernel[NUM_SAMPLES];



void main() {
	
	
	///////////////////////////////////////////////////
	vec4 colorM = texture(colourSampler, fragTexCoord);
	vec4 normalM = texture(normSampler, fragTexCoord);

	if (normalM.a == 0.00f) { 
		outColor = vec4(colorM.rgb, 1);
		return; 
	} 
	
	float depthM = normalM.a;//Sample depth stored in alpha channel of the normal texture
	float subsurfWidth = 0.01;//Fixed width, could be sampled from a texture

	float dist = 1.0 / tan(0.5 * FOVY); //Calculate distance to projection window
    float scale = dist / depthM / 2; 
    vec2 offset = subsurfWidth * scale * blurDir; //Final step for each sample
    vec3 colorBlurred = colorM.xyz; //Set centre pixel value
    colorBlurred *= kernel[0].rgb;
    for (int i = 1; i < NUM_SAMPLES; i++) //For each sample
	{
		//Sample surrounding pixels
        vec2 sampleTexCoord = fragTexCoord + kernel[i].a * offset;
        vec3 color = texture(colourSampler, sampleTexCoord).rgb;
		
		//To help avoid over blurring steep edges which large colours changes,
		//	lerp back to original colour based on diffrence in depth between the centre and sampled pixel
        float depth = texture(normSampler, sampleTexCoord).a;
        float dd = abs(depthM - depth);
		float s = clamp(EDGE_LERP_SCALE * dist * subsurfWidth * dd, 0.0f, 1.0f);
        color = mix(color, colorM.rgb, s);

        colorBlurred += kernel[i].rgb * color; //Multiply colour by the kernel areas and accumulate the result
    }

	outColor = vec4(colorBlurred.rgb, 1);
	
}
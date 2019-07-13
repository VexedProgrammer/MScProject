#version 450
#extension GL_ARB_separate_shader_objects : enable

#define NUM_SAMPLES	17
#define EDGE_LERP_SCALE 100.0f
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
	
	float depthM = normalM.a;
	float subsurfWidth = 0.025;//texture(samplerMaterial, inTexCoord).g;

	float dist = 1.0 / tan(0.5 * FOVY);
    float scale = dist / depthM / 2.0f;

    vec2 offset = subsurfWidth * scale * blurDir;

    vec3 colorBlurred = colorM.xyz;
    colorBlurred *= kernel[0].rgb;
	//offset = subsurfWidth * scale * vec2(0,1);
    for (int i = 1; i < NUM_SAMPLES; i++) {
        vec2 sampleTexCoord = fragTexCoord + kernel[i].a * offset;
        vec3 color = texture(colourSampler, sampleTexCoord).rgb;
		
        float depth = texture(normSampler, sampleTexCoord).a;
        float dd = abs(depthM - depth);
		float s = clamp(EDGE_LERP_SCALE * dist * subsurfWidth * dd, 0.0f, 1.0f);
        color = mix(color, colorM.rgb, s);

        colorBlurred += kernel[i].rgb * color;
    }

	outColor = vec4(colorBlurred.rgb, 1);
	
}
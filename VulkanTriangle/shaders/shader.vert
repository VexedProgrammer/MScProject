#version 450
#extension GL_ARB_separate_shader_objects : enable

#define NUM_SAMPLES	25

layout (binding = 0) uniform GBufferUniformBufferObject 
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec4 kernel[NUM_SAMPLES];
	vec2 blurDirection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec2 blurDir;
layout(location = 3) out vec4 kernel[NUM_SAMPLES];

void main() {

	vec4 pos = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);//Calculate the position
	gl_Position = pos;// vec4(inPosition, 1.0);
	fragTexCoord = inTexCoord; //Pass out the texture coords
	
	 for (int i = 0; i < NUM_SAMPLES; i++)
    {
        kernel[i] = ubo.kernel[i];
    }
	blurDir = ubo.blurDirection;
}
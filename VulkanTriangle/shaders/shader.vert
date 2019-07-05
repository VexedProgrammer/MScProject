#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform GBufferUniformBufferObject 
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 1) out vec2 fragTexCoord;

void main() {

	vec4 pos = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);//Calculate the position
	gl_Position = pos;
	fragTexCoord = inTexCoord; //Pass out the texture coords
}
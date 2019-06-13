#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 lightrot;
	mat4 lightSpace;
	int enableLighting;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;


layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout (location = 4) out vec4 outShadowCoord;
layout(location = 5) out vec3 fragPos;
layout(location = 6) out int enableLighting;

vec3 lDir = vec3(0.0, 5.0, -5.0f);
layout(location = 2) out vec3 lightDir;

const mat4 bias = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {

	//lDir = lDir * mat3(ubo.lightrot); //Calucate the light position
	lightDir = normalize(vec3(0, 0.75, 0)- lDir);  //Calculate the light direction
	fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal; //Calculate the normal
	vec4 pos = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);//Calculate the position
	fragPos =  (ubo.model * vec4(inPosition, 1.0)).xyz;
	gl_Position = pos;
	enableLighting = ubo.enableLighting;
	fragTexCoord = inTexCoord; //Pass out the texture coords
	outShadowCoord = ( bias * ubo.lightSpace) * vec4(inPosition, 1.0);	//Calculate the shadow coords using a bias
}
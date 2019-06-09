#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 12) out;

layout (set = 0, binding = 2) uniform GeomUniformBufferObject 
{
	mat4 projection;
	mat4 model;
	mat4 view;
	vec2 screenSize;
} ubo;

layout (location = 1) in vec3 inNormal[];
layout (location = 2) in vec4 spos[];
layout (location = 3) in vec4 pos[];

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outTexCoords;

void main(void)
{	
	//for(int i=0; i<gl_in.length(); i++)
	//{

		gl_Position = spos[0];
		outColor = vec3(0.0, 0.0, 0.0);
		outTexCoords = vec2(0, 0);
		EmitVertex();
		gl_Position = pos[0];
		outColor = vec3(0.0, 0.0, 0.0);
		outTexCoords = vec2(0, 1);
		EmitVertex();
		gl_Position = pos[1];
		outColor = vec3(0.0, 0.0, 0.0);
		outTexCoords = vec2(1, 1);
		EmitVertex();
		EndPrimitive();
		gl_Position = pos[1];
		outColor = vec3(0.0, 0.0, 0.0);
		outTexCoords = vec2(1, 1);
		EmitVertex();
		gl_Position = spos[1];
		outColor = vec3(0.0, 0.0, 0.0);
		outTexCoords = vec2(1, 0);
		EmitVertex();
		gl_Position = spos[0];
		outColor = vec3(0.0, 0.0, 0.0);
		outTexCoords = vec2(0, 0);
		EmitVertex();
		EndPrimitive();

	//}

    EndPrimitive();
}
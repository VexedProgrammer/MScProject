#pragma once
#define SS_NUM_SAMPLES	17
#define SS_STRENGTH		{	.48f,	.41f,	.28f	}
#define SS_FALLOFF		{	1.f,	.37f,	.3f		}

#include <GLM/glm.hpp>

class SubsurfacePass
{
private:
	glm::vec3 gaussian(glm::vec3 falloff, float variance, float r)
	{
		glm::vec3 g;

		for (int i = 0; i < 3; i++)
		{
			float rr = r / (.001f + falloff[i]);
			g[i] = exp((-(rr * rr)) / (2.0f * variance)) / (2.f * 3.14f * variance);
		}

		return g;
	}
	glm::vec3 profile(glm::vec3 falloff, float r)
	{
		return  	//0.233f * gaussian(falloff, 0.0064f, r) +
			0.100f * gaussian(falloff, 0.0484f, r) +
			0.118f * gaussian(falloff, 0.187f, r) +
			0.113f * gaussian(falloff, 0.567f, r) +
			0.358f * gaussian(falloff, 1.99f, r) +
			0.078f * gaussian(falloff, 7.41f, r);
	}
public:
	glm::vec4 kernel[SS_NUM_SAMPLES];
	
	void computeKernel()
	{
		glm::vec3 strength = SS_STRENGTH;
		glm::vec3 falloff = SS_FALLOFF;


		static const float range = 2;
		static const float exponent = 2;

		float step = 2 * range / (SS_NUM_SAMPLES - 1);

		for (int i = 0; i < SS_NUM_SAMPLES; i++)
		{
			float o = -range + float(i) * step;
			float sign = o < 0 ? -1.f : 1.f;
			kernel[i].w = range * sign * abs(pow(o, exponent)) / pow(range, exponent);
		}

		for (int i = 0; i < SS_NUM_SAMPLES; i++)
		{
			float w0 = i > 0 ? abs(kernel[i].w - kernel[i - 1].w) : 0;
			float w1 = i < SS_NUM_SAMPLES - 1 ? abs(kernel[i].w - kernel[i + 1].w) : 0;
			float area = (w0 + w1) / 2.f;
			glm::vec3 t = area * profile(falloff, kernel[i].w);
			kernel[i].x = t.x;
			kernel[i].y = t.y;
			kernel[i].z = t.z;
		}

		glm::vec4 t = kernel[SS_NUM_SAMPLES / 2];
		for (int i = SS_NUM_SAMPLES / 2; i > 0; i--)
			kernel[i] = kernel[i - 1];
		kernel[0] = t;

		glm::vec3 sum = glm::vec3(0);
		for (int i = 0; i < SS_NUM_SAMPLES; i++)
			sum += glm::vec3(kernel[i].x, kernel[i].y, kernel[i].z);

		for (int i = 0; i < SS_NUM_SAMPLES; i++)
		{
			kernel[i].x /= sum.x;
			kernel[i].y /= sum.y;
			kernel[i].z /= sum.z;
		}

		kernel[0].x = (1.f - strength.x) + strength.x * kernel[0].x;
		kernel[0].y = (1.f - strength.y) + strength.y * kernel[0].y;
		kernel[0].z = (1.f - strength.z) + strength.z * kernel[0].z;

		for (int i = 1; i < SS_NUM_SAMPLES; i++)
		{
			kernel[i].x *= strength.x;
			kernel[i].y *= strength.y;
			kernel[i].z *= strength.z;
		}
	}
};
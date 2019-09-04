#pragma once
#define SAMPLES	25
#define STRENGTH	{	.48f,	.41f,	.28f	}
#define FALLOFF		{	1.f,	.37f,	.3f		}

#define GLFW_INCLUDE_VULKAN
#include <GLM/glm.hpp>

//! SubsufacePass
/*!
Contains the functions required for creation the Separable Kernel used for the Subsurface Scattering render pass.
Also holds the infomations for the required frame buffer and handles clean up.
*/
class SubsurfacePass
{
private:
	//! The gaussian member function
	/*!
	Function for calculating a point on a guassian curve based the offset parameter.
	Returns a vec3 of values for each colour channel (rgb).
	\param falloff vec3 How fast each colour channel diminishes 
	\param variance float Values from the three-layer skin model
	\param offset float Curves parameterised by time (offset acts as time)
	*/
	glm::vec3 gaussian(glm::vec3 falloff, float variance, float offset)
	{
		glm::vec3 returnValue; //Variable to return

		for (int i = 0; i < 3; i++) //Loop once for each colour channel (rgb)
		{
			float rr = offset / (.001f + falloff[i]); //Calculate the new offset based on the falloff value
			returnValue[i] = exp((-(rr * rr)) / (2.0f * variance)) / (2.f * 3.14f * variance); //Calculate gaussian
		}

		return returnValue;
	}
	//! The profile member function
	/*!
	Calulate the area values for the Separable kernel using the sum-of-6-gaussian method (three-layer skin model)
	Returns a vec3 of values for each colour channel.
	\param falloff vec3 How fast each colour channel diminishes
	\param offset float Curves parameterised by time (offset acts as time)
	*/
	glm::vec3 profile(glm::vec3 falloff, float offset)
	{
		return  	
			//0.233f * gaussian(falloff, 0.0064f, r) +
			0.100f * gaussian(falloff, 0.0484f, offset) +
			0.118f * gaussian(falloff, 0.187f, offset) +
			0.113f * gaussian(falloff, 0.567f, offset) +
			0.358f * gaussian(falloff, 1.99f, offset) +
			0.078f * gaussian(falloff, 7.41f, offset);
	}
public:
	//! Public VkImage.
	/*! Stores image data for the frame buffer*/
	VkImage SSImage;
	//! Public VkDeviceMemory.
	/*! Reference to allocated memory for the image data for framebuffer*/
	VkDeviceMemory SSImageMemory;
	//! Public VkImageView.
	/*! Reference to image subresources and meta data */
	VkImageView SSImageView;
	//! Public VkFramebuffer.
	/*! Vulkan Frame Buffer, used by the render pass */
	VkFramebuffer SSFrameBuffer;
	//! Public VkRenderPass.
	/*! Vulkan Render Pass, stores all the required attachments and subpasses*/
	VkRenderPass SSRenderPass;

	//! Public VkRenderPass.
	/*! Vulkan Pipeline, Graphics pipeline for the subsurface scattering passes*/
	VkPipeline SSGraphicsPipeline;
	//! Public VkDescriptorSet.
	/*! Vulkan Descriptor Set, holds the reference to textures and uniform buffers*/
	VkDescriptorSet finalSSet;
	//! Public VkBuffer.
	/*! Vulkan Buffer, Uniform buffer for subsurface scattering pass*/
	VkBuffer SSUniform;
	//! Public VkDeviceMemory.
	/*! Vulkan Device Memory, Refernece to allocated memory for the uniform buffer*/
	VkDeviceMemory SSUniformMemory;

	//! Public vec4 Array.
	/*! Array, holds the 1D kernel (Default contains a precomputed kernel for refernce) */
	glm::vec4 kernel[SAMPLES] = {
			glm::vec4(0.530605, 0.613514, 0.739601, 0),
			glm::vec4(0.000973794, 1.11862e-005, 9.43437e-007, -3),
			glm::vec4(0.00333804, 7.85443e-005, 1.2945e-005, -2.52083),
			glm::vec4(0.00500364, 0.00020094, 5.28848e-005, -2.08333),
			glm::vec4(0.00700976, 0.00049366, 0.000151938, -1.6875),
			glm::vec4(0.0094389, 0.00139119, 0.000416598, -1.33333),
			glm::vec4(0.0128496, 0.00356329, 0.00132016, -1.02083),
			glm::vec4(0.017924, 0.00711691, 0.00347194, -0.75),
			glm::vec4(0.0263642, 0.0119715, 0.00684598, -0.520833),
			glm::vec4(0.0410172, 0.0199899, 0.0118481, -0.333333),
			glm::vec4(0.0493588, 0.0367726, 0.0219485, -0.1875),
			glm::vec4(0.0402784, 0.0657244, 0.04631, -0.0833333),
			glm::vec4(0.0211412, 0.0459286, 0.0378196, -0.0208333),
			glm::vec4(0.0211412, 0.0459286, 0.0378196, 0.0208333),
			glm::vec4(0.0402784, 0.0657244, 0.04631, 0.0833333),
			glm::vec4(0.0493588, 0.0367726, 0.0219485, 0.1875),
			glm::vec4(0.0410172, 0.0199899, 0.0118481, 0.333333),
			glm::vec4(0.0263642, 0.0119715, 0.00684598, 0.520833),
			glm::vec4(0.017924, 0.00711691, 0.00347194, 0.75),
			glm::vec4(0.0128496, 0.00356329, 0.00132016, 1.02083),
			glm::vec4(0.0094389, 0.00139119, 0.000416598, 1.33333),
			glm::vec4(0.00700976, 0.00049366, 0.000151938, 1.6875),
			glm::vec4(0.00500364, 0.00020094, 5.28848e-005, 2.08333),
			glm::vec4(0.00333804, 7.85443e-005, 1.2945e-005, 2.52083),
			glm::vec4(0.000973794, 1.11862e-005, 9.43437e-007, 3),
	};
	
	//! The computeKernel member function
	/*!
	Calulates the separable kernel using the stength and fall of variables
	Result stored in the kernel array.
	*/
	void computeKernel()
	{
		glm::vec3 strength = STRENGTH;
		glm::vec3 falloff = FALLOFF;
		falloff *= 0.9f;
		strength *= 0.85f;


		static const float range = 2; //Max offset
		static const float exponent = 2; //Square

		float step = 2 * range / (SAMPLES - 1); //The step size for each sample

		//Calculate offsets
		for (int i = 0; i < SAMPLES; i++) //For each sample
		{
			float o = -range + float(i) * step;
			float sign = o < 0 ? -1.f : 1.f;
			kernel[i].w = range * sign * abs(pow(o, exponent)) / pow(range, exponent);
		}

		//Calculate strengths
		for (int i = 0; i < SAMPLES; i++) //For each sample
		{
			//Calculate diffrence between offsets
			float w0 = i > 0 ? abs(kernel[i].w - kernel[i - 1].w) : 0;
			float w1 = i < SAMPLES - 1 ? abs(kernel[i].w - kernel[i + 1].w) : 0;
			float area = (w0 + w1) / 2.f; //Average offset
			glm::vec3 t = area * profile(falloff, kernel[i].w); //Multiply by Three-Layer skin model profile
			//Set Values
			kernel[i].x = t.x;
			kernel[i].y = t.y;
			kernel[i].z = t.z;
		}

		glm::vec4 t = kernel[SAMPLES / 2];
		for (int i = SAMPLES / 2; i > 0; i--)
			kernel[i] = kernel[i - 1];
		kernel[0] = t;

		//average areas
		glm::vec3 sum = glm::vec3(0);
		for (int i = 0; i < SAMPLES; i++)
			sum += glm::vec3(kernel[i].x, kernel[i].y, kernel[i].z);

		for (int i = 0; i < SAMPLES; i++)
		{
			kernel[i].x /= sum.x;
			kernel[i].y /= sum.y;
			kernel[i].z /= sum.z;
		}

		//Alter based on strength
		kernel[0].x = (1.f - strength.x) + strength.x * kernel[0].x;
		kernel[0].y = (1.f - strength.y) + strength.y * kernel[0].y;
		kernel[0].z = (1.f - strength.z) + strength.z * kernel[0].z;

		for (int i = 1; i < SAMPLES; i++)
		{
			kernel[i].x *= strength.x;
			kernel[i].y *= strength.y;
			kernel[i].z *= strength.z;
		}

		
	}

	//! The CleanUpBuffer member function
	/*!
	Cleans up vulkan objects for the frame buffer
	*/
	void CleanUpBuffer(VkDevice &device)
	{
		vkDestroyImage(device, SSImage, nullptr);
		vkFreeMemory(device, SSImageMemory, nullptr);
		vkDestroyImageView(device, SSImageView, nullptr);

		vkDestroyFramebuffer(device, SSFrameBuffer, nullptr);
		vkDestroyRenderPass(device, SSRenderPass, nullptr);

		vkDestroyPipeline(device, SSGraphicsPipeline, nullptr);
	}

	//! The CleanUp member function
	/*!
	Cleans up vulkan objects for the uniform buffer
	*/
	void CleanUp(VkDevice &device)
	{
		vkDestroyBuffer(device, SSUniform, nullptr);
		vkFreeMemory(device, SSUniformMemory, nullptr);
	}
};
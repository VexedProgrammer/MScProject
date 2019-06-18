#pragma once

#include <GLM/glm.hpp>

class Lighting
{
public:
	struct PointLight
	{
		glm::vec4 Position;
		glm::vec4 Colour;
		float Intensity;
	};

	static glm::vec4 AmbientColour;
	static glm::vec4 DirectionalColour;
};
#pragma once

#include <GLM/glm.hpp>

//! Lighting
/*!
Lighting class that holds the colours/intensity of the scene lights
*/
class Lighting
{
public:
	//! Public vec4.
	/*! glm::vec4, ambient light colour*/
	static glm::vec4 AmbientColour;
	//! Public vec4.
	/*! glm::vec4, colour of the light*/
	static glm::vec4 LightColour;
};
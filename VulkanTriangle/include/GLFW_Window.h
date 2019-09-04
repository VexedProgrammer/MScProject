#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include <iostream>
#include <GLM\vec2.hpp>

//! GLFW_Window
/*!
Small class that creates a window for rendering too, using GLFW.
*/
class GLFW_Window
{
private:
	//! Private GLFWwindow pointer.
	/*! Pointer too the glfw window*/
	GLFWwindow * window;
	//! Private uint32_t.
	/*! number of extentions*/
	uint32_t extensionCount = 0;

	//! Private unsigned ints.
	/*! Width and Hieght of the window*/
	unsigned int WIDTH, HIEGHT;

public:
	//! GLFW_Window Contructor
	/*!
	Sets up material shader
	\param width unsigned int, Width of the window
	\param height unsigned int, height of the window
	\param title const char pointer, name at the top of the window
	*/
	GLFW_Window(unsigned int width, unsigned int height, const char* title);
	//! GLFW_Window Decontructor
	/*!
	Cleans up memory and objects in class
	*/
	~GLFW_Window();
	//! The UpdateWindow member function
	/*!
	Calls pollevent on the window to handle input events
	*/
	void UpdateWindow();
	//! The ShouldClose member function
	/*!
	Returns a bool, if true then the window is set to close (probally due to the exit button being pressed)
	*/
	const bool ShouldClose() const { return glfwWindowShouldClose(window); }

	//! The Window member function
	/*!
	Returns the point to the window
	*/
	GLFWwindow* Window() const { return window; }


	//! The getSize member function
	/*!
	Returns size of the window
	*/
	glm::vec2 getSize() const { return glm::vec2(WIDTH, HIEGHT); }
	//! The setSize member function
	/*!
	Sets the size of the window to a new size
	\param vec2 size, the new size of the window (x,y)
	*/
	const void setSize(glm::vec2 size) { WIDTH = size.x; HIEGHT = size.y; }
};
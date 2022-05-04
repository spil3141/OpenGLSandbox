#pragma once

#include <iostream>
#include "Utilities/Shader.h"

struct GLFWwindow;

namespace OpenGLSandbox {
	class Application
	{
	public:
		Application();
		~Application();

		void Run();

	private:
		void ProcessInputs();
		void CreateWindows();

	private:
		GLFWwindow* m_Window = nullptr;
		unsigned int m_Width = 800;
		unsigned int m_Height = 600;

		std::unique_ptr<Shader> m_UnlitShader = nullptr;
	};
}
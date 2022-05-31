#pragma once

#include <iostream>
#include "Utilities/Shader.h"


#include <glad/glad.h>
#include <glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <map>
#include <filesystem>
#include "Utilities/CharacterLibrary.h"
struct GLFWwindow;

namespace OpenGLSandbox {

	typedef char byte;

	class Application
	{
	public:
		Application();
		~Application();

		void Run();

	private:
		void ProcessInputs();
		void CreateWindows();
		void RenderText(unsigned int VAO, unsigned int VBO, Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);
		void CreateMSDFTexture();
	private:
		GLFWwindow* m_Window = nullptr;
		unsigned int m_Width = 800;
		unsigned int m_Height = 600;

		std::unique_ptr<Shader> m_UnlitShader;
		std::unique_ptr<Shader> m_ScreenShader;
		std::unique_ptr<Shader> m_TextShader;

		std::map<GLchar, Character> Characters;

		unsigned int m_FontTexture;

		CharacterLibrary m_CharacterLibrary;
	};
}
#include "Shader.h"
#include <fstream>
#include <iostream>
#include <glad/glad.h>

namespace OpenGLSandbox {
	Shader::Shader(const std::string& vertexSrcFilepath, const std::string& fragmentSrcFilepath)
	{
		std::string vertexSource = ReadFromFile(vertexSrcFilepath.c_str());
		std::string fragmentSource = ReadFromFile(fragmentSrcFilepath.c_str());

		m_RendererID = Compile(vertexSource, fragmentSource);
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_RendererID);
	}

	void Shader::Bind()
	{
		glUseProgram(m_RendererID);
	}

	void Shader::Unbind()
	{
		glUseProgram(0);
	}

	std::string Shader::ReadFromFile(const char* filepath)
	{
		std::string content;
		std::ifstream fileStream(filepath, std::ios::in);
		std::string line = "";

		while (!fileStream.eof())
		{
			std::getline(fileStream, line);
			content.append(line + "\n");
		}
		fileStream.close();
		return content;
	}

	unsigned int Shader::Compile(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		// create shader objects (vertex and fragment)
		unsigned int vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	
		// vertex shader setup and error handling
		const char* vertexStringPrt = vertexSrc.c_str();
		glShaderSource(vertexShaderID, 1, &vertexStringPrt, 0); // 0 = NULL
		glCompileShader(vertexShaderID);
		//TODO: Error Handling
		int result;
		char msg[512];
		glGetProgramiv(fragmentShaderID, GL_LINK_STATUS, &result);
		if (!result) {
			glGetProgramInfoLog(fragmentShaderID, 512, NULL, msg);
			std::cout << "ERROR::FRAGMENT_SHADER::PROGRAM::LINKING_FAILED\n" << msg << std::endl;
		}
		// fragment shader setup and error handling
		const char* fragmentStringPrt = fragmentSrc.c_str();
		glShaderSource(fragmentShaderID, 1, &fragmentStringPrt, NULL);
		glCompileShader(fragmentShaderID);
		glGetProgramiv(fragmentShaderID, GL_LINK_STATUS, &result);
		if (!result) {
			glGetProgramInfoLog(fragmentShaderID, 512, NULL, msg);
			std::cout << "ERROR::FRAGMENT_SHADER::PROGRAM::LINKING_FAILED\n" << msg << std::endl;
		}

		// create, attach, and setup shader program
		unsigned int shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShaderID);
		glAttachShader(shaderProgram, fragmentShaderID);
		glLinkProgram(shaderProgram);

		//Clean up Shaders
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);

		return shaderProgram;
	}

	void Shader::SetUniform4f(const std::string& uniformName, float x, float y, float z, float w)
	{
		int vertexColorLocation = glGetUniformLocation(m_RendererID, uniformName.c_str());
		glUniform4f(vertexColorLocation, x, y, x, w);
	}
}


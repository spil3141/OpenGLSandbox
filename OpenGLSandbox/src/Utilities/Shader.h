#pragma once
#include <string>

namespace OpenGLSandbox {

	class Shader
	{
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void Bind();
		void Unbind();
		void SetUniform4f(const std::string& uniformName, float x, float y, float z, float w);

	private:
		std::string ReadFromFile(const char* filepath);
		unsigned int Compile(const std::string& vertexSrc, const std::string& fragmentSrc);
		
	private:
		unsigned int m_RendererID;
	};
}


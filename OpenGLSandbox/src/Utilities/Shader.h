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
		void SetUniform1i(const std::string& uniformName, int data);

		inline unsigned int GetRendererID() { return m_RendererID; }
	private:
		std::string ReadFromFile(const char* filepath);
		unsigned int Compile(const std::string& vertexSrc, const std::string& fragmentSrc);
		
	private:
		unsigned int m_RendererID;
	};
}


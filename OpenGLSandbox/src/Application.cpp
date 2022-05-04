#include "Application.h"
#include <iostream>
#include <cassert>
#include "Utilities/Timer.h"
#include <glad/glad.h>
#include <glfw3.h>

namespace OpenGLSandbox {

	static void OnResize(GLFWwindow* window, int width, int height)
	{
		// make sure the viewport matches the new window dimensions; note that width and 
		// height will be significantly larger than specified on retina displays.
		glViewport(0, 0, width, height);
	}
	
	Application::Application()
	{
		if (m_Window != NULL)
			assert(false);

		CreateWindows();
		m_UnlitShader = std::make_unique<Shader>("res/Shaders/VertexShader.shader", "res/Shaders/FragmentShader.shader");

	}

	Application::~Application()
	{

	}

	void Application::ProcessInputs()
	{
		// poll IO events (keys pressed/released, mouse moved etc.)
		glfwPollEvents();
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(m_Window, true);
	}

	void Application::CreateWindows()
	{
		bool glfwInitResult = glfwInit();
		assert(glfwInitResult); // msg: "Error initializing glfw"

		//GLFW Hints
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // The version major number
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // the version minor number. 
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //GLFW_OPENGL_CORE_PROFILE OR GLFW_OPENGL_COMPAT_PROFILE
		//NOTE: Needed for Mac OS X 
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		// glfw window creation
		// --------------------
		m_Window = glfwCreateWindow(m_Width, m_Height, "LearnOpenGL", NULL, NULL);
		if (m_Window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return;
		}
		glfwMakeContextCurrent(m_Window);
		glfwSetFramebufferSizeCallback(m_Window, OnResize);
		glfwSwapInterval(0); // 0 = Off, 1 = v_sync, 2 = v_sync/2, etc

		// glad: load all OpenGL function pointers
		// ---------------------------------------
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return;
		}
	}

	void Application::Run()
	{
		float vertices[] = {
		 0.5f,  0.5f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
		};
		unsigned int indices[] = {  // note that we start from 0!
			0, 1, 3,  // first Triangle
			1, 2, 3   // second Triangle
		};
		unsigned int VBO, VAO, EBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		unsigned int attributePositionIndex = 0; // 0 is the index of the first vertex attribute
		glVertexAttribPointer(attributePositionIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(attributePositionIndex);

		// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		//glBindBuffer(GL_ARRAY_BUFFER, 0);

		// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		glBindVertexArray(0);

		//it's okay to unbind the EBO after unbinding the VAO. 
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// uncomment this call to draw in wireframe polygons.
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		int nrAttributes;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
		std::cout << "Maximum number of vertex attributes supported: " << nrAttributes << std::endl;

		int frames = 0;
		float timer = 0.0f;
		std::string windowTitle;

		// render loop
		// -----------
		while (!glfwWindowShouldClose(m_Window))
		{
			// input
			// -----
			ProcessInputs();

			// other operations: 
			float timeValue = (float)glfwGetTime();
			float greenValue = (sin(timeValue) / 2.0f) + 0.5f;

			/*int vertexColorLocation = glGetUniformLocation(shaderProgram, "u_Color");
			glUseProgram(shaderProgram);
			glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);*/
			m_UnlitShader->Bind();
			m_UnlitShader->SetUniform4f("u_Color", 0.0f, greenValue, 0.0f, 1.0f);
			m_UnlitShader->Unbind();

			// render
			// ------
			{
				glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
				// Frame begin
				{
					auto timer = Timer();
					glClear(GL_COLOR_BUFFER_BIT || GL_STENCIL_BUFFER_BIT);

					// draw our first triangle
					//glUseProgram(shaderProgram);
					m_UnlitShader->Bind();
					glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

					//glDrawArrays(GL_TRIANGLES, 0, 6);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
					// glBindVertexArray(0); // no need to unbind it every time
					// delay thread
					//std::this_thread::sleep_for(std::chrono::milliseconds(50));

					// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
					glfwSwapBuffers(m_Window);
					// -------------------------------------------------------------------------------
					windowTitle = timer.GetWindowTitle();
				}
			}

			//fps counter 
			if (timeValue - timer > 1.0f)
			{
				timer += 1.0;
				glfwSetWindowTitle(m_Window, (windowTitle + " fps: " + std::to_string(frames)).c_str());
				frames = 0;
			}
			frames++;
		}

		// glfw: terminate, clearing all previously allocated GLFW resources.
		// ------------------------------------------------------------------
		glfwTerminate();
	}

}
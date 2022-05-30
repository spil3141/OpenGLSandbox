#include "Application.h"
#include <iostream>
#include <cassert>
#include "Utilities/Timer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "msdfgen.h"
#include "msdfgen-ext.h"
#include "stb_image.h"

namespace OpenGLSandbox {

	static void OnResize(GLFWwindow* window, int width, int height);

	static void APIENTRY glDebugOutput(GLenum source,
		GLenum type,
		unsigned int id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const void* userParam);

	Application::Application()
	{
		if (m_Window != NULL)
			assert(false);

		CreateWindows();

		m_UnlitShader = std::make_unique<Shader>("res/Shaders/QuadVertexShader.shader", "res/Shaders/QuadFragmentShader.shader");
		m_ScreenShader = std::make_unique<Shader>("res/Shaders/ScreenVertex.shader", "res/Shaders/ScreenFragment.shader");
		m_TextShader = std::make_unique<Shader>("res/Shaders/TextV.shader", "res/Shaders/TextF.shader");

		// FreeType
		FT_Library ft;
		// All functions return a value different than 0 whenever an error occurred
		if (FT_Init_FreeType(&ft))
		{
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			return ;
		}


		// load font as face
		FT_Face face;
		if (FT_New_Face(ft, "res/Fonts/Forte/ForteRegular.ttf", 0, &face)) {
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			return ;
		}
		else {
			// set size to load glyphs as
			FT_Set_Pixel_Sizes(face, 0, 48);

			// disable byte-alignment restriction
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			// load first 128 characters of ASCII set
			for (unsigned char c = 0; c < 128; c++)
			{
				// Load character glyph 
				if (FT_Load_Char(face, c, FT_LOAD_RENDER))
				{
					std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
					continue;
				}
				// generate texture
				unsigned int texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexImage2D(
					GL_TEXTURE_2D,
					0,
					GL_RED,
					face->glyph->bitmap.width,
					face->glyph->bitmap.rows,
					0,
					GL_RED,
					GL_UNSIGNED_BYTE,
					face->glyph->bitmap.buffer
				);
				// set texture options
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				// now store character for later use
				Character character = {
					texture,
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					static_cast<unsigned int>(face->glyph->advance.x)
				};
				Characters.insert(std::pair<char, Character>(c, character));
			}
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		// destroy FreeType once we're finished
		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		CreateMSDFTexture();
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

		// enable OpenGL debug context if context allows for debug context
		int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}


		// OpenGL state
		// ------------
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	void Application::Run()
	{
		
		float vertices[] = {
		//  positions           texture coordinates 
		 0.5f,  0.5f, 0.0f,       1.0f, 1.0f,        // top right 
		 0.5f, -0.5f, 0.0f,       1.0f, 0.0f,        // bottom right
		-0.5f, -0.5f, 0.0f,       0.0f, 0.0f,       // bottom left
		-0.5f,  0.5f, 0.0f,       0.0f, 1.0f      // top left 
		};
		unsigned int indices[] = {  // note that we start from 0!
			0, 1, 3,  // first Triangle
			1, 2, 3   // second Triangle
		};

		float screenQuadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates. NOTE that this plane is now much smaller and at the top of the screen
		// positions   // texCoords
		 1.0f,   1.0f,  0.0f, 1.0f,
		-1.0f,  -1.0f,  1.0f, 0.0f,
		 1.0f,  -1.0f,  0.0f, 0.0f,

		-1.0f,  -1.0f,  0.0f, 1.0f,
		 1.0f,   1.0f,  1.0f, 0.0f,
		-1.0f,   1.0f,  1.0f, 1.0f
		};

		///////////////// Create a framebuffer  ///////////////////////////////////////////
		unsigned int FBO;
		glGenFramebuffers(1, &FBO);
		glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
		// Bind the framebuffer as current
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		// create attachments
		//   1. Texture attachment
		unsigned int textureColorAttachmentBufferID_1;
		glGenTextures(1, &textureColorAttachmentBufferID_1);
		glBindTexture(GL_TEXTURE_2D, textureColorAttachmentBufferID_1);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// attach attachment(s) to framebuffer 
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorAttachmentBufferID_1, 0);

		// check if framebuffer was created successfully
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			assert(false);

		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
																									  // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			assert(false);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);




		///////////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////  Quad VAO, VBO, EBO /////////////////////////////////
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

		// position attribute
		unsigned int attributePositionIndex = 0; // 0 is the index of the first vertex attribute
		glVertexAttribPointer(attributePositionIndex, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(attributePositionIndex);

		// texture coordinate attribute
		unsigned int attributeTexCoordIndex = 1; // 0 is the index of the first vertex attribute
		glVertexAttribPointer(attributeTexCoordIndex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(attributeTexCoordIndex);

		 //note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		 //remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		 //You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		 //VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		//it's okay to unbind the EBO after unbinding the VAO. 
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		

		////////////////////////////////////  Screen VAO, VBO, EBO /////////////////////////////////

		unsigned int Screen_VBO, Screen_VAO;
		glGenVertexArrays(1, &Screen_VAO);
		glGenBuffers(1, &Screen_VBO);
		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(Screen_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, Screen_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertices), screenQuadVertices, GL_STATIC_DRAW);

		// position attribute
		unsigned int screenAttributePositionIndex = 0; // 0 is the index of the first vertex attribute
		glVertexAttribPointer(screenAttributePositionIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(screenAttributePositionIndex);

		// texture coordinate attribute
		unsigned int ScreenAttributeTexCoordIndex = 1; // 0 is the index of the first vertex attribute
		glVertexAttribPointer(ScreenAttributeTexCoordIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(ScreenAttributeTexCoordIndex);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// configure VAO/VBO for texture quads
		unsigned int Text_VBO, Text_VAO;
		glGenVertexArrays(1, &Text_VAO);
		glGenBuffers(1, &Text_VBO);
		glBindVertexArray(Text_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, Text_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		// uncomment this call to draw in wireframe polygons.
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		int nrAttributes;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
		std::cout << "Maximum number of vertex attributes supported: " << nrAttributes << std::endl;

		int frames = 0;
		float timer = 0.0f;
		std::string windowTitle;

		glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(m_Width), 0.0f, static_cast<float>(m_Height));

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));

		// render loop
		// -----------
		while (!glfwWindowShouldClose(m_Window))
		{
			// input
			// -----
			ProcessInputs();

			// other operations: 
			float timeValue = (float)glfwGetTime();
			//float greenValue = (sin(timeValue) / 2.0f) + 0.5f;

			// Quad shader uniform
			m_UnlitShader->Bind();
			m_UnlitShader->SetUniform4m("u_MVP", scale);
			m_UnlitShader->SetUniform4f("u_Color", 0.0f, 0.0f, 0.0f, 1.0f);
			m_UnlitShader->SetUniform1i("u_fontTexture", 0);
			m_UnlitShader->Unbind();


			//Screen shader unforms
			m_ScreenShader->Bind();
			m_ScreenShader->SetUniform1i("u_ColorAttachmentTexIndex", 0);
			m_ScreenShader->Unbind();

			// Text shader 
			m_TextShader->Bind();
			glUniformMatrix4fv(glGetUniformLocation(m_TextShader->GetRendererID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			// render
			// ------
			{
				glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
				
				// Frame begin
				{
					auto timer = Timer();
					// first render pass
					//glBindFramebuffer(GL_FRAMEBUFFER, FBO);
					glClear(GL_COLOR_BUFFER_BIT || GL_STENCIL_BUFFER_BIT);

					// First draw pass 
					m_UnlitShader->Bind();
					glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, msdfTextureID);

					glFrontFace(GL_CW);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
					glBindVertexArray(0);

					// draw text 
					glFrontFace(GL_CCW);
					RenderText(Text_VAO, Text_VBO, *m_TextShader, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
					RenderText(Text_VAO, Text_VBO, *m_TextShader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
					
					// unbind the first render pass framebuffer: use default 
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
					// second render pass
					glClear(GL_COLOR_BUFFER_BIT || GL_STENCIL_BUFFER_BIT);

					// bind attachColorBuffer 
					m_ScreenShader->Bind();
					glBindVertexArray(Screen_VAO);
					glBindTexture(GL_TEXTURE_2D, textureColorAttachmentBufferID_1);
					glFrontFace(GL_CW);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					
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

		// Delete OpenGL Objects
		/*glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);*/
		//glDeleteFramebuffers(1, &FBO);

		// glfw: terminate, clearing all previously allocated GLFW resources.
		// ------------------------------------------------------------------
		glfwTerminate();
	}
	
	void Application::RenderText(unsigned int VAO, unsigned int VBO, Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
	{
		// activate corresponding render state	
		shader.Bind();
		glUniform3f(glGetUniformLocation(shader.GetRendererID(), "textColor"), color.x, color.y, color.z);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO);

		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};
			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Application::CreateMSDFTexture()
	{
		std::filesystem::path filepath = "res/Fonts/Forte/ForteRegular.ttf";
		std::string outputpath = (std::string("res/Exports/") + filepath.stem().string() + std::string(".png"));

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (ft) {
			msdfgen::FontHandle* font = msdfgen::loadFont(ft, filepath.string().c_str());
			if (font) {
				msdfgen::Shape shape;
				if (msdfgen::loadGlyph(shape, font, 'A')) {
					shape.normalize();
					//                      max. angle
					msdfgen::edgeColoringSimple(shape, 3.0);
					//           image width, height
					//msdfgen::Bitmap<float, 1> msdf(32, 32);
					msdfgen::Bitmap<float, 3> msdf(32, 32);

					//output, shape, range, scale, translation
					//msdfgen::generateSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
					msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));

					msdfgen::savePng(msdf, (outputpath).c_str());

					

				}
				msdfgen::destroyFont(font);
			}
			msdfgen::deinitializeFreetype(ft);
		}


		//Load 

		stbi_set_flip_vertically_on_load(true);

		int width, height, channels;
		bool srgb = true;

		stbi_uc* imageData = stbi_load(outputpath.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);

		//// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGB,
			width,
			height,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			imageData
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		msdfTextureID = texture;

		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(imageData);
	}

	static void OnResize(GLFWwindow* window, int width, int height)
	{
		// make sure the viewport matches the new window dimensions; note that width and 
		// height will be significantly larger than specified on retina displays.
		glViewport(0, 0, width, height);
	}

	static void APIENTRY glDebugOutput(GLenum source,
		GLenum type,
		unsigned int id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const void* userParam)
	{
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " << message << std::endl;

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} std::cout << std::endl;

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} std::cout << std::endl;

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
		std::cout << std::endl;
	}
}
#include "Application.h"


int main()
{
	OpenGLSandbox::Application* app = new OpenGLSandbox::Application;
	app->Run();
	delete app;
}

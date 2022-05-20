#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 io_TexCoord;

void main()
{
	io_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position.xyz, 1.0);
}

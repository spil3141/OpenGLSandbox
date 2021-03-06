#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 io_TexCoord;

uniform mat4 u_MVP;

void main()
{
	io_TexCoord = a_TexCoord;
	gl_Position = u_MVP * vec4(a_Position.xyz, 1.0);
}

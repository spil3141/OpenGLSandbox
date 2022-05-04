#version 330 core

out vec4 o_FinalColor;

uniform vec4 u_Color;

void main()
{
	o_FinalColor = u_Color;
}
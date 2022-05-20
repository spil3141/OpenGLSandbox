#version 330 core

out vec4 o_FinalColor;

in vec2 io_TexCoord;

uniform sampler2D u_ColorAttachmentTexIndex;

void main()
{
	vec3 texRGB = texture(u_ColorAttachmentTexIndex, io_TexCoord).rgb;
	o_FinalColor = vec4(texRGB, 1.0); //vec4(io_TexCoord, 0.0, 0.0); // 
}
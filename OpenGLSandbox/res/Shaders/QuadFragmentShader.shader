#version 330 core

out vec4 o_FinalColor;

in vec2 io_TexCoord;

uniform vec4 u_Color; 
uniform sampler2D u_fontTexture;

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() {
	return 4.5;
}

void main()
{
	vec3 msdf = texture(u_fontTexture, io_TexCoord).rgb;
	float sd = median(msdf.r, msdf.g, msdf.b);
	float screenPxDistance = screenPxRange() * (sd - 0.5);
	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	o_FinalColor = mix(u_Color, vec4(1.0, 1.0, 1.0, 1.0), opacity);
}
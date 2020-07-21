#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D display_texture;

void main()
{
	FragColor = texture(display_texture, TexCoord);
}
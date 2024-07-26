#version 330 core

layout (location = 0) in vec2 coords;
layout (location = 1) in vec2 uvcoords;

out vec2 uv;

// TODO: calculate convolution coordinates here

void main()
{
	gl_Position = vec4(coords, 0.0, 1.0);
	uv = uvcoords;
}
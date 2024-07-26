#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D textureIn;
uniform vec2 xy;
uniform vec2 resolution;

const float r =  30;
const float ir =  15;

const float value = .4;

vec2 toPixel(vec2 uv)
{
	return vec2((uv.x * resolution.x), (uv.y * resolution.y));
}

void main()
{
	vec4 colorOut = vec4(0.0);
	
	vec2 p = toPixel(uv - vec2(xy.x, 1.0 - xy.y));
	
	if(p.x * p.x + p.y * p.y <= r * r && p.x * p.x + p.y * p.y > ir * ir)
		colorOut = vec4(0.0,0.0,0.0,value); // state info in alpha channel
	else
		colorOut = vec4(0.0,0.0,0.0,0.0);
		
		
	FragColor = colorOut + texture(textureIn, uv);
}
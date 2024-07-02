#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D textureIn;
uniform vec2 resolution;

const float PI = 3.14159265;

const float kInnerR = 3.0;
const float kOuterR = 12.0;

float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

vec4 getPixel(vec2 c) 
{
	return texture(textureIn, c/resolution);
}

vec2 toPixel(vec2 uv)
{
	return uv * resolution;
}

float newState(float kisum, float kosum)
{
	if(kisum >= 0.5 && 0.26 <= kosum && kosum <= 0.46)
		return lerp(0.8, 1.0, kosum * kisum);
	if(kisum < 0.5 && 0.27 <= kosum && kosum <= 0.36)
		return lerp(0.8, 1.0, kosum * kisum);

	return lerp(0.0, 0.2, kosum * kisum);

	/*
	if(kisum >= 0.5 && 0.26 <= kosum && kosum <= 0.46)
		return 1.0;
	if(kisum < 0.5 && 0.27 <= kosum && kosum <= 0.36)
		return 1.0;*/

	return 0.0;
}

float convolve(vec2 pos, float rad)
{
	float res = 0.0;

	for(float y = -rad; y <= rad; y+=1.0)
	{
		for(float x = -rad; x <= rad; x+=1.0)
		{
			if(x * x + y * y <= rad * rad)
			{
				res += getPixel(pos + vec2(x,y)).r;
			}
		}
	}

	return res;
}

void main()
{
	vec2 pos = toPixel(uv);

	float kisum = convolve(pos, kInnerR);
	float kosum = convolve(pos, kOuterR);

	//kosum -= kisum;

	kisum /= PI * kInnerR * kInnerR;
	kosum /= PI * kOuterR * kOuterR;


	float state = newState(kisum, kosum);
	//FragColor = vec4(state,state,state,1.0);
	FragColor = vec4(state,kisum,kosum,1.0);

	// FragColor = vec4(pos.x / resolution.x, pos.y / resolution.y, 0.0, 1.0);
}
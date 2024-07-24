#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D textureIn;
uniform vec2 resolution;
uniform vec2 invResolution;

const float PI = 3.14159265;

const float kInnerR = 4.0;
const float kOuterR = 14.0;

float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

float newState(float kisum, float kosum)
{
	/*if(kisum >= 0.5 && 0.26 <= kosum && kosum <= 0.46)
		return lerp(0.8, 1.0, kosum * kisum);
	if(kisum < 0.5 && 0.27 <= kosum && kosum <= 0.36)
		return lerp(0.8, 1.0, kosum * kisum);

	return lerp(0.0, 0.1, kosum * kisum);*/

	if(kisum >= 0.5 && 0.16 <= kosum && kosum <= 0.46)
		return lerp(0.8, 1.0, kosum * kisum);
	if(kisum < 0.5 && 0.27 <= kosum && kosum <= 0.36)
		return lerp(0.8, 1.0, kosum * kisum);

	return lerp(0.0, 0.1, kosum * kisum);
}

float convolve(float rad)
{
	float res = 0.0;

	for(float y = -rad; y <= rad; y++)
	{
		for(float x = -rad; x <= rad; x++)
		{
			if(x * x + y * y <= rad * rad)
			{
				vec2 d = vec2(x, y);
				vec2 offset = d * invResolution;
				vec2 spos = fract(uv + offset);

				res += texture(textureIn, spos).r;
			}
		}
	}

	return res;
}

void main()
{
	float kisum = convolve(kInnerR);
	float kosum = convolve(kOuterR);

	//kosum -= kisum;

	kisum /= PI * kInnerR * kInnerR;
	kosum /= PI * kOuterR * kOuterR;

	float state = newState(kisum, kosum);
	//FragColor = vec4(state,state,state,1.0);
	FragColor = vec4(state,kisum,kosum,1.0);
	//FragColor = vec4(state,kisum*state,kosum*state,1.0);
}
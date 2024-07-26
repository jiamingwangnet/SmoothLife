#version 330 core

// https://arxiv.org/pdf/1111.1567

out vec4 FragColor;

in vec2 uv;

uniform sampler2D textureIn;
uniform vec2 resolution;
uniform vec2 invResolution;

const float PI = 3.14159265;

const float ri = 2.0;
const float ra = 12.0;

float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

float transitionFunc(float m, float n) // m := inner radius, n := outer radius
{
	if(m >= 0.5 && 0.26 <= n && n <= 0.46)
		return 1.0;
	if(m <= 0.5 && 0.27 <= n && n <= 0.36)
		return 1.0;

	return 0.0;

	/*if(kisum >= 0.5 && 0.16 <= kosum && kosum <= 0.46)
		return lerp(0.8, 1.0, kosum * kisum);
	if(kisum < 0.5 && 0.27 <= kosum && kosum <= 0.36)
		return lerp(0.8, 1.0, kosum * kisum);

	return lerp(0.0, 0.1, kosum * kisum);*/

	/*if(kisum >= 0.67 && 0.56 <= kosum && kosum <= 0.76)
		return 0.03;
	if(kisum < 0.67 && 0.25 <= kosum && kosum <= 0.56)
		return 1.0;*/
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
				// texture read is calculated within fragment shader
				// very expensive

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
	float m = convolve(ri); // inner
	float n = convolve(ra); // outer

	n -= m;

	m /= PI * ri * ri;
	n /= PI * ra * ra;

	float state = transitionFunc(m, n);
	FragColor = vec4(state,state,state,1.0);
	//FragColor = vec4(state,m,n,1.0);
	//FragColor = vec4(state,kisum*state,kosum*state,1.0);
}
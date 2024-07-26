#version 330 core

// https://arxiv.org/pdf/1111.1567

out vec4 FragColor;

in vec2 uv;

// represents the state function f(x, t)
// uses a texture as float array
uniform sampler2D textureIn;

uniform vec2 resolution;
uniform vec2 invResolution;

uniform float ri; // inner radius
uniform float ra; // outer radius

const float PI = 3.14159265;

const float dt = 0.2;

// width of the step
const float alpha_m = 0.147;
const float alpha_n = 0.028;

// birth and death intervals
// [b1, b2] birth
// [d1, d2] death
const float b1 = 0.257;
const float b2 = 0.336;
const float d1 = 0.365;
const float d2 = 0.549;

// use smooth step sigmoid functions

float sigmoid1(float x, float a, float al)
{
	return 1.0/(1.0 + exp(-(x-a) * 4.0 / al));
}

float sigmoid2(float x, float a, float b, float al)
{
	return sigmoid1(x,a, al) * (1.0 - sigmoid1(x,b, al));
}

float sigmoidm(float x, float y, float m, float al)
{
	return x * (1.0 - sigmoid1(m, 0.5, al))+ y * sigmoid1(m, 0.5, al);
}

// m := inner radius, n := outer radius
float transition(vec2 f) 
{
	return sigmoid2(f.x, sigmoidm(b1, d1, f.y, alpha_m), sigmoidm(b2,d2, f.y,alpha_m), alpha_n);
	//return sigmoidm(sigmoid2(f.x,b1, b2, alpha_n), sigmoid2(f.x,d1,d2 ,alpha_n), f.y, alpha_m);
	//return sigmoid2(f.x, sigmoidm(f.y,b1, d1, alpha_m), sigmoidm(f.y,b2,d2,alpha_m), alpha_n);
}

// use antialiasing to remove jagged edges

// antialiasing zone around the rim of width b
// l = |u| (distance from the centre cell to the current cell)
// we take the function value as it is, when l < ri - b/2
// l > ri + b/2 take 0
// inbetween take f(x + u, t) * (ri + b/2 - l)/b

// Typically, b = 1 is chosen

const float b = 1.0;

// use clamp instead of ifs for efficiency
float ramp(float l, float r)
{
	return clamp(-l/b + (r + b/2.0)/b,0.0,1.0);
}

// convolve both inner and outer ring
// x = outer
// y = inner
vec2 convolve(vec2 r)
{
	vec2 res = vec2(0.0);

	for(float y = -r.x; y <= r.x; y++)
	{
		for(float x = -r.x; x <= r.x; x++)
		{
			float lsq = x * x + y * y;

			if(lsq <= r.x * r.x)
			{
				// texture read is calculated within fragment shader
				// very expensive

				vec2 d = vec2(x, y);
				vec2 offset = d * invResolution;
				vec2 spos = fract(uv + offset);
				float v = texture(textureIn, spos).w; // store state information in alpha channel

				// temp solution
				// TODO: change to a step function
				if(lsq <= r.y * r.y)
					res.y += ramp(sqrt(lsq), r.y) * v;
				else
					res.x += ramp(sqrt(lsq), r.x) * v;
			}	
		}
	}

	return res;
}

const vec4 color = vec4(92.0/255.0,176.0/255.0,255.0/255.0,1.0);

void main()
{
	vec2 rad = vec2(ra, ri);

	vec2 f = convolve(rad);

	// normalise
	f /= PI * rad * rad;
	float v = texture(textureIn, uv).w;

	float state = v + dt * (2.0 * transition(f) - 1.0);
	state = clamp(state, 0.0, 1.0);

	//float state = transition(f);

	//FragColor = vec4(state) * color;
	//FragColor = vec4(state,state*m,state*n,state);
	//FragColor = vec4(state,m,n,state);
	//FragColor = vec4(0.0,n*m,state,state);
	//FragColor = vec4(state,m,state,state) * color;

	FragColor = vec4(vec3(state) * color.rgb + vec3(f.y) * color.rgb + vec3(f.x) * color.rgb, state);
}
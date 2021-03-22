#version 330

#define M_PI        3.14159265358979323846264338327950288   /* pi             */

in GeometryShaderOut
{
    vec3 direction;
} fs_in;

out vec4 color_out_of_fragment_shader;

vec3 Hsv2Rgb(float h, float s, float v)
{
	float r = 0.0f, g = 0.0f, b = 0.0f;
	
//	h = 2.0f / 3.0f * (1.0f - h);// 240.0 / 360.0 * (1 - h);
	
//	if (0 == floor(s))
//	{
//		r = v;
//		g = v;
//		b = v;
//	}
//	else
	{
		float var_h = h * 6.0f;
		
		if (6 == floor(var_h))
		{
			var_h = 0.0f;
		}
		float var_i = floor(var_h);
		float var_1 = v * (1.0f - s);
		float var_2 = v * (1.0f - s * (var_h - var_i));
		float var_3 = v * (1.0f - s * (1.0f - (var_h - var_i)));
		float var_r = 0.0f;
		float var_g = 0.0f;
		float var_b = 0.0f;
		
		if (0 == floor(var_i))
		{
			var_r = v;
			var_g = var_3;
			var_b = var_1;
		}
		else if (1 == floor(var_i))
		{
			var_r = var_2;
			var_g = v;
			var_b = var_1;
		}
		else if (2 == floor(var_i))
		{
			var_r = var_1;
			var_g = v;
			var_b = var_3;
		}
		else if (3 == floor(var_i))
		{
			var_r = var_1;
			var_g = var_2;
			var_b = v;
		}
		else if (4 == floor(var_i))
		{
			var_r = var_3;
			var_g = var_1;
			var_b = v;
		}
		else
		{
			var_r = v;
			var_g = var_1;
			var_b = var_2;
		}
		
		r = var_r;
		g = var_g;
		b = var_b;
	}
	
//	//black values for the test
//	r = 0.0;
//	g = 0.0;
//	b = 0.0;
	return vec3(r, g, b);
}

float atan2(float y, float x)
{
	float phi = 0.0f;
	if (x > 0.0f)
	{
		phi = atan(y / x);
	}
	else if (x < 0.0f && y >= 0.0f)
	{
		phi = atan(y / x) + M_PI;
	}
	else if (x < 0.0f && y < 0.0f)
	{
		phi = atan(y / x) - M_PI;
	}
	else if (x == 0.0f && y > 0.0f)
	{
		phi = M_PI / 2.0f;
	}
	else if (x == 0.0f && y < 0.0f)
	{
		phi = - M_PI / 2.0f;
	}
	else
	{
		phi = 0.0f;
	}
	if (phi < 0.0f)
	{
		phi += 2.0f * M_PI;
	}
	return phi;
}

void main()
{
	vec3 rgb = vec3(0.0f, 0.0f, 0.0f);
//	vec3 rgb = Hsv2Rgb(atan(fs_in.direction[1] / fs_in.direction[0]));
	float phi = atan2(fs_in.direction[1], fs_in.direction[0]) / (2.0f * M_PI);
	if (fs_in.direction[2] >= 0.0f)
	{
		rgb = vec3(1.0f, 1.0f, 0.0f);
//		rgb = Hsv2Rgb(phi, 0.75f, 1.0f);
		color_out_of_fragment_shader = vec4(rgb, 0.5f);
	}
	else
	{
		rgb = vec3(0.0f, 1.0f, 1.0f);
//		rgb = Hsv2Rgb(phi, 1.0f, 0.75f);
		color_out_of_fragment_shader = vec4(rgb, 0.5f);
	}
//    color_out_of_fragment_shader = vec4(rgb, 1.0f);
}

#version 330

#define M_PI        3.14159265358979323846264338327950288   /* pi             */

in GeometryShaderOut
{
    float density;
    float phi;
} fs_in;

out vec4 color_out_of_fragment_shader;

vec3 Jet2Rgb(float j)
{
    //	j *= 3.0f/4.0f;
    float r = 0.0f, g = 0.0f, b = 0.0f;
    if (j <= 1.0f / 8.0f)
    {
        r = 0.0f;
        g = 0.0f;
        b = (1.0f - 9.0f / 16.0f) / (1.0f / 8.0f - 0.0f) * (j - 0.0f) + 9.0f / 16.0f;
    }
    else if (j <= 3.0f / 8.0f)
    {
        r = 0.0f;
        g = (1.0f - 0.0f) / (3.0f / 8.0f - 1.0f / 8.0f) * (j - 1.0f / 8.0f) + 0.0f;
        b = 1.0f;
    }
    else if (j <= 5.0f / 8.0f)
    {
        r = (1.0f - 0.0f) / (5.0f / 8.0f - 3.0f / 8.0f) * (j - 3.0f / 8.0f) + 0.0f;
        g = 1.0f;
        b = (0.0f - 1.0f) / (5.0f / 8.0f - 3.0f / 8.0f) * (j - 3.0f / 8.0f) + 1.0f;
    }
    else if (j <= 7.0f / 8.0f)
    {
        r = 1.0f;
        g = (0.0f - 1.0f) / (7.0f / 8.0f - 5.0f / 8.0f) * (j - 5.0f / 8.0f) + 1.0f;
        b = 0.0f;
    }
    else
    {
        r = (9.0f / 16.0f - 1.0f) / (1.0f - 7.0f / 8.0f) * (j - 7.0f / 8.0f) + 1.0f;
        g = 0.0f;
        b = 0.0f;
    }
    return vec3(r, g, b);
}

vec3 Hsv2Rgb(float h, float s, float v)
{
    float r = 0.0f, g = 0.0f, b = 0.0f;

    //	h = (tanhf(100.0f * (h - red_blue_boundary) + 2.0f)) / 4.0f;
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

float StretchedColorMap(float c)
{
    // lc: 0.49f * (tanh(10.0f * (c - 0.7f)) + 1.0f) + 0.02f;
    return 0.5f * (tanh(3.0f * (c - 0.5f)) + 1.0f);
//    return 1.0f / (1.0f + exp(-10.0f * (c - 0.5f)));
}

float NormalizedStretchedColorMap(float c)
{
    float min_value = StretchedColorMap(0.0f);
    float max_value = StretchedColorMap(1.0f);
    return (StretchedColorMap(c) - min_value) / (max_value - min_value);
}

void main()
{
    vec3 rgb;
    float alpha;

//    rgb = Jet2Rgb(fs_in.density);
     	rgb = Jet2Rgb(NormalizedStretchedColorMap(fs_in.density));
//    	rgb = Jet2Rgb(0.5f * (sin(4.0f * M_PI * (fs_in.density + 8.0f * M_PI / 4.0f)) + 1.0f));
    //	rgb = Hsv2Rgb(fs_in.phi, 0.75f, 1.0f);
    //	rgb = vec3(1.0f, 1.0f, 1.0f);

//        alpha = 1.0f * fs_in.density;
     	alpha = NormalizedStretchedColorMap(fs_in.density);
//    	alpha = 0.05f * (sin(2.0f * M_PI * (fs_in.density + 8.0f * M_PI / 4.0f)) + 1.0f);
    //	if (fs_in.density > 0.9f)
    //	{
    //		alpha = 0.05f;
    //	}
    //	else
    //	{
    //		alpha = 0.05f;
    //	}
    //	alpha = 0.3f * (sin(8.0f * 2.0f * M_PI * fs_in.density) + 1.0f);

    color_out_of_fragment_shader = vec4(rgb, alpha);
}

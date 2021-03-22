#version 330

in vec3 position;
in float density;

out VertexShaderOut
{
	float density;
} vs_out;

//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
//uniform vec3 dims;

void main()
{
	// cell centers are in [0,1)x[0,1)x[0,1)
	vec4 position_opengl = vec4(position.x, position.z, (1.0f - position.y), 1.0f);
	gl_Position = position_opengl;
//	gl_PointSize = 1.0f;
	
	vs_out.density = density;
// 	if (position.x > 0.5f && position.y > 0.5f)
// 	{
// 	    vs_out.density = 0.0f;
// 	}
}

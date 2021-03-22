#version 330

in vec3 position;
in vec3 direction;

out VertexShaderOut
{
	vec3 direction;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position.x, position.z, position.y, 1.0f);
	gl_PointSize = 1.0f;
	
	vs_out.direction = direction;
}

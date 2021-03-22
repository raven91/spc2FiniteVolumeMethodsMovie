#version 330

in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//uniform vec3 dims;

void main()
{
	vec3 dims = vec3(1.0f, 1.0f, 1.0f);
	vec4 position_opengl = vec4(position.x / dims.x, position.z / dims.z, (dims.y - position.y) / dims.y, 1.0f);
	gl_Position = projection * view * model * position_opengl;
}

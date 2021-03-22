#version 330

layout (lines) in;
layout (line_strip, max_vertices = 2) out;
//uniform mat4 mvp;

in VertexShaderOut
{
    vec3 direction;
} gs_in[2];

out GeometryShaderOut
{
    vec3 direction;
} gs_out;

void main()
{
	gs_out.direction = 0.5 * (gs_in[0].direction + gs_in[1].direction);
	
	vec3 point_0 = gl_in[0].gl_Position.xyz;
	vec3 point_1 = gl_in[1].gl_Position.xyz;
	
	gl_Position = vec4(point_0, 1.0f); EmitVertex();
	gl_Position = vec4(point_1, 1.0f); EmitVertex();

	EndPrimitive();
}

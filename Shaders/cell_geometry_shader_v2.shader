#version 330

layout (points) in;
layout (points, max_vertices = 1) out;
//uniform mat4 mvp;

in VertexShaderOut
{
    float density;
} gs_in[1];

out GeometryShaderOut
{
    float density;
	float phi;
} gs_out;

void main()
{
	gs_out.density = gs_in[0].density;
	gs_out.phi = gl_in[0].gl_Position.y;

	vec3 cell_center = gl_in[0].gl_Position.xyz;
	gl_Position = vec4(cell_center, 1.0f); EmitVertex();
	EndPrimitive();
}

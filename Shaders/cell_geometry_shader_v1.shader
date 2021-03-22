#version 330

#define M_PI        3.14159265358979323846264338327950288   /* pi             */

layout (points) in;
layout (triangle_strip, max_vertices = 24) out;

in VertexShaderOut
{
    float density;
} gs_in[1];

out GeometryShaderOut
{
    float density;
	float phi;
} gs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 dims;

void main()
{
	mat4 mvp = projection * view * model;

	// cell centers are now in the normalized (OpenGL) coordinate system
	vec3 cell_center = gl_in[0].gl_Position.xyz;
	float radius_x = 1.0f / dims.x / 2.0f;
	float radius_y = 1.0f / dims.z / 2.0f;
	float radius_phi = 1.0f / dims.y / 2.0f;
	
	vec4 A = mvp * vec4(cell_center.x - radius_x, cell_center.y + radius_y, cell_center.z - radius_phi, 1.0f);
	vec4 B = mvp * vec4(cell_center.x + radius_x, cell_center.y + radius_y, cell_center.z - radius_phi, 1.0f);
	vec4 C = mvp * vec4(cell_center.x + radius_x, cell_center.y + radius_y, cell_center.z + radius_phi, 1.0f);
	vec4 D = mvp * vec4(cell_center.x - radius_x, cell_center.y + radius_y, cell_center.z + radius_phi, 1.0f);
	vec4 E = mvp * vec4(cell_center.x - radius_x, cell_center.y - radius_y, cell_center.z - radius_phi, 1.0f);
	vec4 F = mvp * vec4(cell_center.x + radius_x, cell_center.y - radius_y, cell_center.z - radius_phi, 1.0f);
	vec4 G = mvp * vec4(cell_center.x + radius_x, cell_center.y - radius_y, cell_center.z + radius_phi, 1.0f);
	vec4 H = mvp * vec4(cell_center.x - radius_x, cell_center.y - radius_y, cell_center.z + radius_phi, 1.0f);

	gs_out.density = gs_in[0].density;
	gs_out.phi = gl_in[0].gl_Position.y;

	// upper face
	gl_Position = A; EmitVertex();
	gl_Position = D; EmitVertex();
	gl_Position = B; EmitVertex();
	gl_Position = C; EmitVertex();
	EndPrimitive();

	// lower face
	gl_Position = G; EmitVertex();
	gl_Position = H; EmitVertex();
	gl_Position = F; EmitVertex();
	gl_Position = E; EmitVertex();
	EndPrimitive();

	// right face
	gl_Position = B; EmitVertex();
	gl_Position = C; EmitVertex();
	gl_Position = F; EmitVertex();
	gl_Position = G; EmitVertex();
	EndPrimitive();

	// left face
	gl_Position = A; EmitVertex();
	gl_Position = E; EmitVertex();
	gl_Position = D; EmitVertex();
	gl_Position = H; EmitVertex();
	EndPrimitive();

	// front face
	gl_Position = C; EmitVertex();
	gl_Position = D; EmitVertex();
	gl_Position = G; EmitVertex();
	gl_Position = H; EmitVertex();
	EndPrimitive();

	// back face
	gl_Position = A; EmitVertex();
	gl_Position = B; EmitVertex();
	gl_Position = E; EmitVertex();
	gl_Position = F; EmitVertex();
	EndPrimitive();

//	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
//	//vec3 normal = normalize(vec3(projection * vec4(normalMatrix * aNormal, 1.0)));
//	gl_Position = vec4(cell_center, 1.0f); EmitVertex();
//	gl_Position = projection * vec4(normalMatrix * vec3(1.0f, 0.0f, 0.0f), 1.0f); EmitVertex();
//	gl_Position = projection * vec4(normalMatrix * vec3(0.0f, 1.0f, 0.0f), 1.0f); EmitVertex();
//	gl_Position = projection * vec4(normalMatrix * vec3(0.0f, 0.0f, 1.0f), 1.0f); EmitVertex();
//	EndPrimitive();
}

#version 420

in block
{
	vec3	pos;
	vec3	n;
	vec3	patch_coords;
} In;

layout(location=0) out vec4 fs_out_color;
layout(location=1) out vec3 fs_out_normal;

uniform float patch_boundary_width = 0.015f;

void main()
{
	fs_out_color = vec4(1);
	fs_out_normal = vec3(0);
}
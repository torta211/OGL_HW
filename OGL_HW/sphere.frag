#version 420

in block
{
	vec3	pos;
	vec3	color;
	vec3	n;
	vec3	patch_coords;
} In;

layout(location=0) out vec4 fs_out_color;
layout(location=1) out vec3 fs_out_normal;

uniform vec3 eye_pos;

uniform float patch_boundary_width = 0.015f;

void main()
{
	// middle of lights are almost white
	vec3 additionalLightness = vec3(pow(dot(In.n, normalize(eye_pos - In.pos)), 3));
	fs_out_color = vec4(In.color + additionalLightness, 1.0f);
	fs_out_normal = vec3(0);
}
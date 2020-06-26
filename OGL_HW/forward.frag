#version 400

// per-fragment attributes coming from the pipeline
in vec3 vs_out_pos;
in vec3 vs_out_normal;
in vec2 vs_out_tex0;

// multiple outputs are directed into different color textures by the FBO
layout(location=0) out vec4 fs_out_color;
layout(location=1) out vec3 fs_out_normal;
layout(location=2) out vec4 fs_out_position;
layout(location=3) out vec4 fs_out_material;

uniform vec3 eye_pos;

// Material (they will have the same multiplier for each color...)
uniform float Ka = 1.0f;
uniform float Kd = 1.0f;
uniform float Ks = 1.0f;
uniform float specular_power = 50;
uniform sampler2D texImage;
uniform uint opacity = 255;

void main()
{
	fs_out_color = vec4(texture(texImage, vs_out_tex0.st).xyz, opacity);	
	fs_out_normal = normalize(vs_out_normal);
	fs_out_position = vec4(vs_out_pos, 1);
	fs_out_material = vec4(Ka, Kd, Ks, specular_power);
}
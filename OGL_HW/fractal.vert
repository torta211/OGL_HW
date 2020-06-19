#version 330 core

// variables coming from the VBO
in vec3 vs_in_pos;

// variables going forward through the pipeline
out vec3 vs_out_pos;

void main()
{
	gl_Position = vec4( vs_in_pos, 1 );
	vs_out_pos = vs_in_pos;
}
#version 400

layout(location = 0) in vec3 vs_in_pos;
uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4( vs_in_pos, 1 );
}
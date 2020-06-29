#version 420

uniform vec3 lightPoints[100];
uniform float lightRads[100];
uniform vec3 lightColors[100];

// a pipeline-ban tovább adandó értékek
out block
{
	vec3	pos;
	float	rad;
	vec3	color;
} Out;

void main()
{
	Out.pos		= lightPoints[gl_VertexID];
	Out.rad		= lightRads[gl_VertexID];
	Out.color	= lightColors[gl_VertexID];
}
#version 420

uniform vec3 lightPoints[100];

// a pipeline-ban tov�bb adand� �rt�kek
out block
{
	vec3	pos;
	float	rad;
} Out;

void main()
{
	Out.pos		= lightPoints[gl_VertexID];
	Out.rad		= 1.5;
}
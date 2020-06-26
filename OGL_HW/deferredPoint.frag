#version 130

in vec2 vs_out_tex;

out vec4 fs_out_col;

uniform vec3 eye_pos;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D positionTexture;
uniform sampler2D materialTexture;

uniform vec3 lightPos = vec3(-355, 80, -355);
uniform vec4 La = vec4(0.05, 0.05, 0.05, 1.0);
uniform vec4 Ld = vec4(0.5, 0.5, 0.5, 1);
uniform vec4 Ls = vec4(0.4, 0.4, 0.4, 1);

void main()
{
	vec3 pos = texture(positionTexture, vs_out_tex).rgb;
	vec3 normal = normalize(texture( normalTexture, vs_out_tex ).rgb);
	vec3 toLight = normalize(lightPos - pos);

	vec4 material = texture(materialTexture, vs_out_tex);

	vec4 ambient = La * vec4(material.r);

	float di = clamp(dot(toLight, normal), 0.0f, 1.0f);
	vec4 diffuse = vec4(di * Ld.rgb * vec3(material.g), material.g);

	vec4 specular = vec4(0);
	if (di > 0.0f)
	{
		vec3 toEye = normalize(eye_pos - pos);
		vec3 r = reflect(-toLight, normal);
		float si = pow(clamp(dot(toEye, r), 0.0f, 1.0f), material.a);
		specular = Ls * vec4(material.b) * si;
	}

	fs_out_col = (ambient + diffuse + specular) * texture(colorTexture, vs_out_tex);
}
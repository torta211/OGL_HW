#version 130

in vec2 vs_out_tex;

out vec4 fs_out_col;

uniform vec3 eye_pos;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D positionTexture;
uniform sampler2D materialTexture;

uniform vec3 lightPositions[100];
uniform vec4 La = vec4(0.1, 0.1, 0.1, 1.0);
uniform vec4 Ld = vec4(0.8, 0.8, 0.8, 1);
uniform vec4 Ls = vec4(0.6, 0.6, 0.6, 1);

void main()
{
	vec4 ambient = vec4(0.0f);
	vec4 diffuse = vec4(0.0f);
	vec4 specular = vec4(0.0f);

	for (int i = 0; i < 100; ++i)
	{
		vec3 pos = texture(positionTexture, vs_out_tex).rgb;
		vec3 toLight = lightPositions[i] - pos;
		float dist = length(toLight);
		if (dist < 150.0f)
		{
			toLight = normalize(toLight);
			vec3 normal = normalize(texture( normalTexture, vs_out_tex ).rgb);
			float strength = 100.0f / pow(dist, 2.0f);

			vec4 material = texture(materialTexture, vs_out_tex);

			ambient += La * vec4(material.r) * strength;

			float di = clamp(dot(toLight, normal), 0.0f, 1.0f);
			diffuse += vec4(di * Ld.rgb * vec3(material.g) * strength, material.g);

			if (di > 0.0f)
			{
				vec3 toEye = normalize(eye_pos - pos);
				vec3 r = reflect(-toLight, normal);
				float si = pow(clamp(dot(toEye, r), 0.0f, 1.0f), material.a);
				specular += Ls * vec4(material.b) * si * strength;
			}
		}
	}

	fs_out_col = (ambient + diffuse + specular) * texture(colorTexture, vs_out_tex);
}
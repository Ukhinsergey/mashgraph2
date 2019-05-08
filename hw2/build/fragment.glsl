#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 color;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D ourTexture1;
uniform int isTetrahedron;
void main()
{	

	float specularStrength = 0.5f;
	float ambientStrength = 0.1f;
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 ambient = ambientStrength * lightColor;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);


	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 res = ambient + diffuse + specular;
	if (isTetrahedron == 1) {
		color = vec4(res * objectColor, 1.0f);
	} else {

		color = texture(ourTexture1, TexCoord) * vec4(res, 1.f);
	}
	
		
}
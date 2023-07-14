#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

// naming convention from mesh's Draw() function
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

struct SunLight 
{
	vec3 position;

	float ambientIntensity;
	vec3 ambientColor;

	vec3 diffuse;		// diffuse and specular "intensities" come from the diffuse and specular vectors themselves
	vec3 specular;
	int shininess;
};

uniform SunLight sunlight;
uniform vec3 viewPos;

void main()
{
	// ambient
	vec3 ambient = sunlight.ambientIntensity * sunlight.ambientColor * texture(texture_diffuse1, TexCoords).xyz;

	// diffuse
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(sunlight.position);	// for the sunlight, light's direction is same for all fragments; doesn't depend on fragment position
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * texture(texture_diffuse1, TexCoords).xyz * sunlight.diffuse; 

	// specular
	vec3 reflectDir = reflect(-lightDir, normal);	// lightDir is from origin to light (since it's just the position of the light); need it to be from light to origin, so use negative
	vec3 viewDir = normalize(viewPos - FragPos);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), sunlight.shininess);
	vec3 specular = spec * texture(texture_specular1, TexCoords).xyz * sunlight.specular;

	FragColor = vec4(ambient + diffuse + specular, 1.0);
}
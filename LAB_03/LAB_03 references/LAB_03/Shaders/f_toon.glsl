// Fragment shader: Toon shading
// ================

#version 450 core

in vec3 N;
in vec3 L;
in vec3 E;

out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight{
    vec3 position;
    vec3 color;
    float power;
};

uniform Material material;
uniform PointLight light;

void main() {
	
	vec3 ambient = material.ambient; 
	
    vec3 norm = normalize(N);
    vec3 lightDir = normalize(L);

    float diff = max(dot(norm, lightDir), 0.0);
    float diffuse_factor = smoothstep(0.0, 0.8, diff);
    vec3 diffuse = light.power * light.color * diffuse_factor * material.diffuse;

    vec3 viewDir = normalize(E);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular =  light.power * light.color * spec * material.specular;

		
	float intensity = dot(normalize(L), N);
	
	vec3 shade_buffer = ambient + diffuse;
	vec3 result_buffer;
	
	if (intensity > 0.95){
		result_buffer = vec3(shade_buffer[0],shade_buffer[1],shade_buffer[2]);
	}
	else if (intensity > 0.6){
		result_buffer = vec3(shade_buffer[0] - 0.2, shade_buffer[1] - 0.2, shade_buffer[2] - 0.2);
	}
	else if (intensity > 0.35){
		result_buffer = vec3(shade_buffer[0] - 0.4, shade_buffer[1] - 0.4, shade_buffer[2] - 0.4);
	}
	else{
		result_buffer = vec3(shade_buffer[0] - 0.6, shade_buffer[1] - 0.6, shade_buffer[2] - 0.6);
	}
	
    vec3 result = result_buffer;
    FragColor = vec4(result, 1.0);
}
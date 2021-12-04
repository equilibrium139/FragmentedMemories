#version 330 core

out vec4 color;

struct Material {
    sampler2D diffuse;  
    sampler2D specular;
    sampler2D normal;
    vec3 diffuse_coeff;
    vec3 specular_coeff;
    float shininess;
};

uniform Material material;

in VS_OUT {
    mat3 TBN;
    vec3 fragViewPos;
    vec2 texCoords;
} fs_in;

void main()
{
    vec3 L = -vec3(0, 0, -1);

    vec3 normal = texture(material.normal, fs_in.texCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(fs_in.TBN * normal);

    vec3 diffuse_texel = texture(material.diffuse, fs_in.texCoords).rgb;
    vec3 specular_texel = texture(material.specular, fs_in.texCoords).rgb;

    vec3 ambient = vec3(0.2, 0.2, 0.2) * diffuse_texel;

    float diffuse_strength = max(dot(normal, L), 0.0);
    vec3 diffuse = diffuse_strength * material.diffuse_coeff * diffuse_texel;

    vec3 R = reflect(L, normal);
    vec3 V = normalize(-fs_in.fragViewPos);
    float specular_strength = max(dot(R, V), 0.0);
    vec3 specular = pow(specular_strength, 32) * material.specular_coeff * specular_texel;

    color = vec4(ambient + diffuse + specular, 1.0);
}
#version 450 core


in vec3 fragment_position;
in vec3 normal;
in vec2 texture_coordinates; 
in vec3 view_direction;
in vec3 light_direction;
in mat3 tbn_matrix;

out vec4 frag_color;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};

struct Light {
    vec3 position;           //not for directional
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float theta_min;         //spotlight
    float theta_taper_start; //spotlight
    float constant;          //point light
    float linear;            //point light
    float quadratic;         //point light
    mat4 shadow_matrix;
    sampler2D depth_texture;
    samplerCube cube_map;
};

uniform Material material;
uniform vec3 camera_position;
uniform Light point_light;
uniform samplerCube skybox;
uniform float far_plane;            //Needed for point shadow calculations
const float pi  = 3.14159265;
const float ksh = 16.0;


vec3 calc_directional_light(Light light, vec3 normal,
                            vec3 view_direction)
{
    // These are position independent lights - eg. the sun.
    vec3 light_direction = normalize(-light.direction);
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0),
                     material.shininess);
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,
                                                texture_coordinates));
    vec3 diffuse = light.diffuse * diff * \
                   vec3(texture(material.texture_diffuse1,
                        texture_coordinates));
    vec3 specular = light.specular * spec * \
                    vec3(texture(material.texture_specular1,
                                 texture_coordinates));
    return (ambient + diffuse + specular);
}


float shadow_calculation(Light light, vec4 shadow_position)
{
    /* This division performed automagically by the vert shader normally */
    vec3 projected_coordinates = shadow_position.xyz / shadow_position.w;
    /* light-space position is in [-1,1], gotta move to [0,1] */
    projected_coordinates = projected_coordinates * 0.5 + vec3(0.5);
    float shadow;
    if (projected_coordinates.z > 1.0){
        shadow = 0.0;
    } else {
        float closest_depth = texture(light.depth_texture,
                                      projected_coordinates.xy).r;
        float bias_max = 0.01, bias_min = 0.001;
        float bias = max(bias_max * (1.0 - dot(normal, light_direction)),
                         bias_min);
        float current_depth = projected_coordinates.z;
        shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
    }
    return shadow;
}


float shadow_calculation_upsampled(Light light, vec4 shadow_position)
{
    vec3 projected_coordinates = shadow_position.xyz / shadow_position.w;
    projected_coordinates = projected_coordinates * 0.5 + vec3(0.5);
    float shadow = 0.f;
    vec2 texel_size = 1.0 / textureSize(light.depth_texture, 0);
    float bias_max = 0.01, bias_min = 0.001;
    float bias = max(bias_max * (1.0 - dot(normal, light_direction)),
                     bias_min);
    vec2 texel_corner = floor(projected_coordinates.xy / texel_size) \
                              / textureSize(light.depth_texture, 0);
    vec2 distance = projected_coordinates.xy - texel_corner;
    float upsample_factor = 3;
    vec2 indices = floor(upsample_factor * distance - 1);
    float upsampled_depth = texture(light.depth_texture,
                                    projected_coordinates.xy \
                                    + indices * texel_size).r;
    if (projected_coordinates.z > 1.0){
        shadow = 0.0;
    } else {
        shadow = projected_coordinates.z - bias > upsampled_depth ? 1.0 : 0.0;
    }
    return shadow;
}


float shadow_calculation_pcf(Light light, vec4 shadow_position)
{
    vec3 projected_coordinates = shadow_position.xyz / shadow_position.w;
    projected_coordinates = projected_coordinates * 0.5 + vec3(0.5);
    float shadow = 0.f;
    vec2 texel_size = 1.0 / textureSize(light.depth_texture, 0);
    float bias_max = 0.01, bias_min = 0.001;
    float bias = max(bias_max * (1.0 - dot(normal, light_direction)),
                     bias_min);
    for (int x = -1; x <= 1; x++){
        for (int y = -1; y <= 1; y++){
            float pcf_depth = texture(light.depth_texture,
                                      projected_coordinates.xy \
                                      + vec2(x,y) * texel_size).r;
            shadow += projected_coordinates.z - bias > pcf_depth ? 1.0 : 0.0;
        }
    }
    return shadow /= 9.0;
}


float shadow_calculation_cube(Light light, vec3 normal)
{
    vec3 frag_to_light = fragment_position - light.position;
    float closest_depth = texture(light.cube_map, frag_to_light).r;
    closest_depth *= far_plane;
    float bias_max = 0.05, bias_min = 0.005;
    float bias = max(bias_max * (1.0 - dot(normal, light_direction)),
                     bias_min);
    float current_depth = length(frag_to_light);
    float shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
    return shadow;
}


vec3 calc_point_light(Light light, vec3 fragment_position, vec3 normal)
{
    float shininess_correction;
    vec3 material_diffuse = texture(material.texture_diffuse1,
                                    texture_coordinates).rgb;
    /* Tutorial has -light_direction here. 
     * Drawing seems to indicate it should just be light_direction.
     */
    vec3 material_normal = texture(material.texture_normal1,
                                  texture_coordinates).rgb;
    material_normal = normalize(2.0 * material_normal - vec3(1.0));

    vec3 ambient = light.ambient * material_diffuse;
    float diff = max(dot(material_normal, light_direction), 0.0);
    vec3 diffuse = light.diffuse * diff * material_diffuse;
    vec3 avg_direction = normalize(light_direction + view_direction);
    /* Blinn-Phong mode */
    shininess_correction = (8.0 + ksh) / (8.0 * pi);
    /* Phong */
    //shininess_correction = (2.0 + ksh) / (2.0 * pi);
    float spec = pow(max(dot(material_normal, avg_direction), 0.0),
                     material.shininess);
    spec *= shininess_correction;
    vec3 specular = light.specular * spec * \
                    texture(material.texture_specular1,
                            texture_coordinates).rgb;
    float distance = length(light.position - fragment_position);
    float attenuation = 1.0 / (light.constant + light.linear * distance + \
                               light.quadratic * (distance * distance));
    float shadow = shadow_calculation_cube(light, material_normal);
    ambient *= attenuation;
    diffuse *= (1.0 - shadow) * attenuation;
    specular *= (1.0 - shadow) * attenuation;
    return (ambient + diffuse + specular);
}


vec3 calc_spotlight(Light light, vec3 normal, vec3 fragment_position,
                     vec3 view_direction)
{
    float attenuation;
    vec3 frag_to_light_direction = normalize(light.position - \
                                             fragment_position);
    float theta = dot(-frag_to_light_direction, normalize(light.direction));
    if (theta > light.theta_min){
        theta = theta-light.theta_min;
        attenuation = min(theta / (light.theta_taper_start - light.theta_min),
                          1.0);
    } else{
        attenuation = 0.0;
    }
    vec3 reflect_direction = reflect(-frag_to_light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0),
                     material.shininess);
    vec3 specular = light.specular * spec * \
                    vec3(texture(material.texture_specular1,
                                 texture_coordinates));
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,
                                                texture_coordinates));
    float diff = max(dot(normal, frag_to_light_direction), 0.0);
    vec3 diffuse = light.diffuse * diff * \
                   vec3(texture(material.texture_diffuse1,
                        texture_coordinates));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}


vec4 calc_reflections(vec3 normal, vec3 fragment_position)
{
    vec3 frag_to_cam = camera_position - fragment_position;
    vec3 reflect_direction = reflect(frag_to_cam, normal);
    return texture(skybox, -reflect_direction);
}


vec4 calc_refraction(vec3 normal, vec3 fragment_position,
                     float refractive_index)
{
    float ratio = 1.0 / refractive_index;
    vec3 cam_to_frag = fragment_position - camera_position;
    vec3 refract_direction = refract(cam_to_frag, normal, ratio);
    return texture(skybox, refract_direction);
}


void main(){
    vec3 normalized_normal = normalize(normal);
    vec3 light_effects;
    vec3 material_normal = texture(material.texture_normal1,
                                  texture_coordinates).rgb;
    material_normal = normalize(2.0 * material_normal - vec3(1.0));
    float refractive_index = 1.52;

    light_effects = calc_point_light(point_light, fragment_position,
                                     normalized_normal);
	frag_color = texture(material.texture_diffuse1, texture_coordinates);
    frag_color *= calc_reflections(normalized_normal, fragment_position);
    //frag_color = calc_refraction(normalized_normal, fragment_position,
    //                             refractive_index);
    frag_color *= vec4(light_effects, 1.0);
}

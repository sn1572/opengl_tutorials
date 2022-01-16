#version 450 core


layout (location = 0) in vec3 in_position;

out vec3 texture_coordinates;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    mat4 untranslated_view;
    vec4 position;

    untranslated_view = mat4(mat3(view));
    texture_coordinates = in_position;
    position = projection * untranslated_view * vec4(in_position, 1.0);
    gl_Position = position.xyww;
}

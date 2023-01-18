#version 450

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_normal;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;

layout(binding = 0) uniform sampler2D in_texture;

vec3 remapNormal(vec3 normal) {
    return normal / 2.0 + vec3(0.5);
}

void main() {
    out_color = in_color * vec3(texture(in_texture, in_uv));
    out_normal = remapNormal(normalize(in_normal));
}
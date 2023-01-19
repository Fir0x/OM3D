#version 450

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_normal;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;

layout(binding = 0) uniform sampler2D in_texture;
layout(binding = 1) uniform sampler2D in_normal_texture;

vec3 remapNormal(vec3 normal) {
    return normal / 2.0 + vec3(0.5);
}

void main() {
    #ifdef NORMAL_MAPPED
    const vec3 normal_map = unpack_normal_map(texture(in_normal_texture, in_uv).xy);
    const vec3 normal = normal_map.x * in_tangent +
                        normal_map.y * in_bitangent +
                        normal_map.z * in_normal;
    out_normal = remapNormal(normal);
#else
    out_normal = remapNormal(in_normal);
#endif

#ifdef TEXTURED
    out_color = texture(in_texture, in_uv).xyz;
#else
    out_color = in_color;
#endif
}
#version 450

#include "utils.glsl"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

layout(location = 0) out vec3 out_color;

layout(binding = 0) uniform sampler2D in_color;
layout(binding = 1) uniform sampler2D in_normal;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};

uniform uint light_index;

vec3 remapNormal(vec3 normal) {
    return normalize(normal * 2.0 - vec3(1.0));
}

vec3 unproject(vec2 uv, float depth, mat4 inv_viewproj) {
    const vec3 ndc = vec3(uv * 2.0 - vec2(1.0), depth);
    const vec4 p = inv_viewproj * vec4(ndc, 1.0);
    return p.xyz / p.w;
}

void main() {
    vec3 color = texelFetch(in_color, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 normal = remapNormal(texelFetch(in_normal, ivec2(gl_FragCoord.xy), 0).xyz);
    vec2 uv = gl_FragCoord.xy / vec2(WINDOW_WIDTH, WINDOW_HEIGHT);

    vec3 position = unproject(uv, gl_FragCoord.z, inverse(frame.camera.view_proj));
    PointLight light = point_lights[light_index];

    vec3 pos2light = light.position - position;
    vec3 light_dir = normalize(pos2light);
    float light_dist = length(pos2light);

    float light_factor = max(0.0, dot(light_dir, normal));
    color *= light.color * light_factor;

    out_color = color;
}
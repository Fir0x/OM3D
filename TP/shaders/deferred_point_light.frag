#version 450

#include "utils.glsl"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

layout(location = 0) out vec3 out_color;

layout(binding = 0) uniform sampler2D in_color;
layout(binding = 1) uniform sampler2D in_normal;
layout(binding = 2) uniform sampler2D in_depth;

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
    ivec2 coord = ivec2(gl_FragCoord.xy);
    vec3 color = texelFetch(in_color, coord, 0).xyz;
    
    vec3 normal = remapNormal(texelFetch(in_normal, coord, 0).xyz);
    vec2 uv = coord / vec2(WINDOW_WIDTH, WINDOW_HEIGHT);
    float depth = texelFetch(in_depth, coord, 0).x;

    vec3 position = unproject(uv, depth, inverse(frame.camera.view_proj));
    PointLight light = point_lights[light_index];

    const vec3 pos2light = light.position - position;
    const float light_dist = length(pos2light);
    const vec3 light_dir = pos2light / light_dist;

    const float NoL = max(0.0, dot(light_dir, normal));
    const float att = max(0.0, attenuation(light_dist, light.radius));

    const vec3 acc = light.color * (NoL * att);

    out_color = color * acc;
}
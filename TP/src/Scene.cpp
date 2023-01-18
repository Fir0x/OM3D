#include "Scene.h"

#include <glad/glad.h>

#include <TypedBuffer.h>

#include <shader_structs.h>

#include <iostream>

namespace OM3D {

Scene::Scene() {
}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::add_object(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

static bool isInBound(const glm::vec3& object_dir, const glm::vec3& normal, float radius) {
    float distance = glm::dot(object_dir, normal);
    return distance > -radius;
}

static bool cullObject(const BoundingSphere& bounding_sphere, const glm::vec3& camera_position, const Frustum& frustum) {
    glm::vec3 object_dir = bounding_sphere.center - camera_position;

    {
        if (!isInBound(object_dir, frustum._top_normal, bounding_sphere.radius))
            return true;
    }

    {
        if (!isInBound(object_dir, frustum._bottom_normal, bounding_sphere.radius))
            return true;
    }

    {
        if (!isInBound(object_dir, frustum._right_normal, bounding_sphere.radius))
            return true;
    }

    {
        if (!isInBound(object_dir, frustum._left_normal, bounding_sphere.radius))
            return true;
    }

    {
        if (!isInBound(object_dir, frustum._near_normal, bounding_sphere.radius))
            return true;
    }

    return false;
}

void Scene::render(const Camera& camera) const {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr, std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for(size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {
                light.position(),
                light.radius(),
                light.color(),
                0.0f
            };
        }
    }
    light_buffer.bind(BufferUsage::Storage, 1);

    // Render every object
    glm::vec3 camera_position = camera.position();
    Frustum frustum = camera.build_frustum();
    // Render every object
    for(const SceneObject& obj : _objects) {
        BoundingSphere transformedBoundingSphere = obj.boundingSphere();
        transformedBoundingSphere.center = glm::vec4(transformedBoundingSphere.center, 1.0f) * obj.transform();
        if (cullObject(transformedBoundingSphere, camera_position, frustum))
            continue;

        obj.render();
    }
}

void Scene::deferred_lighting(const Camera& camera, const Material& sun_material,
                              const Material& point_light_material) const {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    
    buffer.bind(BufferUsage::Uniform, 0);
    sun_material.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

}

#include "Scene.h"

#include <glad/glad.h>

#include <TypedBuffer.h>
#include <ObjectBatcher.h>

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

const SceneObject& Scene::get_object(int index) const {
    return _objects[index];
}

void Scene::set_point_light_volume(std::shared_ptr<StaticMesh> volume) {
    _point_light_volume = volume;
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
    ObjectBatcher batcher;
    // Render every object
    for(const SceneObject& obj : _objects) {
        auto model = obj.transform();

        BoundingSphere transformedBoundingSphere = obj.boundingSphere();
        transformedBoundingSphere.center = model * glm::vec4(transformedBoundingSphere.center, 1.0f);

        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(model[0])); // Basis vector X
        scale.y = glm::length(glm::vec3(model[1])); // Basis vector Y
        scale.z = glm::length(glm::vec3(model[2])); // Basis vector Z
        float scale_factor = scale.x;
        if (scale.y > scale_factor)
            scale_factor *= scale.y;
        if (scale.z > scale_factor)
            scale_factor *= scale.z;
        transformedBoundingSphere.radius *= scale_factor;

        if (cullObject(transformedBoundingSphere, camera_position, frustum))
            continue;

        batcher.add_object(obj);
    }

    batcher.render();
}

void Scene::deferred_lighting(const Camera& camera, const Material& sun_material,
                              Material& point_light_material) const {
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

    sun_material.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    point_light_material.bind();

    for (u32 i = 0; i < _point_lights.size(); i++) {
        glm::mat4 light_transform = glm::mat4(1.0);
        light_transform = glm::translate(light_transform, _point_lights[i].position());
        light_transform = glm::scale(light_transform, glm::vec3(_point_lights[i].radius()));

        TypedBuffer<glm::mat4> model_buffer(&light_transform, 1);
        model_buffer.bind(BufferUsage::Storage, 2);
        
        point_light_material.set_uniform("light_index", i);
        _point_light_volume->draw();
    }
}

void Scene::debug_light_volumes(const Camera& camera, const Material& debug_material) const {
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

    debug_material.bind();

    for (u32 i = 0; i < _point_lights.size(); i++) {
        glm::mat4 light_transform = glm::mat4(1.0);
        light_transform = glm::translate(light_transform, _point_lights[i].position());
        light_transform = glm::scale(light_transform, glm::vec3(_point_lights[i].radius()));

        TypedBuffer<glm::mat4> model_buffer(&light_transform, 1);
        model_buffer.bind(BufferUsage::Storage, 2);

        _point_light_volume->draw();
    }
}

}

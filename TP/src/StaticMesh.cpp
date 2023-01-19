#include "StaticMesh.h"

#include <glad/glad.h>

#include <glm/glm.hpp>

namespace OM3D {

StaticMesh::StaticMesh(const MeshData& data) :
    _vertex_buffer(data.vertices),
    _index_buffer(data.indices) {

    if (_vertex_buffer.element_count() == 0)
        return;

    const glm::vec3& first_pos = data.vertices[0].position;
    float min_x = first_pos.x;
    float max_x = first_pos.x;
    float min_y = first_pos.y;
    float max_y = first_pos.y;
    float min_z = first_pos.z;
    float max_z = first_pos.z;

    for (size_t i = 1; i < data.vertices.size(); i++) {
        const glm::vec3& vertex_pos = data.vertices[i].position;
        if (vertex_pos.x < min_x)
            min_x = vertex_pos.x;
        else if (vertex_pos.x > max_x)
            max_x = vertex_pos.x;

        if (vertex_pos.y < min_y)
            min_y = vertex_pos.y;
        else if (vertex_pos.y > max_y)
            max_y = vertex_pos.y;

        if (vertex_pos.z < min_z)
            min_z = vertex_pos.z;
        else if (vertex_pos.z > max_z)
            max_z = vertex_pos.z;
    }

    _bounding_sphere.center = { (max_x + min_x) / 2, (max_y + min_y) / 2, (max_z + min_z) / 2 };

    float max_dist = 0;
    for (const auto& vertex : data.vertices) {
        const glm::vec3& vertex_pos = vertex.position;
        float dist = glm::length(vertex_pos - _bounding_sphere.center);
        if (dist > max_dist)
            max_dist = dist;
    }

    _bounding_sphere.radius = max_dist;
}

void StaticMesh::draw() const {
    draw(1);
}

BoundingSphere StaticMesh::boundingSphere() const {
    return _bounding_sphere;
}

void StaticMesh::draw(size_t count) const {
    _vertex_buffer.bind(BufferUsage::Attribute);
    _index_buffer.bind(BufferUsage::Index);

    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
    // Vertex normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    // Vertex uv
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
    // Tangent / bitangent sign
    glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
    // Vertex color
    glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(12 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glDrawElementsInstanced(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr, count);
}


}

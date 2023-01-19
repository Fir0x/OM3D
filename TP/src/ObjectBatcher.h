#pragma once

#include <unordered_map>

#include "SceneObject.h"

namespace OM3D
{
    class ObjectBatcher {
    public:
        struct Batch {
            std::vector<glm::mat4> models;
            std::shared_ptr<StaticMesh> mesh;
        };

        void add_object(const SceneObject& object);
        void render() const;

    private:
        std::unordered_map<std::shared_ptr<Material>, Batch> _batches;
    };
} // namespace OM3D

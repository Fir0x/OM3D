#include "ObjectBatcher.h"

#include <iostream>

namespace OM3D
{
   void ObjectBatcher::add_object(const SceneObject& object) {
        auto material = object.get_material();
        auto batch = _batches.find(material);
        if (batch == _batches.end()) {
            Batch new_batch;
            new_batch.models.push_back(object.transform());
            new_batch.mesh = object.get_mesh();
            _batches.insert({ material, new_batch });
        }
        else
            batch->second.models.push_back(object.transform());
   }

   void ObjectBatcher::render() const {
        for (auto pair : _batches) {
            auto material = pair.first;
            Batch batch = pair.second;

            TypedBuffer<glm::mat4> model_buffer(batch.models.data(), batch.models.size());
            model_buffer.bind(BufferUsage::Storage, 2);

            material->bind();
            batch.mesh->draw(batch.models.size());
        }
   }
} // namespace OM3D

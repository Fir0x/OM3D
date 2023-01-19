#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>
#include <Framebuffer.h>

#include <vector>
#include <memory>

namespace OM3D {

class Scene : NonMovable {

    public:
        Scene();

        static Result<std::unique_ptr<Scene>> from_gltf(const std::string& file_name);

        void render(const Camera& camera) const;
        void deferred_lighting(const Camera& camera, const Material& sun_material,
                               Material& point_light_material) const;

        void add_object(SceneObject obj);
        void add_object(PointLight obj);
        const SceneObject& get_object(int index) const;
        void set_point_light_volume(std::shared_ptr<StaticMesh> volume);

    private:
        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;
        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
        std::shared_ptr<StaticMesh> _point_light_volume;
};

}

#endif // SCENE_H

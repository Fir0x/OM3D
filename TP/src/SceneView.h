#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <Scene.h>
#include <Camera.h>

namespace OM3D {

class SceneView {
    public:
        SceneView(const Scene* scene = nullptr);

        Camera& camera();
        const Camera& camera() const;

        void render() const;
        void deferred_lighting(const Material& sun_material,
                               Material& point_light_material) const;
        void debug_light_volumes(const Material& debug_material) const;
        void tiled_deferred_lighting(std::shared_ptr<Program> debug_cluster_program,
                                     const glm::uvec2& screen_size, Texture* g_color,
                                     Texture* g_normal, Texture* g_depth, Texture* out_texture) const;

    private:
        const Scene* _scene = nullptr;
        Camera _camera;

};

}

#endif // SCENEVIEW_H

#include "SceneView.h"

namespace OM3D {

SceneView::SceneView(const Scene* scene) : _scene(scene) {
}

Camera& SceneView::camera() {
    return _camera;
}

const Camera& SceneView::camera() const {
    return _camera;
}

void SceneView::render() const {
    if(_scene) {
        _scene->render(_camera);
    }
}

void SceneView::deferred_lighting(const Material& sun_material,
                                  Material& point_light_material) const {
    if (_scene) {
        _scene->deferred_lighting(_camera, sun_material, point_light_material);
    }
}

void SceneView::debug_light_volumes(const Material& debug_material) const {
    if (_scene) {
        _scene->debug_light_volumes(_camera, debug_material);
    }
}

void SceneView::debug_light_cluster(std::shared_ptr<Program> debug_cluster_program,
                                    const glm::uvec2& screen_size, Texture* g_color,
                                    Texture* g_normal, Texture* g_depth, Texture* out_texture) const
{
    if (_scene) {
        _scene->debug_light_cluster(_camera, debug_cluster_program, screen_size, g_color, g_normal, g_depth, out_texture);
    }
}

}

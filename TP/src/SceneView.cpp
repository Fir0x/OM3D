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

void SceneView::render_deferred(const Framebuffer& g_buffer, const Framebuffer& main_buffer,
                                const Material& deferred_lit) const {
    if (_scene) {
        _scene->render_deferred(_camera, g_buffer, main_buffer, deferred_lit);
    }
}

}

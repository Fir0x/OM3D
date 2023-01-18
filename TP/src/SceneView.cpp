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
                                  const Material& point_light_material) const {
    if (_scene) {
        _scene->deferred_lighting(_camera, sun_material, point_light_material);
    }
}

}

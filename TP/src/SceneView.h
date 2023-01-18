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
        void render_deferred(const Framebuffer& g_buffer,
                             const Framebuffer& main_buffer,
                             const Material& deferred_lit) const;

    private:
        const Scene* _scene = nullptr;
        Camera _camera;

};

}

#endif // SCENEVIEW_H

#ifndef MATERIAL_H
#define MATERIAL_H

#include <Program.h>
#include <Texture.h>

#include <memory>
#include <vector>

namespace OM3D {

enum class BlendMode {
    None,
    Alpha,
    Add,
};

enum class DepthTestMode {
    Standard,
    Reversed,
    Equal,
    Always,
    None
};

enum class CullMode {
    Backface,
    Frontface,
};

class Material {

    public:
        Material();

        void set_program(std::shared_ptr<Program> prog);
        void set_blend_mode(BlendMode blend);
        void set_depth_test_mode(DepthTestMode depth);
        void set_texture(u32 slot, std::shared_ptr<Texture> tex);
        void set_write_depth(bool write);
        void set_cull_mode(CullMode face);

        template<typename... Args>
        void set_uniform(Args&&... args) {
            _program->set_uniform(FWD(args)...);
        }


        void bind() const;

        static std::shared_ptr<Material> empty_material();
        static Material textured_material();
        static Material textured_normal_mapped_material();
        static Material deferred_light(const std::string& vert, const std::string& frag);


    private:
        std::shared_ptr<Program> _program;
        std::vector<std::pair<u32, std::shared_ptr<Texture>>> _textures;

        BlendMode _blend_mode = BlendMode::None;
        DepthTestMode _depth_test_mode = DepthTestMode::Standard;
        bool _write_depth = true;
        CullMode _cull_mode = CullMode::Backface;

};

}

#endif // MATERIAL_H

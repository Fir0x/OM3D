#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include <graphics.h>
#include <SceneView.h>
#include <Texture.h>
#include <Framebuffer.h>
#include <ImGuiRenderer.h>

#include <imgui/imgui.h>


using namespace OM3D;

static float delta_time = 0.0f;
const glm::uvec2 window_size(1600, 900);


void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_delta_time() {
    static double time = 0.0;
    const double new_time = program_time();
    delta_time = float(new_time - time);
    time = new_time;
}

void process_inputs(GLFWwindow* window, Camera& camera) {
    static glm::dvec2 mouse_pos;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    {
        glm::vec3 movement = {};
        if(glfwGetKey(window, 'W') == GLFW_PRESS) {
            movement += camera.forward();
        }
        if(glfwGetKey(window, 'S') == GLFW_PRESS) {
            movement -= camera.forward();
        }
        if(glfwGetKey(window, 'D') == GLFW_PRESS) {
            movement += camera.right();
        }
        if(glfwGetKey(window, 'A') == GLFW_PRESS) {
            movement -= camera.right();
        }
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            movement += glm::vec3(0, 1, 0);
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            movement -= glm::vec3(0, 1, 0);
        }

        float speed = 10.0f;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            speed *= 10.0f;
        }

        if(movement.length() > 0.0f) {
            const glm::vec3 new_pos = camera.position() + movement * delta_time * speed;
            camera.set_view(glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * 0.01f;
        if(delta.length() > 0.0f) {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x, glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(camera.position(), camera.position() + (glm::mat3(rot) * camera.forward()), (glm::mat3(rot) * camera.up())));
        }

    }

    mouse_pos = new_mouse_pos;
}

std::shared_ptr<StaticMesh> create_point_light_volume() {
    auto scene = std::make_unique<Scene>();

    // Retrieve sphere mesh
    auto result = Scene::from_gltf(std::string(data_path) + "sphere.glb");
    ALWAYS_ASSERT(result.is_ok, "Unable to load sphere scene");
    scene = std::move(result.value);
    return scene->get_object(0).get_mesh();
}

std::unique_ptr<Scene> create_default_scene(std::shared_ptr<StaticMesh> point_light_volume) {
    auto scene = std::make_unique<Scene>();

    // Load default cube model
    auto result = Scene::from_gltf(std::string(data_path) + "cube.glb");
    ALWAYS_ASSERT(result.is_ok, "Unable to load default scene");
    scene = std::move(result.value);

    scene->set_point_light_volume(point_light_volume);

    // Add lights
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
        light.set_color(glm::vec3(0.0f, 10.0f, 0.0f));
        light.set_radius(100.0f);
        scene->add_object(std::move(light));
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(10.0f, 0.0f, 0.0f));
        light.set_radius(50.0f);
        scene->add_object(std::move(light));
    }

    return scene;
}


int main(int, char**) {
    DEBUG_ASSERT([] { std::cout << "Debug asserts enabled" << std::endl; return true; }());

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(window_size.x, window_size.y, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    ImGuiRenderer imgui(window);

    std::shared_ptr<StaticMesh> point_light_volume = create_point_light_volume();
    std::unique_ptr<Scene> scene = create_default_scene(point_light_volume);
    SceneView scene_view(scene.get());

    auto tonemap_program = Program::from_file("tonemap.comp");

    Texture lit(window_size, ImageFormat::RGBA16_FLOAT);
    Texture color(window_size, ImageFormat::RGBA8_UNORM);
    Framebuffer tonemap_framebuffer(nullptr, std::array{&color});

    auto deferred_color = std::make_shared<Texture>(window_size, ImageFormat::RGBA8_sRGB);
    auto deferred_normal = std::make_shared<Texture>(window_size, ImageFormat::RGBA8_UNORM);
    auto depth = std::make_shared<Texture>(window_size, ImageFormat::Depth32_FLOAT);

    Framebuffer g_buffer(depth.get(), std::array{deferred_color.get(), deferred_normal.get()});
    Framebuffer main_framebuffer(depth.get(), std::array{&lit});

    Texture* debug_refs[] = { &lit, deferred_color.get(), deferred_normal.get(), depth.get() };

    Material deferred_sun = Material::deferred_light("screen.vert", "deferred_sun.frag");
    deferred_sun.set_texture(0u, deferred_color);
    deferred_sun.set_texture(1u, deferred_normal);
    deferred_sun.set_depth_test_mode(DepthTestMode::Reversed);
    deferred_sun.set_write_depth(false);

    Material deferred_point_light = Material::deferred_light("basic.vert", "deferred_point_light.frag");
    deferred_point_light.set_texture(0u, deferred_color);
    deferred_point_light.set_texture(1u, deferred_normal);
    deferred_point_light.set_texture(2u, depth);
    deferred_point_light.set_blend_mode(BlendMode::Add);
    deferred_point_light.set_depth_test_mode(DepthTestMode::Reversed);
    deferred_point_light.set_write_depth(false);
    deferred_point_light.set_cull_mode(CullMode::Frontface);

    int debug_mode = 0;
    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        update_delta_time();

        if(const auto& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            process_inputs(window, scene_view.camera());
        }

        {
            g_buffer.bind();
            scene_view.render();
        }

        {
            main_framebuffer.bind(true, false);
            scene_view.deferred_lighting(deferred_sun, deferred_point_light);
        }

        // Apply a tonemap in compute shader
        {
            tonemap_program->bind();
            debug_refs[debug_mode]->bind(0);
            color.bind_as_image(1, AccessType::WriteOnly);
            glDispatchCompute(align_up_to(window_size.x, 8), align_up_to(window_size.y, 8), 1);
        }

        // Blit tonemap result to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        tonemap_framebuffer.blit();

        // GUI
        imgui.start();
        {
            char buffer[1024] = {};
            if(ImGui::InputText("Load scene", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                auto result = Scene::from_gltf(buffer);
                if(!result.is_ok) {
                    std::cerr << "Unable to load scene (" << buffer << ")" << std::endl;
                } else {
                    scene = std::move(result.value);
                    scene->set_point_light_volume(point_light_volume);
                    scene_view = SceneView(scene.get());
                }
            }

            if (ImGui::BeginTable("Debug_table", 1))
            {
                static std::vector<std::string> radio_names = { "None", "Color", "Normal", "Depth" };
                for (int i = 0; i < radio_names.size(); i++)
                {
                    ImGui::TableNextColumn();
                    ImGui::PushID(i);
                    ImGui::RadioButton(radio_names[i].c_str(), &debug_mode, i);
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        imgui.finish();

        glfwSwapBuffers(window);
    }

    scene = nullptr; // destroy scene and child OpenGL objects
}

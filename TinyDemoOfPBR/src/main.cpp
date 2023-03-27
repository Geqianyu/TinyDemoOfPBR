
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Log/Log.h"
#include "Shader/Shader.h"
#include "Camera/Camera.h"
#include "Sphere/Sphere.h"

void framebuffer_size_callback(GLFWwindow* _window, int _width, int _height);
void process_inport(GLFWwindow* _window);
void mouse_callback(GLFWwindow* _window, double _x_position, double _y_position);
void scroll_callback(GLFWwindow* _window, double _x_offset, double _y_offset);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 0.0f, 20.0f));
float last_x = SCR_WIDTH / 2.0;
float last_y = SCR_HEIGHT / 2.0;
bool first_mouse = true;

float delta_time = 0.0f;
float last_frame = 0.0f;

int main()
{
    Log::init();
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tiny Demo Of PBR", nullptr, nullptr);
    if (window == nullptr)
    {
        GQY_OPENGL_LOAD_MODEL_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        GQY_OPENGL_LOAD_MODEL_ERROR("Failed to initialize GLAD");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    Shader pbr_shader("asset/shader/PBR.vert.glsl", "asset/shader/PBR.frag.glsl");
    Shader equirectangular_to_cubemap_shader("asset/shader/cubeMap.vert.glsl", "asset/shader/equirectangularToCubeMap.frag.glsl");
    Shader irradiance_shader("asset/shader/cubeMap.vert.glsl", "asset/shader/irradianceConvolution.frag.glsl");
    Shader prefilter_shader("asset/shader/cubeMap.vert.glsl", "asset/shader/prefilter.frag.glsl");
    Shader brdf_shader("asset/shader/brdf.vert.glsl", "asset/shader/brdf.frag.glsl");
    Shader background_shader("asset/shader/background.vert.glsl", "asset/shader/background.frag.glsl");

    pbr_shader.use();
    pbr_shader.set_int("ibl.irradiance_map", 0);
    pbr_shader.set_int("ibl.prefilter_map", 1);
    pbr_shader.set_int("ibl.brdf_LUT_map", 2);

    background_shader.use();
    background_shader.set_int("environment_map", 0);

    Sphere iron_sphere;
    iron_sphere.set_albedo("asset/texture/rusted_iron/albedo.png");
    iron_sphere.set_ao("asset/texture/rusted_iron/ao.png");
    iron_sphere.set_metallic("asset/texture/rusted_iron/metallic.png");
    iron_sphere.set_normal("asset/texture/rusted_iron/normal.png");
    iron_sphere.set_roughness("asset/texture/rusted_iron/roughness.png");

    Sphere gold_sphere;
    gold_sphere.set_albedo("asset/texture/gold/albedo.png");
    gold_sphere.set_ao("asset/texture/gold/ao.png");
    gold_sphere.set_metallic("asset/texture/gold/metallic.png");
    gold_sphere.set_normal("asset/texture/gold/normal.png");
    gold_sphere.set_roughness("asset/texture/gold/roughness.png");

    Sphere grass_sphere;
    grass_sphere.set_albedo("asset/texture/grass/albedo.png");
    grass_sphere.set_ao("asset/texture/grass/ao.png");
    grass_sphere.set_metallic("asset/texture/grass/metallic.png");
    grass_sphere.set_normal("asset/texture/grass/normal.png");
    grass_sphere.set_roughness("asset/texture/grass/roughness.png");

    Sphere plastic_sphere;
    plastic_sphere.set_albedo("asset/texture/plastic/albedo.png");
    plastic_sphere.set_ao("asset/texture/plastic/ao.png");
    plastic_sphere.set_metallic("asset/texture/plastic/metallic.png");
    plastic_sphere.set_normal("asset/texture/plastic/normal.png");
    plastic_sphere.set_roughness("asset/texture/plastic/roughness.png");

    Sphere wall_sphere;
    wall_sphere.set_albedo("asset/texture/wall/albedo.png");
    wall_sphere.set_ao("asset/texture/wall/ao.png");
    wall_sphere.set_metallic("asset/texture/wall/metallic.png");
    wall_sphere.set_normal("asset/texture/wall/normal.png");
    wall_sphere.set_roughness("asset/texture/wall/roughness.png");

    HDR_image_texture hdr_image_texture("asset/hdr/newport_loft.hdr", "hdr");
    hdr_image_texture.generate_environment_map(equirectangular_to_cubemap_shader);
    hdr_image_texture.generate_irradiance_map(irradiance_shader);
    hdr_image_texture.generate_prefilter_map(prefilter_shader);
    hdr_image_texture.generate_LUT(brdf_shader);

    glm::mat4 projection = glm::perspective(glm::radians(camera.get_zoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    pbr_shader.use();
    pbr_shader.set_matrix4("projection", projection);
    background_shader.use();
    background_shader.set_matrix4("projection", projection);

    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    while (!glfwWindowShouldClose(window))
    {
        float current_frame = static_cast<float>(glfwGetTime());
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        process_inport(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        pbr_shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, hdr_image_texture.get_irradiance_map());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, hdr_image_texture.get_prefilter_map());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, hdr_image_texture.get_brdf_LUT_map());
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.get_view_matrix();
        pbr_shader.set_matrix4("view", view);
        pbr_shader.set_vec3("camera_position", camera.get_position());

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-5.0, 0.0, 2.0));
        pbr_shader.set_matrix4("model", model);
        iron_sphere.draw(pbr_shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0, 0.0, 2.0));
        pbr_shader.set_matrix4("model", model);
        gold_sphere.draw(pbr_shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0, 0.0, 2.0));
        pbr_shader.set_matrix4("model", model);
        grass_sphere.draw(pbr_shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0, 0.0, 2.0));
        pbr_shader.set_matrix4("model", model);
        plastic_sphere.draw(pbr_shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0, 0.0, 2.0));
        pbr_shader.set_matrix4("model", model);
        wall_sphere.draw(pbr_shader);

        background_shader.use();
        background_shader.set_matrix4("view", view);
        hdr_image_texture.draw_background(background_shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

void process_inport(GLFWwindow* _window)
{
    if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(_window, true);
    }
    if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.process_keyboard(Camera_move_direction::Forward, delta_time);
    }
    if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.process_keyboard(Camera_move_direction::Backward, delta_time);
    }
    if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.process_keyboard(Camera_move_direction::Left, delta_time);
    }
    if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.process_keyboard(Camera_move_direction::Right, delta_time);
    }
}

void framebuffer_size_callback(GLFWwindow* _window, int _width, int _height)
{
    glViewport(0, 0, _width, _height);
}

void mouse_callback(GLFWwindow* _window, double _x_position, double _y_position)
{
    float xpos = static_cast<float>(_x_position);
    float ypos = static_cast<float>(_y_position);

    if (first_mouse)
    {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - last_x;
    float yoffset = last_y - ypos;

    last_x = xpos;
    last_y = ypos;

    camera.process_mouse_movement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* _window, double _x_offset, double _y_offset)
{
    camera.process_mouse_scroll(static_cast<float>(_y_offset));
}

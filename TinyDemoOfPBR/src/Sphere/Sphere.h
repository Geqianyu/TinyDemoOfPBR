
#ifndef GQY_SPHERE_H
#define GQY_SPHERE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <memory>

#include "Shader/Shader.h"
#include "Texture/Texture.h"

class Sphere
{
public:
    Sphere(const float& _radius = 1.0f);
    ~Sphere()
    {
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        glDeleteVertexArrays(1, &m_VAO);
    }

    inline void set_albedo(const std::string& _path)
    {
        m_albedo = std::make_shared<Image_texture>(_path, "material.albedo_map");
    }

    inline void set_normal(const std::string& _path)
    {
        m_normal = std::make_shared<Image_texture>(_path, "material.normal_map");
    }

    inline void set_ao(const std::string& _path)
    {
        m_ao = std::make_shared<Image_texture>(_path, "material.ao_map");
    }

    inline void set_metallic(const std::string& _path)
    {
        m_metallic = std::make_shared<Image_texture>(_path, "material.metallic_map");
    }

    inline void set_roughness(const std::string& _path)
    {
        m_roughness = std::make_shared<Image_texture>(_path, "material.roughness_map");
    }

    void draw(const Shader& _shader);

private:
    GLuint m_VAO, m_VBO, m_EBO;
    GLuint m_index_count;
    std::shared_ptr<Texture> m_albedo;
    std::shared_ptr<Texture> m_normal;
    std::shared_ptr<Texture> m_ao;
    std::shared_ptr<Texture> m_metallic;
    std::shared_ptr<Texture> m_roughness;
};

#endif // !GQY_SPHERE_H

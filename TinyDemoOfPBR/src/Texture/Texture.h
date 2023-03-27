
#ifndef GQY_TEXTURE_H
#define GQY_TEXTURE_H

#include <string>

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "Shader/Shader.h"

class Texture
{
public:
    Texture() = default;
    Texture(const std::string& _type) : m_type(_type) { }
    ~Texture() = default;

    inline void set_type(const std::string& _type)
    {
        m_type = _type;
    }

    virtual void bind_texture(const Shader& _shader, int _texture_index) = 0;

protected:
    std::string m_type;
};

class Image_texture : public Texture
{
public:
    Image_texture(const std::string& _path, const std::string& _type);
    ~Image_texture();

    void set_texture(const std::string& _path);

    virtual void bind_texture(const Shader& _shader, int _texture_index) override;

private:
    GLuint m_id;
};

class HDR_image_texture : public Texture
{
public:
    HDR_image_texture(const std::string& _path, const std::string& _type);
    ~HDR_image_texture();

    void set_texture(const std::string& _path);
    void generate_environment_map(const Shader& _shader);
    void generate_irradiance_map(const Shader& _shader);
    void generate_prefilter_map(const Shader& _shader);
    void generate_LUT(const Shader& _shader);

    virtual void bind_texture(const Shader& _shader, int _texture_index) override;
    void draw_background(const Shader& _shader);

    inline GLuint get_irradiance_map() const
    {
        return m_irradiance_map;
    }

    inline GLuint get_brdf_LUT_map() const
    {
        return m_brdf_LUT_texture;
    }

    inline GLuint get_prefilter_map() const
    {
        return m_prefilter_map;
    }

private:
    void render_cube();
    void render_quad();

private:
    GLuint m_cube_VAO, m_cube_VBO;
    GLuint m_quad_VAO, m_quad_VBO;
    GLuint m_capture_FBO, m_capture_RBO;
    GLuint m_hdr_texture;
    GLuint m_env_cube_map;
    GLuint m_irradiance_map;
    GLuint m_prefilter_map;
    GLuint m_brdf_LUT_texture;
};

#endif // !GQY_TEXTURE_H

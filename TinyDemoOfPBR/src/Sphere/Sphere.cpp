
#include "Sphere.h"

Sphere::Sphere(const float& _radius)
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<GLuint> indices;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = std::asinf(1.0f) * 2.0f;
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            float x_segment = static_cast<float>(x) / static_cast<float>(X_SEGMENTS);
            float y_segment = static_cast<float>(y) / static_cast<float>(Y_SEGMENTS);
            float x_position = std::cos(x_segment * 2.0f * PI) * std::sin(y_segment * PI);
            float y_position = std::cos(y_segment * PI);
            float z_position = std::sin(x_segment * 2.0f * PI) * std::sin(y_segment * PI);

            positions.push_back(glm::vec3(x_position, y_position, z_position));
            uvs.push_back(glm::vec2(x_segment, y_segment));
            normals.push_back(glm::vec3(x_position, y_position, z_position));
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }

    std::vector<float> data;
    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
        if (uvs.size() > 0)
        {
            data.push_back(uvs[i].x);
            data.push_back(uvs[i].y);
        }
    }
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    m_index_count = static_cast<GLuint>(indices.size());
}

void Sphere::draw(const Shader& _shader)
{
    m_albedo->bind_texture(_shader, 4);
    m_normal->bind_texture(_shader, 5);
    m_ao->bind_texture(_shader, 6);
    m_metallic->bind_texture(_shader, 7);
    m_roughness->bind_texture(_shader, 8);
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLE_STRIP, m_index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

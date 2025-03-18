#ifndef CUBE_MESH_H
#define CUBE_MESH_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

class CubeMesh {
public:
    CubeMesh();
    ~CubeMesh();
    void draw(std::vector<unsigned int> textures);
    void updateVertices(const std::vector<float>& newVertices);
    std::vector<float> getVertices();
    int findClosestFace(float mouseX, float mouseY);
    std::vector<int> findConnectedVertices(float mouseX, float mouseY, const glm::mat4& view, const glm::mat4& projection);
    std::vector<int> getFaceIndices(int faceIndex);

private:
    GLuint VAO, VBO, EBO, vertex_count;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

#endif

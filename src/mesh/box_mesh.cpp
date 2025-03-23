#include "box_mesh.h"
#include <vector>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>

BoxMesh::BoxMesh() {
    vertices = {
        // Posición                      // Color         // TexCoords
        // Cara frontal
       -0.6f, -0.25f,  0.15f,  1,0,0,    0,0,
        0.6f, -0.25f,  0.15f,  0,1,0,    1,0,
        0.6f,  0.25f,  0.15f,  0,0,1,    1,1,
       -0.6f,  0.25f,  0.15f,  1,1,0,    0,1,
    
        // Cara derecha
        0.6f, -0.25f,  0.15f,  1,0,1,    0,0,
        0.6f, -0.25f, -0.15f,  0,1,1,    1,0,
        0.6f,  0.25f, -0.15f,  1,1,1,    1,1,
        0.6f,  0.25f,  0.15f,  0.5,0.5,0.5, 0,1,
    
        // Cara superior
       -0.6f,  0.25f,  0.15f,  1,0.5,0,    0,0,
        0.6f,  0.25f,  0.15f,  0.5,1,0.5,  1,0,
        0.6f,  0.25f, -0.15f,  0.5,0.5,1,  1,1,
       -0.6f,  0.25f, -0.15f,  1,0,0.5,    0,1
    };
    
    

    indices = {
        // Cara frontal
        0, 1, 2, 2, 3, 0,
        // Cara derecha
        4, 5, 6, 6, 7, 4,
        // Cara superior
        8, 9, 10, 10, 11, 8
    };

    vertex_count = indices.size(); // Se asegura que vertex_count tenga el tamaño correcto

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Posición
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Coordenadas de textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void BoxMesh::updateVertices(const std::vector<float>& newVertices) {
    if (newVertices.size() != vertices.size()) {
        std::cerr << "Error: nuevo tamaño de vértices incorrecto" << std::endl;
        return;
    }
    vertices = newVertices;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
}

void BoxMesh::draw(std::vector<unsigned int> textures) {
    glBindVertexArray(VAO);

    for (int i = 0; i < 6; ++i) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
    }

    glBindVertexArray(0);
}

std::vector<float> BoxMesh::getVertices() {
    return vertices;
}

int BoxMesh::findClosestFace(float mouseX, float mouseY) {
    int closestFace = -1;
    float minDistance = FLT_MAX;

    for (int i = 0; i < indices.size(); i += 6) {
        glm::vec3 center(0.0f);
        for (int j = 0; j < 6; j += 2) {
            int index = indices[i + j];
            center += glm::vec3(vertices[index * 6], vertices[index * 6 + 1], vertices[index * 6 + 2]);
        }
        center /= 3.0f;

        float distance = glm::length2(glm::vec3(mouseX, mouseY, 0.0f) - glm::vec3(center.x, center.y, 0.0f));
        if (distance < minDistance) {
            minDistance = distance;
            closestFace = i / 6;
        }
    }

    return closestFace;
}

std::vector<int> BoxMesh::findConnectedVertices(float mouseX, float mouseY, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    std::vector<int> connectedVertices;
    float minDistance = FLT_MAX;
    glm::vec3 targetVertex(0.0f);

    for (int i = 0; i < vertices.size(); i += 8) {
        glm::vec4 vertexPos(vertices[i], vertices[i + 1], vertices[i + 2], 1.0f);
        glm::vec4 projected = projection * view * model * vertexPos;

        // Convertir a coordenadas de pantalla normalizadas (-1 a 1)
        projected /= projected.w;
        float screenX = projected.x;
        float screenY = projected.y;

        float distance = glm::length2(glm::vec2(mouseX, mouseY) - glm::vec2(screenX, screenY));

        if (distance < minDistance) {
            minDistance = distance;
            targetVertex = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
        }
    }

    // Umbral de selección (ajustable para mayor precisión)
    float selectionThreshold = 0.01f;

    for (int i = 0; i < vertices.size(); i += 8) {
        glm::vec3 vertexPos(vertices[i], vertices[i + 1], vertices[i + 2]);

        if (glm::length2(targetVertex - vertexPos) < selectionThreshold) {
            connectedVertices.push_back(i);
        }
    }

    return connectedVertices;
}

std::vector<int> BoxMesh::getFaceIndices(int faceIndex) {
    std::vector<int> faceIndices;
    if (faceIndex < 0 || faceIndex * 6 >= indices.size()) return faceIndices;

    for (int i = 0; i < 6; i++) {
        faceIndices.push_back(indices[faceIndex * 6 + i]);
    }
    
    return faceIndices;
}

std::pair<int, float> BoxMesh::findClosestVertexIndex(float mouseX, float mouseY, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    int closestIndex = -1;
    float minDistance = FLT_MAX;

    for (int i = 0; i < vertices.size(); i += 8) {
        glm::vec4 vertexPos(vertices[i], vertices[i + 1], vertices[i + 2], 1.0f);
        glm::vec4 projected = projection * view * model * vertexPos;
        projected /= projected.w;

        float dx = mouseX - projected.x;
        float dy = mouseY - projected.y;
        float distSq = dx * dx + dy * dy;

        if (distSq < minDistance) {
            minDistance = distSq;
            closestIndex = i;
        }
    }

    return { closestIndex, minDistance };
}


BoxMesh::~BoxMesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

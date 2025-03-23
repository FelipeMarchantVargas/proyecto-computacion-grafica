#include "config.h"
#include "mesh/cube_mesh.h"
#include "mesh/box_mesh.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../dependencies/stb_image.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath);
unsigned int make_module(const std::string& filepath, unsigned int module_type);
unsigned int loadTexture(const char* path);

CubeMesh* cube = nullptr; 
BoxMesh* box = nullptr;
bool isDragging = false;
double lastX = 0, lastY = 0;
int selectedFace  = -1;
std::vector<int> selectedVertices;
enum class SelectedMesh { NONE, CUBE, BOX };
SelectedMesh selectedMesh = SelectedMesh::NONE;


int main(){
    if(!glfwInit()){ 
        std::cerr << "GLFW couldn't start" << std::endl;
        return -1;
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);


    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Proyecto Felipe Marchant", monitor, NULL);

    if (!window) { 
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { 
        std::cerr << "Error al inicializar GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);

    cube = new CubeMesh();
    box = new BoxMesh();
    unsigned int shader = make_shader(
        "../src/shaders/vertex.txt",
        "../src/shaders/fragment.txt"
    );

    // Cargar diferentes texturas para cada cara del cubo
    std::vector<unsigned int> textures(3);
    textures[0] = loadTexture("../textures/cube_textures/dice-six-faces-one.png");
    textures[1] = loadTexture("../textures/cube_textures/dice-six-faces-three.png");
    textures[2] = loadTexture("../textures/cube_textures/dice-six-faces-five.png");
    // Cargar diferentes texturas para cada cara del box
    std::vector<unsigned int> texturesBox(3);
    texturesBox[0] = loadTexture("../textures/cube_textures/dice-six-faces-one.png");
    texturesBox[1] = loadTexture("../textures/cube_textures/dice-six-faces-three.png");
    texturesBox[2] = loadTexture("../textures/cube_textures/dice-six-faces-five.png");

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 modelCube = glm::mat4(1.0f);
        glm::mat4 modelBox = glm::translate(glm::mat4(1.0f), glm::vec3(1.8f, 0.0f, -0.6f)); // glm::vec3(2.0f, 0.0f, -0.7f)

        glm::mat4 view = glm::lookAt(glm::vec3(3.5f, 2.5f, 3.5f), 
                             glm::vec3(1.0f, 0.0f, 0.0f), 
                             glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)mode->width / (float)mode->height, 0.1f, 100.0f);

        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Dibujar el cubo (más protagonista, primero)
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(modelCube));
        cube->draw(textures);

        // Dibujar el cartucho al lado derecho
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(modelBox));
        box->draw(texturesBox);


        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glDeleteProgram(shader);
    delete cube;
    glfwTerminate();
    return 0;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float normalizedX = (2.0f * xpos) / width - 1.0f;
        float normalizedY = 1.0f - (2.0f * ypos) / height;

        glm::mat4 modelCube = glm::mat4(1.0f);
        glm::mat4 modelBox = glm::translate(glm::mat4(1.0f), glm::vec3(1.8f, 0.0f, -0.6f)); // glm::vec3(2.0f, 0.0f, -0.7f)

        glm::mat4 view = glm::lookAt(glm::vec3(3.5f, 2.5f, 3.5f), 
                             glm::vec3(1.0f, 0.0f, 0.0f), 
                             glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)mode->width / (float)mode->height, 0.1f, 100.0f);

        auto [cubeIndex, cubeDist] = cube->findClosestVertexIndex(normalizedX, normalizedY, modelCube, view, projection);
        auto [boxIndex, boxDist] = box->findClosestVertexIndex(normalizedX, normalizedY, modelBox, view, projection);
        
        float threshold = 0.05f;
        
        if (cubeDist < boxDist && cubeDist < threshold) {
            selectedVertices = cube->findConnectedVertices(normalizedX, normalizedY, modelCube, view, projection);
            selectedMesh = SelectedMesh::CUBE;
            isDragging = true;
        }
        else if (boxDist < threshold) {
            selectedVertices = box->findConnectedVertices(normalizedX, normalizedY, modelBox, view, projection);
            selectedMesh = SelectedMesh::BOX;
            isDragging = true;
        }
        

        lastX = xpos;
        lastY = ypos;
    } 
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        isDragging = false;
        selectedVertices.clear();
        selectedMesh = SelectedMesh::NONE;
    }
}


void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging && !selectedVertices.empty()) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float factor = 500.0f;
        float movementX = (float)(xpos - lastX) / factor;
        float movementY = -(float)(ypos - lastY) / factor;

        std::vector<float> newVertices;

        if (selectedMesh == SelectedMesh::CUBE) {
            newVertices = cube->getVertices();
        } else if (selectedMesh == SelectedMesh::BOX) {
            newVertices = box->getVertices();
        }

        for (int i : selectedVertices) {
            newVertices[i] += movementX;
            newVertices[i + 1] += movementY;
        }

        if (selectedMesh == SelectedMesh::CUBE) {
            cube->updateVertices(newVertices);
        } else if (selectedMesh == SelectedMesh::BOX) {
            box->updateVertices(newVertices);
        }

        lastX = xpos;
        lastY = ypos;
    }
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action == GLFW_PRESS) {
        std::cout << "Tecla presionada: " << key << std::endl;

        static std::vector<float> newVertices = {
        // Posición           // Color       // Coordenadas de textura
        // Cara frontal
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,  
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,  
    
        // Cara derecha
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f,  
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  1.0f, 0.0f,  
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  
         0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,  0.0f, 1.0f,  
    
        // Cara superior
        -0.5f,  0.5f,  0.5f,  1.0f, 0.5f, 0.0f,  0.0f, 0.0f,  
         0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.5f,  1.0f, 0.0f,  
         0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 1.0f,  1.0f, 1.0f,  
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.5f,  0.0f, 1.0f   
        };

        // Modificar TODOS los vértices
        for (size_t i = 0; i < newVertices.size(); i += 8) { // Avanzar de 6 en 6 (x, y, z, r, g, b)
            if (key == GLFW_KEY_UP) {
                newVertices[i + 1] += 0.05f; // Mueve en eje Y
            } 
            if (key == GLFW_KEY_DOWN) {
                newVertices[i + 1] -= 0.05f; // Mueve en eje Y
            } 
            if (key == GLFW_KEY_RIGHT) {
                newVertices[i] += 0.05f; // Mueve en eje X
            }
            if (key == GLFW_KEY_LEFT) {
                newVertices[i] -= 0.05f; // Mueve en eje X
            }
        }

        if(cube){
            cube->updateVertices(newVertices);
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

unsigned int make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath){
    std::vector<unsigned int> modules;
    modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));
    modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));

    unsigned int shader = glCreateProgram();
    for(unsigned int shaderModule : modules){
        glAttachShader(shader, shaderModule);
    }
    glLinkProgram(shader);

    int success;
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if(!success){
        char errorLog[1024];
        glGetProgramInfoLog(shader, 1024, NULL, errorLog);
        std::cout << "Error de Linkeo de Shaders:\n" << errorLog << std::endl;
    }

    for(unsigned int shaderModule : modules){
        glDeleteShader(shaderModule);
    }

    return shader;
}

unsigned int make_module(const std::string& filepath, unsigned int module_type){

    std::ifstream file;
    std::stringstream bufferedLines;
    std::string line;

    file.open(filepath);
    while(std::getline(file, line)){
        bufferedLines << line << "\n";
    }

    std::string shaderSource = bufferedLines.str();
    const char* shaderSrc = shaderSource.c_str();
    bufferedLines.str("");
    file.close();

    unsigned int shaderModule = glCreateShader(module_type);
    glShaderSource(shaderModule, 1, &shaderSrc, NULL);
    glCompileShader(shaderModule);

    int success;
    glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
    if(!success){
        char errorLog[1024];
        glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
        std::cout << "Error de compilación de Shader Module:\n" << errorLog << std::endl;
    }

    return shaderModule;
}

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Configurar parámetros de la textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Error al cargar la textura: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

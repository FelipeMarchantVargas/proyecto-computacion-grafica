#include "config.h"
#include "cube_mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath);
unsigned int make_module(const std::string& filepath, unsigned int module_type);

CubeMesh* cube = nullptr; 
bool isDragging = false;
double lastX = 0, lastY = 0;
int selectedFace  = -1; // -1 indica que no hay selección
std::vector<int> selectedVertices;

int main(){
    if(!glfwInit()){ 
        std::cerr << "GLFW couldn't start" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "My window", NULL, NULL);
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
    unsigned int shader = make_shader(
        "../src/shaders/vertex.txt",
        "../src/shaders/fragment.txt"
    );

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        cube->draw();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glDeleteProgram(shader);
    delete cube;
    glfwTerminate();
    return 0;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float normalizedX = (2.0f * xpos) / width - 1.0f;
        float normalizedY = 1.0f - (2.0f * ypos) / height;

        selectedVertices = cube->findConnectedVertices(normalizedX, normalizedY);
        isDragging = !selectedVertices.empty(); // Si hay vértices, comenzamos a arrastrar
        lastX = xpos;
        lastY = ypos;
    } 
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        isDragging = false;
        selectedVertices.clear(); // Limpiar selección al soltar el botón
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging && !selectedVertices.empty()) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Reducir la sensibilidad del movimiento dividiendo entre un factor
        float factor = 500.0f; // Ajusta este valor para suavizar más o menos
        float movementX = (float)(xpos - lastX) / factor;
        float movementY = -(float)(ypos - lastY) / factor;

        std::vector<float> newVertices = cube->getVertices();

        for (int i : selectedVertices) {
            newVertices[i] += movementX;
            newVertices[i + 1] += movementY;
        }

        cube->updateVertices(newVertices);
        lastX = xpos;
        lastY = ypos;
    }
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action == GLFW_PRESS) {
        std::cout << "Tecla presionada: " << key << std::endl;

        static std::vector<float> newVertices = {
            // Posición          // Color
            // Cara frontal
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  

            // Cara derecha
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  
             0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,  

            // Cara superior
            -0.5f,  0.5f,  0.5f,  1.0f, 0.5f, 0.0f,  
             0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.5f,  
             0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 1.0f,  
            -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.5f   
        };

        // Modificar TODOS los vértices
        for (size_t i = 0; i < newVertices.size(); i += 6) { // Avanzar de 6 en 6 (x, y, z, r, g, b)
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
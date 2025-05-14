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
bool loadAllVideos();
void updateVideoFrames();
void changeVideo(int newIndex);
void changeBoxImage(int newIndex, std::vector<unsigned int>& texturesBox);

std::vector<unsigned int> texturesBox(3);
CubeMesh* cube = nullptr; 
BoxMesh* box = nullptr;
bool isDragging = false;
double lastX = 0, lastY = 0;
int selectedFace  = -1;
std::vector<int> selectedVertices;
enum class SelectedMesh { NONE, CUBE, BOX };
SelectedMesh selectedMesh = SelectedMesh::NONE;
static glm::vec3 cameraPos = glm::vec3(3.5f, 2.5f, 3.5f);
static glm::vec3 cameraLookAt = glm::vec3(1.0f, 0.0f, 0.0f);
static float cameraSpeed = 0.1f;
static cv::VideoCapture captures[3];
static GLuint textureIDs[3];
double lastTime = glfwGetTime();
int frameCount = 0;
bool useTextures = false;
// Rutas de video disponibles
std::vector<std::string> videoPaths = {
    "../textures/videos/a.mp4",
    "../textures/videos/b.mp4",
    "../textures/videos/c.mp4",
    "../textures/videos/d.mp4",
    "../textures/videos/e.mp4",
};

// Rutas de texturas para el cartucho
std::vector<std::string> boxImagePaths = {
    "../textures/other/Super_Mario_64_Cartucho.jpg",
    "../textures/other/Zelda oot.png",
    "../textures/other/Starfox.png",
    "../textures/other/Mario kart.png",
    "../textures/other/Donkey kong.png",
};

int currentVideoIndex = 0;
int currentImageIndex = 0;

std::vector<glm::vec2> texCoordsSet1 = {
    // Cara frontal (4 vértices)
    {0.25f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.25f, 1.0f},
    // Cara derecha (4 vértices)
    {0.0f, 0.0f}, {0.228f, 0.0f}, {0.228f, 0.69f}, {0.0f, 0.69f},
    // Cara superior (4 vértices)
    {0.0f, 0.7f}, {0.228f, 0.7f}, {0.228f, 1.0f}, {0.0f, 1.0f}
};
std::vector<glm::vec2> texCoordsSet2 = {
    // Cara frontal (4 vértices)
    {0.25f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.25f, 1.0f},
    // Cara derecha (4 vértices)
    {0.0f, 0.28f}, {0.25f, 0.28f}, {0.25f, 1.0f}, {0.0f, 1.0f},
    // Cara superior (4 vértices)
    {0.0f, 0.0f}, {0.25f, 0.0f}, {0.25f, 0.28f}, {0.0f, 0.28f}
};
std::vector<glm::vec2> texCoordsSet3 = {
    // Cara frontal (4 vértices)
    {0.31f, 0.16f}, {0.999f, 0.16f}, {0.999f, 0.95f}, {0.31f, 0.95f},
    // Cara derecha (4 vértices)
    {0.05f, 0.45f}, {0.28f, 0.45f}, {0.28f, 0.97f}, {0.05f, 0.97f},
    // Cara superior (4 vértices)
    {0.05f, 0.18f}, {0.27f, 0.18f}, {0.27f, 0.4f}, {0.05f, 0.4f}
};
std::vector<glm::vec2> texCoordsSet4 = {
    // Cara frontal (4 vértices)
    {0.05f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.96f}, {0.05f, 0.96f},
    // Cara derecha (4 vértices)
    {0.0f, 0.02f}, {0.33f, 0.02f}, {0.33f, 0.14f}, {0.0f, 0.14f},
    // Cara superior (4 vértices)
    {0.8f, 0.69f}, {0.95f, 0.69f}, {0.95f, 0.965f}, {0.8f, 0.965f}
};
std::vector<glm::vec2> texCoordsSet5 = {
    // Cara frontal (4 vértices)
    {0.235f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.235f, 1.0f},
    // Cara derecha (4 vértices)
    {0.0f, 0.0f}, {0.23f, 0.0f}, {0.23f, 0.66f}, {0.0f, 0.66f},
    // Cara superior (4 vértices)
    {0.0f, 0.66f}, {0.23f, 0.66f}, {0.23f, 1.0f}, {0.0f, 1.0f}
};





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
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

    cube = new CubeMesh();
    box = new BoxMesh();
    unsigned int shader = make_shader(
        "../src/shaders/vertex.txt",
        "../src/shaders/fragment.txt"
    );

    if (!loadAllVideos()) {
        std::cerr << "Error al cargar videos." << std::endl;
        glfwTerminate();
        return -1;
    }
    // Cargar texturas para el cubo

    glGenTextures(3, textureIDs);
for (int i = 0; i < 3; i++) {
    glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


    // Cargar diferentes texturas para cada cara del box
    texturesBox[0] = loadTexture("../textures/other/Super_Mario_64_Cartucho.jpg");
    texturesBox[1] = loadTexture("../textures/other/8b8888.png");
    texturesBox[2] = loadTexture("../textures/other/8b8888.png");

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (useTextures) {
            updateVideoFrames();
        }
        

        glm::mat4 modelCube = glm::mat4(1.0f);
        glm::mat4 modelBox = glm::translate(glm::mat4(1.0f), glm::vec3(1.8f, 0.0f, -0.6f)); // glm::vec3(2.0f, 0.0f, -0.7f)
        
        // -------------- USO DE LA POSICIÓN DE LA CÁMARA --------------
        glm::mat4 view = glm::lookAt(
            cameraPos, 
            cameraLookAt, 
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)mode->width / (float)mode->height, 0.1f, 100.0f);

        glUseProgram(shader);
        glUniform1i(glGetUniformLocation(shader, "useTextures"), useTextures);

        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        // Cubo
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(modelCube));
        cube->draw({textureIDs[0], textureIDs[1], textureIDs[2]}, useTextures);
        
        // Paralelepípedo
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(modelBox));
        box->draw(texturesBox, useTextures);


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

        glm::mat4 view = glm::lookAt(
                            cameraPos, 
                            cameraLookAt, 
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

        /* static std::vector<float> newVertices = {
        // Posición           // Color       // Coordenadas de textura
        // Cara frontal
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.25f, 0.0f,  
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.25f, 1.0f,  
    
        // Cara derecha (Timer)
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.228f, 0.0f,  
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.228f, 0.69f,  
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.69f,  
    
        // Cara superior
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.7f,  
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.228f, 0.7f,  
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.228f, 1.0f,  
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f   
        }; */

        std::vector<float> newVertices = cube->getVertices();

        // Modificar TODOS los vértices
        for (size_t i = 0; i < newVertices.size(); i += 8) { // Avanzar de 8 en 8 (x, y, z, r, g, b, u, v)
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
        // ------------ Controles WASD + Q/E para la cámara ------------
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_W) {
            cameraPos.z -= cameraSpeed;  // W = acercarse en -Z 
        }
        if (key == GLFW_KEY_S) {
            cameraPos.z += cameraSpeed;  // S = alejarse en +Z
        }
        if (key == GLFW_KEY_A) {
            cameraPos.x -= cameraSpeed;  // A = moverse a la izquierda en -X
        }
        if (key == GLFW_KEY_D) {
            cameraPos.x += cameraSpeed;  // D = moverse a la derecha en +X
        }
        if (key == GLFW_KEY_Q) {
            cameraPos.y += cameraSpeed;  // Q = moverse hacia arriba en +Y
        }
        if (key == GLFW_KEY_E) {
            cameraPos.y -= cameraSpeed;  // E = moverse hacia abajo en -Y
        }
    }

    // ------------ Controles IKJL + U/O para la cámara ------------
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_I) {
            cameraLookAt.z -= cameraSpeed;  // I = acercarse en -Z
        }
        if (key == GLFW_KEY_K) {
            cameraLookAt.z += cameraSpeed;  // K = alejarse en +Z
        }
        if (key == GLFW_KEY_J) {
            cameraLookAt.x -= cameraSpeed;  // J = moverse a la izquierda en -X
        }
        if (key == GLFW_KEY_L) {
            cameraLookAt.x += cameraSpeed;  // L = moverse a la derecha en +X
        }
        if (key == GLFW_KEY_U) {
            cameraLookAt.y += cameraSpeed;  // U = moverse hacia arriba en +Y
        }
        if (key == GLFW_KEY_O) {
            cameraLookAt.y -= cameraSpeed;  // O = moverse hacia abajo en -Y
        }
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        useTextures = !useTextures;  // Alternar entre usar texturas o no
    }    

    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
        int nextVideo = (currentVideoIndex + 1) % videoPaths.size();
        int nextImage = (currentImageIndex + 1) % boxImagePaths.size();
    
        changeVideo(nextVideo);
        changeBoxImage(nextImage, texturesBox);

        switch (nextVideo)
        {
        case 0:
            cube->updateTexCoords(texCoordsSet1);
            break;
        
        case 1:
            cube->updateTexCoords(texCoordsSet2);
            break;
        case 2: 
            cube->updateTexCoords(texCoordsSet3);
            break;
        case 3:
            cube->updateTexCoords(texCoordsSet4);
            break;
        case 4: 
            cube->updateTexCoords(texCoordsSet5);
            break;

        default:
            break;
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
    stbi_set_flip_vertically_on_load(true);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Configuración de parámetros de la textura
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

bool loadAllVideos() {
    // Rutas de ejemplo, cambia según tus archivos reales
    const char* videoPath = "../textures/videos/a.mp4";

    for (int i = 0; i < 3; i++) {
        captures[i].open(videoPath);
        if(!captures[i].isOpened()){
            std::cerr << "No se pudo abrir el video " << videoPath << std::endl;
            return false;
        }
    }
    return true;
}

void updateVideoFrames() {
    for (int i = 0; i < 3; i++) {
        cv::Mat frame;
        if (captures[i].read(frame)) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

            cv::flip(frame, frame, 0);
            
            glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            captures[i].set(cv::CAP_PROP_POS_FRAMES, 0); // Reiniciar video si termina
        }
    }
}

void changeVideo(int newIndex) {
    if (newIndex < 0 || newIndex >= videoPaths.size()) return;

    currentVideoIndex = newIndex;

    for (int i = 0; i < 3; ++i) {
        captures[i].release(); // Libera anterior
        captures[i].open(videoPaths[newIndex]);

        if (!captures[i].isOpened()) {
            std::cerr << "No se pudo abrir el video: " << videoPaths[newIndex] << std::endl;
        }
    }
}

void changeBoxImage(int newIndex, std::vector<unsigned int>& texturesBox) {
    if (newIndex < 0 || newIndex >= boxImagePaths.size()) return;

    currentImageIndex = newIndex;

    // Cambiar solo una cara del cartucho (por ejemplo, la frontal)
    glDeleteTextures(1, &texturesBox[0]);
    texturesBox[0] = loadTexture(boxImagePaths[newIndex].c_str());
}

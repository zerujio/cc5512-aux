#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <chrono>

// RAII: Resource Acquisition Is Initialization
class  WindowManager {
public:
    WindowManager(int width, int height, std::string name) 
    : m_window_ptr(glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr))
    {
        if (!m_window_ptr)
            throw std::runtime_error("window creation failed");
    }

    ~WindowManager() {
        if (m_window_ptr)
            glfwDestroyWindow(m_window_ptr);
    }

    GLFWwindow* ptr() const {
        return m_window_ptr;
    }

private:
    GLFWwindow* m_window_ptr;
};

struct PosColorVertex {
    float x;
    float y;
    float z;
    uint32_t abgr;
};

void errorCallback(int code, const char* message) {
    std::cerr << "[GLFW ERROR] CODE=" << code << " MSG='" << message << "'\n";
}

bgfx::ShaderHandle loadShader(const char* filename) {
    std::ifstream file {filename};

    if (!file.is_open()) {
        throw std::runtime_error(std::string("Failed to open file:") + filename);
    }

    std::stringstream sstream;
    sstream << file.rdbuf();

    auto string = sstream.str();

    const bgfx::Memory* mem = bgfx::copy(string.data(), string.size() + 1);
    bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, filename);

    return handle;
}

int main() {

    constexpr unsigned int window_width {1024};
    constexpr unsigned int window_height {768};

    // glfw init
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << "\n";
        return 1;
    }

    // glfw window creation
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    { // window scope start
    WindowManager window {window_width, window_height, "Hello bgfx!"};
    //GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Hello bgfx!", nullptr, nullptr);
    //if (!window) {
    //    std::cerr << "Window creation failed!" << "\n";
    //    return 2;
    //}

    // bgfx init
    bgfx::renderFrame();    // previene la creacion de un rendering thread

    bgfx::Init bgfx_init_args {};

    // platform data: info sobre la ventana
    auto& pdata = bgfx_init_args.platformData;

    pdata.ndt = glfwGetX11Display();
    if (!bgfx_init_args.platformData.ndt) {
        std::cerr << "could not retrieve display info from GLFW.\n";
        return 3;
    }

    pdata.nwh = (void*) (uintptr_t) glfwGetX11Window(window.ptr());
    if (!bgfx_init_args.platformData.nwh) {
        std::cerr << "could not retrieve window handle from GLFW.\n";
        return 4;
    }

    // resolucion de renderizado
    auto& resolution = bgfx_init_args.resolution;
    resolution.width = window_width;
    resolution.height = window_height;
    resolution.reset = BGFX_RESET_VSYNC;

    // inicializacion del renderer
    bgfx::init(bgfx_init_args);

    // activa el texto de debug
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    constexpr int view_id {0};

    // crear geometría
    PosColorVertex vertices[] {
        {-1.0f,  1.0f,  1.0f, 0xff000000 },
        { 1.0f,  1.0f,  1.0f, 0xff0000ff },
        {-1.0f, -1.0f,  1.0f, 0xff00ff00 },
        { 1.0f, -1.0f,  1.0f, 0xff00ffff },
        {-1.0f,  1.0f, -1.0f, 0xffff0000 },
        { 1.0f,  1.0f, -1.0f, 0xffff00ff },
        {-1.0f, -1.0f, -1.0f, 0xffffff00 },
        { 1.0f, -1.0f, -1.0f, 0xffffffff },
    };

    bgfx::VertexLayout v_decl {};
    
    // vertex attribute declaration
    v_decl.begin()
          .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
          .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
          .end();

    // vertex buffer handle
    auto vbh = bgfx::createVertexBuffer(bgfx::makeRef(vertices, sizeof(vertices)), v_decl);

    uint16_t indices[] {
        0, 1, 2, // 0
        1, 3, 2,
        4, 6, 5, // 2
        5, 6, 7,
        0, 2, 4, // 4
        4, 2, 6,
        1, 5, 3, // 6
        5, 7, 3,
        0, 4, 1, // 8
        4, 5, 1,
        2, 3, 6, // 10
        6, 3, 7,
    };

    // index buffer handle
    auto ibh = bgfx::createIndexBuffer(bgfx::makeRef(indices, sizeof(indices)));

    // shader
    bgfx::ProgramHandle program = bgfx::createProgram(loadShader("shaders/v_simple.bin"), loadShader("shaders/f_simple.bin"), true);

    // camara, transformaciones
    const bx::Vec3 at = { 0.0f, 0.0f, 0.0f };
    const bx::Vec3 eye = { 0.0f, 0.0f, -10.0f };

    float view_mtx[16];
    bx::mtxLookAt(view_mtx, eye, at);

    float proj_mtx[16];
    bx::mtxProj(proj_mtx, 60.0f, float(window_width)/float(window_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);

    float model_mtx[16];
    bx::Vec3 rotation {0.0f, 0.0f, 0.0f};

    bgfx::setViewClear(view_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x553344FF);
    bgfx::setViewRect(view_id, 0, 0, window_width, window_height);
    bgfx::setState(BGFX_STATE_DEFAULT);

    // loop
    auto ti = std::chrono::steady_clock::now();
    decltype(ti) tf;
    using sec = std::chrono::duration<float>;
    sec delta {0.0f};

    while (!glfwWindowShouldClose(window.ptr())) {

        tf = std::chrono::steady_clock::now();
        delta = tf - ti;
        ti = tf;

        glfwPollEvents();

        bgfx::setViewTransform(view_id, view_mtx, proj_mtx);

        bgfx::dbgTextPrintf(0, 0, 0x0F, "hello bgfx!");

        bgfx::touch(view_id);

        rotation = bx::add(rotation, bx::mul(bx::Vec3(.25f, .5f, 1.0f), delta.count()));
        bx::mtxRotateXYZ(model_mtx, rotation.x, rotation.y, rotation.z);

        bgfx::setTransform(model_mtx);
        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);

        bgfx::submit(view_id, program);

        bgfx::frame();
    }

    bgfx::destroy(program);
    bgfx::destroy(vbh);
    bgfx::destroy(ibh);

    // cerrar bgfx
    bgfx::shutdown();

    // cerrar ventana y glfw
    } // window scope end
    glfwTerminate();

    return 0; 
}
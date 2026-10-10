#include <Corrade/Containers/ArrayView.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Shaders/VertexColorGL.h>

using namespace Magnum;

namespace {

class GlfwApp: public Platform::GlfwApplication {
    public:
    virtual ~GlfwApp() = default;

    explicit GlfwApp(const Arguments& arguments);

    private:
        /* Event handlers are private in GlfwApplication; keep that access. */
        void drawEvent() override;
        void viewportEvent(ViewportEvent& event) override;
        void keyPressEvent(KeyEvent& event) override;

        GL::Mesh _mesh;
        Shaders::VertexColorGL2D _shader;
};

GlfwApp::GlfwApp(const Arguments& arguments):
    Platform::GlfwApplication{arguments, Configuration{}
        .setTitle("Magnum GLFW")
        .setSize({800, 600})
        .addWindowFlags(Configuration::WindowFlag::Resizable)}
{
    using namespace Math::Literals;

    setSwapInterval(1);
    GL::Renderer::setClearColor(0x1b1e24_rgbf);

    struct Vertex {
        Vector2 position;
        Color3 color;
    };
    constexpr Vertex data[]{
        {.position = {-0.5f, -0.5f}, .color = 0xff5555_rgbf},
        {.position = { 0.5f, -0.5f}, .color = 0x55ff88_rgbf},
        {.position = { 0.0f,  0.5f}, .color = 0x5599ff_rgbf}
    };

    GL::Buffer buffer;
    buffer.setData(Containers::arrayView(data));
    _mesh.setCount(Containers::arraySize(data))
        .addVertexBuffer(std::move(buffer), 0,
            Shaders::VertexColorGL2D::Position{},
            Shaders::VertexColorGL2D::Color3{});
}

void GlfwApp::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);
    _shader.draw(_mesh);
    swapBuffers();
    redraw();
}

void GlfwApp::viewportEvent(ViewportEvent& event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});
}

void GlfwApp::keyPressEvent(KeyEvent& event) {
    if(event.key() == Key::Esc) exit();
}

}

MAGNUM_APPLICATION_MAIN(GlfwApp)

#include "GLRenderer.hpp"
#include "Log.hpp"
#include "Matrix.hpp"
#include "Node.hpp"

#include "GLComponentRenderData.hpp"
#include <cstdint>

#include <fmt/format.h>

void GLRenderer::init(uint32_t major, uint32_t minor, const std::string profile) {
#if defined(__WINDOWS__)
    glewInit();
#endif
    GLCHECK;
    glEnable(GL_DEPTH_TEST);
    GLCHECK;
    glDepthFunc(GL_LESS);
    GLCHECK;
    GLShader::major = major;
    GLShader::minor = minor;
    GLShader::profile = profile;
    ComponentRenderData::builder.addSearchPath(fmt::format("{}{}{}0", profile, major, minor));
    ComponentRenderData::builder.addSearchPath(fmt::format("{}{}{}", profile, major, minor));
    ComponentRenderData::builder.addSearchPath(fmt::format("{}{}", profile, major));
    ComponentRenderData::builder.addSearchPath(fmt::format("{}", profile));
    ComponentRenderData::builder.addSearchPath(fmt::format("opengl", profile));
}

void GLRenderer::shutdown() {
    ComponentRenderData::shaderCache.clear();
}

void GLRenderer::draw(Scene* scene) {
    GLCHECK;
    glViewport(0, 0, scene->width, scene->height);
    GFXLOG("glViewport", 0, 0, scene->width, scene->height);
    GLCHECK;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.1f, 0.0f, 1.0f);
    GLCHECK;

    queue.clear();

    Matrix transform;
    transform.setPosition(-scene->camera->position);
    enqueueComponents(transform, *scene->root, scene);
    GLCHECK;

    drawComponents();
    GLCHECK;
}

void GLRenderer::enqueueComponents(const Matrix& transform, Node& node, Scene* scene) {
    if (!node.visible)
        return;

    Matrix& mat = node.transform;
    mat = transform;
    mat *= Matrix::position(node.position.x, node.position.y, node.position.z);
    mat *= node.rotation;
    mat *= Matrix::scale(node.scale.x, node.scale.y, node.scale.z);

    if (auto renderable = node.renderable()) {
        auto Z = (mat.v[11] - scene->near) / (scene->far - scene->near);
        if (Z < 1.0f && Z > 0.0f) {
            for (auto& component : *renderable->components.write()) {
                if (!component.rendererData) {
                    component.rendererData = std::make_shared<ComponentRenderData>();
                }
                auto crd = reinterpret_cast<ComponentRenderData*>(component.rendererData.get());
                crd->transform = mat;
                if (!crd->update(*renderable, component)) {
                    continue;
                }
                float priority = component.material->key() & (~uint32_t{} >> 4);
                priority += Z;
                if (component.material->isTransparent)
                    priority = -priority;
                crd->priority = priority;
                queue.push_back(crd);
            }
        }
    }

    for (auto& child : *node.children.read())
        enqueueComponents(mat, *child, scene);
}

void GLRenderer::drawComponents() {
    std::sort(queue.begin(), queue.end(), [](auto a, auto b){return a->priority < b->priority;});
    for (auto cmp : queue)
        cmp->draw();
}

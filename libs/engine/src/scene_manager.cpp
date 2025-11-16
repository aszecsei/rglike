#include "engine/scene_manager.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace engine {

void SceneManager::set_scene(std::unique_ptr<Scene> scene) {
    if (current_scene_) {
        current_scene_->on_exit();
    }

    current_scene_ = std::move(scene);

    if (current_scene_) {
        current_scene_->on_enter();
    }
}

void SceneManager::update() {
    if (current_scene_) {
        current_scene_->update();
    }
}

ftxui::Component SceneManager::get_component() {
    using namespace ftxui;

    // Return a component that dynamically delegates to the current scene
    // Note: Update is called explicitly in Engine::run(), not during rendering
    return Renderer([this] {
        if (!current_scene_) {
            return text("No active scene");
        }
        return current_scene_->get_component()->Render();
    }) | CatchEvent([this](Event event) {
        if (!current_scene_) {
            return false;
        }
        return current_scene_->get_component()->OnEvent(event);
    });
}

} // namespace engine

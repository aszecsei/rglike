#pragma once

#include "scene.h"
#include <memory>
#include <ftxui/component/component.hpp>

namespace engine {

/**
 * @brief Manages the active game scene.
 *
 * The SceneManager maintains a single active scene at a time (not a stack).
 * Calling set_scene() replaces the current scene entirely.
 *
 * Scene Lifecycle:
 * 1. set_scene() calls on_exit() on the old scene (if any)
 * 2. The new scene is installed
 * 3. on_enter() is called on the new scene
 *
 * The manager provides:
 * - Scene transitions with lifecycle hooks
 * - Component delegation for input handling and rendering
 * - Update propagation to the active scene
 *
 * Note: The Engine's main loop calls update() each frame to update scene logic.
 */
class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager() = default;

    // Set the active scene (replaces current scene)
    void set_scene(std::unique_ptr<Scene> scene);

    // Update the active scene
    void update();

    // Get the active scene's component (for input handling)
    ftxui::Component get_component();

    // Check if there's an active scene
    [[nodiscard]] bool empty() const { return !current_scene_; }

private:
    std::unique_ptr<Scene> current_scene_;
};

} // namespace engine

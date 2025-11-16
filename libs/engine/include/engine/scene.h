#pragma once

#include <ftxui/component/component.hpp>
#include <memory>

namespace engine {

class Engine;

class Scene {
public:
    explicit Scene(Engine* engine) : engine_(engine) {}
    virtual ~Scene() = default;

    // Update scene logic (turn-based, no delta time)
    virtual void update() = 0;

    // Get the FTXUI component for this scene (for input handling)
    virtual ftxui::Component get_component() = 0;

    // Called when scene becomes active
    virtual void on_enter() {}

    // Called when scene is no longer active
    virtual void on_exit() {}

protected:
    Engine* get_engine() { return engine_; }
    const Engine* get_engine() const { return engine_; }

private:
    Engine* engine_;
};

} // namespace engine

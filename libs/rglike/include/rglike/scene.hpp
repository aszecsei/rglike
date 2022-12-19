/**
 * @file scene.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include <string_view>

/// Contains the rglike classes and functions.
namespace rglike {
    class Game;

    /// @brief A scene in the game.
    class Scene {
    private:
        Game* owner = nullptr;

    public:
        explicit Scene(Game* owner)
            : owner(owner) { }

        virtual ~Scene() = default;

        template<class T> inline void ChangeScene() { owner->ChangeScene<T>(); }

        virtual auto SceneName() -> std::string_view = 0;

        virtual void Initialize() = 0;
        virtual void Render() = 0;
    };
} // namespace rglike
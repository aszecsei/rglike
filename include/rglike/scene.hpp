/**
 * @file scene.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string_view>

namespace rglike {
    class Game;

    class Scene {
    private:
        Game* owner = nullptr;

    public:
        explicit Scene(Game* owner)
            : owner(owner) { }

        virtual ~Scene() = default;

        template<class T> inline void ChangeScene() { owner->ChangeScene<T>(); }

        virtual auto SceneName() -> std::string_view = 0;
        virtual void Render() = 0;
    };
} // namespace rglike
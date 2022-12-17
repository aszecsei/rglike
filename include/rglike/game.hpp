/**
 * @file game.hpp
 * @author Alic Szecsei
 * @date 12/15/2022
 */

#pragma once

#include <memory>

namespace rglike {
    class Scene;

    class Game {
    private:
        std::unique_ptr<Scene> m_current_scene{nullptr};
        std::unique_ptr<Scene> m_next_scene{nullptr};

    public:
        void Initialize();
        void Run();

        template<class T> void ChangeScene() { m_next_scene = std::make_unique<T>(this); }
    };
} // namespace rglike

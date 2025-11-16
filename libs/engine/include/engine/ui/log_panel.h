//
// Created by Alic Szecsei on 10/4/2025.
//

#include <engine/game_log.h>
#include <ftxui/component/component.hpp>

#pragma once
namespace engine::ui {
    [[nodiscard]] auto create_log_panel(const GameLog& gameLog) -> ftxui::Component;
}
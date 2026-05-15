#include <engine/ui/stats_panel.h>
#include <engine/components.h>
#include <engine/stats.h>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <array>
#include <string>

namespace engine::ui {
using namespace ftxui;

namespace {

// One row per resource pool: label, current/max numeric, gauge with color.
Element resource_row(const Stats& stats, ResourcePool pool, Color bar_color) {
    int cur = stats.get_resource(pool);
    int max = stats.get_max_resource(pool);
    float ratio = max > 0 ? static_cast<float>(cur) / static_cast<float>(max) : 0.0F;

    auto label = text(get_resource_name(pool)) | size(WIDTH, EQUAL, 12);
    auto numeric = text(std::to_string(cur) + "/" + std::to_string(max))
                   | size(WIDTH, EQUAL, 10);
    auto bar = gauge(ratio) | color(bar_color) | flex;

    return vbox({
        hbox({label, numeric}),
        bar,
    });
}

} // namespace

auto create_stats_panel(const World& world) -> ftxui::Component {
    return Renderer([&world] {
        const auto& registry = world.get_registry();
        const auto player = world.get_player_entity();
        const auto* sc = registry.try_get<StatsComponent>(player);

        if (!sc) {
            return vbox({
                text("Stats") | bold | center,
                separator(),
                text("(no character)") | dim | center,
            }) | border;
        }

        // Level / XP at top.
        float xp_ratio = sc->get_experience_progress();
        auto level_line = hbox({
            text("Lvl ") | dim,
            text(std::to_string(sc->level)) | bold,
            text("   XP ") | dim,
            text(std::to_string(sc->experience) + "/" + std::to_string(sc->experience_to_next_level)),
        });
        auto xp_bar = gauge(xp_ratio) | color(Color::YellowLight);

        // Resource rows, colored by pool to make them scannable.
        // ftxui::Color is not constexpr-constructible from Palette16 enums,
        // so this stays a runtime-initialized array.
        const std::array<std::pair<ResourcePool, Color>, 5> rows = {{
            {ResourcePool::HEALTH, Color::Red},
            {ResourcePool::STAMINA, Color::Green},
            {ResourcePool::MANA, Color::Blue},
            {ResourcePool::FOCUS_POINTS, Color::Magenta},
            {ResourcePool::WILLPOWER, Color::Cyan},
        }};

        Elements children;
        children.push_back(text("Character") | bold | center);
        children.push_back(separator());
        children.push_back(level_line);
        children.push_back(xp_bar);
        children.push_back(separator());
        for (const auto& [pool, col] : rows) {
            children.push_back(resource_row(*sc, pool, col));
        }

        return vbox(std::move(children)) | border;
    });
}

} // namespace engine::ui

//
// Created by Alic Szecsei on 10/4/2025.
//

#include <engine/ui/world_panel.h>
#include <engine/components.h>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <algorithm>

namespace engine::ui {
    using namespace ftxui;

    class WorldPanel : public ComponentBase {
    public:
        explicit WorldPanel(World& world) : world_(world) {}

        Element Render() override {
            // Get box dimensions from reflection
            // Box coordinates are inclusive, so add 1 to get actual size
            int viewport_width = box_.x_max - box_.x_min + 1;
            int viewport_height = box_.y_max - box_.y_min + 1;

            // Get world dimensions
            int world_width = world_.get_width();
            int world_height = world_.get_height();

            // Get camera position from ECS (updated by World::update_systems())
            // Camera position is centered on the target (player)
            auto& registry = world_.get_registry();
            auto camera_entity = world_.get_camera_entity();
            const auto* camera = registry.try_get<Camera>(camera_entity);
            int camera_center_x = camera ? camera->x : 0;
            int camera_center_y = camera ? camera->y : 0;

            // Calculate top-left corner of viewport (camera is centered)
            int camera_x = camera_center_x - viewport_width / 2;
            int camera_y = camera_center_y - viewport_height / 2;

            // Clamp camera to world bounds
            camera_x = std::max(0, std::min(camera_x, world_width - viewport_width));
            camera_y = std::max(0, std::min(camera_y, world_height - viewport_height));

            // Calculate actual render dimensions (clamped to world size)
            int render_width = std::min(viewport_width, world_width - camera_x);
            int render_height = std::min(viewport_height, world_height - camera_y);

            std::vector<Element> rows;
            rows.reserve(render_height);

            for (int screen_y = 0; screen_y < render_height; ++screen_y) {
                std::vector<Element> cells;
                cells.reserve(render_width);

                for (int screen_x = 0; screen_x < render_width; ++screen_x) {
                    // Convert screen coordinates to world coordinates
                    int world_x = screen_x + camera_x;
                    int world_y = screen_y + camera_y;

                    Element cell;

                    // Check visibility using World methods
                    bool is_visible = world_.is_visible(world_x, world_y);
                    bool is_revealed = world_.is_revealed(world_x, world_y);

                    // If never revealed, render nothing (black/empty)
                    if (!is_revealed) {
                        cell = text(" ");
                    }
                    // If revealed but not currently visible, render terrain dimly (no entities)
                    else if (!is_visible) {
                        if (auto terrain = world_.get_terrain(world_x, world_y); terrain.has_value()) {
                            cell = text(terrain->glyph);
                            // Dim colors for fog-of-war
                            cell |= color(Color::GrayDark);
                        } else {
                            cell = text(" ");
                        }
                    }
                    // If currently visible, render normally
                    else {
                        // Get entities at this position using World method
                        if (auto entities = world_.get_entities_at(world_x, world_y); !entities.empty()) {
                            // Render the entity with highest render_order (last in sorted list)
                            auto entity = entities.back();
                            const auto& renderable = registry.get<Renderable>(entity);

                            cell = text(renderable.glyph);
                            if (renderable.fg_color != Color::Default) {
                                cell |= color(renderable.fg_color);
                            }
                            if (renderable.bg_color != Color::Default) {
                                cell |= bgcolor(renderable.bg_color);
                            }
                            if (renderable.bold) {
                                cell |= ftxui::bold;
                            }
                        } else {
                            // Render terrain
                            if (auto terrain = world_.get_terrain(world_x, world_y); terrain.has_value()) {
                                cell = text(terrain->glyph);
                                if (terrain->fg_color != Color::Default) {
                                    cell |= color(terrain->fg_color);
                                }
                                if (terrain->bg_color != Color::Default) {
                                    cell |= bgcolor(terrain->bg_color);
                                }
                            } else {
                                cell = text(" ");
                            }
                        }
                    }

                    cells.push_back(cell);
                }

                rows.push_back(hbox(cells));
            }

            auto world_display = vbox(rows);

            // Center the world if it's smaller than viewport
            if (render_width < viewport_width || render_height < viewport_height) {
                world_display = world_display | center;
            }

            return world_display | reflect(box_);
        }

        [[nodiscard]] bool Focusable() const override {
            return true;
        }

        bool OnEvent(Event event) override {
            if (event.is_mouse() && box_.Contain(event.mouse().x, event.mouse().y)) { TakeFocus(); }

            if (!Focused()) { return false; }

            int dx = 0, dy = 0;
            bool moved = false;

            // Arrow keys
            if (event == Event::ArrowUp) {
                dx = 0; dy = -1; moved = true;
            } else if (event == Event::ArrowDown) {
                dx = 0; dy = 1; moved = true;
            } else if (event == Event::ArrowLeft) {
                dx = -1; dy = 0; moved = true;
            } else if (event == Event::ArrowRight) {
                dx = 1; dy = 0; moved = true;
            }
            // Numpad movement
            else if (event == Event::Character('8')) {  // Up
                dx = 0; dy = -1; moved = true;
            } else if (event == Event::Character('2')) {  // Down
                dx = 0; dy = 1; moved = true;
            } else if (event == Event::Character('4')) {  // Left
                dx = -1; dy = 0; moved = true;
            } else if (event == Event::Character('6')) {  // Right
                dx = 1; dy = 0; moved = true;
            } else if (event == Event::Character('7')) {  // Up-Left
                dx = -1; dy = -1; moved = true;
            } else if (event == Event::Character('9')) {  // Up-Right
                dx = 1; dy = -1; moved = true;
            } else if (event == Event::Character('1')) {  // Down-Left
                dx = -1; dy = 1; moved = true;
            } else if (event == Event::Character('3')) {  // Down-Right
                dx = 1; dy = 1; moved = true;
            } else if (event == Event::Character('5')) {  // Wait/Stay
                dx = 0; dy = 0; moved = true;
            }

            if (moved) {
                world_.move_player(dx, dy);
                return true;
            }

            return false;
        }

    private:
        World& world_;
        Box box_;
    };

    auto create_world_panel(World& world) -> ftxui::Component {
        return Make<WorldPanel>(world);
    }
}

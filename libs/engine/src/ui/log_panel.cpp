//
// Created by Alic Szecsei on 10/4/2025.
//

#include <engine/ui/log_panel.h>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <algorithm>

namespace engine::ui {
    using namespace ftxui;

    class LogPanel : public ComponentBase {
    public:
        explicit LogPanel(const GameLog& log) : log_(log) {}

        Element Render() override {
            // Auto-select the most recent entry if count changed
            int current_count = log_.get_entry_count();
            if (current_count != last_entry_count_) {
                selected_ = current_count - 1;
                last_entry_count_ = current_count;
            }

            ftxui::Elements log{};
            log.reserve(current_count);
            int index = 0;
            for (const auto& line : log_.get_entries()) {
                bool is_focus = (index++ == selected_);
                Decorator line_decorator = nothing;
                if (is_focus) {
                    line_decorator = line_decorator | focus;
                } else {
                    line_decorator = line_decorator | dim;
                }
                log.push_back(line.render() | line_decorator);
            }
            return ftxui::vbox({
                   ftxui::text("> Game Log"),
                   ftxui::separator(),
                   ftxui::vbox(log) | vscroll_indicator | yframe | reflect(box_),
           });
        }

        [[nodiscard]] bool Focusable() const override {
            return true;
        }

        bool OnEvent(Event event) override {
            if (event.is_mouse() && box_.Contain(event.mouse().x, event.mouse().y)) { TakeFocus(); }

            if (!Focused()) {
                return false;
            }

            if (event == Event::ArrowUp || event == Event::Character('k') ||
                (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
                selected_ = std::max(0, selected_ - 1);
                return true;
            } else if (event == Event::ArrowDown || event == Event::Character('j') ||
                       (event.is_mouse() && event.mouse().button == Mouse::WheelDown)) {
                selected_ = std::min(log_.get_entry_count() - 1, selected_ + 1);
                return true;
            } else if (event == Event::PageUp) {
                int height = box_.y_max - box_.y_min;
                selected_ = std::max(0, selected_ - height);
                return true;
            } else if (event == Event::PageDown) {
                int height = box_.y_max - box_.y_min;
                selected_ = std::min(log_.get_entry_count() - 1, selected_ + height);
                return true;
            } else if (event == Event::Home) {
                selected_ = 0;
                return true;
            } else if (event == Event::End) {
                selected_ = log_.get_entry_count() - 1;
                return true;
            }

            return false;
        }

    private:
        const GameLog& log_;
        int selected_ = 0;
        int last_entry_count_ = 0;
        Box box_;
    };

    auto create_log_panel(const GameLog& gameLog) -> ftxui::Component {
        return Make<LogPanel>(gameLog);
    }
}
/**
 * @file log_component.cpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#include "log_component.hpp"
#include <sstream>

using namespace ftxui;

namespace rglike::components {
    auto Split(const std::vector<GameLogFragment>& the_text) -> ftxui::Elements {
        Elements output{};
        output.push_back(text(":: "));
        for (const auto& fragment : the_text) {
            std::stringstream ss(fragment.text);
            std::string word;
            bool is_first = true;
            while (std::getline(ss, word, ' ')) {
                Decorator text_decorator = color(fragment.color) | bgcolor(fragment.bg_color);
                if (fragment.bold) { text_decorator = text_decorator | bold; }
                if (fragment.dim) { text_decorator = text_decorator | dim; }
                if (fragment.underlined) { text_decorator = text_decorator | underlined; }
                if (fragment.blinking) { text_decorator = text_decorator | blink; }

                // TODO(BUG): Handle the case where fragments should be concatenated w/o spaces

                output.push_back(text(word) | text_decorator);
            }
        }
        return output;
    }

    auto logParagraphAlignLeft(const std::vector<GameLogFragment>& the_text) -> ftxui::Element {
        static const auto config = ftxui::FlexboxConfig().SetGap(1, 0);
        return ftxui::flexbox(Split(the_text), config);
    }

    auto logParagraph(const std::vector<GameLogFragment>& the_text) -> ftxui::Element {
        return logParagraphAlignLeft(the_text);
    }

    class GameLogComponent : public ComponentBase {
    private:
        int m_selected = 0;
        const GameLog& m_log;
        Box m_box;

    public:
        explicit GameLogComponent(const GameLog& gameLog)
            : m_log(gameLog) { }

        [[nodiscard]] auto Render() -> ftxui::Element final;
        auto OnEvent(ftxui::Event event) -> bool final;

        [[nodiscard]] auto Focusable() const -> bool final { return true; }

        [[nodiscard]] inline auto selected() const -> int { return m_selected; }
    };

    auto GameLogComponent::Render() -> ftxui::Element {
        ftxui::Elements log{};
        log.reserve(m_log.size());
        int index = 0;
        for (const auto& line : m_log.entries()) {
            bool is_focus = (index++ == m_selected);
            Decorator line_decorator = nothing;
            if (is_focus) {
                line_decorator = line_decorator | focus;
                if (Focused()) { line_decorator = line_decorator | bold; }
            }
            log.push_back(logParagraph(line) | line_decorator);
        }
        return ftxui::vbox({
            ftxui::text("> Game Log"),
            ftxui::separatorCharacter("-"),
            ftxui::vbox(log) | vscroll_indicator | yframe | reflect(m_box),
        });
    }

    auto GameLogComponent::OnEvent(ftxui::Event event) -> bool {
        if (event.is_mouse() && m_box.Contain(event.mouse().x, event.mouse().y)) { TakeFocus(); }

        if (!Focused()) { return false; }

        int old_selected = m_selected;
        if (event == Event::ArrowUp || event == Event::Character('k') ||
            (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
            m_selected--;
        }
        if (event == Event::ArrowDown || event == Event::Character('j') ||
            (event.is_mouse() && event.mouse().button == Mouse::WheelDown)) {
            m_selected++;
        }
        if (event == Event::PageDown) { m_selected += m_box.y_max - m_box.y_min; }
        if (event == Event::PageUp) { m_selected -= m_box.y_max - m_box.y_min; }
        if (event == Event::Home) { m_selected = 0; }
        if (event == Event::End) { m_selected = (int)m_log.size() - 1; }
        m_selected = std::max(0, std::min((int)m_log.size() - 1, m_selected));

        return m_selected != old_selected;
    }

    auto game_log(const GameLog& gameLog) -> ftxui::Component {
        return Make<GameLogComponent>(gameLog);
    }
} // namespace rglike::components
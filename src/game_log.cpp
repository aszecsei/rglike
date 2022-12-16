/**
 * @file game_log.cpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */
#include "rglike/game_log.hpp"

#include "rglike/constants.hpp"

namespace rglike {
    void GameLog::log(const std::string_view& text) {
        if (m_log.size() == MAX_LOG_LINES) { m_log.pop_back(); }
        m_log.emplace_front(text);
        m_selected = 0;
    }

    auto GameLog::Render() -> ftxui::Element {
        ftxui::Elements log{};
        log.reserve(m_log.size());
        int index = 0;
        for (const auto& line : m_log) {
            bool is_focus = (index++ == m_selected);
            Decorator line_decorator = nothing;
            if (is_focus) {
                line_decorator = line_decorator | focus;
                if (Focused()) { line_decorator = line_decorator | inverted; }
            }
            log.push_back(ftxui::paragraph(line) | line_decorator);
        }
        return ftxui::vbox(log) | vscroll_indicator | yframe | reflect(m_box);
    }

    auto GameLog::OnEvent(Event event) -> bool {
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

} // namespace rglike

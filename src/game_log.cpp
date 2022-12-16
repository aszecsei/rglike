/**
 * @file game_log.cpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */
#include "rglike/game_log.hpp"

#include "rglike/constants.hpp"
#include <sstream>

namespace rglike {
    auto LogEntry::color(Color color) -> LogEntry& {
        m_current_color = color;
        return *this;
    }

    auto LogEntry::bg_color(Color color) -> LogEntry& {
        m_current_bg_color = color;
        return *this;
    }

    auto LogEntry::bold(bool bold) -> LogEntry& {
        m_current_bold = bold;
        return *this;
    }

    auto LogEntry::dim(bool dim) -> LogEntry& {
        m_current_dim = dim;
        return *this;
    }

    auto LogEntry::underline(bool underline) -> LogEntry& {
        m_current_underlined = underline;
        return *this;
    }

    auto LogEntry::blink(bool blink) -> LogEntry& {
        m_current_blinking = blink;
        return *this;
    }

    auto LogEntry::text(const std::string_view& text) -> LogEntry& {
        m_fragments.emplace_back(
            m_current_color, m_current_bg_color, m_current_bold, m_current_dim,
            m_current_underlined, m_current_blinking, text
        );
        return *this;
    }

    void LogEntry::log() const { m_log->log(*this); }

    auto Split(const std::vector<GameLogFragment>& the_text) -> ftxui::Elements {
        Elements output{};
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

    auto logParagraph(const std::vector<GameLogFragment>& the_text) -> ftxui::Element {
        return logParagraphAlignLeft(the_text);
    }

    auto logParagraphAlignLeft(const std::vector<GameLogFragment>& the_text) -> ftxui::Element {
        static const auto config = ftxui::FlexboxConfig().SetGap(1, 0);
        return ftxui::flexbox(Split(the_text), config);
    }

    void GameLog::log(const std::string_view& text) { entry().text(text).log(); }

    void GameLog::log(const LogEntry& entry) {
        if (m_log.size() == MAX_LOG_LINES) { m_log.pop_back(); }
        m_log.push_front(entry.fragments());
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

    auto GameLog::entry() -> LogEntry { return LogEntry(this); }
} // namespace rglike

#include <engine/game_log.h>
#include <ftxui/dom/elements.hpp>

namespace engine {

using namespace ftxui;

void LogEntry::add_fragment(const std::string& text, const LogFragmentStyle& style) {
    fragments_.push_back({.text = text, .style = style});
}

Element LogEntry::render() const {
    if (fragments_.empty()) {
        return text("");
    }

    // Split fragments into words and create styled elements
    std::vector<Element> word_elements;
    word_elements.push_back(text(":: "));

    for (const auto& fragment : fragments_) {
        // Split the fragment text into words
        std::string current_word;
        for (char c : fragment.text) {
            if (c == ' ' || c == '\t' || c == '\n') {
                // End of word - add it if non-empty
                if (!current_word.empty()) {
                    Element word_elem = text(current_word);

                    // Apply styling
                    if (fragment.style.color != Color::Default) {
                        word_elem |= color(fragment.style.color);
                    }
                    if (fragment.style.bg_color != Color::Default) {
                        word_elem |= bgcolor(fragment.style.color);
                    }
                    if (fragment.style.bold) {
                        word_elem |= bold;
                    }
                    if (fragment.style.dim) {
                        word_elem |= dim;
                    }
                    if (fragment.style.underlined) {
                        word_elem |= underlined;
                    }
                    if (fragment.style.blink) {
                        word_elem |= blink;
                    }

                    word_elements.push_back(word_elem);
                    current_word.clear();
                }
                // Add space as a separator (will be used for word spacing in hflow)
                if (c == ' ' && !word_elements.empty()) {
                    word_elements.push_back(text(" "));
                }
            } else {
                current_word += c;
            }
        }

        // Add any remaining word
        if (!current_word.empty()) {
            Element word_elem = text(current_word);

            // Apply styling
            if (fragment.style.color != Color::Default) {
                word_elem |= color(fragment.style.color);
            }
            if (fragment.style.bold) {
                word_elem |= bold;
            }
            if (fragment.style.dim) {
                word_elem |= dim;
            }
            if (fragment.style.underlined) {
                word_elem |= underlined;
            }
            if (fragment.style.blink) {
                word_elem |= blink;
            }

            word_elements.push_back(word_elem);
        }
    }

    // Use flexbox wrapping for word wrap
    return hflow(word_elements);
}

LogEntryBuilder& LogEntryBuilder::text(const std::string& text) {
    entry_.add_fragment(text, current_style_);
    return *this;
}

LogEntryBuilder& LogEntryBuilder::color(ftxui::Color color) {
    current_style_.color = color;
    return *this;
}

LogEntryBuilder& LogEntryBuilder::bold(bool bold) {
    current_style_.bold = bold;
    return *this;
}

LogEntryBuilder& LogEntryBuilder::dim(bool dim) {
    current_style_.dim = dim;
    return *this;
}

LogEntryBuilder& LogEntryBuilder::underlined(bool underlined) {
    current_style_.underlined = underlined;
    return *this;
}

LogEntryBuilder& LogEntryBuilder::blink(bool blink) {
    current_style_.blink = blink;
    return *this;
}

LogEntryBuilder& LogEntryBuilder::reset_style() {
    current_style_ = LogFragmentStyle{};
    return *this;
}

void LogEntryBuilder::log() {
    log_->add_entry(std::move(entry_));
}

LogEntryBuilder GameLog::entry() {
    return LogEntryBuilder(this);
}

void GameLog::add_entry(LogEntry entry) {
    entries_.push_back(std::move(entry));
}

} // namespace engine

/**
 * @file resource.hpp
 * @author Alic Szecsei
 * @date 12/18/2022
 */

#pragma once

#include <variant>
#include <vector>
#include <string>

#include "ast.hpp"

namespace fluent {
    class FluentResource {
    private:
        std::vector<std::variant<ast::Message, ast::Term>> m_body;
        
    public:
        explicit FluentResource(std::string source);
    };
} // namespace fluent
/**
 * @file ast.hpp
 * @author Alic Szecsei
 * @date 12/18/2022
 */

#pragma once

#include <fmt/core.h>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace fluent::ast {
    struct Message;
    struct Term;
    struct SelectExpression;
    struct VariableReference;
    struct TermReference;
    struct MessageReference;
    struct FunctionReference;
    struct Variant;
    struct NamedArgument;

    using NumberLiteral = std::variant<int64_t, double>;
    using StringLiteral = std::string;

    using Literal = std::variant<StringLiteral, NumberLiteral>;

    using Expression = std::variant<
        SelectExpression, VariableReference, TermReference, MessageReference, FunctionReference,
        Literal>;

    using PatternElement = std::variant<std::string, Expression>;

    using ComplexPattern = std::vector<PatternElement>;

    using Pattern = std::variant<std::string, ComplexPattern>;

    struct Message {
        std::string id;
        std::optional<Pattern> value;
        std::unordered_map<std::string, Pattern> attributes;
    };

    struct Term {
        std::string id;
        Pattern value;
        std::unordered_map<std::string, Pattern> attributes;
    };

    struct SelectExpression {
        std::shared_ptr<Expression> selector;
        std::vector<Variant> variants;
        size_t star;
    };

    struct VariableReference {
        std::string name;

        explicit VariableReference(std::string&& name)
            : name(std::move(name)) { }
    };

    struct TermReference {
        std::string name;
        std::optional<std::string> attr;
        std::vector<std::variant<Expression, NamedArgument>> args;
    };

    struct MessageReference {
        std::string name;
        std::optional<std::string> attr;

        explicit MessageReference(
            std::string&& name, std::optional<std::string>&& attr = std::optional<std::string>()
        )
            : name(std::move(name))
            , attr(std::move(attr)) { }
    };

    struct FunctionReference {
        std::string name;
        std::vector<std::variant<Expression, NamedArgument>> args;
    };

    struct Variant {
        Literal key;
        Pattern value;
    };

    struct NamedArgument {
        std::string name;
        Literal value;
    };
} // namespace fluent::ast
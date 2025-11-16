#pragma once

#include <sol/sol.hpp>
#include <spdlog/logger.h>
#include <memory>

namespace engine {

class Engine;

class LuaBindings {
public:
    // Initialize Lua state with all bindings
    static void initialize(sol::state& lua, Engine* engine, std::shared_ptr<spdlog::logger> logger);

    // Generate Lua type definition file for editor support
    static void generate_type_definitions(const std::string& output_path);
};

} // namespace engine

#pragma once

#include "registry.h"
#include "stats.h"
#include <string>

namespace engine {

// Type alias for growth pattern registry
using GrowthPatternRegistry = Registry<StatGrowthPattern>;

} // namespace engine
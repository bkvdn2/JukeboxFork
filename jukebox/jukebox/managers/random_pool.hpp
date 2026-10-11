#pragma once

#include <optional>
#include <string>
#include <vector>

namespace jukebox::random_pool {

/**
 * Songs the user allowed for random switching, per GD song ID.
 * No entry means the user never customised it, so every downloaded song is allowed.
 */
std::optional<std::vector<std::string>> get(int songID);

void set(int songID, std::vector<std::string> uniqueIDs);

bool isAllowed(int songID, const std::string& uniqueID);

}  // namespace jukebox::random_pool

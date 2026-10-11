#pragma once

#include <vector>

#include <Geode/binding/GJGameLevel.hpp>

namespace jukebox {

/**
 * Collects the Jukebox song IDs used by a level.
 * Robtop songs use the negative ID convention ( -(audioTrack) - 1 ).
 * Multi-asset levels return every song of the level, otherwise a single ID is returned.
 */
std::vector<int> getLevelSongIDs(GJGameLevel* level);

}  // namespace jukebox

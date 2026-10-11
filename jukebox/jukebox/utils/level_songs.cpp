#include <jukebox/utils/level_songs.hpp>

#include <string>

#include <Geode/utils/general.hpp>
#include <Geode/utils/string.hpp>

namespace jukebox {

std::vector<int> getLevelSongIDs(GJGameLevel* level) {
    std::vector<int> ids;
    if (!level) {
        return ids;
    }

    const std::string multi = std::string(level->m_songIDs);
    for (const std::string& part : geode::utils::string::split(multi, ",")) {
        if (auto id = geode::utils::numFromString<int>(part); id.isOk() && id.unwrap() != 0) {
            ids.push_back(id.unwrap());
        }
    }
    if (ids.size() > 1) {
        return ids;
    }

    ids.clear();
    if (level->m_songID != 0) {
        ids.push_back(level->m_songID);
    } else {
        ids.push_back(-level->m_audioTrack - 1);
    }
    return ids;
}

}  // namespace jukebox

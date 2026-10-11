#include <filesystem>
#include <random>
#include <string>
#include <vector>

#include <asp/fs/fs.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/modify/PlayLayer.hpp>  // IWYU pragma: keep

#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/managers/random_pool.hpp>
#include <jukebox/nong/nong.hpp>
#include <jukebox/utils/level_songs.hpp>

using namespace geode::prelude;
using namespace jukebox;

namespace {

bool isPlayable(const Song* song) {
    const auto path = song->path();
    if (!path.has_value() || !asp::fs::exists(path.value())) {
        return false;
    }
    std::error_code ec;
    const auto size = std::filesystem::file_size(path.value(), ec);
    return !ec && size > 0;
}

/**
 * Picks a random song from the user's pool (downloaded, default included) different from the
 * active one for every song of the level. Songs with nothing else to pick from are left alone.
 */
void randomizeLevelSongs(GJGameLevel* level) {
    static std::mt19937 rng{std::random_device{}()};

    for (const int id : getLevelSongIDs(level)) {
        const std::optional<Nongs*> opt = NongManager::get().getNongs(id);
        if (!opt) {
            continue;
        }
        Nongs* nongs = opt.value();

        std::vector<std::string> candidates;
        const auto consider = [&](const Song* song) {
            const std::string& uniqueID = song->metadata()->uniqueID;
            if (song == nongs->active() || !random_pool::isAllowed(id, uniqueID)) {
                return;
            }
            if (!isPlayable(song)) {
                log::debug("Random song: skipping {} for {}, file missing or empty", uniqueID, id);
                return;
            }
            candidates.push_back(uniqueID);
        };
        consider(nongs->defaultSong());
        for (auto& s : nongs->locals()) consider(s.get());
        for (auto& s : nongs->youtube()) consider(s.get());
        for (auto& s : nongs->hosted()) consider(s.get());

        if (candidates.empty()) {
            continue;
        }

        const std::string& pick = candidates[std::uniform_int_distribution<size_t>(0, candidates.size() - 1)(rng)];
        if (auto res = NongManager::get().setActiveSong(id, pick); res.isErr()) {
            log::error("Failed to randomize song {}: {}", id, res.unwrapErr());
        } else {
            log::info("Random song: {} -> {} ({})", id, pick, nongs->active()->path().value_or("").string());
        }
    }
}

}  // namespace

class $modify(JBPlayLayer, PlayLayer) {
    struct Fields {
        int restarts = 0;
        bool started = false;
    };

    void resetLevel() {
        Fields* f = m_fields.operator->();
        // The first call is the initial start of the level, not a new attempt
        if (!f->started) {
            f->started = true;
        } else if (!m_isPracticeMode && Mod::get()->getSettingValue<bool>("random-song-enabled")) {
            const int every = static_cast<int>(Mod::get()->getSettingValue<int64_t>("random-song-attempts"));
            if (++f->restarts >= every) {
                f->restarts = 0;
                // Original resetLevel restarts the music, which picks up the new active song
                randomizeLevelSongs(m_level);
            }
        }
        PlayLayer::resetLevel();
    }
};

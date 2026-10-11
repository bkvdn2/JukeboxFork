#include <jukebox/managers/random_pool.hpp>

#include <algorithm>
#include <map>

#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

namespace jukebox::random_pool {

namespace {
using Pools = std::map<std::string, std::vector<std::string>>;
constexpr const char* s_key = "random-pool";
}  // namespace

std::optional<std::vector<std::string>> get(int songID) {
    const Pools pools = Mod::get()->getSavedValue<Pools>(s_key);
    if (const auto it = pools.find(std::to_string(songID)); it != pools.end()) {
        return it->second;
    }
    return std::nullopt;
}

void set(int songID, std::vector<std::string> uniqueIDs) {
    Pools pools = Mod::get()->getSavedValue<Pools>(s_key);
    pools[std::to_string(songID)] = std::move(uniqueIDs);
    Mod::get()->setSavedValue<Pools>(s_key, std::move(pools));
}

bool isAllowed(int songID, const std::string& uniqueID) {
    const auto pool = get(songID);
    if (!pool) {
        return true;
    }
    return std::ranges::find(*pool, uniqueID) != pool->end();
}

}  // namespace jukebox::random_pool

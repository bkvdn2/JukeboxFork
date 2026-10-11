#pragma once

#include <set>
#include <string>
#include <vector>

#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/ui/Popup.hpp>

namespace jukebox {

/**
 * Lets the user choose which downloaded songs of a song ID can be picked by the random switcher
 */
class RandomPoolPopup : public geode::Popup {
protected:
    struct Entry {
        std::string uniqueID;
        std::string label;
    };

    int m_songID = 0;
    std::vector<Entry> m_entries;
    std::set<std::string> m_selected;

    bool init(int songID);
    void onToggle(cocos2d::CCObject* sender);
    void onAll(cocos2d::CCObject*);
    void onNone(cocos2d::CCObject*);
    void save();
    void createList();

    geode::Ref<cocos2d::CCNode> m_listHolder;

public:
    static RandomPoolPopup* create(int songID);
};

}  // namespace jukebox

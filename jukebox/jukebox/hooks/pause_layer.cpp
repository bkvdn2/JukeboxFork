#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PauseLayer.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/modify/PauseLayer.hpp>  // IWYU pragma: keep
#include <Geode/ui/Layout.hpp>

#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/ui/nong_dropdown_layer.hpp>
#include <jukebox/utils/level_songs.hpp>

using namespace geode::prelude;
using namespace jukebox;

class $modify(JBPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        PlayLayer* pl = PlayLayer::get();
        if (!pl || !pl->m_level) {
            return;
        }

        CCSprite* spr = CCSprite::createWithSpriteFrameName("JB_PinDisc.png"_spr);
        spr->setScale(0.7f);
        CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(JBPauseLayer::onJukebox));
        btn->setID("nong-button"_spr);

        CCMenu* menu = CCMenu::create();
        menu->setID("nong-menu"_spr);
        menu->addChild(btn);
        btn->setPosition({0.f, 0.f});
        const CCSize winSize = CCDirector::get()->getWinSize();
        // top right corner, clear of the pause menu buttons
        menu->setPosition({winSize.width - 30.f, winSize.height - 30.f});
        this->addChild(menu, 10);
    }

    void onJukebox(CCObject*) {
        PlayLayer* pl = PlayLayer::get();
        if (!pl || !pl->m_level) {
            return;
        }

        std::vector<int> ids = getLevelSongIDs(pl->m_level);
        NongManager& manager = NongManager::get();
        for (int id : ids) {
            if (manager.hasSongID(id)) {
                continue;
            }
            const Result<Nongs*> res = manager.initSongID(nullptr, id, id < 0);
            if (res.isErr()) {
                log::error("Failed to init song {} from pause menu: {}", id, res.unwrapErr());
                FLAlertLayer::create("Error", "Failed to initialize Jukebox data for this level's song.", "Ok")->show();
                return;
            }
        }

        const int defaultID = ids.front();
        NongDropdownLayer* layer = NongDropdownLayer::create(ids, nullptr, defaultID, pl->m_level->m_levelID.value());
        if (!layer) {
            return;
        }
        layer->setZOrder(106);
        layer->show();

        FLAlertLayer::create("Jukebox",
                             "The new song will be used after the next <cy>restart</c> of the level.", "Ok")
            ->show();
    }
};

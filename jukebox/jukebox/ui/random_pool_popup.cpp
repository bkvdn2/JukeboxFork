#include <jukebox/ui/random_pool_popup.hpp>

#include <asp/fs/fs.hpp>
#include <fmt/format.h>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/ui/Label.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/SimpleAxisLayout.hpp>

#include <jukebox/managers/nong_manager.hpp>
#include <jukebox/managers/random_pool.hpp>
#include <jukebox/nong/nong.hpp>

using namespace geode::prelude;

namespace jukebox {

namespace {
bool isDownloaded(const Song* song) {
    const auto path = song->path();
    return path.has_value() && asp::fs::exists(path.value());
}
}  // namespace

RandomPoolPopup* RandomPoolPopup::create(int songID) {
    auto ret = new RandomPoolPopup();
    if (ret->init(songID)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool RandomPoolPopup::init(int songID) {
    if (!Popup::init(300.f, 250.f)) {
        return false;
    }
    this->setTitle("Random song pool");
    m_songID = songID;

    const std::optional<Nongs*> opt = NongManager::get().getNongs(songID);
    if (opt) {
        Nongs* nongs = opt.value();
        const auto add = [this](const Song* song) {
            if (!isDownloaded(song)) {
                return;
            }
            m_entries.push_back({song->metadata()->uniqueID,
                                 fmt::format("{} - {}", song->metadata()->name, song->metadata()->artist)});
        };
        add(nongs->defaultSong());
        for (auto& s : nongs->locals()) add(s.get());
        for (auto& s : nongs->youtube()) add(s.get());
        for (auto& s : nongs->hosted()) add(s.get());
    }

    for (const Entry& e : m_entries) {
        if (random_pool::isAllowed(songID, e.uniqueID)) {
            m_selected.insert(e.uniqueID);
        }
    }

    const CCSize size = m_mainLayer->getContentSize();

    auto* hint = geode::Label::create("Checked songs can be picked by the random switcher", "chatFont.fnt");
    hint->setScale(0.6f);
    hint->setColor({220, 220, 220});
    m_mainLayer->addChildAtPosition(hint, Anchor::Top, {0.f, -38.f});

    auto* menu = CCMenu::create();
    menu->setContentSize({size.width, 30.f});
    auto* allBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("All", "goldFont.fnt", "GJ_button_04.png", 0.6f),
                                                 this, menu_selector(RandomPoolPopup::onAll));
    auto* noneBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("None", "goldFont.fnt", "GJ_button_04.png", 0.6f), this,
        menu_selector(RandomPoolPopup::onNone));
    menu->addChild(allBtn);
    menu->addChild(noneBtn);
    menu->setLayout(SimpleRowLayout::create()->setGap(10.f));
    m_mainLayer->addChildAtPosition(menu, Anchor::Bottom, {0.f, 22.f});

    this->createList();
    return true;
}

void RandomPoolPopup::createList() {
    if (m_listHolder) {
        m_listHolder->removeFromParent();
    }
    const CCSize size = m_mainLayer->getContentSize();
    const CCSize listSize{size.width - 30.f, size.height - 120.f};

    m_listHolder = CCNode::create();
    m_listHolder->setContentSize(listSize);
    m_listHolder->setAnchorPoint({0.5f, 0.5f});

    auto* list = ScrollLayer::create(listSize);
    list->m_contentLayer->setLayout(SimpleColumnLayout::create()
                                        ->setMainAxisDirection(AxisDirection::TopToBottom)
                                        ->setMainAxisAlignment(MainAxisAlignment::Start)
                                        ->setMainAxisScaling(AxisScaling::Grow)
                                        ->setGap(4.f));

    if (m_entries.empty()) {
        auto* empty = geode::Label::create("No downloaded songs", "bigFont.fnt");
        empty->setScale(0.5f);
        empty->setPosition(listSize / 2);
        m_listHolder->addChild(empty);
    }

    for (size_t i = 0; i < m_entries.size(); i++) {
        auto* row = CCNode::create();
        row->setContentSize({listSize.width, 28.f});

        auto* rowMenu = CCMenu::create();
        rowMenu->setPosition({0.f, 0.f});
        rowMenu->setContentSize(row->getContentSize());
        auto* toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(RandomPoolPopup::onToggle),
                                                                      0.6f);
        toggler->setTag(static_cast<int>(i));
        toggler->toggle(m_selected.contains(m_entries[i].uniqueID));
        toggler->setPosition({16.f, 14.f});
        rowMenu->addChild(toggler);
        row->addChild(rowMenu);

        auto* label = geode::Label::create(m_entries[i].label, "bigFont.fnt");
        label->setAnchorPoint({0.f, 0.5f});
        label->setLimitLabelWidth(listSize.width - 50.f, 0.5f, 0.1f);
        label->setPosition({36.f, 14.f});
        row->addChild(label);

        list->m_contentLayer->addChild(row);
    }
    list->m_contentLayer->updateLayout();

    m_listHolder->addChild(list);
    m_mainLayer->addChildAtPosition(m_listHolder, Anchor::Center, {0.f, -2.f});
    handleTouchPriority(this);
}

void RandomPoolPopup::onToggle(CCObject* sender) {
    const auto* toggler = static_cast<CCMenuItemToggler*>(sender);
    const int idx = toggler->getTag();
    if (idx < 0 || idx >= static_cast<int>(m_entries.size())) {
        return;
    }
    // isToggled() is the state before the click is applied
    if (toggler->isToggled()) {
        m_selected.erase(m_entries[idx].uniqueID);
    } else {
        m_selected.insert(m_entries[idx].uniqueID);
    }
    this->save();
}

void RandomPoolPopup::onAll(CCObject*) {
    for (const Entry& e : m_entries) m_selected.insert(e.uniqueID);
    this->save();
    this->createList();
}

void RandomPoolPopup::onNone(CCObject*) {
    m_selected.clear();
    this->save();
    this->createList();
}

void RandomPoolPopup::save() {
    random_pool::set(m_songID, std::vector<std::string>(m_selected.begin(), m_selected.end()));
}

}  // namespace jukebox

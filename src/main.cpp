#include "Labels.hpp"
#include "LabelEditor.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(ILUILayer, UILayer) {
    bool init(GJBaseGameLayer* level) {
        if (!UILayer::init(level)) return false;

        auto container = CCNode::create();
        container->setID("il-container"_spr);
        this->addChild(container);
        il::rebuildContainer(container);

        return true;
    }
};

class $modify(ILPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton btn) {
        PlayerObject::pushButton(btn);
        if (btn == PlayerButton::Jump && PlayLayer::get()) il::setPressed(true);
        return true;
    }

    bool releaseButton(PlayerButton btn) {
        PlayerObject::releaseButton(btn);
        if (btn == PlayerButton::Jump && PlayLayer::get()) il::setPressed(false);
        return true;
    }
};

class $modify(ILPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        if (!Mod::get()->getSettingValue<bool>("pbutton")) return;

        auto menu = this->getChildByIDRecursive("left-button-menu");
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
        auto ame = CCSprite::create("ame.png"_spr);
            ame->setPosition(sprite->getContentSize() / 2);
            ame->setScale(0.2f);
            sprite->addChild(ame);
            sprite->setScale(0.7f);

        auto btn = CCMenuItemExt::createSpriteExtra(sprite, [](CCMenuItemSpriteExtra*) {
            if (auto popup = LabelEditor::create()) popup->show();
        });

        btn->setID("il-settings-button");

        menu->addChild(btn);
        menu->updateLayout();
    }
};

$on_mod(Loaded) {
    il::loadLabels();
}

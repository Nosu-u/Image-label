#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

float scale;
float posx;
float posy;
int opacity;
int rotation;
std::filesystem::path image;

namespace Zorder {
    constexpr int behind = 1;
    constexpr int above = 1001;
}

void updateSettings() {
	auto mod = Mod::get();

    scale = mod->getSettingValue<float>("scale");
    posx = mod->getSettingValue<float>("posx");
    posy = mod->getSettingValue<float>("posy");
    opacity = mod->getSettingValue<int>("opacity");
    rotation = mod->getSettingValue<int>("rotation");
    image = mod->getSettingValue<std::filesystem::path>("image");
}

class $modify(ILUILayer, UILayer) {
    bool init(GJBaseGameLayer* level) {
        if (!UILayer::init(level)) return false;

        auto mod = Mod::get();
		auto path = Mod::get()->getSettingValue<std::filesystem::path>("image");
		if (path.empty()) return true;
		auto nimage = CCSprite::create(utils::string::pathToString(path).c_str());

        nimage->setAnchorPoint({ 0.5f, 0.275f });
        nimage->setScale(scale);
        nimage->setPosition({ posx, posy });
        nimage->setOpacity((GLubyte)opacity);
        nimage->setRotation(rotation);
        nimage->setID("nosu.image_label/image"_spr);

		if (mod->getSettingValue<bool>("above")) {
            this->addChild(nimage, Zorder::above);
        } else {
            this->addChild(nimage, Zorder::behind);
        }

		return true;
    }
};

class $modify(IL, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        if (Mod::get()->getSettingValue<bool>("pbutton")) {
        
            auto menu = this->getChildByID("left-button-menu");
            auto sprite = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
            auto ame = CCSprite::create("ame.png"_spr);
            ame->setPosition(sprite->getContentSize() / 2);
            ame->setScale(0.2f);
            sprite->addChild(ame);
            sprite->setScale(0.7f);
            auto btn = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(IL::onILSettings)
            );

            btn->setID("nosu.image_label/settings-button");
            menu->addChild(btn);
            menu->updateLayout();
        }
    }

    void onILSettings(CCObject*) {
        openSettingsPopup(Mod::get(), false);
    }
};

$on_mod(Loaded) {
	updateSettings();
	
	listenForAllSettingChanges([](std::string_view key, std::shared_ptr<SettingV3> setting) {
		updateSettings();

        auto mod = Mod::get();
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        
        auto spr = static_cast<CCSprite*>(scene->getChildByIDRecursive("nosu.image_label/image"_spr));
        if (!spr) return;
        
        spr->setScale(scale);
        spr->setPosition({ posx, posy });
        spr->setOpacity((GLubyte)opacity);
        spr->setRotation(rotation);

		if (mod->getSettingValue<bool>("above")) {
            spr->setZOrder(Zorder::above);
        } else {
            spr->setZOrder(Zorder::behind);
        }

        // handle image swap
        auto currentPath = static_cast<CCString*>(spr->getUserObject("il-path"));
        if (!currentPath || currentPath->getCString() != utils::string::pathToString(image)) {
            auto parent = spr->getParent();
            auto newSpr = CCSprite::create(utils::string::pathToString(image).c_str());
            if (!newSpr) return;
            newSpr->setAnchorPoint({ 0.5f, 0.275f });
            newSpr->setScale(scale);
            newSpr->setPosition({ posx, posy });
            newSpr->setOpacity((GLubyte)opacity);
            newSpr->setRotation(rotation);
            newSpr->setID("nosu.image_label/image"_spr);
            newSpr->setUserObject("il-path", CCString::create(utils::string::pathToString(image)));
            spr->removeFromParentAndCleanup(true);

            if (mod->getSettingValue<bool>("above")) {
                parent->addChild(newSpr, Zorder::above);
            } else {
                parent->addChild(newSpr, Zorder::behind);
            }
        }
	});
}
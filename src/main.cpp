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
bool above;
std::filesystem::path image;

void updateSettings() {
	auto mod = Mod::get();

    scale = mod->getSettingValue<float>("scale");
    posx = mod->getSettingValue<float>("posx");
    posy = mod->getSettingValue<float>("posy");
    opacity = mod->getSettingValue<int>("opacity");
    rotation = mod->getSettingValue<int>("rotation");
    above = mod->getSettingValue<bool>("above");
    image = mod->getSettingValue<std::filesystem::path>("image");
}

class $modify(ILUILayer, UILayer) {
    bool init(GJBaseGameLayer* level) {
        if (!UILayer::init(level)) return false;

		auto path = Mod::get()->getSettingValue<std::filesystem::path>("image");
		if (path.empty()) return true;
		auto nimage = CCSprite::create(utils::string::pathToString(path).c_str());

        nimage->setAnchorPoint({ 0.5f, 0.275f });
        nimage->setScale(scale);
        nimage->setPosition({ posx, posy });
        nimage->setOpacity((GLubyte)opacity);
        nimage->setRotation(rotation);
        nimage->setID("IL-image");
		this->addChild(nimage, above ? 1001 : 1);

		return true;
    }
};

class $modify(IL, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        if (Mod::get()->getSettingValue<bool>("pbutton")) {
        
            auto menu = this->getChildByID("left-button-menu");
            auto sprite = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
            sprite->setScale(0.7f);
            auto btn = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(IL::onILSettings)
            );

            btn->setID("IL-button");
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
		
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        
        auto spr = static_cast<CCSprite*>(scene->getChildByIDRecursive("IL-image"));
        if (!spr) return;
        
        spr->setScale(scale);
        spr->setPosition({ posx, posy });
        spr->setOpacity((GLubyte)opacity);
        spr->setRotation(rotation);
		spr->setZOrder(above ? 1001 : 1);

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
            newSpr->setID("IL-image");
            newSpr->setUserObject("il-path", CCString::create(utils::string::pathToString(image)));
            spr->removeFromParentAndCleanup(true);
            parent->addChild(newSpr, above ? 1001 : 1);
        }
	});
}
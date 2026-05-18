#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

float scale;
float posx;
float posy;
int opacity;
int rotation;
std::filesystem::path image;

float inputScale;
float inputPosx;
float inputPosy;
int inputOpacity;
int inputRotation;
bool shouldInput;
std::filesystem::path inputImage;

void updateSettings() {
	auto mod = Mod::get();

    scale = mod->getSettingValue<float>("scale");
    posx = mod->getSettingValue<float>("posx");
    posy = mod->getSettingValue<float>("posy");
    opacity = mod->getSettingValue<int>("opacity");
    rotation = mod->getSettingValue<int>("rotation");
    image = mod->getSettingValue<std::filesystem::path>("image");

    inputScale = mod->getSettingValue<float>("input-scale");
    inputPosx = mod->getSettingValue<float>("input-posx");
    inputPosy = mod->getSettingValue<float>("input-posy");
    inputOpacity = mod->getSettingValue<int>("input-opacity");
    inputRotation = mod->getSettingValue<int>("input-rotation");
    inputImage = mod->getSettingValue<std::filesystem::path>("input-image");
    shouldInput = mod->getSettingValue<bool>("enable-input");
}

class $modify(ILUILayer, UILayer) {
    bool init(GJBaseGameLayer* level) {
        if (!UILayer::init(level)) return false;

        auto mod = Mod::get();
        auto winSize = CCDirector::get()->getWinSize();
		auto path = mod->getSettingValue<std::filesystem::path>("image");
        auto npath = mod->getSettingValue<std::filesystem::path>("input-image");
        if (std::filesystem::exists(path)) {
		    auto normalimage = CCSprite::create(utils::string::pathToString(path).c_str());

            normalimage->setAnchorPoint({ 0.5f, 0.275f });
            normalimage->setScale(scale);
            normalimage->setPosition({ (posx / 100.f) * winSize.width, (posy / 100.f) * winSize.height });
            normalimage->setOpacity((GLubyte)opacity);
            normalimage->setRotation(rotation);
            normalimage->setID("il-image"_spr);
            this->addChild(normalimage);
        }
        if (std::filesystem::exists(npath) && mod->getSettingValue<bool>("enable-input")) {
            auto nimage = CCSprite::create(utils::string::pathToString(npath).c_str());

            nimage->setAnchorPoint({ 0.5f, 0.275f });
	        nimage->setScale(inputScale);
	        nimage->setPosition({ (inputPosx / 100.f) * winSize.width, (inputPosy / 100.f) * winSize.height });
	        nimage->setOpacity((GLubyte)inputOpacity);
	        nimage->setRotation(inputRotation);
	        nimage->setID("il-input-image"_spr);
            nimage->setVisible(false);
            this->addChild(nimage);
        }

		return true;
    }
};

class $modify(ILPlayerObject, PlayerObject) {
	bool pushButton(PlayerButton btn) {
		PlayerObject::pushButton(btn);
		
		if (btn == PlayerButton::Jump && Mod::get()->getSettingValue<bool>("enable-input") && PlayLayer::get()) {
			auto normalimage = PlayLayer::get()->m_uiLayer->getChildByIDRecursive("il-image"_spr);
            auto nimage = PlayLayer::get()->m_uiLayer->getChildByIDRecursive("il-input-image"_spr);

            if (normalimage && nimage) {
                normalimage->setVisible(false);
                normalimage->pauseSchedulerAndActions(); // so it wont continue playing if its a gif (idk how to check and i dont think it matters tbh)
                nimage->setVisible(true);
                nimage->resumeSchedulerAndActions();
            }
		}
		
		return true;
	}

	bool releaseButton(PlayerButton btn) {
		PlayerObject::releaseButton(btn);
		
		if (btn == PlayerButton::Jump && Mod::get()->getSettingValue<bool>("enable-input") && PlayLayer::get()) {
			auto normalimage = PlayLayer::get()->m_uiLayer->getChildByIDRecursive("il-image"_spr);
            auto nimage = PlayLayer::get()->m_uiLayer->getChildByIDRecursive("il-input-image"_spr);

            if (normalimage && nimage) {
                normalimage->setVisible(true);
                normalimage->resumeSchedulerAndActions();
                nimage->setVisible(false);
                nimage->pauseSchedulerAndActions(); // so it wont continue playing if its a gif (idk how to check and i dont think it matters tbh)
            }
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

            btn->setID("il-settings-button");
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
        auto winSize = CCDirector::get()->getWinSize();
        auto normalimage = static_cast<CCSprite*>(PlayLayer::get()->m_uiLayer->getChildByIDRecursive("il-image"_spr));
        if (!normalimage) return;
        auto nimage = static_cast<CCSprite*>(PlayLayer::get()->m_uiLayer->getChildByIDRecursive("il-input-image"_spr));
        if (!nimage) return;
        
        normalimage->setScale(scale);
        normalimage->setPosition({ (posx / 100.f) * winSize.width, (posy / 100.f) * winSize.height });
        normalimage->setOpacity((GLubyte)opacity);
        normalimage->setRotation(rotation);

        // handle image swap
        auto currentPath = static_cast<CCString*>(normalimage->getUserObject("il-path"));
        auto currentnPath = static_cast<CCString*>(nimage->getUserObject("il-input-path"));
        if (!currentPath || currentPath->getCString() != utils::string::pathToString(image)) {
            auto parent = normalimage->getParent();
            if (std::filesystem::exists(image)) {
                auto newSpr = CCSprite::create(utils::string::pathToString(image).c_str());
                
                newSpr->setAnchorPoint({ 0.5f, 0.275f });
                newSpr->setScale(scale);
                newSpr->setPosition({ (posx / 100.f) * winSize.width, (posy / 100.f) * winSize.height });
                newSpr->setOpacity((GLubyte)opacity);
                newSpr->setRotation(rotation);
                newSpr->setID("il-image"_spr);
                newSpr->setUserObject("il-path", CCString::create(utils::string::pathToString(image)));
                normalimage->removeFromParentAndCleanup(true);
                parent->addChild(newSpr);
            }
        }

        if (!currentnPath || currentnPath->getCString() != utils::string::pathToString(inputImage)) {
            auto parent = nimage->getParent();
            if (std::filesystem::exists(inputImage)) {
                auto newiSpr = CCSprite::create(utils::string::pathToString(inputImage).c_str());
                
                newiSpr->setAnchorPoint({ 0.5f, 0.275f });
                newiSpr->setScale(inputScale);
                newiSpr->setPosition({ (inputPosx / 100.f) * winSize.width, (inputPosy / 100.f) * winSize.height });
                newiSpr->setOpacity((GLubyte)inputOpacity);
                newiSpr->setRotation(rotation);
                newiSpr->setID("il-input-image"_spr);
                newiSpr->setUserObject("il-input-path", CCString::create(utils::string::pathToString(inputImage)));
                newiSpr->setVisible(false);
                nimage->removeFromParentAndCleanup(true);
                parent->addChild(newiSpr);
            }
        }
	});
}

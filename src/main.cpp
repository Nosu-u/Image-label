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
std::filesystem::path inputImage;

// track if player is holding jump
static bool s_isHoldingJump = false;

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
}

void updateImageState() {
	// check if input image feature is enabled
	if (!Mod::get()->getSettingValue<bool>("enable-input")) return;

	// check if we have both images configured
	if (image.empty() || inputImage.empty()) return;

	auto scene = CCDirector::sharedDirector()->getRunningScene();
	if (!scene) return;

	auto spr = static_cast<CCSprite*>(scene->getChildByIDRecursive("il-image"_spr));
	if (!spr) return;

	auto winSize = CCDirector::get()->getWinSize();
	
	if (s_isHoldingJump) {
		// swap to input image
		auto newPath = utils::string::pathToString(inputImage);
		auto newTexture = CCTextureCache::sharedTextureCache()->addImage(newPath.c_str(), false);
		if (newTexture) {
			spr->setTexture(newTexture);
			auto size = newTexture->getContentSize();
			spr->setTextureRect(CCRect(0, 0, size.width, size.height));
			spr->setScale(inputScale);
			spr->setPosition({ (inputPosx / 100.f) * winSize.width, (inputPosy / 100.f) * winSize.height });
			spr->setOpacity((GLubyte)inputOpacity);
			spr->setRotation(inputRotation);
		}
	} else {
		// swap back to original image
		auto newPath = utils::string::pathToString(image);
		auto newTexture = CCTextureCache::sharedTextureCache()->addImage(newPath.c_str(), false);
		if (newTexture) {
			spr->setTexture(newTexture);
			auto size = newTexture->getContentSize();
			spr->setTextureRect(CCRect(0, 0, size.width, size.height));
			spr->setScale(scale);
			spr->setPosition({ (posx / 100.f) * winSize.width, (posy / 100.f) * winSize.height });
			spr->setOpacity((GLubyte)opacity);
			spr->setRotation(rotation);
		}
	}
}

class $modify(ILUILayer, UILayer) {
    bool init(GJBaseGameLayer* level) {
        if (!UILayer::init(level)) return false;

        auto mod = Mod::get();
        auto winSize = CCDirector::get()->getWinSize();
		auto path = Mod::get()->getSettingValue<std::filesystem::path>("image");
		if (path.empty()) return true;
		auto nimage = CCSprite::create(utils::string::pathToString(path).c_str());

        nimage->setAnchorPoint({ 0.5f, 0.275f });
        nimage->setScale(scale);
        nimage->setPosition({ (posx / 100.f) * winSize.width, (posy / 100.f) * winSize.height });
        nimage->setOpacity((GLubyte)opacity);
        nimage->setRotation(rotation);
        nimage->setID("il-image"_spr);
        this->addChild(nimage);

		return true;
    }
};

class $modify(ILPlayerObject, PlayerObject) {
	bool pushButton(PlayerButton btn) {
		bool result = PlayerObject::pushButton(btn);
		
		// only track jump button
		if (btn == PlayerButton::Jump) {
			s_isHoldingJump = true;
			updateImageState();
		}
		
		return result;
	}

	bool releaseButton(PlayerButton btn) {
		bool result = PlayerObject::releaseButton(btn);
		
		// only track jump button
		if (btn == PlayerButton::Jump) {
			s_isHoldingJump = false;
			updateImageState();
		}
		
		return result;
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
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        
        auto spr = static_cast<CCSprite*>(scene->getChildByIDRecursive("il-image"_spr));
        if (!spr) return;
        
        spr->setScale(scale);
        spr->setPosition({ (posx / 100.f) * winSize.width, (posy / 100.f) * winSize.height });
        spr->setOpacity((GLubyte)opacity);
        spr->setRotation(rotation);

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
            newSpr->setID("il-image"_spr);
            newSpr->setUserObject("il-path", CCString::create(utils::string::pathToString(image)));
            spr->removeFromParentAndCleanup(true);
            parent->addChild(newSpr);
        }
	});
}

#include "LabelEditor.hpp"
#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

class LabelEditorSettingV3 : public SettingV3 {
public:
    static Result<std::shared_ptr<SettingV3>> parse(
        std::string const& key, std::string const& modID, matjson::Value const& json
    ) {
        auto res = std::make_shared<LabelEditorSettingV3>();
        auto root = checkJson(json, "LabelEditorSettingV3");
        res->init(key, modID, root);
        res->parseNameAndDescription(root);
        res->parseEnableIf(root);
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }

    bool load(matjson::Value const& json) override { return true; }
    bool save(matjson::Value& json) const override { return true; }
    bool isDefaultValue() const override { return true; }
    void reset() override {}

    SettingNodeV3* createNode(float width) override;
};

class LabelEditorSettingNodeV3 : public SettingNodeV3 {
protected:
    ButtonSprite* m_buttonSprite = nullptr;
    CCMenuItemSpriteExtra* m_button = nullptr;
    CCMenu* menu = nullptr;

    bool init(std::shared_ptr<LabelEditorSettingV3> setting, float width) {
        if (!SettingNodeV3::init(setting, width)) return false;

        menu = CCMenu::create();
        menu->ignoreAnchorPointForPosition(true);
        menu->setPosition({ 284.0f, 16.0f });
        menu->setID("il-settings-menu");

        m_buttonSprite = ButtonSprite::create("Edit labels", "goldFont.fnt", "GJ_button_01.png", .8f);
        m_buttonSprite->setScale(.5f);
        m_button = CCMenuItemSpriteExtra::create(
            m_buttonSprite, this, menu_selector(LabelEditorSettingNodeV3::onButton)
        );

        m_button->m_baseScale = 1.5f;
        m_button->setScale(1.5f);
        m_button->setID("il-open-labelsEditor-button");

        this->addChild(menu);
        menu->addChildAtPosition(m_button, Anchor::Center);
        menu->updateLayout();

        this->updateState(nullptr);
        return true;
    }

    void updateState(CCNode* invoker) override {
        SettingNodeV3::updateState(invoker);

        auto shouldEnable = this->getSetting()->shouldEnable();
        m_button->setEnabled(shouldEnable);
        m_buttonSprite->setCascadeColorEnabled(true);
        m_buttonSprite->setCascadeOpacityEnabled(true);
        m_buttonSprite->setOpacity(shouldEnable ? 255 : 155);
        m_buttonSprite->setColor(shouldEnable ? ccWHITE : ccGRAY);
    }

    void onButton(CCObject*) {
        if (auto popup = LabelEditor::create()) popup->show();
    }

    void onCommit() override {}
    void onResetToDefault() override {}

public:
    bool hasUncommittedChanges() const override { return false; }
    bool hasNonDefaultValue() const override { return false; }

    static LabelEditorSettingNodeV3* create(std::shared_ptr<LabelEditorSettingV3> setting, float width) {
        auto ret = new LabelEditorSettingNodeV3();
        if (ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

SettingNodeV3* LabelEditorSettingV3::createNode(float width) {
    return LabelEditorSettingNodeV3::create(
        std::static_pointer_cast<LabelEditorSettingV3>(shared_from_this()), width
    );
}

$on_mod(Loaded) {
    (void)Mod::get()->registerCustomSettingType("labels-editor", &LabelEditorSettingV3::parse);
}

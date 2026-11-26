#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>

class LabelEditor : public geode::Popup {
public:
    static LabelEditor* create();

protected:
    bool init();

    // il::labels()
    void populate();
    cocos2d::CCNode* makeRow(size_t idx);

    // path + settings
    void addImageBlock(
        cocos2d::CCNode* row, cocos2d::CCMenu* menu, size_t idx, bool isInput,
        float pathY, float captionY, float inputY
    );

    void pickImage(size_t idx, bool isInput, geode::Ref<cocos2d::CCLabelBMFont> nameLabel);

    geode::ScrollLayer* m_scroll = nullptr;
    cocos2d::CCLabelBMFont* m_emptyLabel = nullptr;
};

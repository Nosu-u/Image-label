#include "Labels.hpp"
#include <Geode/utils/file.hpp>

using namespace geode::prelude;

namespace il {
    static std::vector<LabelCfg> s_labels;
    static bool s_pressed = false;

    static constexpr int NORMAL_TAG = 1000;
    static constexpr int INPUT_TAG = 2000;

    std::vector<LabelCfg>& labels() {
        return s_labels;
    }

    void saveLabels() {
        Mod::get()->setSavedValue("labels", s_labels);
    }

    // loads old settings so people dont burn me alive
    static void migrateLegacy() {
        auto mod = Mod::get();
        if (mod->getSavedValue<bool>("migrated-v2", false)) return;
        mod->setSavedValue<bool>("migrated-v2", true);

        auto res = file::readJson(mod->getSaveDir() / "settings.json");
        if (res.isErr()) return;
        auto const& json = res.unwrap();

        // input images
        auto read = [&](bool input) {
            ImageCfg c;
            auto key = [&](std::string_view name) {
                return input ? fmt::format("input-{}", name) : std::string(name);
            };
            c.path = json[key("image")].asString().unwrapOr("");
            c.scale = static_cast<float>(json[key("scale")].asDouble().unwrapOr(0.3));
            c.posx = static_cast<float>(json[key("posx")].asDouble().unwrapOr(95.0));
            c.posy = static_cast<float>(json[key("posy")].asDouble().unwrapOr(80.0));
            c.opacity = static_cast<int>(json[key("opacity")].asInt().unwrapOr(150));
            c.rotation = static_cast<int>(json[key("rotation")].asInt().unwrapOr(0));
            return c;
        };

        LabelCfg label;
        label.normal = read(false);
        label.input = read(true);
        label.useInput = json["enable-input"].asBool().unwrapOr(false);

        bool hasNormal = !label.normal.path.empty() && asp::fs::Path(label.normal.path).exists();
        bool hasInput = !label.input.path.empty() && asp::fs::Path(label.input.path).exists();
        if (!hasNormal && !hasInput) return;

        s_labels.push_back(label);
        saveLabels();
    }

    void loadLabels() {
        s_labels = Mod::get()->getSavedValue<std::vector<LabelCfg>>("labels");
        migrateLegacy();
    }

    static CCSprite* makeSprite(ImageCfg const& c, CCSize const& winSize) {
        if (c.path.empty() || !asp::fs::Path(c.path).exists()) return nullptr;

        auto spr = CCSprite::create(c.path.c_str());
        if (!spr) return nullptr;

        spr->setAnchorPoint({ 0.5f, 0.275f });
        spr->setScale(c.scale);
        spr->setPosition({ (c.posx / 100.f) * winSize.width, (c.posy / 100.f) * winSize.height });
        spr->setOpacity(static_cast<GLubyte>(std::clamp(c.opacity, 0, 255)));
        spr->setRotation(static_cast<float>(c.rotation));
        return spr;
    }

    static void applyPressed(CCNode* container, bool pressed) {
        for (size_t i = 0; i < s_labels.size(); i++) {
            auto normal = container->getChildByTag(NORMAL_TAG + static_cast<int>(i));
            auto input = container->getChildByTag(INPUT_TAG + static_cast<int>(i));
            // labels without an input image just stay as they are
            if (!normal || !input) continue;

            normal->setVisible(!pressed);
            input->setVisible(pressed);
            if (pressed) {
                normal->pauseSchedulerAndActions(); // so it wont continue playing if its a gif (idk how to check and i dont think it matters tbh)
                input->resumeSchedulerAndActions();
            } else {
                normal->resumeSchedulerAndActions();
                input->pauseSchedulerAndActions();
            }
        }
    }

    void rebuildContainer(CCNode* container) {
        if (!container) return;
        container->removeAllChildrenWithCleanup(true);

        auto winSize = CCDirector::get()->getWinSize();
        for (size_t i = 0; i < s_labels.size(); i++) {
            auto const& label = s_labels[i];

            if (auto normal = makeSprite(label.normal, winSize)) {
                normal->setTag(NORMAL_TAG + static_cast<int>(i));
                container->addChild(normal);
            }
            if (label.useInput) {
                if (auto input = makeSprite(label.input, winSize)) {
                    input->setTag(INPUT_TAG + static_cast<int>(i));
                    input->setVisible(false);
                    input->pauseSchedulerAndActions();
                    container->addChild(input);
                }
            }
        }

        applyPressed(container, s_pressed);
    }

    void refreshLive() {
        auto pl = PlayLayer::get();
        if (!pl || !pl->m_uiLayer) return;
        rebuildContainer(pl->m_uiLayer->getChildByID("il-container"_spr));
    }

    void setPressed(bool pressed) {
        s_pressed = pressed;

        auto pl = PlayLayer::get();
        if (!pl || !pl->m_uiLayer) return;
        if (auto container = pl->m_uiLayer->getChildByID("il-container"_spr)) {
            applyPressed(container, pressed);
        }
    }
}

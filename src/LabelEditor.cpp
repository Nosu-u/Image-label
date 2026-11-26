#include "LabelEditor.hpp"
#include "Labels.hpp"
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/async.hpp>

using namespace geode::prelude;

static constexpr float ROW_W = 400.f;
static constexpr float ROW_H = 200.f;

// returns just the file name instead of the whole path, or "(none)" if the path is empty
static std::string shortName(std::string const& path) {
    if (path.empty()) return "(none)";
    auto pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

static void setInputBlockEnabled(CCNode* node, bool enabled) {
    if (!node) return;

    if (auto input = typeinfo_cast<TextInput*>(node)) {
        input->setEnabled(enabled);
        auto bg = input->getBGSprite();

        if (!enabled) {
            bg->setOpacity(0.f);
            input->setFontScale(0.f);
        } else {
            bg->setOpacity(90.f);
            input->setFontScale(0.8f);
        }
        
        return;
    }

    if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(node)) {
        btn->setEnabled(enabled);
        btn->setVisible(enabled);
        return;
    }

    if (auto rgba = typeinfo_cast<CCRGBAProtocol*>(node)) {
        rgba->setOpacity(enabled ? 255 : 0);
    }
}

LabelEditor* LabelEditor::create() {
    auto ret = new LabelEditor();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LabelEditor::init() {
    if (!Popup::init(460.f, 310.f)) return false;

    this->setTitle("Image Labels");

    auto size = m_mainLayer->getContentSize();
    auto listSize = CCSize { ROW_W + 10.f, 215.f };
    auto listCenter = ccp(size.width / 2, size.height / 2 - 8.f);

    auto bg = CCScale9Sprite::create("square02_small.png");
    bg->setColor({ 0, 0, 0 });
    bg->setOpacity(90);
    bg->setContentSize(listSize + CCSize { 6.f, 6.f });
    bg->setPosition(listCenter);
    bg->setID("il-list-bg");
    m_mainLayer->addChild(bg);

    m_scroll = ScrollLayer::create(listSize);
    m_scroll->ignoreAnchorPointForPosition(true);
    m_scroll->setPosition({ listCenter.x - listSize.width / 2, listCenter.y - listSize.height / 2 });
    m_scroll->setID("il-scroll");
    m_scroll->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(listSize.height)
            ->setGap(6.f)
    );
    m_mainLayer->addChild(m_scroll);

    auto bar = Scrollbar::create(m_scroll);
    bar->setPosition(listCenter + ccp(listSize.width / 2 + 10.f, 0.f));
    bar->setID("il-scrollbar");
    m_mainLayer->addChild(bar);

    m_emptyLabel = CCLabelBMFont::create("No labels yet! Press \"Add label\"", "bigFont.fnt");
    m_emptyLabel->setScale(0.5f);
    m_emptyLabel->setPosition(listCenter);
    m_emptyLabel->setID("il-empty-label");
    m_mainLayer->addChild(m_emptyLabel);

    auto addSpr = ButtonSprite::create("Add label", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    auto addBtn = CCMenuItemExt::createSpriteExtra(addSpr, [this](CCMenuItemSpriteExtra*) {
        il::labels().push_back(LabelCfg {});
        il::saveLabels();
        il::refreshLive();
        this->populate();
    });
    addBtn->setID("il-add-button");
    m_buttonMenu->addChildAtPosition(addBtn, Anchor::Bottom, ccp(0.f, 22.f));

    this->populate();
    return true;
}

void LabelEditor::populate() {
    m_scroll->m_contentLayer->removeAllChildrenWithCleanup(true);

    auto count = il::labels().size();
    for (size_t i = 0; i < count; i++) {
        m_scroll->m_contentLayer->addChild(this->makeRow(i));
    }
    m_emptyLabel->setVisible(count == 0);

    m_scroll->m_contentLayer->updateLayout();
    m_scroll->scrollToTop();
}

CCNode* LabelEditor::makeRow(size_t idx) {
    auto row = CCNode::create();
    row->setContentSize({ ROW_W, ROW_H });

    auto bg = CCScale9Sprite::create("square02_small.png");
    bg->setOpacity(100);
    bg->setContentSize({ ROW_W, ROW_H });
    bg->setPosition({ ROW_W / 2, ROW_H / 2 });
    bg->setID("il-row-bg");
    row->addChild(bg);

    auto title = CCLabelBMFont::create(fmt::format("Label {}", idx + 1).c_str(), "goldFont.fnt");
    title->setScale(0.5f);
    title->setAnchorPoint({ 0.f, 0.5f });
    title->setPosition({ 12.f, ROW_H - 14.f });
    title->setID("il-title-label");
    row->addChild(title);

    auto menu = CCMenu::create();
    menu->setContentSize({ ROW_W, ROW_H });
    menu->setPosition({ 0.f, 0.f });
    menu->setID("il-row-menu");
    row->addChild(menu);

    auto trash = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    trash->setScale(0.6f);
    auto delBtn = CCMenuItemExt::createSpriteExtra(trash, [this, idx](CCMenuItemSpriteExtra*) {
        Ref<LabelEditor> self = this;
        createQuickPopup(
            "Delete label",
            fmt::format("Delete <cr>Label {}</c>?", idx + 1),
            "Cancel", "Delete",
            [self, idx](FLAlertLayer*, bool confirmed) {
                if (!confirmed) return;
                auto& labels = il::labels();
                if (idx >= labels.size()) return;
                labels.erase(labels.begin() + idx);
                il::saveLabels();
                il::refreshLive();
                self->populate();
            }
        );
    });
    delBtn->setPosition({ ROW_W - 20.f, ROW_H - 16.f });
    delBtn->setID("il-delete-button");
    menu->addChild(delBtn);

    // normal image block
    this->addImageBlock(row, menu, idx, false, ROW_H - 42.f, ROW_H - 62.f, ROW_H - 82.f);

    auto toggleText = CCLabelBMFont::create("Different image while clicking", "bigFont.fnt");
    toggleText->setScale(0.36f);
    toggleText->setAnchorPoint({ 0.f, 0.5f });
    toggleText->setPosition({ 40.f, ROW_H - 112.f });
    toggleText->setID("il-input-label");
    row->addChild(toggleText);

    auto rowStart = row->getChildrenCount();
    auto menuStart = menu->getChildrenCount();

    // input image block
    this->addImageBlock(row, menu, idx, true, ROW_H - 136.f, ROW_H - 156.f, ROW_H - 176.f);

    std::vector<Ref<CCNode>> inputWidgets;
    for (auto i = rowStart; i < row->getChildrenCount(); i++) {
        inputWidgets.push_back(static_cast<CCNode*>(row->getChildren()->objectAtIndex(i)));
    }
    for (auto i = menuStart; i < menu->getChildrenCount(); i++) {
        inputWidgets.push_back(static_cast<CCNode*>(menu->getChildren()->objectAtIndex(i)));
    }

    auto applyInputBlockEnabled = [inputWidgets](bool enabled) {
        for (auto const& widget : inputWidgets) setInputBlockEnabled(widget, enabled);
    };
    applyInputBlockEnabled(il::labels()[idx].useInput);

    auto toggler = CCMenuItemExt::createToggler(
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        [idx, applyInputBlockEnabled](CCMenuItemToggler* t) {
            auto& labels = il::labels();
            if (idx >= labels.size()) return;
            labels[idx].useInput = !t->isToggled();
            il::saveLabels();
            il::refreshLive();
            applyInputBlockEnabled(labels[idx].useInput);
        }
    );

    toggler->setScale(0.4f);
    toggler->toggle(il::labels()[idx].useInput);
    toggler->setPosition({ 24.f, ROW_H - 112.f });
    toggler->setID("il-input-toggler");
    menu->addChild(toggler);

    return row;
}

void LabelEditor::addImageBlock(
    CCNode* row, CCMenu* menu, size_t idx, bool isInput,
    float pathY, float captionY, float inputY
) {
    auto const& cfg = isInput ? il::labels()[idx].input : il::labels()[idx].normal;

    // file name label (updated after picking)
    auto nameLabel = CCLabelBMFont::create(shortName(cfg.path).c_str(), "chatFont.fnt");
    nameLabel->setAnchorPoint({ 0.f, 0.5f });
    nameLabel->setPosition({ 135.f, pathY });
    nameLabel->limitLabelWidth(250.f, 0.8f, 0.3f);
    nameLabel->setID(isInput ? "il-input-name-label" : "il-normal-name-label");
    row->addChild(nameLabel);

    auto pickSpr = ButtonSprite::create(
        isInput ? "Pick click image" : "Pick image", "goldFont.fnt", "GJ_button_04.png", 0.6f
    );
    Ref<CCLabelBMFont> nameRef = nameLabel;
    auto pickBtn = CCMenuItemExt::createSpriteExtra(pickSpr, [this, idx, isInput, nameRef](CCMenuItemSpriteExtra*) {
        this->pickImage(idx, isInput, nameRef);
    });
    pickBtn->setPosition({ 72.f, pathY });
    pickBtn->setScale(0.8f);
    pickBtn->m_baseScale = 0.8f;
    pickBtn->setID(isInput ? "il-pick-input-button" : "il-pick-normal-button");
    menu->addChild(pickBtn);

    struct Field { char const* caption; std::string value; };
    std::array<Field, 5> fields = {{
        { "Scale", fmt::format("{}", cfg.scale) },
        { "X %", fmt::format("{}", cfg.posx) },
        { "Y %", fmt::format("{}", cfg.posy) },
        { "Opacity", fmt::format("{}", cfg.opacity) },
        { "Rotation", fmt::format("{}", cfg.rotation) }
    }};

    for (size_t k = 0; k < fields.size(); k++) {
        float x = 50.f + static_cast<float>(k) * 75.f;

        auto caption = CCLabelBMFont::create(fields[k].caption, "bigFont.fnt");
        caption->setScale(0.3f);
        caption->setPosition({ x, captionY });
        row->addChild(caption);

        auto input = TextInput::create(64.f, fields[k].caption, "bigFont.fnt");
        input->setScale(0.8f);
        input->setPosition({ x, inputY });
        input->setNumericMode(true);
        input->setCommonFilter(k == 3 ? CommonFilter::Int : CommonFilter::Float);
        input->setString(fields[k].value, false); // false: don't fire the callback yet
        input->setCallback([idx, isInput, k](std::string const& text) {
            auto& labels = il::labels();
            if (idx >= labels.size()) return;

            auto parsed = utils::numFromString<float>(text);
            if (parsed.isErr()) return; // stuff like "-" or "."
            float v = parsed.unwrap();

            auto& c = isInput ? labels[idx].input : labels[idx].normal;
            switch (k) {
                case 0: c.scale = std::max(v, 0.01f); break;
                case 1: c.posx = v; break;
                case 2: c.posy = v; break;
                case 3: c.opacity = std::clamp(static_cast<int>(v), 0, 255); break;
                case 4: c.rotation = static_cast<int>(v); break;
            }
            il::saveLabels();
            il::refreshLive();
        });
        row->addChild(input);
    }
}

void LabelEditor::pickImage(size_t idx, bool isInput, Ref<CCLabelBMFont> nameLabel) {
    file::FilePickOptions options;
    options.filters.push_back({
        "Image files",
        { "*.gif", "*.png", "*.jpg", "*.jpeg", "*.webp" }
    });

    async::spawn(
        file::pick(file::PickMode::OpenFile, options),
        [idx, isInput, nameLabel](Result<std::optional<asp::fs::Path>> result) {
            if (result.isErr()) {
                log::warn("file picker failed: {}", result.unwrapErr());
                return;
            }
            auto picked = std::move(result).unwrap();
            if (!picked.has_value()) return;

            std::string path = std::string(picked->string());

            auto& labels = il::labels();
            if (idx >= labels.size()) return;

            (isInput ? labels[idx].input : labels[idx].normal).path = path;
            il::saveLabels();
            il::refreshLive();

            nameLabel->setString(shortName(path).c_str());
            nameLabel->limitLabelWidth(250.f, 0.8f, 0.3f);
        }
    );
}

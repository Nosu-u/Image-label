#pragma once
#include <Geode/Geode.hpp>

// settings of the images
struct ImageCfg {
    std::string path;
    float scale = 0.3f;
    float posx = 95.f;
    float posy = 80.f;
    int opacity = 150;
    int rotation = 0;
};

// images (a bit confusing ik)
struct LabelCfg {
    ImageCfg normal;
    ImageCfg input;
    bool useInput = false;
};

template <>
struct matjson::Serialize<ImageCfg> {
    static geode::Result<ImageCfg> fromJson(matjson::Value const& v) {
        ImageCfg c;
        c.path = v["path"].asString().unwrapOr(c.path);
        c.scale = static_cast<float>(v["scale"].asDouble().unwrapOr(c.scale));
        c.posx = static_cast<float>(v["posx"].asDouble().unwrapOr(c.posx));
        c.posy = static_cast<float>(v["posy"].asDouble().unwrapOr(c.posy));
        c.opacity = static_cast<int>(v["opacity"].asInt().unwrapOr(c.opacity));
        c.rotation = static_cast<int>(v["rotation"].asInt().unwrapOr(c.rotation));
        return geode::Ok(c);
    }

    static matjson::Value toJson(ImageCfg const& c) {
        return matjson::makeObject({
            { "path", c.path },
            { "scale", static_cast<double>(c.scale) },
            { "posx", static_cast<double>(c.posx) },
            { "posy", static_cast<double>(c.posy) },
            { "opacity", c.opacity },
            { "rotation", c.rotation }
        });
    }
};

template <>
struct matjson::Serialize<LabelCfg> {
    static geode::Result<LabelCfg> fromJson(matjson::Value const& v) {
        LabelCfg l;
        l.normal = v["normal"].as<ImageCfg>().unwrapOr(ImageCfg{});
        l.input = v["input"].as<ImageCfg>().unwrapOr(ImageCfg{});
        l.useInput = v["use-input"].asBool().unwrapOr(false);
        return geode::Ok(l);
    }

    static matjson::Value toJson(LabelCfg const& l) {
        return matjson::makeObject({
            { "normal", matjson::Value(l.normal) },
            { "input", matjson::Value(l.input) },
            { "use-input", l.useInput }
        });
    }
};

namespace il {
    std::vector<LabelCfg>& labels();
    void loadLabels();
    void saveLabels();

    // recreates every sprite inside the container node
    void rebuildContainer(cocos2d::CCNode* container);
    void refreshLive();

    // swaps images
    void setPressed(bool pressed);
}

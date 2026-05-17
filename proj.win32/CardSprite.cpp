#include "CardSprite.h"
#include <string>

using std::to_string;

CardSprite* CardSprite::create(const CardData& data) {
    auto card = new NormalCardSprite();
    if (card && card->init(data)) {
        card->autorelease();
        return card;
    }
    CC_SAFE_DELETE(card);
    return nullptr;
}

bool CardSprite::init(const CardData& data) {
    if (!Sprite::init()) return false;
    _data = data;
    _touchListener = nullptr;

    _back = Sprite::create("res/card_general.png");
    if (_back) {
        _back->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        addChild(_back);
    }

    auto rt = RenderTexture::create(1, 1, Texture2D::PixelFormat::RGBA8888);
    rt->beginWithClear(1.0f, 1.0f, 1.0f, 1.0f);
    rt->end();
    _faceBg = Sprite::createWithTexture(rt->getSprite()->getTexture());
    if (_faceBg) {
        _faceBg->setScale(180, 280);
        _faceBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        addChild(_faceBg);
    }

    setContentSize(Size(182, 282));

    return true;
}

void CardSprite::onEnter() {
    Sprite::onEnter();
    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->setSwallowTouches(true);
    _touchListener->onTouchBegan = CC_CALLBACK_2(CardSprite::onTouchBegan, this);
    _touchListener->onTouchEnded = CC_CALLBACK_2(CardSprite::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);
}

void CardSprite::onExit() {
    if (_touchListener) {
        _eventDispatcher->removeEventListener(_touchListener);
        _touchListener = nullptr;
    }
    Sprite::onExit();
}

string CardSprite::valueToFileName(const string& prefix) const {
    if (_data.value == 1) return prefix + "A";
    else if (_data.value <= 10) return prefix + to_string(_data.value);
    else if (_data.value == 11) return prefix + "J";
    else if (_data.value == 12) return prefix + "Q";
    else return prefix + "K";
}

bool CardSprite::onTouchBegan(Touch* touch, Event* event) {
    if (!isVisible()) return false;
    Vec2 locationInNode = convertToNodeSpace(touch->getLocation());
    Size s = getContentSize();
    Rect rect = Rect(-s.width / 2, -s.height / 2, s.width, s.height);
    return rect.containsPoint(locationInNode);
}

void CardSprite::onTouchEnded(Touch* touch, Event* event) {
    if (_onClicked) {
        _onClicked(this);
    }
}

void CardSprite::flipCard(bool open) {
    _data.isOpen = open;
    if (_faceBg) _faceBg->setVisible(open);
    if (_back) _back->setVisible(!open);
}

void CardSprite::flipCardAnimated(bool open, const function<void()>& callback) {
    _data.isOpen = open;
    stopAllActions();

    float duration = 0.15f;
    auto scaleDown = ScaleTo::create(duration, 0.0f, 1.0f);
    auto switchSide = CallFunc::create([this, open]() {
        if (_back) _back->setVisible(!open);
        if (_faceBg) _faceBg->setVisible(open);
        });
    auto scaleUp = ScaleTo::create(duration, 1.0f, 1.0f);

    if (callback) {
        runAction(Sequence::create(scaleDown, switchSide, scaleUp, CallFunc::create(callback), nullptr));
    }
    else {
        runAction(Sequence::create(scaleDown, switchSide, scaleUp, nullptr));
    }
}

void CardSprite::revealCovered(const function<void(CardSprite*)>& cb) {
    for (auto card : _coveredCards) {
        card->flipCardAnimated(true, [card, cb]() {
            card->setOnClicked(cb);
            });
    }
    _coveredCards.clear();
}

NormalCardSprite* NormalCardSprite::create(const CardData& data) {
    auto card = new NormalCardSprite();
    if (card && card->init(data)) {
        card->autorelease();
        return card;
    }
    CC_SAFE_DELETE(card);
    return nullptr;
}

bool NormalCardSprite::init(const CardData& data) {
    if (!CardSprite::init(data)) return false;
    loadFace();
    flipCard(_data.isOpen);
    return true;
}

void NormalCardSprite::loadFace() {
    bool isRed = (_data.suit == "heart" || _data.suit == "diamond");
    string colorPrefix = isRed ? "red" : "black";

    string valueStr = valueToFileName("");

    string bigPath = "res/number/big_" + colorPrefix + "_" + valueStr + ".png";
    _bigNumber = Sprite::create(bigPath);
    if (_bigNumber) {
        _bigNumber->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        _bigNumber->setPosition(Vec2(0, -25));
        addChild(_bigNumber);
    }

    string smallPath = "res/number/small_" + colorPrefix + "_" + valueStr + ".png";
    _smallNumber = Sprite::create(smallPath);
    if (_smallNumber) {
        _smallNumber->setAnchorPoint(Vec2(0, 1));
        _smallNumber->setPosition(Vec2(-55, 115));
        addChild(_smallNumber);
    }

    string suitFile;
    if (_data.suit == "heart") suitFile = "heart.png";
    else if (_data.suit == "diamond") suitFile = "diamond.png";
    else if (_data.suit == "club") suitFile = "club.png";
    else suitFile = "spade.png";

    _suitIcon = Sprite::create("res/suits/" + suitFile);
    if (_suitIcon) {
        _suitIcon->setAnchorPoint(Vec2(1, 1));
        _suitIcon->setPosition(Vec2(55, 115));
        addChild(_suitIcon);
    }
}

void NormalCardSprite::flipCard(bool open) {
    CardSprite::flipCard(open);
    if (_bigNumber) _bigNumber->setVisible(open);
    if (_smallNumber) _smallNumber->setVisible(open);
    if (_suitIcon) _suitIcon->setVisible(open);
}

void NormalCardSprite::flipCardAnimated(bool open, const function<void()>& callback) {
    _data.isOpen = open;
    stopAllActions();

    float duration = 0.15f;
    auto scaleDown = ScaleTo::create(duration, 0.0f, 1.0f);
    auto switchSide = CallFunc::create([this, open]() {
        if (_back) _back->setVisible(!open);
        if (_faceBg) _faceBg->setVisible(open);
        if (_bigNumber) _bigNumber->setVisible(open);
        if (_smallNumber) _smallNumber->setVisible(open);
        if (_suitIcon) _suitIcon->setVisible(open);
        });
    auto scaleUp = ScaleTo::create(duration, 1.0f, 1.0f);

    if (callback) {
        runAction(Sequence::create(scaleDown, switchSide, scaleUp, CallFunc::create(callback), nullptr));
    }
    else {
        runAction(Sequence::create(scaleDown, switchSide, scaleUp, nullptr));
    }
}

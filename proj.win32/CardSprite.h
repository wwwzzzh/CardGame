#ifndef CARD_SPRITE_H
#define CARD_SPRITE_H

#include "cocos2d.h"
#include "CardModel.h"
#include <functional>
#include <string>
#include <vector>
USING_NS_CC;

using std::function;
using std::string;
using std::vector;

class CardSprite : public Sprite
{
public:
    static CardSprite* create(const CardData& data);

    virtual bool init(const CardData& data);

    virtual void onEnter() override;
    virtual void onExit() override;

    virtual void flipCard(bool open);
    virtual void flipCardAnimated(bool open, const function<void()>& callback = nullptr);
    const CardData& getData() { return _data; }
    void setData(const CardData& data) { _data = data; }

    void setOnClicked(const function<void(CardSprite*)>& cb) { _onClicked = cb; }

    void setCoveredCards(const vector<CardSprite*>& cards) { _coveredCards = cards; }
    const vector<CardSprite*>& getCoveredCards() { return _coveredCards; }
    void revealCovered(const function<void(CardSprite*)>& cb = nullptr);

protected:
    CardSprite() {}
    virtual void loadFace() {}
    virtual string valueToFileName(const string& prefix) const;

    bool onTouchBegan(Touch* touch, Event* event);
    void onTouchEnded(Touch* touch, Event* event);

    CardData _data;
    Sprite* _back;
    Sprite* _faceBg;
    EventListenerTouchOneByOne* _touchListener;
    function<void(CardSprite*)> _onClicked;
    vector<CardSprite*> _coveredCards;
};

class NormalCardSprite : public CardSprite
{
public:
    static NormalCardSprite* create(const CardData& data);
    virtual bool init(const CardData& data) override;
    NormalCardSprite() {}
    virtual void loadFace() override;
    virtual void flipCard(bool open) override;
    virtual void flipCardAnimated(bool open, const function<void()>& callback = nullptr) override;

private:
    Sprite* _bigNumber;
    Sprite* _smallNumber;
    Sprite* _suitIcon;
};

#endif
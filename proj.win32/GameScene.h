#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "CardSprite.h"
#include <functional>
#include <vector>
USING_NS_CC;
using namespace ui;

using std::function;
using std::vector;

class GameScene : public Layer
{
public:
    static Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(GameScene);

private:
    void initUI();
    void dealCards();
    void onCardClicked(CardSprite* card);
    void onDeckClicked();
    void onUndoClicked(Ref*, Widget::TouchEventType);
    void onRestartClicked(Ref*, Widget::TouchEventType);
    void moveCardTo(CardSprite* card, Vec2 to, function<void()> cb = nullptr);
    void refreshUndoButton();
    bool hasAvailableMatch();
    void checkVictory();
    void checkDefeat();

    CardSprite* _baseCard;
    vector<CardSprite*> _deskCards;
    vector<CardSprite*> _deckPile;
    Button* _undoBtn;
    Button* _restartBtn;
    Label* _defeatLabel;
    bool _isAnimating;
    bool _gameOver;
};

#endif

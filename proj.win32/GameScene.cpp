#include "GameScene.h"
#include "CardModel.h"
#include "SimpleAudioEngine.h"
#include <algorithm>

USING_NS_CC;

Scene* GameScene::createScene() {
    auto s = Scene::create();
    auto layer = GameScene::create();
    s->addChild(layer);
    return s;
}

bool GameScene::init() {
    if (!Layer::init()) return false;

    auto size = Director::getInstance()->getWinSize();

    CardModel::getInstance()->initData();
    _isAnimating = false;
    _gameOver = false;
    _defeatLabel = nullptr;
    _restartBtn = nullptr;
    initUI();
    dealCards();

    return true;
}

void GameScene::initUI() {
    auto size = Director::getInstance()->getWinSize();

    auto bg = Sprite::create("res/bg.png");
    if (bg) {
        bg->setAnchorPoint(Vec2::ZERO);
        bg->setPosition(Vec2::ZERO);
        bg->setScale(size.width / bg->getContentSize().width,
                     size.height / bg->getContentSize().height);
        addChild(bg, -1);
    }

    _undoBtn = Button::create("res/card_general.png", "res/card_general.png");
    _undoBtn->setScale9Enabled(true);
    _undoBtn->setContentSize(Size(180, 90));
    _undoBtn->setTitleFontSize(40);
    _undoBtn->setTitleText("UNDO");
    _undoBtn->setTitleColor(Color3B::BLACK);
    _undoBtn->setPosition(Vec2(size.width - 130, size.height - 80));
    _undoBtn->addTouchEventListener(CC_CALLBACK_2(GameScene::onUndoClicked, this));
    addChild(_undoBtn, 100);

    _restartBtn = Button::create("res/card_general.png", "res/card_general.png");
    _restartBtn->setScale9Enabled(true);
    _restartBtn->setContentSize(Size(220, 90));
    _restartBtn->setTitleFontSize(36);
    _restartBtn->setTitleText("RESTART");
    _restartBtn->setTitleColor(Color3B::BLACK);
    _restartBtn->setPosition(Vec2(160, size.height - 80));
    _restartBtn->addTouchEventListener(CC_CALLBACK_2(GameScene::onRestartClicked, this));
    addChild(_restartBtn, 100);

    CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("res/fanpai.mp3");
}

// setup card columns on the table
void GameScene::dealCards() {
    auto deck = CardModel::getInstance()->generateDeck();
    deck = CardModel::getInstance()->shuffle(deck);

    Size size = Director::getInstance()->getWinSize();
    float cardW = 182;
    float cardH = 282;

    if (deck.empty()) return;

    CardData baseData = deck.back();
    deck.pop_back();
    baseData.isOpen = true;
    baseData.originPos = Vec2(size.width / 2, 300);
    _baseCard = CardSprite::create(baseData);
    _baseCard->setPosition(baseData.originPos);
    _baseCard->setContentSize(Size(cardW, cardH));
    _baseCard->setOnClicked(nullptr);
    addChild(_baseCard, 10);

    float colGap = 212;
    float startX = (size.width - 4 * colGap) / 2 + colGap / 2;
    int cardsPerCol = 4;
    float stackOffset = 141;
    float columnBottomY = 980;
    int colZBase = 20;

    for (int col = 0; col < 4 && !deck.empty(); col++) {
        vector<CardSprite*> colCards;
        for (int row = 0; row < cardsPerCol && !deck.empty(); row++) {
            CardData cd = deck.back();
            deck.pop_back();

            Vec2 pos(startX + col * colGap, columnBottomY + row * stackOffset);
            cd.originPos = pos;

            bool isBottom = (row == 0);
            cd.isOpen = isBottom;

            auto card = CardSprite::create(cd);
            card->setPosition(pos);
            card->setContentSize(Size(cardW, cardH));
            card->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
            card->setOnClicked(isBottom ? function<void(CardSprite*)>(CC_CALLBACK_1(GameScene::onCardClicked, this)) : nullptr);
            addChild(card, colZBase + (cardsPerCol - 1 - row));
            _deskCards.push_back(card);
            colCards.push_back(card);
        }
        for (int r = 0; r < cardsPerCol - 1; r++) {
            colCards[r]->setCoveredCards({ colCards[r + 1] });
        }
    }

    float deckStartX = 120;
    float deckStartY = 330;
    int deckCount = 0;
    for (auto it = deck.rbegin(); it != deck.rend(); ++it) {
        CardData cd = *it;
        cd.isOpen = false;
        Vec2 pos(deckStartX + deckCount * 3, deckStartY);
        cd.originPos = pos;

        auto card = CardSprite::create(cd);
        card->setPosition(pos);
        card->setContentSize(Size(cardW, cardH));
        card->setOnClicked([this](CardSprite*) { onDeckClicked(); });
        addChild(card, 5);
        _deckPile.push_back(card);
        deckCount++;
    }

    refreshUndoButton();
}

// handle clicking a card on the table
void GameScene::onCardClicked(CardSprite* card) {
    if (_isAnimating || _gameOver || _defeatLabel) return;

    auto model = CardModel::getInstance();
    int v1 = card->getData().value;
    int v2 = _baseCard->getData().value;

    if (!model->canMatch(v1, v2)) return;

    _isAnimating = true;
    scheduleOnce([this](float) { _isAnimating = false; }, 2.0f, "anim_timeout");

    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("res/fanpai.mp3");

    Vec2 targetPos = _baseCard->getPosition();
    Vec2 oldPos = card->getPosition();

    card->setOnClicked(nullptr);

    CardData revealedData;
    if (!card->getCoveredCards().empty()) {
        revealedData = card->getCoveredCards().back()->getData();
    }
    card->revealCovered(CC_CALLBACK_1(GameScene::onCardClicked, this));

    model->pushUndoStep(_baseCard->getData(), card->getData(), false, revealedData);

    moveCardTo(card, targetPos, [this, card, oldPos, targetPos]() {
        card->setVisible(false);

        _baseCard->removeFromParentAndCleanup(true);

        CardData cd = card->getData();
        cd.originPos = oldPos;
        _baseCard = CardSprite::create(cd);
        _baseCard->setPosition(targetPos);
        _baseCard->setContentSize(Size(182, 282));
        _baseCard->setOnClicked(nullptr);
        addChild(_baseCard, 10);

        refreshUndoButton();
        checkVictory();
        checkDefeat();
        unschedule("anim_timeout");
        _isAnimating = false;
        });
}

void GameScene::checkVictory() {
    for (auto c : _deskCards) {
        if (c->isVisible()) return;
    }

    _gameOver = true;

    Size size = Director::getInstance()->getWinSize();
    auto label = Label::createWithSystemFont("SUCCESS!", "Arial", 120);
    if (label) {
        label->setTextColor(Color4B(255, 220, 0, 255));
        label->enableOutline(Color4B(100, 80, 0, 255), 6);
        label->setPosition(Vec2(size.width / 2, size.height / 2));
        addChild(label, 1000);
    }
}

void GameScene::checkDefeat() {
    if (!_deckPile.empty()) return;

    bool hasVisible = false;
    for (auto c : _deskCards) {
        if (c->isVisible()) {
            hasVisible = true;
            break;
        }
    }
    if (!hasVisible) return;

    int baseVal = _baseCard->getData().value;
    for (auto c : _deskCards) {
        if (c->isVisible() && CardModel::getInstance()->canMatch(baseVal, c->getData().value)) {
            return;
        }
    }

    if (_defeatLabel) return;

    Size size = Director::getInstance()->getWinSize();
    _defeatLabel = Label::createWithSystemFont("DEFEAT!", "Arial", 120);
    if (_defeatLabel) {
        _defeatLabel->setTextColor(Color4B(0, 0, 0, 255));
        _defeatLabel->enableOutline(Color4B(100, 100, 100, 255), 6);
        _defeatLabel->setPosition(Vec2(size.width / 2, size.height / 2));
        addChild(_defeatLabel, 1000);
    }
}

// draw a card from the deck pile
void GameScene::onDeckClicked() {
    if (_isAnimating || _gameOver || _defeatLabel || _deckPile.empty()) return;
    if (hasAvailableMatch()) return;

    _isAnimating = true;

    auto model = CardModel::getInstance();
    auto drawn = _deckPile.back();
    _deckPile.pop_back();

    model->pushUndoStep(_baseCard->getData(), drawn->getData(), true);

    Vec2 targetPos = _baseCard->getPosition();

    drawn->setOnClicked(nullptr);
    drawn->stopAllActions();

    drawn->flipCardAnimated(true, [this, drawn, targetPos]() {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("res/fanpai.mp3");

        moveCardTo(drawn, targetPos, [this, drawn, targetPos]() {
            drawn->setVisible(false);

            _baseCard->removeFromParentAndCleanup(true);

            CardData cd = drawn->getData();
            _baseCard = CardSprite::create(cd);
            _baseCard->setPosition(targetPos);
            _baseCard->setContentSize(Size(182, 282));
            _baseCard->setOnClicked(nullptr);
            addChild(_baseCard, 10);

            refreshUndoButton();
            checkVictory();
            checkDefeat();
            _isAnimating = false;
            });
        });
}

void GameScene::onUndoClicked(Ref*, Widget::TouchEventType type) {
    if (type != Widget::TouchEventType::ENDED) return;
    if (_isAnimating || _gameOver) return;

    if (_defeatLabel) {
        _defeatLabel->removeFromParentAndCleanup(true);
        _defeatLabel = nullptr;
        _gameOver = false;
    }

    UndoAction* action = CardModel::getInstance()->popUndoAction();
    if (!action) return;

    CardData oldBase = action->getOldBase();
    CardData oldMatched = action->getOldMatched();
    CardData oldRevealed = action->getOldRevealed();
    bool fromDeck = action->isFromDeck();
    delete action;

    _isAnimating = true;

    Vec2 basePos = _baseCard->getPosition();

    if (fromDeck) {
        auto movingCard = _baseCard;
        Vec2 deckTarget = oldMatched.originPos;

        _baseCard = CardSprite::create(oldBase);
        _baseCard->setPosition(basePos);
        _baseCard->setContentSize(Size(182, 282));
        _baseCard->setOnClicked(nullptr);
        addChild(_baseCard, 10);

        movingCard->stopAllActions();
        moveCardTo(movingCard, deckTarget, [this, movingCard]() {
            movingCard->flipCard(false);
            movingCard->setOnClicked([this](CardSprite*) { onDeckClicked(); });
            _deckPile.push_back(movingCard);
            refreshUndoButton();
            _isAnimating = false;
            });
    }
    else {
        _baseCard->removeFromParentAndCleanup(true);

        _baseCard = CardSprite::create(oldBase);
        _baseCard->setPosition(basePos);
        _baseCard->setContentSize(Size(182, 282));
        _baseCard->setOnClicked(nullptr);
        addChild(_baseCard, 10);

        for (auto it = _deskCards.begin(); it != _deskCards.end(); ++it) {
            CardSprite* c = *it;
            if (c->getData().originPos.equals(oldMatched.originPos) && !c->isVisible()) {
                c->setVisible(true);
                c->setData(oldMatched);
                c->flipCard(true);
                c->setOnClicked(CC_CALLBACK_1(GameScene::onCardClicked, this));
                moveCardTo(c, oldMatched.originPos, [this, c, oldRevealed]() {
                     if (oldRevealed.originPos != Vec2::ZERO) {
                         for (auto c2 : _deskCards) {
                             if (c2->getData().originPos.equals(oldRevealed.originPos) && c2->isVisible()) {
                                 c2->flipCard(false);
                                 c2->setOnClicked(nullptr);
                                 c->setCoveredCards({ c2 });
                                 break;
                             }
                         }
                     }
                    refreshUndoButton();
                    _isAnimating = false;
                    });
                return;
            }
        }

        refreshUndoButton();
        _isAnimating = false;
    }
}

void GameScene::onRestartClicked(Ref*, Widget::TouchEventType type) {
    if (type != Widget::TouchEventType::ENDED) return;

    auto scene = GameScene::createScene();
    Director::getInstance()->replaceScene(scene);
}

void GameScene::moveCardTo(CardSprite* card, Vec2 to, function<void()> cb) {
    if (!card) return;
    card->stopAllActions();
    auto move = MoveTo::create(0.3f, to);
    if (cb) {
        auto seq = Sequence::create(move, CallFunc::create(cb), nullptr);
        card->runAction(seq);
    }
    else {
        card->runAction(move);
    }
}

void GameScene::refreshUndoButton() {
    bool canUndo = CardModel::getInstance()->hasUndoStep();
    _undoBtn->setEnabled(canUndo);
    _undoBtn->setBright(canUndo);
}

bool GameScene::hasAvailableMatch() {
    int baseVal = _baseCard->getData().value;
    for (auto c : _deskCards) {
        if (c->isVisible() && c->getData().isOpen &&
            CardModel::getInstance()->canMatch(baseVal, c->getData().value)) {
            return true;
        }
    }
    return false;
}

#ifndef CARD_MODEL_H
#define CARD_MODEL_H

#include "cocos2d.h"
#include <vector>
#include <string>
USING_NS_CC;

using std::vector;
using std::string;

struct CardData {
    CardData() : value(0), isOpen(false), originPos(Vec2::ZERO), originZOrder(0) {}

    string suit;       
    int value;         
    bool isOpen;
    Vec2 originPos;  
    float originZOrder;
};

class UndoAction
{
public:
    virtual ~UndoAction() {}
    virtual bool isFromDeck() const = 0;
    virtual CardData getOldBase() const = 0;
    virtual CardData getOldMatched() const = 0;
    virtual CardData getOldRevealed() const { return CardData(); }
};

class MatchUndoAction : public UndoAction
{
public:
    MatchUndoAction(CardData oldBase, CardData oldMatched, CardData oldRevealed);
    virtual bool isFromDeck() const override { return false; }
    virtual CardData getOldBase() const override { return _oldBase; }
    virtual CardData getOldMatched() const override { return _oldMatched; }
    virtual CardData getOldRevealed() const override { return _oldRevealed; }

private:
    CardData _oldBase;
    CardData _oldMatched;
    CardData _oldRevealed;
};

class DrawUndoAction : public UndoAction
{
public:
    DrawUndoAction(CardData oldBase, CardData oldMatched);
    virtual bool isFromDeck() const override { return true; }
    virtual CardData getOldBase() const override { return _oldBase; }
    virtual CardData getOldMatched() const override { return _oldMatched; }

private:
    CardData _oldBase;
    CardData _oldMatched;
};

class CardModel
{
public:
    static CardModel* getInstance();
    void initData();

    bool canMatch(int a, int b);
    bool canMatch(CardData& a, CardData& b);

    void pushUndoStep(CardData top, CardData desk, bool fromDeck, CardData revealed = CardData());
    UndoAction* popUndoAction();
    bool hasUndoStep();

    vector<CardData> generateDeck();
    vector<CardData> shuffle(vector<CardData> deck);

private:
    static CardModel* _instance;
    vector<UndoAction*> _undoStack;

    CardModel() {}
    void clearUndoSteps();
};

#endif

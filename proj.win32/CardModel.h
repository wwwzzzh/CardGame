#ifndef CARD_MODEL_H
#define CARD_MODEL_H

#include "cocos2d.h"
#include <vector>
#include <string>
USING_NS_CC;

using std::vector;
using std::string;

struct CardData {
    string suit;       
    int value;         
    bool isOpen;
    Vec2 originPos;  
    float originZOrder;
};

struct UndoStep {
    CardData oldBase;
    CardData oldMatched;
    CardData oldRevealed;
    bool fromDeck;
};

class CardModel
{
public:
    static CardModel* getInstance();
    void initData();

    bool canMatch(int a, int b);
    bool canMatch(CardData& a, CardData& b);

    void pushUndoStep(CardData top, CardData desk, bool fromDeck, CardData revealed = CardData());
    bool popUndoStep(CardData& outTop, CardData& outDesk, bool& outFromDeck, CardData& outRevealed);
    bool hasUndoStep();

    vector<CardData> generateDeck();
    vector<CardData> shuffle(vector<CardData> deck);

private:
    static CardModel* _instance;
    vector<UndoStep> _undoStack;

    CardModel() {}
};

#endif

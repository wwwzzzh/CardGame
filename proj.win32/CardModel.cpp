#include "CardModel.h"
#include <random>
#include <algorithm>

using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;
using std::swap;

CardModel* CardModel::_instance = nullptr;

CardModel* CardModel::getInstance() {
    if (!_instance) _instance = new CardModel();
    return _instance;
}

void CardModel::initData() {
    _undoStack.clear();
}

bool CardModel::canMatch(int a, int b) {
    return abs(a - b) == 1;
}

bool CardModel::canMatch(CardData& a, CardData& b) {
    return canMatch(a.value, b.value);
}

void CardModel::pushUndoStep(CardData top, CardData desk, bool fromDeck, CardData revealed) {
    _undoStack.push_back({ top, desk, revealed, fromDeck });
}

bool CardModel::popUndoStep(CardData& outTop, CardData& outDesk, bool& outFromDeck, CardData& outRevealed) {
    if (_undoStack.empty()) return false;
    UndoStep step = _undoStack.back();
    _undoStack.pop_back();
    outTop = step.oldBase;
    outDesk = step.oldMatched;
    outFromDeck = step.fromDeck;
    outRevealed = step.oldRevealed;
    return true;
}

bool CardModel::hasUndoStep() {
    return !_undoStack.empty();
}

vector<CardData> CardModel::generateDeck() {
    vector<CardData> deck;
    vector<string> suits = { "heart", "diamond", "club", "spade" };
    for (auto& suit : suits) {
        for (int v = 1; v <= 13; v++) {
            CardData cd;
            cd.suit = suit;
            cd.value = v;
            cd.isOpen = false;
            cd.originPos = Vec2::ZERO;
            cd.originZOrder = 0;
            deck.push_back(cd);
        }
    }
    return deck;
}


vector<CardData> CardModel::shuffle(vector<CardData> deck) {
    random_device rd;
    mt19937 g(rd());
    for (int i = (int)deck.size() - 1; i > 0; i--) {
        uniform_int_distribution<int> dist(0, i);
        int j = dist(g);
        swap(deck[i], deck[j]);
    }
    return deck;
}

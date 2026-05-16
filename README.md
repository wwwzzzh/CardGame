# CardGame - Card Matching Game

A card matching game built with Cocos2d-x 3.17.

---

## Environment

- Cocos2d-x 3.17
- Visual Studio 2022
- Resolution 1080x2080 portrait

## How to Play

When the game starts, there is one face-up base card at the bottom of the screen. Above it there are 4 columns of stacked cards, each column has 4 cards. Only the bottom card of each column is face-up and clickable.

Click a face-up card. If its number differs from the base card by exactly 1 (e.g. base is 7, click 6 or 8), the match succeeds. The clicked card moves to the base position and replaces it. Suit does not matter — only the number counts.

Each column works like this: the bottom face-up card sits on top of 3 face-down cards. When you remove the face-up card, the card above it flips over automatically.

If none of the face-up cards on the table can match the base, click the deck pile on the left to draw a new card. (You can only draw from the deck when no match is available on the table.)

### End Conditions

- All 16 cards cleared -> yellow SUCCESS! appears in the center (no further actions allowed)
- Deck is empty and the base card cannot match any remaining card -> black DEFEAT!
  - You can click UNDO after DEFEAT to continue playing.

### Controls

| Action | Result |
|--------|--------|
| Click a face-up card | Match with base card |
| Click the deck pile | Draw a new card (only when no match available on table) |
| Click UNDO | Undo last move |
| Click RESTART | Restart the game |

---

## Project Structure

```
proj.win32/
├── CardGame.sln
├── CardGame.vcxproj
├── main.cpp
├── AppDelegate.cpp/h
├── GameScene.cpp/h
├── CardModel.cpp/h
├── CardSprite.cpp/h
Resources/
└── res/
```

Simple MVC structure:

- CardModel manages data and rules, no UI code
- CardSprite handles card visuals, no game logic
- GameScene connects the two, handles clicks and animations

### Class Hierarchy

```
Sprite (cocos2d)
  └── CardSprite               ← base: touch, flip animation, cover relations
         │  virtual: loadFace()
         │
         └── NormalCardSprite  ← normal cards: renders number and suit
```

### Extensibility

To add a new card type (e.g. wildcard), just inherit from CardSprite and override loadFace(). No other files need changing.

---

## Implementation Details

### Flip Animation

When drawing from the deck, the card flips in place first, then moves:

```
ScaleTo(0.15s, shrink) -> switch face -> ScaleTo(0.15s, expand) -> MoveTo(0.3s, target)
```

### Undo

Every operation pushes the previous state onto a stack. Undo pops and restores. Two cases:

- Card from table -> find its original position, move it back, flip face-up
- Card from deck -> move back to deck pile, flip face-down

UNDO is also available after DEFEAT to continue the game.

### Anti-Race-Condition

An _isAnimating lock blocks clicks during animations. A 2-second timeout prevents permanent lock.

### Restart

Clicking the RESTART button at any time creates a brand new GameScene to replace the current one, resetting all states and starting a fresh game.

### Deck Drawing Rule

Drawing from the deck is only allowed when no visible card on the table can match the current base card. This prevents players from skipping matchable cards and adds strategic depth.

---

## Build

Open proj.win32/CardGame.sln, press F7 to build, F5 to run.

---

May 2026

#pragma once

#include "state.cpp"
#include "GUI/Game/game.cpp"

class GameState : public State {

public:

    GameState(sf::RenderWindow*, int, bool);
    ~GameState();

    void updateState();
    void drawState(sf::RenderTarget*);
    void reset();

private:
    Game* game;

    Button* gameEndButton;
};
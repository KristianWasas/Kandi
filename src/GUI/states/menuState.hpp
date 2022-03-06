#pragma once

#include "state.cpp"

class MenuState : public State {

public:

    MenuState(sf::RenderWindow*);
    ~MenuState();

    void updateState();
    void drawState(sf::RenderTarget*);
    void reset();

    int difficulty = 1;
    float timer = 0;
    sf::Clock clock;

private:

    Button* startGameButt;
    Button* easyButton;
    Button* hardButton;
    Button* colorButton;
    Button* testButton;

    float buttonTime = 0;

    sf::Text title;
    sf::Text diff;
};
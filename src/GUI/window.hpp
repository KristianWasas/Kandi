#pragma once

#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"

#include "states/gameState.cpp"
#include "states/menuState.cpp"
#include "states/testState.cpp"
#include "states/state.cpp"

using namespace std;

class Window{

public:

    Window();
    ~Window();

    void update();
    void draw();
    void run();
    void newCurrentState();

private:

    sf::Event eventSFML;
    sf::RenderWindow* window;

    MenuState* menu;
    GameState* game;
    TestState* test;

    State* currentState;

    void initializeStates();
    void initializeWindow();
};
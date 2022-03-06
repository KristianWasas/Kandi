#pragma once

#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"

#include "GUI/button.cpp"

using namespace std;

class State{

public:

    State(sf::RenderWindow*);
    virtual ~State(){};
    
    virtual void reset() = 0;
    virtual void drawState(sf::RenderTarget*) = 0;
    virtual void updateState() = 0;
    void updateMousePos();

    bool goNextState = false;
    int nextStateNum;
    bool aiColor;

protected:

    sf::Vector2i mousePos;
    sf::RenderWindow* window;
    sf::Font font;

private:

};
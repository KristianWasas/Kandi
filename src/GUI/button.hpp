#pragma once

#include <iostream>
#include <ctime>
#include <stack>

#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "SFML/System.hpp"

class Button{

public:

    Button(float x, float y, float width, float height, std::string text, int fontSize, sf::Color passiveColour, sf::Color hoverColour, sf::Color activeColouw);
    ~Button();

    void updateButton(const sf::Vector2i);
    void updateText(std::string);
    void setColors(sf::Color);

    void drawButton(sf::RenderTarget*);

    bool activated = false;
    bool rightActivated = false;
    bool hover = false;

private:

    short unsigned buttonCase;    

    sf::RectangleShape shape;
    sf::Font font;
    sf::Text buttonText;
    int fontSize;

    sf::Texture texture;
    sf::Sprite gameTileSprite;

    sf::Color passiveColour;
    sf::Color hoverColour;
    sf::Color activeColour; 
    
};
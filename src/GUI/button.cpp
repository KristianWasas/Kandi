#pragma once

#include "button.hpp"

enum button_cases{BTN_PASSIVE = 0, BTN_HOVER, BTN_ACTIVE, BTN_ACTIVE_RIGHT};

Button::Button(float x, float y, float width, float height, std::string inputText, int fontSize, sf::Color passiveColour, sf::Color hoverColour, sf::Color activeColour){
    
    if(!font.loadFromFile("fonts/Montserrat-Regular.ttf")){
        std::cout << "ERROR FONT FILE NOT FOUND BUTTON" << std::endl;
    }

    this->shape.setPosition(sf::Vector2f(x, y));
    this->shape.setSize(sf::Vector2f(width, height));

    
    this->buttonText.setFont(this->font);
    this->buttonText.setString(inputText);
    this->buttonText.setCharacterSize(fontSize);
    this->buttonText.setFillColor(sf::Color::White);
    this->buttonText.setOutlineColor(sf::Color::Black);
    this->buttonText.setPosition(
            this->shape.getPosition().x + (this->shape.getGlobalBounds().width/2.f) - this->buttonText.getGlobalBounds().width/2.f,
            this->shape.getPosition().y + (this->shape.getGlobalBounds().height/2.f) - this->buttonText.getGlobalBounds().height/2.f
    );

    this->passiveColour = passiveColour;
    this->hoverColour = hoverColour;
    this->activeColour = activeColour;

}
    
Button::~Button(){}

void Button::updateText(std::string inputString){
    buttonText.setString(inputString);
    buttonText.setPosition(this->shape.getPosition().x + (this->shape.getGlobalBounds().width/2.f) - this->buttonText.getGlobalBounds().width/2.f,
                           this->shape.getPosition().y + (this->shape.getGlobalBounds().height/2.f) - this->buttonText.getGlobalBounds().height/2.f);
}

void Button::setColors(sf::Color color){
    this->passiveColour = color;
    this->hoverColour = color;
    this->activeColour = color;
}

void Button::updateButton(const sf::Vector2i mousePos){

    this->buttonCase = BTN_PASSIVE;

    if(this->shape.getGlobalBounds().contains(sf::Vector2f(mousePos.x, mousePos.y))){
        this->buttonCase = BTN_HOVER;
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            this->buttonCase = BTN_ACTIVE;
        }else if(sf::Mouse::isButtonPressed(sf::Mouse::Right)){
            this->buttonCase = BTN_ACTIVE_RIGHT;
        }
    }

    switch (this->buttonCase){

        case BTN_PASSIVE:
            this->shape.setFillColor(this->passiveColour);
            this->activated = false;
            this->rightActivated = false;
            this->hover = false;
            break;
        case BTN_HOVER:
            this->shape.setFillColor(this->hoverColour);
            this->hover = true;
            this->activated = false;
            this->rightActivated = false;
            break;
        case BTN_ACTIVE:
            this->shape.setFillColor(this->activeColour);
            this->activated = true;
            this->rightActivated = false;
            break;
        case BTN_ACTIVE_RIGHT:
            this->shape.setFillColor(this->activeColour);
            this->rightActivated = true;
            this->activated = false;
            break;
        default:
            this->shape.setFillColor(sf::Color::Blue);
            this->activated = false;
            this->rightActivated = false;
            this->hover = false;
            break;
    
    }

}
    
void Button::drawButton(sf::RenderTarget* target){

    target->draw(this->shape);
    target->draw(this->buttonText);

}

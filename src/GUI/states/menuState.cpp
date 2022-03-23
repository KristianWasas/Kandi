#pragma once

#include "menuState.hpp"

MenuState::MenuState(sf::RenderWindow* win) : State(win){
    
    startGameButt = new Button(560/2-100, 165, 200, 50, "Start New Game", 16, sf::Color::Red, sf::Color::Green, sf::Color::Green);
    easyButton = new Button(560/2-100, 400, 60, 60, "Down", 16, sf::Color::Red, sf::Color::Green, sf::Color::Green);
    hardButton = new Button(560/2+50, 400, 60, 60, "Up", 16, sf::Color::Red, sf::Color::Green, sf::Color::Green);
    colorButton = new Button(560/2-75, 250, 150, 50, "Play as W", 16, sf::Color::Red, sf::Color::Red, sf::Color::Red);
    testButton = new Button(560/2-50, 485, 100, 50, "Test", 16, sf::Color::Red, sf::Color::Green, sf::Color::Green);
    
    title.setFont(font);
    title.setString("Chess");
    title.setFillColor(sf::Color::White);
    title.setCharacterSize(50);
    title.setPosition(560/2-title.getGlobalBounds().width/2, 80);
    
    diff.setFont(font);
    diff.setFillColor(sf::Color::White);
    diff.setCharacterSize(30);
    diff.setString("Difficulty = 1");
    diff.setPosition(560/2-diff.getGlobalBounds().width/2, 330);
}

MenuState::~MenuState(){
    delete startGameButt;
    delete easyButton;
    delete hardButton;
    delete colorButton;
    delete testButton;
}

void MenuState::reset(){
    goNextState = false;
    difficulty = 1;
    timer = 0;
}

void MenuState::updateState(){

    float dt = clock.restart().asSeconds();
    timer = timer + dt;
    buttonTime = buttonTime + dt;

    updateMousePos();
    startGameButt->updateButton(mousePos);
    easyButton->updateButton(mousePos);
    hardButton->updateButton(mousePos);
    colorButton->updateButton(mousePos);
    testButton->updateButton(mousePos);

    if(startGameButt->activated && timer > 0.6){
        goNextState = true;
        nextStateNum = difficulty;
    }
    if(hardButton->activated && buttonTime > 0.25){
        difficulty = min(5, difficulty+1);
        buttonTime = 0;
    }
    if(easyButton->activated && buttonTime > 0.25){
        difficulty = max(1, difficulty-1);
        buttonTime = 0;
    }
    if(colorButton->activated && buttonTime > 0.25){
        aiColor = !aiColor;
        buttonTime = 0;
    }
    if(testButton->activated && timer > 0.6){
        goNextState = true;
        nextStateNum = -1;
    }
}

void MenuState::drawState(sf::RenderTarget* target){

    startGameButt->drawButton(target);
    easyButton->drawButton(target);
    hardButton->drawButton(target);
    testButton->drawButton(target);
    
    string a;
    if(!aiColor){
        a = "Play as W";
    }else{
        a = "Play as B";
    }
    colorButton->updateText(a);
    colorButton->drawButton(target);

    target->draw(title);
    diff.setString("Difficulty: " + to_string(difficulty));
    target->draw(diff);
}
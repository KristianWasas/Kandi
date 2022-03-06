#pragma once

#include "window.hpp"

using namespace std;

void Window::initializeWindow(){
    window = new sf::RenderWindow(sf::VideoMode(560, 560), "Chess");
    window->setFramerateLimit(60);
}

void Window::initializeStates(){
    menu = new MenuState(window);
    game = new GameState(window, 0, false);
    test = new TestState(window);
    currentState = menu;
}

Window::Window(){
    initializeWindow();
    initializeStates();
}

Window::~Window(){
    delete menu;
    delete game;
    delete test;
    delete window;
}

void Window::newCurrentState(){
    currentState->reset();
    if(currentState->nextStateNum == 0){
        menu->timer = 0;
        menu->clock.restart();
        currentState = menu;
    }else if(currentState->nextStateNum == -1){
        test->timer = 0;
        test->clock.restart();
        currentState = test;
    }    
    else{
        delete game;
        game = new GameState(window, currentState->nextStateNum, currentState->aiColor);
        currentState = game;
    }
}

void Window::update(){
    while(window->pollEvent(eventSFML)){ 
        if(eventSFML.type == sf::Event::Closed){
            window->close();
        }
    }

    currentState->updateState();

    if(currentState->goNextState){
        newCurrentState();
    }
}

void Window::draw(){
    window->clear();
    currentState->drawState(window);
    window->display();
}

void Window::run(){
    while(window->isOpen()){
        update();
        draw();
    }
}
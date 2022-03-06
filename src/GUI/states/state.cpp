#pragma once

#include <iostream>

#include "state.hpp"

State::State(sf::RenderWindow* win){
    window = win;
    if(!font.loadFromFile("fonts/Montserrat-Regular.ttf")){
        std::cout << "ERROR FONT FILE NOT FOUND" << "\n";
    }
    aiColor = false;
}

void State::updateMousePos(){
    mousePos = sf::Mouse::getPosition(*window);
}
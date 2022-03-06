#pragma once

#include "gameState.hpp"

GameState::GameState(sf::RenderWindow* win, int difficulty, bool ai) : State(win){
    game = new Game(difficulty, ai);
    gameEndButton = new Button(560/2-200, 560/2-110, 400, 150, " ", 30, sf::Color::Green, sf::Color::Green, sf::Color::Green);
}

GameState::~GameState(){
    delete game;
    delete gameEndButton;
}

void GameState::reset(){}

void GameState::updateState(){
    updateMousePos();
    if(!game->gameEnd){
        game->update(mousePos);
    }else{
        gameEndButton->updateButton(mousePos);
        if(gameEndButton->activated){
            goNextState = true;
            nextStateNum = 0;
        }
    }
}

void GameState::drawState(sf::RenderTarget* target){
    game->drawGame(target);
    if(game->gameEnd){
        string i;
        if(game->whiteWon){
            gameEndButton->setColors(sf::Color::Green);
            i = "Checkmate White Won!\n       Press for menu";
        }else{
            gameEndButton->setColors(sf::Color::Red);
            i = "Checkmate Black Won!\n       Press for menu";
        }
        if(game->staleMate){
            gameEndButton->setColors(sf::Color::Blue);
            i = "    Stalemate!\nPress for menu";
        }
        gameEndButton->updateText(i);
        gameEndButton->drawButton(target);
    }
}
#pragma once

#include "piece.hpp"

using namespace std;

Piece::Piece(string name, pair<int, int> position){
    if(!pieceTexture.loadFromFile("images/" + name + ".png")){
        cout << "ERROR LOADING TEXTURE" << "\n";
    }
    pieceName = name;
    pieceSprite.setTexture(pieceTexture);
    if(name[0] == 'w'){
        colour = true;
    }else{
        colour = false;
    }
    positionOnBoard = position;
}

Piece::~Piece(){}

void Piece::setBoardPos(pair<int, int> position){
    positionOnBoard = position;
}

pair<int, int> Piece::getPos(){
    return positionOnBoard;
}

void Piece::updateMousePos(sf::Vector2i mousePosition){
    mousePos = mousePosition;
}

void Piece::setSprite(string name){
    if(!pieceTexture.loadFromFile("images/" + name + ".png")){
        cout << "ERROR LOADING TEXTURE" << "\n";
    }
    pieceSprite.setTexture(pieceTexture);
}

void Piece::drawPiece(sf::RenderTarget* target){
    if(!grabbed){
        pieceSprite.setPosition(positionOnBoard.second * 70 + 3, positionOnBoard.first * 70 + 3);
    }else{
        pieceSprite.setPosition(mousePos.x - 32, mousePos.y -32);
    }
    target->draw(pieceSprite);
}
#pragma once

#include <vector>
#include <iostream>
#include <map>

#include "SFML/Graphics.hpp"

using namespace std;

class Piece{

public:

    Piece(string, pair<int, int>);
    ~Piece();

    pair<int, int> getPos();
    int getType();

    void setSprite(string);
    void setBoardPos(pair<int, int>);

    void updateMousePos(sf::Vector2i);
    void drawPiece(sf::RenderTarget*);

    bool grabbed = false;
    bool colour;        //White is true
    string pieceName;
private:

    pair<int, int> positionOnBoard;

    sf::Texture pieceTexture;
    sf::Sprite pieceSprite;
    sf::Vector2i mousePos;
};
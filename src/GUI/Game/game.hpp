#pragma once

#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"

#include "piece.cpp"
#include "backend/bitboard.hpp"
#include "backend/movegenerator.hpp"
#include "backend/chessAI.hpp"

using namespace std;

class Game{

public:

    Game(int, bool);
    ~Game();

    void update(sf::Vector2i);
    void drawGame(sf::RenderTarget*);

    bool whiteWon;
    bool staleMate = false;
    bool gameEnd = false;

private:

    bool whiteTurn;

    bool highLight = false;
    float highLightTime = 0;
    int highLightSquare;
    sf::RectangleShape highLightRect;

    int turnCount = -1;

    bool aiColour;

    bool hasPossibleMovesUpdated = false;

    int diff;
    
    sf::Texture boardGraphic;
    sf::Sprite boardGraphicSprite;

    bitBoard board;

    vector<bitBoard> allPossibleMoves;

    vector<Piece*> pieces;                        
    Piece* grabbedPiece;                            
    bool hasPieceGrabbed = false;                   
    sf::Vector2i cordsForGrabbedPiece;              

    bool mousePressed = false;
    sf::Vector2i mousePos;
    sf::Vector2i mousePosGrid;
    sf::RenderTarget* target;

    sf::Clock clock;
};
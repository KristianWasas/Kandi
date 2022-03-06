#pragma once

#include "state.cpp"
#include "backend/bitboard.hpp"
#include "testing/tests.hpp"

class TestState : public State {

public:

    TestState(sf::RenderWindow*);
    ~TestState();

    void updateState();
    void drawState(sf::RenderTarget*);
    void reset();

    float timer = 0;
    sf::Clock clock;

private:

    float buttonTimer = 0;

    Button* loadPosition;
    Button* perftPosition;
    Button* testAll;
    Button* menuButton;
    Button* depthMinus;
    Button* depthAdd;

    sf::Text currentDepth;
    int currentDepthValue = 1;
    string FENposition = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";   //Kiwipete for debugging
    bitBoard position = FENtoBitBoard(FENposition);

};
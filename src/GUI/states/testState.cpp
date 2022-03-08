#pragma once 

#include "testState.hpp"

TestState::TestState(sf::RenderWindow* win) : State(win){
    initializeZValues();            //Initialize zobrist hash values
    loadPosition = new Button(560/2-100, 100, 200, 50, "Load position", 16, sf::Color::Red, sf::Color::Green, sf::Color::Green);
    perftPosition = new Button(560/2-100, 200, 200, 50, "Perft position", 16, sf::Color::Red, sf::Color::Green, sf::Color::Green);
    testAll = new Button(560/2-100, 300, 200, 50, "Test all", 16, sf::Color::Red, sf::Color::Green, sf::Color::Green);
    menuButton = new Button(560/2-100, 400, 200, 50, "Back to menu", 16, sf ::Color::Red, sf::Color::Green, sf::Color::Green);
    depthMinus = new Button(560/2-150, 20, 50, 50, "-", 16, sf::Color::Red, sf::Color::Green, sf::Color::Green);
    depthAdd = new Button(560/2+100, 20, 50, 50, "+", 16, sf::Color::Red, sf::Color::Green, sf::Color::Green);
    currentDepth.setFont(font);
    currentDepth.setString("Depth 1");
    currentDepth.setFillColor(sf::Color::White);
    currentDepth.setCharacterSize(30);
    currentDepth.setPosition(560/2-currentDepth.getGlobalBounds().width/2, 20);
}

TestState::~TestState(){
    delete loadPosition;
    delete perftPosition;
    delete testAll;
    delete menuButton;
}

void TestState::reset(){
    timer = 0;
    goNextState = false;
}

void TestState::updateState(){

    float dt = clock.restart().asSeconds();
    timer = timer + dt;
    buttonTimer = buttonTimer + dt;

    updateMousePos();
    loadPosition->updateButton(mousePos);
    perftPosition->updateButton(mousePos);
    testAll->updateButton(mousePos);
    menuButton->updateButton(mousePos);
    depthAdd->updateButton(mousePos);
    depthMinus->updateButton(mousePos);

    if(loadPosition->activated){
        string input;
        std::cout << "Input a position in Fen notation and hit enter\n";
        std::getline(cin, input); 
        FENposition = input;
        position = FENtoBitBoard(FENposition);
        drawBoard(position);
        cout << "\n";
    }
    if(perftPosition->activated && buttonTimer > 0.5){
        
        #if PERFT_DEBUG    //For debugging
            resetTestData();
            vector<bitBoard> a;
            generateWhiteMoves(position, a);
            for(auto i : a){
                drawMove(position, i, true);
                cout << perft(i, 0, currentDepthValue-1, false) << endl;
            }
        #endif
    
        cout << table.count(0LL) << "\n";

        resetTestData();
        std::cout << "Position: " << FENposition <<"\n";
        drawBoard(position);
        std::cout << "Running perft algorithm to depth: " << currentDepthValue << "\n";
        perft(position, 0, currentDepthValue, true);
        printOutData();
        cout << "\n";
        buttonTimer = 0;
    }
    if(testAll->activated && buttonTimer > 0.5){
        testAllminimax();
        buttonTimer = 0;
    }
    if(menuButton->activated && timer > 0.6){
        goNextState = true;
        nextStateNum = 0;
    }
    if(depthAdd->activated && buttonTimer >0.25){
        currentDepthValue += 1;
        string text = "Depth " + to_string(currentDepthValue);
        currentDepth.setString(text);
        buttonTimer = 0;
    }
    if(depthMinus->activated && buttonTimer >0.25){
        currentDepthValue = max(0, currentDepthValue-1);
        string text = "Depth " + to_string(currentDepthValue);
        currentDepth.setString(text);
        buttonTimer = 0;
    }

}

void TestState::drawState(sf::RenderTarget* target){

    loadPosition->drawButton(target);
    perftPosition->drawButton(target);
    testAll->drawButton(target);
    menuButton->drawButton(target);
    depthAdd->drawButton(target);
    depthMinus->drawButton(target);
    target->draw(currentDepth);
}
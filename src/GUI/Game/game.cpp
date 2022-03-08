#pragma once 

#include "game.hpp"

using namespace std;

Game::Game(int difficulty, bool aiCol){

    if(!boardGraphic.loadFromFile("images/chess_board.png")){
        cout << "ERROR FILE NOT FOUND" << "\n";
    }
    boardGraphicSprite.setTexture(boardGraphic);

    highLightRect.setFillColor(sf::Color::Red);
    highLightRect.setSize(sf::Vector2f(70, 70));

    diff = difficulty;
    aiColour = aiCol;

    board = FENtoBitBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    createPieces(board, pieces);

    //Initialize zobrist hash values 
    initializeZValues();
}

Game::~Game(){
    for(auto i : pieces){
        delete i;
    }
}

void Game::update(sf::Vector2i mouse){
    //Escape to menu
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)){
        gameEnd = true;
    }

    //updates all possible moves that can be made in a position, also increments the turn counter.
    if(!hasPossibleMovesUpdated){
        hasPossibleMovesUpdated = true;
        turnCount += 1;
        allPossibleMoves.clear();
        if(whiteTurn){
            generateWhiteMoves(board, allPossibleMoves);
        }else{
            generateBlackMoves(board, allPossibleMoves);
        }
    }
    mousePos = mouse;
    mousePosGrid = sf::Vector2i(mousePos.x/70, mousePos.y/70);

    highLightTime += clock.restart().asSeconds();

    //If there are no moves, the game ends, all possible outcomes checked
    if(allPossibleMoves.size() == 0){
        uint64_t whitePieces = board.wP|board.wB|board.wH|board.wR|board.wQ|board.wK;
        uint64_t blackPieces = board.bP|board.bB|board.bH|board.bR|board.bQ|board.bK;
        uint64_t allPieces = blackPieces|whitePieces;
        uint64_t unsafeForBlack = unSafeBlack(board.wK, board.wQ, board.wB, board.wH, board.wR, board.wP, board.bK, allPieces);
        uint64_t unsafeForWhite = unSafeWhite(board.bK, board.bQ, board.bB, board.bH, board.bR, board.bP, board.wK, allPieces);
        gameEnd = true;
        if(whiteTurn){
            if((unsafeForWhite & board.wK) != 0){
                whiteWon = false;
            }else{
                staleMate = true;
            }
        }else{
            if((unsafeForBlack & board.bK) != 0){
                whiteWon = true;
            }else{
                staleMate = true;
            }
        }
    }else{
        if(!hasPossibleMovesUpdated && !aiColour){
            cout << "\nYour turn!" << "\n";
            hasPossibleMovesUpdated = true;
        }
    }

    //Human moves
    //First we try to grab a piece
    if(!gameEnd && !hasPieceGrabbed){
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            for(Piece* i : pieces){
                if((i->getPos().first == mousePosGrid.y) && (i->getPos().second == mousePosGrid.x)){
                    if(whiteTurn == i->colour){
                        grabbedPiece = i;
                        grabbedPiece->grabbed = true;
                        hasPieceGrabbed = true;
                        cordsForGrabbedPiece = mousePosGrid;
                    }
                }
            }
        }
    }

    //AI moves, human moves continues after this
    if(!gameEnd && (aiColour == whiteTurn)){ 
        sf::Clock time;
        //Place holder variables for highlighting moves piece
        bitBoard placeHolder = board;
        uint64_t placeholderWhitePieces = board.wP|board.wH|board.wB|board.wR|board.wQ|board.wK;
        uint64_t placeholderBlackPieces = board.bP|board.bH|board.bB|board.bR|board.bQ|board.bK;

        time.restart();
        board = chessAI(board, diff, whiteTurn);
        cout << "The AI used " << time.restart().asSeconds() << " seconds to move\n";

        for(Piece* i : pieces){
            delete i;
        }
        hasPossibleMovesUpdated = false;
        pieces.clear();
        createPieces(board, pieces);

        //Variables for highlighting
        uint64_t whitePieces = board.wP|board.wH|board.wB|board.wR|board.wQ|board.wK;
        uint64_t blackPieces =board.bP|board.bH|board.bB|board.bR|board.bQ|board.bK;

        if(!whiteTurn){               
            highLightSquare = __builtin_ctzll((blackPieces^placeholderBlackPieces) & blackPieces);
            highLight = true;
            highLightTime = 0;
        }else{
            highLightSquare = __builtin_ctzll((whitePieces^placeholderWhitePieces) & whitePieces);
            highLight = true;
            highLightTime = 0;
        }
        whiteTurn = !whiteTurn;
    }
    
    //If we have a grabbed piece, we then make the move
    if(hasPieceGrabbed){
        grabbedPiece->updateMousePos(mousePos);
        bool legalMove = false;
        //First we simulate the proposed move on a placeholder board, if legal, we update it to be the current board
        if(!sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            bitBoard placeHolderBoard = board;
            uint64_t whitePieces = board.wP|board.wB|board.wH|board.wR|board.wQ|board.wK;
            uint64_t blackPieces = board.bP|board.bB|board.bH|board.bR|board.bQ|board.bK;
            uint64_t allPieces = blackPieces|whitePieces;
            uint64_t unsafeForBlack = unSafeBlack(board.wK, board.wQ, board.wB, board.wH, board.wR, board.wP, board.bK, allPieces);
            uint64_t unsafeForWhite = unSafeWhite(board.bK, board.bQ, board.bB, board.bH, board.bR, board.bP, board.wK, allPieces);
            int originalSquare = cordsForGrabbedPiece.x + cordsForGrabbedPiece.y*8;
            int targetSquare = mousePosGrid.x + mousePosGrid.y*8;
            //Erase own ep pawns
            if(!whiteTurn){
                placeHolderBoard.eP &= 1095216660480LL;
            }else{
                placeHolderBoard.eP &= 4278190080LL;
            }
            //Erase the original square
            placeHolderBoard.wP &= ~(1LL<<originalSquare);
            placeHolderBoard.wH &= ~(1LL<<originalSquare);
            placeHolderBoard.wB &= ~(1LL<<originalSquare);
            placeHolderBoard.wR &= ~(1LL<<originalSquare);
            placeHolderBoard.wQ &= ~(1LL<<originalSquare);
            placeHolderBoard.wK &= ~(1LL<<originalSquare);
            placeHolderBoard.bP &= ~(1LL<<originalSquare);
            placeHolderBoard.bH &= ~(1LL<<originalSquare);
            placeHolderBoard.bB &= ~(1LL<<originalSquare);
            placeHolderBoard.bR &= ~(1LL<<originalSquare);
            placeHolderBoard.bQ &= ~(1LL<<originalSquare);
            placeHolderBoard.bK &= ~(1LL<<originalSquare);
            //Erase the target square
            placeHolderBoard.wP &= ~(1LL<<targetSquare);
            placeHolderBoard.wH &= ~(1LL<<targetSquare);
            placeHolderBoard.wB &= ~(1LL<<targetSquare);
            placeHolderBoard.wR &= ~(1LL<<targetSquare);
            placeHolderBoard.wQ &= ~(1LL<<targetSquare);
            placeHolderBoard.wK &= ~(1LL<<targetSquare);
            placeHolderBoard.bP &= ~(1LL<<targetSquare);
            placeHolderBoard.bH &= ~(1LL<<targetSquare);
            placeHolderBoard.bB &= ~(1LL<<targetSquare);
            placeHolderBoard.bR &= ~(1LL<<targetSquare);
            placeHolderBoard.bQ &= ~(1LL<<targetSquare);
            placeHolderBoard.bK &= ~(1LL<<targetSquare);
            //Simulate the move, move is just simulated by dropping the piece that is grabbed on the hovered square,
            //promotion, enpassant, double push and castling are special cases, which are all handled accordingly
            if(grabbedPiece->pieceName == "w_pawn"){
                //Promotion special case, 
                if((cordsForGrabbedPiece.y == 1) && (mousePosGrid.y == 0)){
                    if(((abs(mousePosGrid.x-cordsForGrabbedPiece.x) == 1) && ((blackPieces & 1LL<<targetSquare) != 0)) || ((mousePosGrid.x == cordsForGrabbedPiece.x) && (~blackPieces & 1LL<<targetSquare))){
                        clock.restart();
                        while(true){
                            if(sf::Keyboard::isKeyPressed(sf::Keyboard::K)){
                                placeHolderBoard.wH |= 1LL<<targetSquare;
                                break;
                            }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q)){
                                placeHolderBoard.wQ |= 1LL<<targetSquare;
                                break;
                            }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::R)){
                                placeHolderBoard.wR |= 1LL<<targetSquare;
                                break;
                            }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::B)){
                                placeHolderBoard.wB |= 1LL<<targetSquare;
                                break;
                            }else if(clock.getElapsedTime().asSeconds() > 10){
                                break;
                            }
                        }
                    }
                //En passant
                }else if(placeHolderBoard.eP & (1LL<<(originalSquare-1))){
                    placeHolderBoard.wP |= 1LL<<targetSquare;
                    placeHolderBoard.bP &= ~(1LL<<(targetSquare+8));
                }else if(placeHolderBoard.eP & (1LL<<(originalSquare+1))){
                    placeHolderBoard.wP |= 1LL<<targetSquare;
                    placeHolderBoard.bP &= ~(1LL<<(targetSquare+8));
                //double push
                }else if(cordsForGrabbedPiece.y-mousePosGrid.y == 2){
                    placeHolderBoard.wP |= 1LL<<targetSquare;
                    placeHolderBoard.eP = 1LL<<targetSquare;
                //normal move
                }else{
                    placeHolderBoard.wP |= 1LL<<targetSquare;
                }
            }else if(grabbedPiece->pieceName == "b_pawn"){
                //Promotion special case, 
                if((cordsForGrabbedPiece.y == 6) && (mousePosGrid.y == 7)){
                    if(((abs(mousePosGrid.x-cordsForGrabbedPiece.x) == 1) && ((whitePieces & 1LL<<targetSquare) != 0)) || ((mousePosGrid.x == cordsForGrabbedPiece.x) && (~whitePieces & 1LL<<targetSquare))){
                        clock.restart();
                        while(true){
                            if(sf::Keyboard::isKeyPressed(sf::Keyboard::K)){
                                placeHolderBoard.bH |= 1LL<<targetSquare;
                                break;
                            }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q)){
                                placeHolderBoard.bQ |= 1LL<<targetSquare;
                                break;
                            }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::R)){
                                placeHolderBoard.bR |= 1LL<<targetSquare;
                                break;
                            }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::B)){
                                placeHolderBoard.bB |= 1LL<<targetSquare;
                                break;
                            }else if(clock.getElapsedTime().asSeconds() > 10){
                                break;
                            }
                        }
                    }
                //En passant
                }else if(placeHolderBoard.eP & (1LL<<(originalSquare-1))){
                    placeHolderBoard.bP |= 1LL<<targetSquare;
                    placeHolderBoard.wP &= ~(1LL<<(targetSquare-8));
                }else if(placeHolderBoard.eP & (1LL<<(originalSquare+1))){
                    placeHolderBoard.bP |= 1LL<<targetSquare;
                    placeHolderBoard.wP &= ~(1LL<<(targetSquare-8));
                //double push
                }else if(cordsForGrabbedPiece.y-mousePosGrid.y == -2){
                    placeHolderBoard.bP |= 1LL<<targetSquare;
                    placeHolderBoard.eP = 1LL<<targetSquare;
                }else{
                //normal move
                    placeHolderBoard.bP |= 1LL<<targetSquare;
                }
            }else if(grabbedPiece->pieceName == "w_king"){
                //normal move
                if(abs(cordsForGrabbedPiece.x - mousePosGrid.x) < 2){
                    placeHolderBoard.wK = 1LL<<targetSquare;
                }
                //castling
                if(placeHolderBoard.cWQ){
                    if(((allPieces & 1008806316530991104LL) == 0) && ((unsafeForWhite & 2017612633061982208LL) == 0)){
                        placeHolderBoard.wR &= ~(1LL<<56);
                        placeHolderBoard.wR |= 1LL<<59;
                        placeHolderBoard.wK = 1LL<<targetSquare;
                    }
                }
                if(placeHolderBoard.cWK){
                    if(((allPieces & 6917529027641081856LL) == 0) && ((unsafeForWhite & 8070450532247928832LL) == 0)){
                        placeHolderBoard.wR &= ~(1LL<<63);
                        placeHolderBoard.wR |= 1LL<<61;
                        placeHolderBoard.wK = 1LL<<targetSquare;
                    }
                }
                placeHolderBoard.cWQ = false;
                placeHolderBoard.cWK = false;
            }else if(grabbedPiece->pieceName == "b_king"){
                //normal move
                if(abs(cordsForGrabbedPiece.x - mousePosGrid.x) < 2){
                    placeHolderBoard.bK = 1LL<<targetSquare;
                }
                //castling
                if(placeHolderBoard.cBQ){
                    if(((allPieces & 14LL) == 0) && ((unsafeForBlack & 28LL) == 0)){
                        placeHolderBoard.bR &= ~(1LL);
                        placeHolderBoard.bR |= 1LL<<3;
                        placeHolderBoard.bK = 1LL<<targetSquare;
                    }
                }
                if(placeHolderBoard.cBK){
                    if(((allPieces & 96LL) == 0) && ((unsafeForBlack & 112LL) == 0)){
                        placeHolderBoard.bR &= ~(1LL<<7);
                        placeHolderBoard.bR |= 1LL<<5;
                        placeHolderBoard.bK = 1LL<<targetSquare;
                    }
                }
                placeHolderBoard.cBQ = false;
                placeHolderBoard.cBK = false;
            }else if(grabbedPiece->pieceName == "w_bishop"){
                placeHolderBoard.wB |= 1LL<<targetSquare;
            }else if(grabbedPiece->pieceName == "b_bishop"){
                placeHolderBoard.bB |= 1LL<<targetSquare;
            }else if(grabbedPiece->pieceName == "w_knight"){
                placeHolderBoard.wH |= 1LL<<targetSquare;
            }else if(grabbedPiece->pieceName == "b_knight"){
                placeHolderBoard.bH |= 1LL<<targetSquare;
            }else if(grabbedPiece->pieceName == "w_rook"){
                if(originalSquare == 56){
                    placeHolderBoard.cWQ = false;
                }else if(originalSquare == 63){
                    placeHolderBoard.cWK = false;
                }
                placeHolderBoard.wR |= 1LL<<targetSquare;
            }else if(grabbedPiece->pieceName == "b_rook"){
                if(originalSquare == 0){
                    placeHolderBoard.cBQ = false;
                }else if(originalSquare == 7){
                    placeHolderBoard.cBK = false;
                }
                placeHolderBoard.bR |= 1LL<<targetSquare;
            }else if(grabbedPiece->pieceName == "w_queen"){
                placeHolderBoard.wQ |= 1LL<<targetSquare;
            }else if(grabbedPiece->pieceName == "b_queen"){
                placeHolderBoard.bQ |= 1LL<<targetSquare;
            }

            //Check is the simulated move legal
            for(bitBoard i : allPossibleMoves){
                if(i==placeHolderBoard){
                    legalMove = true;
                    break;
                }
            }

            //Depending on is the move legal or not, do required actions
            if(legalMove){
                grabbedPiece->grabbed = false;
                hasPieceGrabbed = false;
                hasPossibleMovesUpdated = false;
                whiteTurn = !whiteTurn;
                board = placeHolderBoard;
                for(Piece* i : pieces){
                    delete i;
                }
                pieces.clear();
                createPieces(board, pieces);
            }else{
                grabbedPiece->grabbed = false;
                hasPieceGrabbed = false;
                cout << "NON LEGAL MOVE" << "\n";
            }
        }
    }
}

void Game::drawGame(sf::RenderTarget* target){
    target->draw(boardGraphicSprite);
    
    if(highLight && highLightTime < 5){
        highLightRect.setPosition((highLightSquare%8) * 70, (highLightSquare/8) * 70);
        target->draw(highLightRect);
    }

    for(Piece* i : pieces){
        i->drawPiece(target);
    }
}
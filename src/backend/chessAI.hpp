#pragma once

#include "minimax.hpp"

bitBoard chessAI(bitBoard &board, int difficulty, bool color){

    vector<string> ok = {"Black", "White"};
    cout << "\n";
    cout << "The "<< ok[color] <<" AI is thinking" << "\n";
    sf::Clock clock;

    vector<bitBoard> possibleMoves; 
    possibleMoves.reserve(50);
    int bestMoveEval;
    bitBoard bestCurrentMove = board;
    int moveEval;
    int searchDepth = 1 + difficulty;

    if(color){
        generateWhiteMoves(board, possibleMoves);
        bestMoveEval = NEGINF;
    }else{
        generateBlackMoves(board, possibleMoves);
        bestMoveEval = INF;
    }
    cout << "Conducting inital checkmate sequence search" <<"\n";
    //First search for fastest checkmates
    clock.restart();
    uint8_t a = 0;
    float searchTime = (float)(0.5)/(float)((6-difficulty)*(6-difficulty));
    while(clock.getElapsedTime().asSeconds()<searchTime){
        for(bitBoard &i : possibleMoves){
            moveEval = minimax(i, 0, !color, a, NEGINF, INF);
            if(!color && (moveEval == NEGINF)){
                return i;
            }else if(color && (moveEval == INF)){
                return i;
            }
        }
        a += 1;
    }

    cout << "No checkmates found\nSearch Depth: " << a+1 <<"\n";

    //Primary search, NOTE if all moves lead to checkmate for self (eval INF for black and NEGINF for white), doesnt update anything
    for(bitBoard &i : possibleMoves){
        moveEval = minimax(i, 0, !color, a, NEGINF, INF);
        if(!color && (bestMoveEval > moveEval)){
            bestCurrentMove = i;
            bestMoveEval = moveEval;
        }else if(color && (bestMoveEval < moveEval)){
            bestCurrentMove = i;
            bestMoveEval = moveEval;
        }
    }

    //If all moves lead into checkmate, make the move that requires the most moves, ie mate in 5 is better than mate in 3
    //Note that this will always update the move, since the algorithm cant detect checkmates if searching 0 moves ahead
    //Then it will just return the static eval of the possible move
    if(color){
        while(bestMoveEval == NEGINF){
            a -= 1;
            for(bitBoard &i : possibleMoves){
                moveEval = minimax2(i, 0, false, a, NEGINF, INF);
                if(bestMoveEval < moveEval){
                    bestCurrentMove = i;
                    bestMoveEval = moveEval;
                }
            }
        }
    }else{
        while(bestMoveEval == INF){
            a -= 1;
            for(bitBoard &i : possibleMoves){
                moveEval = minimax2(i, 0, true, a, NEGINF, INF);
                if(bestMoveEval > moveEval){
                    bestCurrentMove = i;
                    bestMoveEval = moveEval;
                }
            }
        }
    }
    return bestCurrentMove;
}

//This is used for AI vs AI in testing different minimax version by pitting minimax vs minimax2
bitBoard chessAIvsAI(bitBoard &board, int searchDepth, bool color){
    
    vector<string> ok = {"Black", "White"};
    cout << "\n";
    cout << "The "<< ok[color] <<" AI is thinking" << "\n";
    
    vector<bitBoard> possibleMoves; 
    possibleMoves.reserve(50);
    int bestMoveEval;
    bitBoard bestCurrentMove = board;
    int moveEval;
    cout << "Search Depth: " << searchDepth + 1 <<"\n";

    if(color){
        generateWhiteMoves(board, possibleMoves);
        bestMoveEval = NEGINF;
    }else{
        generateBlackMoves(board, possibleMoves);
        bestMoveEval = INF;
    }

    //Here since we have AI vs AI, we have two different minimax algos depending on the color of the AI
    if(!color){
        //First search for fastest checkmates
        for(int a = 0; a<searchDepth; a++){
            for(bitBoard &i : possibleMoves){
                table.clear();
                table.reserve(100000);
                moveEval = minimax(i, 0, !color, a, NEGINF, INF);
                if(!color && (moveEval == NEGINF)){
                    return i;
                }else if(color && (moveEval == INF)){
                    return i;
                }
            }
        }

        //Primary search, NOTE if all moves lead to checkmate for self (eval INF for black and NEGINF for white), doesnt update anything
        for(bitBoard &i : possibleMoves){
            table.clear();
            table.reserve(100000);
            moveEval = minimax(i, 0, !color, searchDepth, NEGINF, INF);
            if(!color && (bestMoveEval > moveEval)){
                bestCurrentMove = i;
                bestMoveEval = moveEval;
            }else if(color && (bestMoveEval < moveEval)){
                bestCurrentMove = i;
                bestMoveEval = moveEval;
            }
        }

        //If all moves lead into checkmate, make the move that requires the most moves, ie mate in 5 is better than mate in 3
        //Note that this will always update the move, since the algorithm cant detect checkmates if searching 0 moves ahead
        //Then it will just return the static eval of the possible move
        if(color){
            while(bestMoveEval == NEGINF){
                searchDepth -= 1;
                for(bitBoard &i : possibleMoves){
                    table.clear();
                    table.reserve(100000);
                    moveEval = minimax(i, 0, false, searchDepth, NEGINF, INF);
                    if(bestMoveEval < moveEval){
                        bestCurrentMove = i;
                        bestMoveEval = moveEval;
                    }
                }
            }
        }else{
            while(bestMoveEval == INF){
                searchDepth -= 1;
                for(bitBoard &i : possibleMoves){
                    table.clear();
                    table.reserve(100000);
                    moveEval = minimax(i, 0, true, searchDepth, NEGINF, INF);
                    if(bestMoveEval > moveEval){
                        bestCurrentMove = i;
                        bestMoveEval = moveEval;
                    }
                }
            }
        }
    }else{
        //First search for fastest checkmates
        for(int a = 0; a<searchDepth; a++){
            for(bitBoard &i : possibleMoves){
                table.clear();
                table.reserve(100000);
                moveEval = minimax2(i, 0, !color, a, NEGINF, INF);
                if(!color && (moveEval == NEGINF)){
                    return i;
                }else if(color && (moveEval == INF)){
                    return i;
                }
            }
        }

        //Primary search, NOTE if all moves lead to checkmate for self (eval INF for black and NEGINF for white), doesnt update anything
        for(bitBoard &i : possibleMoves){
            table.clear();
            table.reserve(100000);
            moveEval = minimax2(i, 0, !color, searchDepth, NEGINF, INF);
            if(!color && (bestMoveEval > moveEval)){
                bestCurrentMove = i;
                bestMoveEval = moveEval;
            }else if(color && (bestMoveEval < moveEval)){
                bestCurrentMove = i;
                bestMoveEval = moveEval;
            }
        }

        //If all moves lead into checkmate, make the move that requires the most moves, ie mate in 5 is better than mate in 3
        //Note that this will always update the move, since the algorithm cant detect checkmates if searching 0 moves ahead
        //Then it will just return the static eval of the possible move
        if(color){
            while(bestMoveEval == NEGINF){
                searchDepth -= 1;
                for(bitBoard &i : possibleMoves){
                    table.clear();
                    table.reserve(100000);
                    moveEval = minimax2(i, 0, false, searchDepth, NEGINF, INF);
                    if(bestMoveEval < moveEval){
                        bestCurrentMove = i;
                        bestMoveEval = moveEval;
                    }
                }
            }
        }else{
            while(bestMoveEval == INF){
                searchDepth -= 1;
                for(bitBoard &i : possibleMoves){
                    table.clear();
                    table.reserve(100000);
                    moveEval = minimax2(i, 0, true, searchDepth, NEGINF, INF);
                    if(bestMoveEval > moveEval){
                        bestCurrentMove = i;
                        bestMoveEval = moveEval;
                    }
                }
            }
        }
    }
    return bestCurrentMove;
}
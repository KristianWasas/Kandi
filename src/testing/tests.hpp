#pragma once

#include "backend/movegenerator.hpp"
#include "backend/minimax.hpp"
#include "backend/chessAI.hpp"
#include <string>

#define PERFT_DEBUG false

//Values for checkmates
#define INF (int)(99999999)
#define NEGINF -INF

//Global containers for storing data about tree structure
static uint64_t nodesAtDepth[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static uint64_t quiescenceNodesAtDepth[40] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static uint64_t leafNodes = 0;
static std::pair<bitBoard, int> bestMove;

using namespace std;

//Need to initialize, source is in minimax.hpp
int minimax(bitBoard&, uint32_t, bool, uint8_t, int, int);

//Used to debug move generator
uint64_t perft(bitBoard& board, int currentDepth, int maxDepth, bool whiteTurn){

    nodesAtDepth[currentDepth] += 1;

    if(currentDepth == maxDepth){
        leafNodes += 1;
        return 1;
    }

    vector<bitBoard> possibleMoves;
    possibleMoves.reserve(50);

    if(whiteTurn){
        generateWhiteMoves(board, possibleMoves);
    }else{
        generateBlackMoves(board, possibleMoves);
    }

    if(possibleMoves.size() == 0){
        leafNodes += 1;
    }

    uint64_t count = 0;
    for(bitBoard& move : possibleMoves){
        count += perft(move, currentDepth+1, maxDepth, !whiteTurn);
    }
    return count;
}

//Resets the global variables used in keeping track of the search tree size and shape
void resetTestData(){
    bestMove.first = FENtoBitBoard("8/8/8/8/8/8/8/8 w - -");
    bestMove.second = 0;
    leafNodes = 0;
    for(int i=0; i<20; i++){
        nodesAtDepth[i] = 0;
    }
    for(int i=0; i<40; i++){
        quiescenceNodesAtDepth[i] = 0;
    }
}

//Prints out relevenat data of the search tree
void printOutData(){

    int count = 0;

    uint64_t totalNodes = 0;
    uint64_t primarySearchNodes = 0;
    uint64_t quiescenceSearchNodes = 0;

    cout << "Primary search:" << "\n";
    for(int i=0; nodesAtDepth[i] != 0; i++){
        count += 1;
        primarySearchNodes += nodesAtDepth[i];
        totalNodes += nodesAtDepth[i];
        cout << "Nodes at depth " << i << ": " << nodesAtDepth[i] << "\n";
    }
    cout << "Primary search total nodes: " << primarySearchNodes << "\n";

    cout << "Quiescence search:" << "\n";
    for(int i=count; quiescenceNodesAtDepth[i] != 0; i++){
        quiescenceSearchNodes += quiescenceNodesAtDepth[i];
        totalNodes += quiescenceNodesAtDepth[i];
        cout << "Nodes at depth " << i << ": " << quiescenceNodesAtDepth[i] << "\n";
    }
    cout << "Quiescence search total nodes: " << quiescenceSearchNodes << "\n";
    
    cout << "Leafnodes: " << leafNodes << "\n";
    cout << "Total amount of nodes: " << totalNodes << "\n";
}

//Return the size of the search tree
uint64_t treeSize(){

    int count = 0;
    uint64_t totalNodes = 0;

    for(int i=0; nodesAtDepth[i] != 0; i++){
        count += 1;
        totalNodes += nodesAtDepth[i];
    }

    for(int i=count; quiescenceNodesAtDepth[i] != 0; i++){
        totalNodes += quiescenceNodesAtDepth[i];
    }

    return totalNodes;
}

uint64_t qNodesPerTree(){
    int count = 0;
    for(int i=0; nodesAtDepth[i] != 0; i++){
        count += 1;
    }
    uint64_t out = 0;
    for(int i = count; quiescenceNodesAtDepth[i] != 0; i++){
        out += quiescenceNodesAtDepth[i];
    }
    return out;
}

//Returns the branching factor of the tree
float treeBranchingFactor(){

    return (float)(treeSize()-1)/(float)(treeSize()-leafNodes);

}

void testAllminimax(){

    /*This function tests the minimax algortihm what is implemented in minimax.hpp with certain positions to fix depths, and prints out 
    the relevant data, ie time and amount of nodes at each depth, also leafnodes and best move with the evaluation of best move
    List of positions in FEN and the depth the primary search runs to:
    */

    int x = 0;

    vector<float> testInputTrueEvals = {4.5, 6.2, 2.8, 4.5, 5.3, 5.1, 0.4, 2.1, 2.9, 0, -14.9, 0, 0.8,
                                        7.1, 1.5, 4.5, 0.7, 2.9, 1.7, 2.2, 1.0, 3.2, -7.5, 2.7};

    vector<pair<string, int>> testInputs = {
        {"r3qb1k/1b4p1/p2pr2p/3n4/Pnp1N1N1/6RP/1B3PP1/1B1QR1K1 w - -",              5+x},
        {"r4rk1/pp1n1p1p/1nqP2p1/2b1P1B1/4NQ2/1B3P2/PP2K2P/2R5 w - -",              5+x},
        {"r2qk2r/ppp1b1pp/2n1p3/3pP1n1/3P2b1/2PB1NN1/PP4PP/R1BQK2R w KQkq -",       5+x},
        {"r1b1kb1r/1p1n1ppp/p2ppn2/6BB/2qNP3/2N5/PPP2PPP/R2Q1RK1 w kq -",           5+x},
        {"r2qrb1k/1p1b2p1/p2ppn1p/8/3NP3/1BN5/PPP3QP/1K3RR1 w - -",                 5+x},
        {"1r1bk2r/2R2ppp/p3p3/1b2P2q/4QP2/4N3/1B4PP/3R2K1 w k -",                   5+x},
        {"r3rbk1/ppq2ppp/2b1pB2/8/6Q1/1P1B3P/P1P2PP1/R2R2K1 w - -",                 5+x},

        {"r4r1k/4bppb/2n1p2p/p1n1P3/1p1p1BNP/3P1NP1/qP2QPB1/2RR2K1 w - -",          5+x},
        {"r1b2rk1/1p1nbppp/pq1p4/3B4/P2NP3/2N1p3/1PP3PP/R2Q1R1K w - -",             5+x},
        {"r1b3k1/p2p1nP1/2pqr1Rp/1p2p2P/2B1PnQ1/1P6/P1PP4/1K4R1 w - -",             5+x},
        {"8/kp5p/p4p2/P3p1p1/1Pb1P1P1/2P1b2P/3rN3/4RK2 w - -",                      7+x},   //Hor pos 10
        {"6kb/4p3/3p2P1/2p2P2/1p2P1P1/3P4/1QPq4/K1R5 w - -",                        6+x},   //Hor pos 11
        {"r2q1rk1/4bppp/p2p4/2pP4/3pP3/3Q4/PP1B1PPP/R3R1K1 w - -",                  5+x},

        {"rnb2r1k/pp2p2p/2pp2p1/q2P1p2/8/1Pb2NP1/PB2PPBP/R2Q1RK1 w - -",            5+x},
        {"2r3k1/1p2q1pp/2b1pr2/p1pp4/6Q1/1P1PP1R1/P1PN2PP/5RK1 w - -",              5+x},
        {"3rn2k/ppb2rpp/2ppqp2/5N2/2P1P3/1P5Q/PB3PPP/3RR1K1 w - -",                 5+x},
        {"4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - -",                          7+x},
        {"2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - -",                         6+x},
        {"1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P2R2/2P1B1PP/R2Q2K1 w - -",        5+x},

        {"r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - -",               5+x},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",       5+x},
        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - ",    5+x},
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - ",           6+x}, 
        {"3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - -",                       6+x},
    };

    sf::Clock clock;

    uint64_t treeSizes[testInputs.size()];
    uint64_t leafNodesPertree[testInputs.size()];
    uint64_t qNodesPertree[testInputs.size()];
    float branchingFactors[testInputs.size()];
    float evals[testInputs.size()];
    float timerForPos[testInputs.size()];
    int counter = 0;

    cout << "---------------------------------------------\n" << "Starting tests\n" << "---------------------------------------------\n";
    
    for(pair<string, int> i : testInputs){
        resetTestData();
        bitBoard testPosition = FENtoBitBoard(i.first);
        cout << "Testing Position " << counter << ": " << i.first << "\n";
        drawBoard(testPosition);
        cout << "Primary search to depth: " << i.second << "\n";
        clock.restart();
        minimax(testPosition, 0, true, i.second, NEGINF, INF);
        cout << "Time to create and traverse the search tree: " << clock.getElapsedTime().asSeconds() << " seconds\n";
        timerForPos[counter] = clock.getElapsedTime().asSeconds();
        treeSizes[counter] = treeSize();
        leafNodesPertree[counter] = leafNodes;
        qNodesPertree[counter] = qNodesPerTree();
        evals[counter] = (float)(bestMove.second)/100.f;
        printOutData();
        cout << "Branching factor: " << treeBranchingFactor() << "\n";
        branchingFactors[counter] = treeBranchingFactor();
        cout << "Best move: ";
        boardToFEN(bestMove.first);
        drawBoard(bestMove.first);
        cout << "Evaluation: " << (float)(bestMove.second)/100.f << "\n\n";
        cout << "---------------------------------------------\n\n";
        counter += 1;
    }

    //Print out nodes per tree in list
    uint64_t totalNodesAll = 0;
    for(int i=0; i<testInputs.size(); i++){
        totalNodesAll += treeSizes[i];
        cout << "Nodes for position " << i << ":\t" << treeSizes[i] << "\n";
    }
    cout << "\nTotal nodes: " << totalNodesAll << "\n\n";
    
    //Print out quiescence nodes per tree in list
    uint64_t totalQNodesAll = 0;
    for(int i=0; i<testInputs.size(); i++){
        totalQNodesAll += qNodesPertree[i];
        cout << "Quiescencenodes for position " << i << ": \t" << qNodesPertree[i] << "\n";
    }
    cout << "\nTotal quiescencenodes: " << totalQNodesAll << "\n\n";

    //Print out leaf nodes per tree in list
    uint64_t totalLeafNodesAll = 0;
    for(int i=0; i<testInputs.size(); i++){
        totalLeafNodesAll += leafNodesPertree[i];
        cout << "Leafnodes for position " << i << ":\t" << leafNodesPertree[i] << "\n";
    }
    cout << "\nTotal leafnodes: " << totalLeafNodesAll << "\n\n";
    
    //Print out branching factor per tree in list
    float totalBranchingFactor = 0;
    for(int i=0; i<testInputs.size(); i++){
        totalBranchingFactor += branchingFactors[i];
        cout << "Branching factor for position " << i << ":\t" << branchingFactors[i] << "\n";
    }
    cout << "\nAverage branching factor: " << totalBranchingFactor/(float)(testInputs.size()) << "\n\n";

    float averageDiff = 0;
    for(int i=0; i<testInputs.size(); i++){
        averageDiff += abs(evals[i] - testInputTrueEvals[i]);
        cout << "My eval pos " << i << ": " << evals[i] << " vs " << testInputTrueEvals[i] << "\n";
    }

    cout << "\nAverage difference: " << averageDiff/(float)(testInputs.size()) << "\n\n";

    //Print out time took per tree in list
    float totalTime = 0;
    for(int i=0; i<testInputs.size(); i++){
        totalTime += timerForPos[i];
        cout << "Time for position " << i << ":\t" << timerForPos[i] << "\n";
    }
    cout << "Total time:  " << totalTime << "\n";
    cout << "\n---------------------------------------------\n" << "Tests ended\n" <<"---------------------------------------------\n";
}
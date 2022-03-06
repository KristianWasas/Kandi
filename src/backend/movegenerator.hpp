#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <type_traits>
#include <bit>
#include <bitset>

#include "bitboard.hpp"
#include "lookuptable.hpp"

using namespace std;

//READ ME
//Here are the functions used to generate a vector of bitBoards of all possible gamestates from a given bitboard for either white or black one move forward
//The turn parameter is handled outside these functions, there are seperate functions for black and white
//Use the generateWhiteMoves function to generate moves for white and generateBlackMoves for black

//Helper functions, these generate sliding moves from a given square upto, and ontop of the first pieces they encounter INCLUDING OWN PIECES, return has the original square bit set to 0
uint64_t verMoves(const int &square, const uint64_t &Occupied){
    const uint64_t piece = 1LL<<square;
    const uint64_t piecesup = (0x0101010101010101ULL << square) & ((Occupied^piece) | 0xFF00000000000000ULL); /* find the pieces up */
    const uint64_t piecesdo = (0x8080808080808080ULL >> (63 - square)) & ((Occupied^piece) | 0x00000000000000FFULL); /* find the pieces down */
    return ((0x8080808080808080ULL >> (63 - __builtin_ctzll(piecesup))) & (0x0101010101010101ULL << (63 - __builtin_clzll(piecesdo)))) ^ piece;
}

uint64_t horMoves(const int &square, const uint64_t &Occupied){
    const uint64_t piece = 1ULL << square;
    const uint64_t piecesri = (0x00000000000000FFULL << square) & ((Occupied^piece) | 0x8080808080808080ULL);            
    const uint64_t piecesle = (0xFF00000000000000ULL >> (63 - square)) & ((Occupied^piece) | 0x0101010101010101ULL);      
    return ((0xFF00000000000000ULL >> (63 - __builtin_ctzll(piecesri))) & (0x00000000000000FFULL << (63 - __builtin_clzll(piecesle)))) ^ piece;
}

uint64_t diagMoves(const int &square, const uint64_t &Occupied){
    const uint64_t piece = 1LL<<square;
    const uint64_t piecesle = (0x8102040810204081ULL << square) & ((Occupied^piece) | 0xFF01010101010101ULL);
    const uint64_t piecesri = (0x8102040810204081ULL >> (63 - square)) & ((Occupied^piece) | 0x80808080808080FFULL);
    return ((0x8102040810204081ULL >> (63 - __builtin_ctzll(piecesle))) & (0x8102040810204081ULL << (63 - __builtin_clzll(piecesri)))) ^ piece;
}

uint64_t otherDiagMoves(const int &square, const uint64_t &Occupied){
    const uint64_t piece = 1LL<<square;
    const uint64_t piecesup = (0x8040201008040201ULL << square) & ((Occupied^piece) | 0xFF80808080808080ULL);
    const uint64_t piecesdo = (0x8040201008040201ULL >> (63 - square)) & ((Occupied^piece) | 0x01010101010101FFULL);
    return ((0x8040201008040201ULL >> (63 - __builtin_ctzll(piecesup))) & (0x8040201008040201ULL << (63 - __builtin_clzll(piecesdo)))) ^ piece;
}

//All hor ver moves from certain square, can still move ontop first encountered square, own or enemy, own king removed, used in calculating unsafe
uint64_t vertAndHorMovesNoKing(const int &square, const uint64_t &myKing, const uint64_t &Occupied){
    uint64_t piece = 1LL<<square;
    uint64_t piecesup = (0x0101010101010101ULL << square) & (((Occupied^piece)^myKing) | 0xFF00000000000000ULL);
    uint64_t piecesdo = (0x8080808080808080ULL >> (63 - square)) & (((Occupied^piece)^myKing) | 0x00000000000000FFULL);
    uint64_t piecesri = (0x00000000000000FFULL << square) & (((Occupied^piece)^myKing) | 0x8080808080808080ULL);
    uint64_t piecesle = (0xFF00000000000000ULL >> (63 - square)) & (((Occupied^piece)^myKing) | 0x0101010101010101ULL);
    return (((0x8080808080808080ULL >> (63 - __builtin_ctzll(piecesup))) & (0x0101010101010101ULL << (63 - __builtin_clzll(piecesdo)))) |
           ((0xFF00000000000000ULL >> (63 - __builtin_ctzll(piecesri))) & (0x00000000000000FFULL << (63 - __builtin_clzll(piecesle))))) ^ piece;
}

//All diagonal moves from certain square, can still move ontop first encountered square, own or enemy, own king removed, used in calculating unsafe
uint64_t diagonalMovesNoKing(const int &square, const uint64_t &myKing, const uint64_t &Occupied){
    uint64_t piece = 1LL<<square;
    uint64_t piecesup = (0x8040201008040201ULL << square) & (((Occupied^piece)^myKing) | 0xFF80808080808080ULL);
    uint64_t piecesdo = (0x8040201008040201ULL >> (63 - square)) & (((Occupied^piece)^myKing) | 0x01010101010101FFULL);
    uint64_t piecesle = (0x8102040810204081ULL << square) & (((Occupied^piece)^myKing) | 0xFF01010101010101ULL);
    uint64_t piecesri = (0x8102040810204081ULL >> (63 - square)) & (((Occupied^piece)^myKing) | 0x80808080808080FFULL);
    return (((0x8040201008040201ULL >> (63 - __builtin_ctzll(piecesup))) & (0x8040201008040201ULL << (63 - __builtin_clzll(piecesdo)))) |
           ((0x8102040810204081ULL >> (63 - __builtin_ctzll(piecesle))) & (0x8102040810204081ULL << (63 - __builtin_clzll(piecesri))))) ^ piece;
}

/**********************************************************************************************************************/

//Returns all squares what black is attacking/threatening, ie unsafe for white
uint64_t unSafeWhite(const uint64_t &blackKing, const uint64_t &blackQueens, const uint64_t &blackBishops, 
                     uint64_t blackHorses, const uint64_t &blackRooks, const uint64_t &blackPawns,        //horses are not passed by reference since they are modified, rest can be passed by reference
                     const uint64_t &whiteKing, const uint64_t &occupied){
    uint64_t unSafe = ((blackPawns<<7) & ~HFile);        //blackpawn right left
    unSafe |= ((blackPawns<<9) & ~AFile);       //blackpawn attacks right
    uint64_t possibleMoves;
    uint64_t i = blackHorses&~(blackHorses-1);        //Horse attacks
    int iLocation;
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = knightMasks[iLocation];
        unSafe|=possibleMoves;
        blackHorses &=~i;
        i = blackHorses&~(blackHorses-1);
    }
    uint64_t sliderPiece = blackQueens | blackBishops;       //Bishop and queen diagonal attacks
    i = sliderPiece &~(sliderPiece - 1);
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = diagonalMovesNoKing(iLocation, whiteKing, occupied);
        unSafe |= possibleMoves;
        sliderPiece &=~i;
        i = sliderPiece&~(sliderPiece-1);
    }
    sliderPiece = blackQueens | blackRooks;           //Rook and queen horVer attacks, variablename is reused for performance 
    i = sliderPiece &~(sliderPiece-1);
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = vertAndHorMovesNoKing(iLocation, whiteKing, occupied);
        unSafe |= possibleMoves;
        sliderPiece &=~i;
        i = sliderPiece&~(sliderPiece-1);
    }
    iLocation = __builtin_ctzll(blackKing);
    possibleMoves = kingMasks[iLocation];
    unSafe |= possibleMoves;
    return unSafe;
}

//Returns all squares what white is attacking/threatening, ie unsafe for black
uint64_t unSafeBlack(const uint64_t &whiteKing, const uint64_t &whiteQueens, const uint64_t &whiteBishops,            //Again all but horses passed by reference
                     uint64_t whiteHorses, const uint64_t &whiteRooks, const uint64_t &whitePawns,
                     const uint64_t &blackKing, const uint64_t &occupied){
    uint64_t unSafe = ((whitePawns>>7) & ~AFile);        //whitepawn attacks right
    unSafe |= ((whitePawns>>9) & ~HFile);       //whitepawn attacks left
    uint64_t possibleMoves;
    uint64_t i = whiteHorses&~(whiteHorses-1);        //Horse attacks
    int iLocation;
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = knightMasks[iLocation];
        unSafe|=possibleMoves;
        whiteHorses &=~i;
        i = whiteHorses&~(whiteHorses-1);
    }
    uint64_t sliderPieces = whiteQueens | whiteBishops;       //Bishop and queen diagonal attacks
    i = sliderPieces &~(sliderPieces - 1);
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = diagonalMovesNoKing(iLocation, blackKing, occupied);
        unSafe |= possibleMoves;
        sliderPieces &=~i;
        i = sliderPieces&~(sliderPieces-1);
    }
    sliderPieces = whiteQueens | whiteRooks;           //Rook and queen horVer attacks
    i = sliderPieces &~(sliderPieces-1);
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = vertAndHorMovesNoKing(iLocation, blackKing, occupied);
        unSafe |= possibleMoves;
        sliderPieces &=~i;
        i = sliderPieces&~(sliderPieces-1);
    }
    iLocation = __builtin_ctzll(whiteKing);
    unSafe |= kingMasks[iLocation];
    return unSafe;
}

/**********************************************************************************************************************/

//Returns enemy horses attacking own king
uint64_t threatHorses(const uint64_t &ownKing, const uint64_t &enemyHorses){
    int kingLocation = __builtin_ctzll(ownKing);
    return knightMasks[kingLocation] & enemyHorses;
}

//Returns diagonal sliders attacking king
uint64_t threatDiag(const uint64_t &ownKing, const uint64_t &enemyBishops, const uint64_t &enemyQueens, const uint64_t &occupied){
    int kingLoc = __builtin_ctzll(ownKing);
    return (diagMoves(kingLoc, occupied) | otherDiagMoves(kingLoc, occupied)) & (enemyBishops | enemyQueens); 
}

//Returns hor ver sliders attacking king
uint64_t threatHorVer(const uint64_t &ownKing, const uint64_t &enemyRooks, const uint64_t &enemyQueens, const uint64_t &occupied){
    int kingLoc = __builtin_ctzll(ownKing);
    return (verMoves(kingLoc, occupied) | horMoves(kingLoc, occupied)) & (enemyRooks | enemyQueens); 
}

/**********************************************************************************************************************/

//Helper functinos to return all pinned pieces one way, ie diag or hor...
uint64_t pinnedDiag(const uint64_t &ownKing, const uint64_t &ownPieces, const uint64_t &enemyQueens, const uint64_t &enemyBishops, const uint64_t &occupied){     //Pieces pinned from top right to bottom left diagonal
    uint64_t pinnedPiecesDiag = 0LL;
    int kingLoc = __builtin_ctzll(ownKing);
    uint64_t kingDiag = diagMoves(kingLoc, occupied);
    uint64_t enemyDiagPieces = enemyBishops | enemyQueens;
    uint64_t i = enemyDiagPieces&~(enemyDiagPieces-1);
    uint64_t possibleMoves;
    int iLocation;
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = diagMoves(iLocation, occupied);
        pinnedPiecesDiag |= possibleMoves & kingDiag;
        enemyDiagPieces&=~i;
        i=enemyDiagPieces&~(enemyDiagPieces-1);
    }
    return pinnedPiecesDiag & ownPieces;
}

uint64_t pinnedOtherDiag(const uint64_t &ownKing, const uint64_t &ownPieces, const uint64_t &enemyQueens, const uint64_t &enemyBishops, const uint64_t &occupied){        //Pieces pinned from top left to bottom right diagonal
    uint64_t pinnedPiecesOtherDiag = 0LL;
    int kingLoc = __builtin_ctzll(ownKing);
    uint64_t kingOtherDiag = otherDiagMoves(kingLoc, occupied);
    uint64_t enemyDiagPieces = enemyBishops | enemyQueens;
    uint64_t i = enemyDiagPieces&~(enemyDiagPieces-1);
    uint64_t possibleMoves;
    int iLocation;
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = otherDiagMoves(iLocation, occupied);
        pinnedPiecesOtherDiag |= possibleMoves & kingOtherDiag;
        enemyDiagPieces&=~i;
        i=enemyDiagPieces&~(enemyDiagPieces-1);
    }
    return pinnedPiecesOtherDiag & ownPieces;
}

uint64_t pinnedHor(const uint64_t &ownKing, const uint64_t &ownPieces, const uint64_t &enemyQueens, const uint64_t &enemyRooks, const uint64_t &occupied){        //Horizontally pinned pieces
    uint64_t pinnedPiecesHor = 0LL;
    int kingLoc = __builtin_ctzll(ownKing);
    uint64_t kingHor = horMoves(kingLoc, occupied);
    uint64_t enemyHorPieces = enemyRooks | enemyQueens;
    uint64_t i = enemyHorPieces&~(enemyHorPieces-1);
    uint64_t possibleMoves;
    int iLocation;
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = horMoves(iLocation, occupied);
        pinnedPiecesHor |= possibleMoves & kingHor;
        enemyHorPieces&=~i;
        i=enemyHorPieces&~(enemyHorPieces-1);
    }
    return pinnedPiecesHor & ownPieces;
}

uint64_t pinnedVer(const uint64_t &ownKing, const uint64_t &ownPieces, const uint64_t &enemyQueens, const uint64_t &enemyRooks, const uint64_t &occupied){        //Horizontally pinned pieces
    uint64_t pinnedPiecesVer = 0LL;
    int kingLoc = __builtin_ctzll(ownKing);
    uint64_t kingVer = verMoves(kingLoc, occupied);
    uint64_t enemyHorPieces = enemyRooks | enemyQueens;
    uint64_t i = enemyHorPieces&~(enemyHorPieces-1);
    uint64_t possibleMoves;
    int iLocation;
    while(i != 0){
        iLocation = __builtin_ctzll(i);
        possibleMoves = verMoves(iLocation, occupied);
        pinnedPiecesVer |= possibleMoves & kingVer;
        enemyHorPieces&=~i;
        i=enemyHorPieces&~(enemyHorPieces-1);
    }
    return pinnedPiecesVer & ownPieces;
}

/**********************************************************************************************************************/

//Function to check if enpassant is possible, special case of were enpassant capture reveals check
bool pinnedEnpassant(const int &enPassantCaptured, const int &enPassantCapturing, const uint64_t &ownKing, const uint64_t &enemyRooks, const uint64_t &enemyQueens, const uint64_t &occupied){    
    uint64_t enemyThreats = enemyRooks | enemyQueens;
    uint64_t enPassantRowRays = horMoves(enPassantCaptured, occupied) | horMoves(enPassantCapturing, occupied); 
    uint64_t kingOnRow = enPassantRowRays & ownKing;
    uint64_t threatOnRow = enPassantRowRays & enemyThreats;
    if((__builtin_popcountll(kingOnRow) == 1) && (__builtin_popcountll(threatOnRow) == 1)){
        return true;
    }
    return false;
}

/**********************************************************************************************************************/

//Generates all possible king moves for white when attackers >1
void generateWhiteKingMoves(const bitBoard &board, vector<bitBoard> &returnVector, const uint64_t &MovableSquares, const uint64_t &unSafe){
    bitBoard placeholder;
    uint64_t possibleMoves;
    int iLocation = __builtin_ctzll(board.wK);
    possibleMoves = kingMasks[iLocation] & MovableSquares & ~unSafe;
    uint64_t j = possibleMoves&~(possibleMoves-1);
    uint64_t move;
    while(j != 0){
        placeholder = board;
        move = j & -j;
        placeholder.wK = move;
        placeholder.bQ &= ~move;
        placeholder.bR &= ~move;
        placeholder.bB &= ~move;
        placeholder.bH &= ~move;
        placeholder.bP &= ~move;
        placeholder.cWK = false;
        placeholder.cWQ = false;
        returnVector.push_back(placeholder);
        possibleMoves &= ~j;
        j = possibleMoves &~(possibleMoves-1);
    }
}

//Generates all possible king moves for black when attackers >1
void generateBlackKingMoves(const bitBoard &board, vector<bitBoard> &returnVector, const uint64_t &MovableSquares, const uint64_t &unSafe){
    bitBoard placeholder;
    uint64_t possibleMoves;
    int iLocation = __builtin_ctzll(board.bK);
    possibleMoves = kingMasks[iLocation] & MovableSquares & ~unSafe;
    uint64_t j = possibleMoves&~(possibleMoves-1);
    while(j != 0){
        placeholder = board;
        placeholder.bK = j;
        placeholder.wQ &= ~j;
        placeholder.wR &= ~j;
        placeholder.wB &= ~j;
        placeholder.wH &= ~j;
        placeholder.wP &= ~j;
        placeholder.cBK = false;
        placeholder.cBQ = false;
        returnVector.push_back(placeholder);
        possibleMoves &= ~j;
        j = possibleMoves &~(possibleMoves-1);
    }
}

/**********************************************************************************************************************/

//White move generator functions, used in generateWhiteMoves
void generateWPawnPromotions(uint64_t &whitePawns, const uint64_t &EnemyPieces, const uint64_t &protectKing, const uint64_t &diagPinnedPawns,
                             const uint64_t &diagOtherPinnedPawns, const uint64_t &Empty, uint64_t &possibleMoves, uint64_t &possibleMove,
                             bitBoard &placeholder, const bitBoard &board, const uint64_t &pinnedPieces, vector<bitBoard> &returnVector){

    //Remove all pinned pieces at the start, add them accordingly, ie when generating rook horizontal moves, add horPinnedRooks to wRooks, since a hor pinned rook can still move horizontally
    //Use these variables since board is a reference, NOTE, these variables are changed, and need to be "reset" if wanted to be used again, reset happens at the start of each functino call
    whitePawns = board.wP & ~pinnedPieces;      

    whitePawns |= diagPinnedPawns;         //Add diagonally pinned pawns to movable
    possibleMoves = (whitePawns>>7) & EnemyPieces & Rank8 & ~AFile & protectKing;       //Captures right, with promotion
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;                                    //Since its a capture, remove all possible enemy pieces occuping that square (no pawn can be on last rank)
        placeholder.bQ &= ~possibleMove;
        placeholder.bR &= ~possibleMove;
        placeholder.bB &= ~possibleMove;
        placeholder.bH &= ~possibleMove;
        placeholder.wP &= ~(possibleMove<<7);                   //Remove the white pawn that is doing the promoting
        placeholder.wQ |= possibleMove;                         //Add a queen to the return board
        returnVector.push_back(placeholder);
        placeholder.wQ &= ~possibleMove;                        //remove the queen from the place holder tile and replace it with a horse
        placeholder.wH |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.wH &= ~possibleMove;                        //remove the horse from the place holder tile and replace it with a rook
        placeholder.wR |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.wR &= ~possibleMove;                        //remove the rook from the place holder tile and replace it with a bishop
        placeholder.wB |= possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    whitePawns |= diagOtherPinnedPawns;         //Add diagonally other pinned pawns to movable
    whitePawns &= ~diagPinnedPawns;             //Remove diagonally pinned pawns, since we are capturing left now
    possibleMoves = (whitePawns>>9) & EnemyPieces & Rank8 & ~HFile & protectKing;       //Captures left, with promotion
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;                                    //Since its a capture, remove all possible enemy pieces occuping that square (no pawn can be on last rank)
        placeholder.bQ &= ~possibleMove;
        placeholder.bR &= ~possibleMove;
        placeholder.bB &= ~possibleMove;
        placeholder.bH &= ~possibleMove;
        placeholder.wP &= ~(possibleMove<<9);                   //Remove the white pawn that is doing the promoting
        placeholder.wQ |= possibleMove;                         //Add a queen to the return board
        returnVector.push_back(placeholder);
        placeholder.wQ &= ~possibleMove;                        //remove the queen from the place holder tile and replace it with a horse
        placeholder.wH |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.wH &= ~possibleMove;                        //remove the horse from the place holder tile and replace it with a rook
        placeholder.wR |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.wR &= ~possibleMove;                        //remove the rook from the place holder tile and replace it with a bishop
        placeholder.wB |= possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    whitePawns &= ~diagOtherPinnedPawns;         //Remove other diagonally pinned pawns, since its a push to last rank, we know it cant be a pinned pawn
    possibleMoves = (whitePawns>>8) & Empty & Rank8 & protectKing;       //Promotion by pushing pawn
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;
        placeholder.wP &= ~(possibleMove<<8);                   //Remove the white pawn that is doing the promoting, since its a push promotion, we dont need to erase the black pieces from the square
        placeholder.wQ |= possibleMove;                         //Add a queen to the return board
        returnVector.push_back(placeholder);
        placeholder.wQ &= ~possibleMove;                        //remove the queen from the place holder tile and replace it with a horse
        placeholder.wH |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.wH &= ~possibleMove;                        //remove the horse from the place holder tile and replace it with a rook
        placeholder.wR |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.wR &= ~possibleMove;                        //remove the rook from the place holder tile and replace it with a bishop
        placeholder.wB |= possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
}


void generateWPawnCaptures(uint64_t &whitePawns, const uint64_t &EnemyPieces, const uint64_t &protectKing, const uint64_t &diagPinnedPawns,
                           const uint64_t &diagOtherPinnedPawns, const uint64_t &Empty, uint64_t &possibleMoves, uint64_t &possibleMove,
                           bitBoard &placeholder, const bitBoard &board, const uint64_t &pinnedPieces, const uint64_t &occupied, vector<bitBoard> &returnVector){
    
    //Removce pinned pawns at the start
    whitePawns = board.wP & ~pinnedPieces;
    //Captures right
    whitePawns |= diagPinnedPawns;         //Add diagonally pinned pawns to movable
    possibleMoves = (whitePawns>>7) & EnemyPieces & ~Rank8 & ~AFile & protectKing;       //Captures right, minus captures to last rank and enpassant
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;                                    //Since its a capture, remove all possible enemy pieces occuping that square (no pawn can be on last rank)
        placeholder.bQ &= ~possibleMove;
        placeholder.bR &= ~possibleMove;
        placeholder.bB &= ~possibleMove;
        placeholder.bH &= ~possibleMove;
        placeholder.bP &= ~possibleMove;
        placeholder.wP &= ~(possibleMove<<7);                   //Remove the white pawn that is doing the capturing
        placeholder.wP |= possibleMove;                         //Add a pawn to the place were it captures 
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    //Captures left
    whitePawns |= diagOtherPinnedPawns;         //Add diagonally other pinned pawns to movable
    whitePawns &= ~(diagPinnedPawns);           //Remove diagonally pinned pawns
    possibleMoves = (whitePawns>>9) & EnemyPieces & ~Rank8 & ~HFile & protectKing;       //Captures left, minus captures to last rank and enpassant
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;                                    //Since its a capture, remove all possible enemy pieces occuping that square (no pawn can be on last rank)
        placeholder.bQ &= ~possibleMove;
        placeholder.bR &= ~possibleMove;
        placeholder.bB &= ~possibleMove;
        placeholder.bH &= ~possibleMove;
        placeholder.bP &= ~possibleMove;
        placeholder.wP &= ~(possibleMove<<9);                   //Remove the white pawn that is doing the capturing
        placeholder.wP |= possibleMove;                         //Add a pawn to the place were it captures 
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    //En passant captures, first left then right, since only one enpassant can be happend max we use possible move, NOTE WE DIDNT RESET WHITEPAWNS SINCE LAST USING THEM
    possibleMove = ((whitePawns>>1) & board.bP & ~HFile & board.eP) & protectKing & EnemyPieces;       //First check that there is a pawn left to you, and you dont loop around, and its also enpassant, then check that the place youre moving to, 1 up or >>8 is on the protect king board
    if(possibleMove != 0){
        int eLoc = __builtin_ctzll(possibleMove);
        int eLoc2 = eLoc + 1;
        if(!pinnedEnpassant(eLoc, eLoc2, board.wK, board.bR, board.bQ, occupied)){ //Checks that the enpassant capture doesnt result in a illegal move
            placeholder = board;
            placeholder.wP &= ~(possibleMove<<1);       //erases the pawn that was moved
            placeholder.wP |= possibleMove>>8;             //adds the pawn to the new location
            placeholder.bP &= ~(possibleMove);       //removes the black pawn that was captured
            returnVector.push_back(placeholder);
        }
    }
    whitePawns &= ~diagOtherPinnedPawns;
    whitePawns |= diagPinnedPawns;
    possibleMove = ((whitePawns<<1) & board.bP & ~AFile & board.eP) & protectKing & EnemyPieces;       //First check that there is a pawn right to you, and you dont loop around, and its also enpassant, then check that the place youre moving to, 1 up or >>8 is on the protect king board
    if(possibleMove != 0){
        int eLoc = __builtin_ctzll(possibleMove);
        int eLoc2 = eLoc - 1;
        if(!pinnedEnpassant(eLoc, eLoc2, board.wK, board.bR, board.bQ, occupied)){ //Checks that the enpassant capture doesnt result in a illegal move
            placeholder = board;
            placeholder.wP &= ~(possibleMove>>1);       //erases the pawn that was moved
            placeholder.wP |= possibleMove>>8;             //adds the pawn to the new location
            placeholder.bP &= ~(possibleMove);       //removes the black pawn that was captured
            returnVector.push_back(placeholder);
        }
    }
}

void generateWHorseCaptures(uint64_t &horses, const uint64_t &EnemyPieces, const uint64_t &protectKing, uint64_t &possibleMoves, const uint64_t &pinnedPieces,
                            uint64_t &possibleMove, uint64_t &onePiece, int &iLocation, bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){
    
    horses = board.wH & ~pinnedPieces;

    onePiece = horses&~(horses-1LL);        //A bit board with the least significant horse, incremently moves to the next horse etc.
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);                //Counts traliing zeros, ie gives position of piece, 0 is top left bottom right is 63
        possibleMoves = knightMasks[iLocation] & EnemyPieces & protectKing;
        possibleMove = possibleMoves & ~(possibleMoves - 1LL);        //To loop over the possible moves bitboard, and take one bit at a time
        while(possibleMove != 0){
            placeholder = board;
            placeholder.wH &= ~onePiece;                               //Remove the place were the horse was moving from
            placeholder.wH |= possibleMove;                            //Add the move
            placeholder.bQ &= ~possibleMove;
            placeholder.bR &= ~possibleMove;
            placeholder.bB &= ~possibleMove;
            placeholder.bH &= ~possibleMove;
            placeholder.bP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                 //Erasing the move from the bitboard which was added to string
            possibleMove = possibleMoves&~(possibleMoves - 1LL);
        }
        horses&=~onePiece;                                        //erases the onePiece from horses for which all moves were calculated
        onePiece=horses&~(horses-1LL);                            //Gets the next least significant bit after removing the last one, if last one was the last, is 0
    }
}

void generateWBishopCaptures(uint64_t &bishops, const uint64_t &EnemyPieces, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                             uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &pinnedDiagonalBishops, const uint64_t &pinnedOtherDiagonalBishops,
                             bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){
    
    bishops = board.wB & ~pinnedPieces;
    
    bishops |= pinnedDiagonalBishops;              //Add diagonally pinned bishops 
    onePiece = bishops&~(bishops-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = diagMoves(iLocation, occupied) & EnemyPieces & protectKing;       //Diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wB &= ~onePiece;
            placeholder.wB |= possibleMove;                                     //Since its a capture, remove all possible enemy pieces occuping that square
            placeholder.bQ &= ~possibleMove;
            placeholder.bR &= ~possibleMove;
            placeholder.bB &= ~possibleMove;
            placeholder.bH &= ~possibleMove;
            placeholder.bP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        bishops &= ~onePiece;
        onePiece = bishops&~(bishops-1);
    }
    bishops = board.wB & ~pinnedPieces;                          //Reset bishops and remove all pinned bishops
    bishops |= pinnedOtherDiagonalBishops;                       //Add other diagonally pinned bishops
    onePiece = bishops&~(bishops-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = otherDiagMoves(iLocation, occupied) & EnemyPieces & protectKing;       //Diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wB &= ~onePiece;
            placeholder.wB |= possibleMove;                                     //Since its a capture, remove all possible enemy pieces occuping that square
            placeholder.bQ &= ~possibleMove;
            placeholder.bR &= ~possibleMove;
            placeholder.bB &= ~possibleMove;
            placeholder.bH &= ~possibleMove;
            placeholder.bP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        bishops &= ~onePiece;
        onePiece = bishops&~(bishops-1);
    }
}

void generateWRookCaptures(uint64_t &rooks, const uint64_t &EnemyPieces, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                           uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &horPinnedRooks, const uint64_t &verPinnedRooks,
                           bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    rooks = board.wR & ~pinnedPieces;

    rooks |= horPinnedRooks;              //Add horizontally pinned rooks
    onePiece = rooks&~(rooks-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = horMoves(iLocation, occupied) & EnemyPieces & protectKing;       
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wR &= ~onePiece;
            placeholder.wR |= possibleMove;                     //Since its a capture, remove all possible enemy pieces occuping that square
            placeholder.bQ &= ~possibleMove;
            placeholder.bR &= ~possibleMove;
            placeholder.bB &= ~possibleMove;
            placeholder.bH &= ~possibleMove;
            placeholder.bP &= ~possibleMove;
            if(iLocation == 63){
                placeholder.cWK = false;                        //Disallow future castling
            }else if(iLocation == 56){
                placeholder.cWQ = false;
            }
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        rooks &= ~onePiece;
        onePiece = rooks&~(rooks-1);
    }
    rooks = board.wR & ~pinnedPieces;                          //Reset rooks and remove all pinned rooks
    rooks |= verPinnedRooks;                                   //Add vertically pinned rooks
    onePiece = rooks&~(rooks-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = verMoves(iLocation, occupied) & EnemyPieces & protectKing;      //vermoves generates all horizontal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                            //Takes the least significant bit
        while(possibleMove != 0){                                                       //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wR &= ~onePiece;
            placeholder.wR |= possibleMove;                     //Since its a capture, remove all possible enemy pieces occuping that square
            placeholder.bQ &= ~possibleMove;
            placeholder.bR &= ~possibleMove;
            placeholder.bB &= ~possibleMove;
            placeholder.bH &= ~possibleMove;
            placeholder.bP &= ~possibleMove;
            if(iLocation == 63){
                placeholder.cWK = false;                        //Disallow future castling
            }else if(iLocation == 56){
                placeholder.cWQ = false;
            }
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        rooks &= ~onePiece;
        onePiece = rooks&~(rooks-1);
    }
}

void generateWQueenCaptures(uint64_t &queens, const uint64_t &EnemyPieces, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                           uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &horPinnedQueens, const uint64_t &verPinnedQueens, const uint64_t &diagPinnedQueens, const uint64_t &diagOtherPinnedQueens,
                           bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){
    
    queens = board.wQ & ~pinnedPieces;
    
    queens |= horPinnedQueens;              //Add horizontally pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = horMoves(iLocation, occupied) & EnemyPieces & protectKing;       
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wQ &= ~onePiece;
            placeholder.wQ |= possibleMove;                                     //Since its a capture, remove all possible enemy pieces occuping that square
            placeholder.bQ &= ~possibleMove;
            placeholder.bR &= ~possibleMove;
            placeholder.bB &= ~possibleMove;
            placeholder.bH &= ~possibleMove;
            placeholder.bP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.wQ & ~pinnedPieces;
    //Diagonal queen captures
    queens |= diagPinnedQueens;              //Add diagonally pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = diagMoves(iLocation, occupied) & EnemyPieces & protectKing;       //diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wQ &= ~onePiece;
            placeholder.wQ |= possibleMove;                                     //Since its a capture, remove all possible enemy pieces occuping that square
            placeholder.bQ &= ~possibleMove;
            placeholder.bR &= ~possibleMove;
            placeholder.bB &= ~possibleMove;
            placeholder.bH &= ~possibleMove;
            placeholder.bP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.wQ & ~pinnedPieces;  
    //OtherDiagonal queen captures
    queens |= diagOtherPinnedQueens;              //Add otherdiag pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = otherDiagMoves(iLocation, occupied) & EnemyPieces & protectKing;       //otherdiagmoves generates all otherdiagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wQ &= ~onePiece;
            placeholder.wQ |= possibleMove;                                     //Since its a capture, remove all possible enemy pieces occuping that square
            placeholder.bQ &= ~possibleMove;
            placeholder.bR &= ~possibleMove;
            placeholder.bB &= ~possibleMove;
            placeholder.bH &= ~possibleMove;
            placeholder.bP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.wQ & ~pinnedPieces;  
    //Vertical queen captures
    queens |= verPinnedQueens;              //Add ver pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = verMoves(iLocation, occupied) & EnemyPieces & protectKing;       //vermoves generates all vertical moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wQ &= ~onePiece;
            placeholder.wQ |= possibleMove;                                     //Since its a capture, remove all possible enemy pieces occuping that square
            placeholder.bQ &= ~possibleMove;
            placeholder.bR &= ~possibleMove;
            placeholder.bB &= ~possibleMove;
            placeholder.bH &= ~possibleMove;
            placeholder.bP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
}

void generateWKingCaptures(const uint64_t &whiteKing, const uint64_t &EnemyPieces, const uint64_t &unSafe, uint64_t &possibleMoves,
                           uint64_t &possibleMove, int &iLocation, bitBoard &placeholder, const bitBoard &board, vector<bitBoard> &returnVector){
    iLocation = __builtin_ctzll(whiteKing);
    possibleMoves = kingMasks[iLocation] & EnemyPieces & ~unSafe;
    possibleMove = possibleMoves&~(possibleMoves-1);
    while(possibleMove != 0){
        placeholder = board;
        placeholder.wK = possibleMove;              //Add the move to the placeholder board, normally we would have to add it with | and remove the original, but since only one king we can replace
        placeholder.bQ &= ~possibleMove;
        placeholder.bR &= ~possibleMove;
        placeholder.bB &= ~possibleMove;
        placeholder.bH &= ~possibleMove;
        placeholder.bP &= ~possibleMove;
        placeholder.cWK = false;                    //King move, castling privilidge revoked
        placeholder.cWQ = false;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;             //goes to next move 
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
}

void generateWPawnMoves(uint64_t &whitePawns, const uint64_t &protectKing, const uint64_t &verPinnedPawns,
                        const uint64_t &Empty, uint64_t &possibleMoves, uint64_t &possibleMove,
                        bitBoard &placeholder, const bitBoard &board, const uint64_t &pinnedPieces, vector<bitBoard> &returnVector){
    
    whitePawns = board.wP & ~pinnedPieces;
    
    whitePawns |= verPinnedPawns;           //Add Vertiaclly pinned pawns
    possibleMoves = (whitePawns>>16) & Empty & (Empty>>8) & Rank4 & protectKing;
    possibleMove = possibleMoves&~(possibleMoves-1);
    while(possibleMove != 0){
        placeholder = board;
        placeholder.wP &= ~(possibleMove<<16);
        placeholder.wP |= possibleMove;
        placeholder.eP = possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    //Pawn push, note that whitePawns contains only non pinned and hor pinned pawns
    possibleMoves = (whitePawns>>8) & Empty & ~Rank8 & protectKing;
    possibleMove = possibleMoves&~(possibleMoves-1);
    while(possibleMove != 0){
        placeholder = board;
        placeholder.wP &= ~(possibleMove<<8);
        placeholder.wP |= possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
}

void generateWHorseMoves(uint64_t &horses, const uint64_t &Empty, const uint64_t &protectKing, uint64_t &possibleMoves, const uint64_t &pinnedPieces,
                         uint64_t &possibleMove, uint64_t &onePiece, int &iLocation, bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){
    
    horses = board.wH & ~pinnedPieces;      //Reset horses, note that horse cannot move if its pinned
    
    onePiece = horses&~(horses-1LL);        //A bit board with the least significant horse, incremently moves to the next horse etc.
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);                //Counts traliing zeros, ie gives position of piece, 0 is top left bottom right is 63
        possibleMoves = knightMasks[iLocation] & Empty & protectKing;
        possibleMove = possibleMoves & ~(possibleMoves - 1LL);        //To loop over the possible moves bitboard, and take one bit at a time
        while(possibleMove != 0){
            placeholder = board;
            placeholder.wH &= ~onePiece;                               //Remove the place were the horse was moving from
            placeholder.wH |= possibleMove;                            //Add the move
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                 //Erasing the move from the bitboard which was added to string
            possibleMove = possibleMoves&~(possibleMoves - 1LL);
        }
        horses&=~onePiece;                                        //erases the onePiece from horses for which all moves were calculated
        onePiece=horses&~(horses-1LL);                            //Gets the next least significant bit after removing the last one, if last one was the last, is 0
    }
}

void generateWBishopMoves(uint64_t &bishops, const uint64_t &Empty, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                          uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &pinnedDiagonalBishops, const uint64_t &pinnedOtherDiagonalBishops,
                          bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    bishops = board.wB & ~pinnedPieces;

    bishops |= pinnedDiagonalBishops;              //Add diagonally pinned bishops 
    onePiece = bishops&~(bishops-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = diagMoves(iLocation, occupied) & Empty & protectKing;       //Diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wB &= ~onePiece;
            placeholder.wB |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        bishops &= ~onePiece;
        onePiece = bishops&~(bishops-1);
    }
    bishops = board.wB & ~pinnedPieces;                          //Reset bishops and remove all pinned bishops
    bishops |= pinnedOtherDiagonalBishops;                       //Add other diagonally pinned bishops
    onePiece = bishops&~(bishops-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = otherDiagMoves(iLocation, occupied) & Empty & protectKing;        //Diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wB &= ~onePiece;
            placeholder.wB |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        bishops &= ~onePiece;
        onePiece = bishops&~(bishops-1);
    }         
}   

void generateWRookMoves(uint64_t &rooks, const uint64_t &Empty, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                        uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &horPinnedRooks, const uint64_t &verPinnedRooks,
                        bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    rooks = board.wR & ~pinnedPieces;

    rooks |= horPinnedRooks;              //Add horizontally pinned rooks
    onePiece = rooks&~(rooks-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = horMoves(iLocation, occupied) & Empty & protectKing;       
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wR &= ~onePiece;
            placeholder.wR |= possibleMove;
            if(iLocation == 63){
                placeholder.cWK = false;                        //Disallow future castling
            }else if(iLocation == 56){
                placeholder.cWQ = false;
            }
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        rooks &= ~onePiece;
        onePiece = rooks&~(rooks-1);
    }
    rooks = board.wR & ~pinnedPieces;                          //Reset rooks and remove all pinned rooks
    rooks |= verPinnedRooks;              //Add vertically pinned rooks
    onePiece = rooks&~(rooks-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = verMoves(iLocation, occupied) & Empty & protectKing;       //vermoves generates all horizontal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wR &= ~onePiece;
            placeholder.wR |= possibleMove;
            if(iLocation == 63){
                placeholder.cWK = false;                        //Disallow future castling
            }else if(iLocation == 56){
                placeholder.cWQ = false;
            }
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        rooks &= ~onePiece;
        onePiece = rooks&~(rooks-1);
    } 
}

void generateWQueenMoves(uint64_t &queens, const uint64_t &Empty, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                           uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &horPinnedQueens, const uint64_t &verPinnedQueens, const uint64_t &diagPinnedQueens, const uint64_t &diagOtherPinnedQueens,
                           bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){
    
    queens = board.wQ & ~pinnedPieces;

    queens |= horPinnedQueens;              //Add horizontally pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = horMoves(iLocation, occupied) & Empty & protectKing;       
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wQ &= ~onePiece;
            placeholder.wQ |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.wQ & ~pinnedPieces;
    //Diagonal queen moves
    queens |= diagPinnedQueens;              //Add diagonally pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = diagMoves(iLocation, occupied) & Empty & protectKing;       //diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wQ &= ~onePiece;
            placeholder.wQ |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.wQ & ~pinnedPieces;  
    //OtherDiagonal queen moves
    queens |= diagOtherPinnedQueens;              //Add otherdiag pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = otherDiagMoves(iLocation, occupied) & Empty & protectKing;       //otherdiagmoves generates all otherdiagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wQ &= ~onePiece;
            placeholder.wQ |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.wQ & ~pinnedPieces;  
    //Vertical queen moves
    queens |= verPinnedQueens;              //Add ver pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = verMoves(iLocation, occupied) & Empty & protectKing;       //vermoves generates all vertical moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.wQ &= ~onePiece;
            placeholder.wQ |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
}

void generateWKingMoves(const uint64_t &whiteKing, const uint64_t &Empty, const uint64_t &unSafe, uint64_t &possibleMoves,
                           uint64_t &possibleMove, int &iLocation, bitBoard &placeholder, const bitBoard &board, vector<bitBoard> &returnVector){
    
    iLocation = __builtin_ctzll(board.wK);
    possibleMoves = kingMasks[iLocation] & Empty & ~unSafe;
    possibleMove = possibleMoves&~(possibleMoves-1);
    while(possibleMove != 0){
        placeholder = board;
        placeholder.wK = possibleMove;              //Add the move to the placeholder board, normally we would have to add it with | and remove the original, but since only one king we can replace
        placeholder.cWK = false;                    //King move, castling privilidge revoked
        placeholder.cWQ = false;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
}

void generateWCastling(const bitBoard &board, const uint64_t &Empty, const uint64_t &unSafe, bitBoard &placeholder, vector<bitBoard> &returnVector){
    if(board.cWK){
        if((Empty>>62 & 1 == 1) && (Empty>>61 & 1 == 1) && (~unSafe>>62 & 1 == 1) && (~unSafe>>61 & 1 == 1) && (~unSafe>>60 & 1 == 1) && (board.wR>>63 & 1 == 1)){
            placeholder = board;
            placeholder.wK = 1LL<<62;
            placeholder.wR &= ~(1LL<<63);
            placeholder.wR |= 1LL<<61;
            placeholder.cWK = false;
            placeholder.cWQ = false;
            returnVector.push_back(placeholder);
        }
    }
    if(board.cWQ){
        if((Empty>>59 & 1 == 1) && (Empty>>58 & 1 == 1) && (Empty>>57 & 1 == 1) && (~unSafe>>60 & 1 == 1) && (~unSafe>>59 & 1 == 1) && (~unSafe>>58 & 1 == 1) && (board.wR>>56 & 1 == 1)){
            placeholder = board;
            placeholder.wK = 1LL<<58;
            placeholder.wR &= ~(1LL<<56);
            placeholder.wR |= 1LL<<59;
            placeholder.cWK = false;
            placeholder.cWQ = false;
            returnVector.push_back(placeholder);
        }
    }
}

//Generates all possible moves for white if <2 attackers
void whiteMoves(const bitBoard &board, vector<bitBoard> &returnVector, const uint64_t &pinnedHorizontal, const uint64_t &pinnedVertical, 
                        const uint64_t &pinnedDiagonal, const uint64_t &pinnedOtherDiagonal, const uint64_t &pinnedPieces, const uint64_t &EnemyPieces,
                        const uint64_t &protectKing, const uint64_t &Empty, const uint64_t &unSafe, const uint64_t &occupied){
    
    //First we initialize all needed variables

    bitBoard placeholder;           //A copy of this is pushed into returnVector for each possible move
    uint64_t possibleMoves;         //Used to store were a onePiece can move to, has multiple set bits as possible move locations
    uint64_t possibleMove;          //Used to loop over all possibleMoves
    uint64_t onePiece;              //Used to loop over all the pieces in for example horses etc. Note pawns have no need for this since their moves are generated by just bitshifting all of them
    int iLocation;                  //Used as a location on board for given onePiece

    //List of all types of pinned pieces that are needed
    const uint64_t horPinnedPawns = board.wP & pinnedHorizontal;
    const uint64_t verPinnedPawns = board.wP & pinnedVertical;
    const uint64_t diagPinnedPawns = board.wP & pinnedDiagonal;
    const uint64_t diagOtherPinnedPawns = board.wP & pinnedOtherDiagonal;

    const uint64_t horPinnedQueens = board.wQ & pinnedHorizontal;
    const uint64_t verPinnedQueens = board.wQ & pinnedVertical;
    const uint64_t diagPinnedQueens = board.wQ & pinnedDiagonal;
    const uint64_t diagOtherPinnedQueens = board.wQ & pinnedOtherDiagonal;

    const uint64_t pinnedDiagonalBishops = board.wB & pinnedDiagonal;
    const uint64_t pinnedOtherDiagonalBishops = board.wB & pinnedOtherDiagonal;

    const uint64_t horPinnedRooks = board.wR & pinnedHorizontal;
    const uint64_t verPinnedRooks = board.wR & pinnedVertical;

    //Use these variables since board is a reference, NOTE, these variables are changed, and need to be "reset" before being used again
    uint64_t pawns;
    uint64_t horses;
    uint64_t bishops;
    uint64_t rooks;
    uint64_t queens;

    //Enemypieces by value
    const uint64_t enemy359     = board.bH | board.bB | board.bR | board.bQ;
    const uint64_t enemy59      = board.bR | board.bQ;
    const uint64_t enemy9       = board.bQ;
    const uint64_t enemy135     = board.bP | board.bH | board.bB | board.bR;
    const uint64_t enemy13      = board.bP | board.bH | board.bB;
    const uint64_t enemy1       = board.bP;
    const uint64_t enemy3       = board.bB | board.bH;
    const uint64_t enemy5       = board.bR;

    //Pawn promotions
    generateWPawnPromotions(pawns, EnemyPieces, protectKing, diagPinnedPawns, diagOtherPinnedPawns, Empty, possibleMoves, possibleMove, placeholder, 
                            board, pinnedPieces, returnVector);
    
    //King captures
    generateWKingCaptures(board.wK, EnemyPieces, unSafe, possibleMoves, possibleMove, iLocation, placeholder, board, returnVector);
    
    //Pawn captures
    generateWPawnCaptures(pawns, EnemyPieces, protectKing, diagPinnedPawns, diagOtherPinnedPawns, Empty, possibleMoves, possibleMove, 
                            placeholder, board, pinnedPieces, occupied, returnVector);

    //Horse moves capturing enemy pieces, note a pinned horse can NEVER move
    generateWHorseCaptures(horses, EnemyPieces, protectKing, possibleMoves, pinnedPieces, possibleMove, onePiece, iLocation, placeholder, board, returnVector);

    //Bishop captures
    generateWBishopCaptures(bishops, EnemyPieces, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, 
                            pinnedDiagonalBishops, pinnedOtherDiagonalBishops, placeholder, board, returnVector);

    //All rook captures
    generateWRookCaptures(rooks, EnemyPieces, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, horPinnedRooks, 
                            verPinnedRooks, placeholder, board, returnVector);

    //All queen captures
    generateWQueenCaptures(queens, EnemyPieces, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, horPinnedQueens, 
                            verPinnedQueens, diagPinnedQueens, diagOtherPinnedQueens, placeholder, board, returnVector);

    //Castling
    generateWCastling(board, Empty, unSafe, placeholder, returnVector);

    //Pawn moves, double push and normal push, no push promotion, that is handled in pawn promotions
    generateWPawnMoves(pawns, protectKing, verPinnedPawns, Empty, possibleMoves, possibleMove, placeholder, board, pinnedPieces, returnVector);

    //Horse moves, no captures
    generateWHorseMoves(horses, Empty, protectKing, possibleMoves, pinnedPieces, possibleMove, onePiece, iLocation, placeholder, board, returnVector);

    //Bishop moves, no captures
    generateWBishopMoves(bishops, Empty, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, pinnedDiagonalBishops, 
                            pinnedOtherDiagonalBishops, placeholder, board, returnVector);

    //Rook moves, no captures
    generateWRookMoves(rooks, Empty, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, horPinnedRooks, 
                            verPinnedRooks, placeholder, board, returnVector);
                            
    //Queen moves, no captures
    generateWQueenMoves(queens, Empty, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, horPinnedQueens, 
                        verPinnedQueens, diagPinnedQueens, diagOtherPinnedQueens, placeholder, board, returnVector);

    //King moves, no castling or capturing
    generateWKingMoves(board.wK, Empty, unSafe, possibleMoves, possibleMove, iLocation, placeholder, board, returnVector);

}

/**********************************************************************************************************************/

//Black move generator functions, used in generateBlackMoves
void generateBPawnPromotions(uint64_t &blackPawns, const uint64_t &EnemyPieces, const uint64_t &protectKing, const uint64_t &diagPinnedPawns,
                             const uint64_t &diagOtherPinnedPawns, const uint64_t &Empty, uint64_t &possibleMoves, uint64_t &possibleMove,
                             bitBoard &placeholder, const bitBoard &board, const uint64_t &pinnedPieces, vector<bitBoard> &returnVector){

    blackPawns = board.bP & ~pinnedPieces;

    blackPawns |= diagPinnedPawns;         //Add diagonally pinned pawns to movable
    possibleMoves = (blackPawns<<7) & EnemyPieces & Rank1 & ~HFile & protectKing;       //Captures left, with promotion
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;                                    //Since its a capture, remove all possible enemy pieces occuping that square (no pawn can be on last rank)
        placeholder.wQ &= ~possibleMove;
        placeholder.wR &= ~possibleMove;
        placeholder.wB &= ~possibleMove;
        placeholder.wH &= ~possibleMove;
        placeholder.bP &= ~(possibleMove>>7);                   //Remove the black pawn that is doing the promoting
        placeholder.bQ |= possibleMove;                         //Add a queen to the return board
        returnVector.push_back(placeholder);
        placeholder.bQ &= ~possibleMove;                        //remove the queen from the place holder tile and replace it with a horse
        placeholder.bH |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.bH &= ~possibleMove;                        //remove the horse from the place holder tile and replace it with a rook
        placeholder.bR |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.bR &= ~possibleMove;                        //remove the rook from the place holder tile and replace it with a bishop
        placeholder.bB |= possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    blackPawns |= diagOtherPinnedPawns;         //Add diagonally other pinned pawns to movable
    blackPawns &= ~diagPinnedPawns;             //Remove diagonally pinned pawns, since we are capturing left now
    possibleMoves = (blackPawns<<9) & EnemyPieces & Rank1 & ~AFile & protectKing;       //Captures right, with promotion
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;                                    //Since its a capture, remove all possible enemy pieces occuping that square (no pawn can be on last rank)
        placeholder.wQ &= ~possibleMove;
        placeholder.wR &= ~possibleMove;
        placeholder.wB &= ~possibleMove;
        placeholder.wH &= ~possibleMove;
        placeholder.bP &= ~(possibleMove>>9);                   //Remove the black pawn that is doing the promoting
        placeholder.bQ |= possibleMove;                         //Add a queen to the return board
        returnVector.push_back(placeholder);
        placeholder.bQ &= ~possibleMove;                        //remove the queen from the place holder tile and replace it with a horse
        placeholder.bH |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.bH &= ~possibleMove;                        //remove the horse from the place holder tile and replace it with a rook
        placeholder.bR |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.bR &= ~possibleMove;                        //remove the rook from the place holder tile and replace it with a bishop
        placeholder.bB |= possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    blackPawns &= ~diagOtherPinnedPawns;         //Remove other diagonally pinned pawns, since its a push to last rank, we know it cant be a pinned pawn
    possibleMoves = (blackPawns<<8) & Empty & Rank1 & protectKing;       //Promotion by pushing pawn
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;
        placeholder.bP &= ~(possibleMove>>8);                   //Remove the white pawn that is doing the promoting, since its a push promotion, we dont need to erase the black pieces from the square
        placeholder.bQ |= possibleMove;                         //Add a queen to the return board
        returnVector.push_back(placeholder);
        placeholder.bQ &= ~possibleMove;                        //remove the queen from the place holder tile and replace it with a horse
        placeholder.bH |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.bH &= ~possibleMove;                        //remove the horse from the place holder tile and replace it with a rook
        placeholder.bR |= possibleMove;
        returnVector.push_back(placeholder);
        placeholder.bR &= ~possibleMove;                        //remove the rook from the place holder tile and replace it with a bishop
        placeholder.bB |= possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }             
}

void generateBPawnCaptures(uint64_t &blackPawns, const uint64_t &EnemyPieces, const uint64_t &protectKing, const uint64_t &diagPinnedPawns,
                           const uint64_t &diagOtherPinnedPawns, const uint64_t &Empty, uint64_t &possibleMoves, uint64_t &possibleMove,
                           bitBoard &placeholder, const bitBoard &board, const uint64_t &pinnedPieces, const uint64_t &occupied, vector<bitBoard> &returnVector){

    blackPawns = board.bP & ~pinnedPieces;

    blackPawns |= diagPinnedPawns;         //Add diagonally pinned pawns to movable
    possibleMoves = (blackPawns<<7) & EnemyPieces & ~Rank1 & ~HFile & protectKing;       //Captures right, minus captures to last rank and enpassant
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;                                    //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
        placeholder.wQ &= ~possibleMove;
        placeholder.wR &= ~possibleMove;
        placeholder.wB &= ~possibleMove;
        placeholder.wH &= ~possibleMove;
        placeholder.wP &= ~possibleMove;
        placeholder.bP &= ~(possibleMove>>7);                   //Remove the black pawn that is doing the capturing
        placeholder.bP |= possibleMove;                         //Add a pawn to the place were it captures 
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    //Captures left
    blackPawns |= diagOtherPinnedPawns;         //Add diagonally other pinned pawns to movable
    blackPawns &= ~(diagPinnedPawns);           //Remove diagonally pinned pawns
    possibleMoves = (blackPawns<<9) & EnemyPieces & ~Rank1 & ~AFile & protectKing;       //Captures left, minus captures to last rank and enpassant
    possibleMove = possibleMoves&~(possibleMoves-1);            //One by one lists the possible moves
    while(possibleMove != 0){
        placeholder = board;                                    //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
        placeholder.wQ &= ~possibleMove;
        placeholder.wR &= ~possibleMove;
        placeholder.wB &= ~possibleMove;
        placeholder.wH &= ~possibleMove;
        placeholder.wP &= ~possibleMove;
        placeholder.bP &= ~(possibleMove>>9);                   //Remove the black pawn that is doing the capturing
        placeholder.bP |= possibleMove;                           //Add a pawn to the place were it captures 
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to the next pawn
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    possibleMove = ((blackPawns>>1) & board.wP & ~HFile & board.eP) & protectKing & EnemyPieces;       //First check that there is a pawn left to you, and you dont loop around, and its also enpassant, then check that the place youre moving to, 1 up or >>8 is on the protect king board
    if(possibleMove != 0){
        int ePloc = __builtin_ctzll(possibleMove);
        int ePloc2 = ePloc + 1;
        if(!pinnedEnpassant(ePloc, ePloc2, board.bK, board.wR, board.wQ, occupied)){ //Checks that the enpassant capture doesnt result in a illegal move
            placeholder = board;
            placeholder.bP &= ~(possibleMove<<1);       //erases the pawn that was moved
            placeholder.bP |= possibleMove<<8;             //adds the pawn to the new location
            placeholder.wP &= ~(possibleMove);       //removes the black pawn that was captured
            returnVector.push_back(placeholder);
        }
    }
    blackPawns &= ~diagOtherPinnedPawns;
    blackPawns |= diagPinnedPawns;
    possibleMove = ((blackPawns<<1) & board.wP & ~AFile & board.eP) & protectKing & EnemyPieces;       //First check that there is a pawn right to you, and you dont loop around, and its also enpassant, then check that the place youre moving to, 1 up or >>8 is on the protect king board
    if(possibleMove != 0){
        int ePloc = __builtin_ctzll(possibleMove);
        int ePloc2 = ePloc - 1;
        if(!pinnedEnpassant(ePloc, ePloc2, board.bK, board.wR, board.wQ, occupied)){ //Checks that the enpassant capture doesnt result in a illegal move
            placeholder = board;
            placeholder.bP &= ~(possibleMove>>1);       //erases the pawn that was moved
            placeholder.bP |= possibleMove<<8;             //adds the pawn to the new location
            placeholder.wP &= ~(possibleMove);       //removes the black pawn that was captured
            returnVector.push_back(placeholder);
        }
    }
}

void generateBHorseCaptures(uint64_t &horses, const uint64_t &EnemyPieces, const uint64_t &protectKing, uint64_t &possibleMoves, const uint64_t &pinnedPieces,
                            uint64_t &possibleMove, uint64_t &onePiece, int &iLocation, bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    horses = board.bH & ~pinnedPieces;

    onePiece = horses&~(horses-1LL);        //A bit board with the least significant horse, incremently moves to the next horse etc.
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);                //Counts traliing zeros, ie gives position of piece, 0 is top left bottom right is 63
        possibleMoves = knightMasks[iLocation] & EnemyPieces & protectKing;
        possibleMove = possibleMoves & ~(possibleMoves - 1LL);        //To loop over the possible moves bitboard, and take one bit at a time
        while(possibleMove != 0){
            placeholder = board;
            placeholder.bH &= ~onePiece;                               //Remove the place were the horse was moving from
            placeholder.bH |= possibleMove;                            //Add the move
            placeholder.wQ &= ~possibleMove;                           //Capture, we remove white pieces that are on that square
            placeholder.wR &= ~possibleMove;
            placeholder.wB &= ~possibleMove;
            placeholder.wH &= ~possibleMove;
            placeholder.wP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                 
            possibleMove = possibleMoves&~(possibleMoves - 1LL);
        }
        horses&=~onePiece;                                        //erases the onePiece from horses for which all moves were calculated
        onePiece=horses&~(horses-1LL);                            //Gets the next least significant bit after removing the last one, if last one was the last, is 0
    }
}

void generateBBishopCaptures(uint64_t &bishops, const uint64_t &EnemyPieces, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                             uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &pinnedDiagonalBishops, const uint64_t &pinnedOtherDiagonalBishops,
                             bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    bishops = board.bB & ~pinnedPieces;

    bishops |= pinnedDiagonalBishops;              //Add diagonally pinned bishops 
    onePiece = bishops&~(bishops-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = diagMoves(iLocation, occupied) & EnemyPieces & protectKing;       //Diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bB &= ~onePiece;
            placeholder.bB |= possibleMove;                                     //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
            placeholder.wQ &= ~possibleMove;
            placeholder.wR &= ~possibleMove;
            placeholder.wB &= ~possibleMove;
            placeholder.wH &= ~possibleMove;
            placeholder.wP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        bishops &= ~onePiece;
        onePiece = bishops&~(bishops-1);
    }
    bishops = board.bB & ~pinnedPieces;                          //Reset bishops and remove all pinned bishops
    bishops |= pinnedOtherDiagonalBishops;                       //Add other diagonally pinned bishops
    onePiece = bishops&~(bishops-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = otherDiagMoves(iLocation, occupied) & EnemyPieces & protectKing;       //Diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bB &= ~onePiece;
            placeholder.bB |= possibleMove;                                     //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
            placeholder.wQ &= ~possibleMove;
            placeholder.wR &= ~possibleMove;
            placeholder.wB &= ~possibleMove;
            placeholder.wH &= ~possibleMove;
            placeholder.wP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        bishops &= ~onePiece;
        onePiece = bishops&~(bishops-1);
    }

}

void generateBRookCaptures(uint64_t &rooks, const uint64_t &EnemyPieces, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                           uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &horPinnedRooks, const uint64_t &verPinnedRooks,
                           bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    rooks = board.bR & ~pinnedPieces;

    rooks |= horPinnedRooks;              //Add horizontally pinned rooks
    onePiece = rooks&~(rooks-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = horMoves(iLocation, occupied) & EnemyPieces & protectKing;       
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bR &= ~onePiece;
            placeholder.bR |= possibleMove;                     //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
            placeholder.wQ &= ~possibleMove;
            placeholder.wR &= ~possibleMove;
            placeholder.wB &= ~possibleMove;
            placeholder.wH &= ~possibleMove;
            placeholder.wP &= ~possibleMove;
            if(iLocation == 7){
                placeholder.cBK = false;                        //Disallow future castling
            }else if(iLocation == 0){
                placeholder.cBQ = false;
            }
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        rooks &= ~onePiece;
        onePiece = rooks&~(rooks-1);
    }
    rooks = board.bR & ~pinnedPieces;                          //Reset rooks and remove all pinned rooks
    rooks |= verPinnedRooks;              //Add vertically pinned rooks
    onePiece = rooks&~(rooks-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = verMoves(iLocation, occupied) & EnemyPieces & protectKing;       //vermoves generates all horizontal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bR &= ~onePiece;
            placeholder.bR |= possibleMove;                     //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
            placeholder.wQ &= ~possibleMove;
            placeholder.wR &= ~possibleMove;
            placeholder.wB &= ~possibleMove;
            placeholder.wH &= ~possibleMove;
            placeholder.wP &= ~possibleMove;
            if(iLocation == 7){
                placeholder.cBK = false;                        //Disallow future castling
            }else if(iLocation == 0){
                placeholder.cBQ = false;
            }
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        rooks &= ~onePiece;
        onePiece = rooks&~(rooks-1);
    }
}

void generateBQueenCaptures(uint64_t &queens, const uint64_t &EnemyPieces, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                           uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &horPinnedQueens, const uint64_t &verPinnedQueens, const uint64_t &diagPinnedQueens, const uint64_t &diagOtherPinnedQueens,
                           bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    queens = board.bQ & ~pinnedPieces;

    queens |= horPinnedQueens;              //Add horizontally pinned rooks
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = horMoves(iLocation, occupied) & EnemyPieces & protectKing;       
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bQ &= ~onePiece;
            placeholder.bQ |= possibleMove;                                     //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
            placeholder.wQ &= ~possibleMove;
            placeholder.wR &= ~possibleMove;
            placeholder.wB &= ~possibleMove;
            placeholder.wH &= ~possibleMove;
            placeholder.wP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.bQ & ~pinnedPieces;
    //Diagonal queen captures
    queens |= diagPinnedQueens;              //Add diagonally pinned rooks
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = diagMoves(iLocation, occupied) & EnemyPieces & protectKing;       //diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bQ &= ~onePiece;
            placeholder.bQ |= possibleMove;                                     //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
            placeholder.wQ &= ~possibleMove;
            placeholder.wR &= ~possibleMove;
            placeholder.wB &= ~possibleMove;
            placeholder.wH &= ~possibleMove;
            placeholder.wP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.bQ & ~pinnedPieces;  
    //OtherDiagonal queen captures
    queens |= diagOtherPinnedQueens;              //Add otherdiag pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = otherDiagMoves(iLocation, occupied) & EnemyPieces & protectKing;       //otherdiagmoves generates all otherdiagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bQ &= ~onePiece;
            placeholder.bQ |= possibleMove;                                     //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
            placeholder.wQ &= ~possibleMove;
            placeholder.wR &= ~possibleMove;
            placeholder.wB &= ~possibleMove;
            placeholder.wH &= ~possibleMove;
            placeholder.wP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.bQ & ~pinnedPieces;  
    //Vertical queen captures
    queens |= verPinnedQueens;              //Add ver pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = verMoves(iLocation, occupied) & EnemyPieces & protectKing;       //vermoves generates all vertical moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bQ &= ~onePiece;
            placeholder.bQ |= possibleMove;                                     //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
            placeholder.wQ &= ~possibleMove;
            placeholder.wR &= ~possibleMove;
            placeholder.wB &= ~possibleMove;
            placeholder.wH &= ~possibleMove;
            placeholder.wP &= ~possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
}

void generateBKingCaptures(const uint64_t &blackKing, const uint64_t &EnemyPieces, const uint64_t &unSafe, uint64_t &possibleMoves,
                           uint64_t &possibleMove, int &iLocation, bitBoard &placeholder, const bitBoard &board, vector<bitBoard> &returnVector){

    iLocation = __builtin_ctzll(blackKing);
    possibleMoves = kingMasks[iLocation] & EnemyPieces & ~unSafe;
    possibleMove = possibleMoves&~(possibleMoves-1);
    while(possibleMove != 0){
        placeholder = board;
        placeholder.bK = possibleMove;              //Add the move to the placeholder board, normally we would have to add it with | and remove the original, but since only one king we can replace
        placeholder.wQ &= ~possibleMove;            //Since we are capturing, we need to erase the piece being captured, we dont know what kind of piece it is, so we erase all
        placeholder.wR &= ~possibleMove;
        placeholder.wB &= ~possibleMove;
        placeholder.wH &= ~possibleMove;
        placeholder.wP &= ~possibleMove;
        placeholder.cBK = false;                    //King move, castling privilidge revoked
        placeholder.cBQ = false;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         //goes to next move 
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
}

void generateBPawnMoves(uint64_t &blackPawns, const uint64_t &protectKing, const uint64_t &verPinnedPawns,
                        const uint64_t &Empty, uint64_t &possibleMoves, uint64_t &possibleMove,
                        bitBoard &placeholder, const bitBoard &board, const uint64_t &pinnedPieces, vector<bitBoard> &returnVector){

    blackPawns = board.bP & ~pinnedPieces;

    //Double push
    blackPawns |= verPinnedPawns;           //Add Vertiaclly pinned pawns
    possibleMoves = (blackPawns<<16) & Empty & (Empty<<8) & Rank5 & protectKing;
    possibleMove = possibleMoves&~(possibleMoves-1);
    while(possibleMove != 0){
        placeholder = board;
        placeholder.bP &= ~(possibleMove>>16);
        placeholder.bP |= possibleMove;
        placeholder.eP = possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
    //Pawn push
    possibleMoves = (blackPawns<<8) & Empty & ~Rank1 & protectKing;
    possibleMove = possibleMoves&~(possibleMoves-1);
    while(possibleMove != 0){
        placeholder = board;
        placeholder.bP &= ~(possibleMove>>8);
        placeholder.bP |= possibleMove;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
}

void generateBHorseMoves(uint64_t &horses, const uint64_t &Empty, const uint64_t &protectKing, uint64_t &possibleMoves, const uint64_t &pinnedPieces,
                         uint64_t &possibleMove, uint64_t &onePiece, int &iLocation, bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    horses = board.bH & ~pinnedPieces;

    onePiece = horses&~(horses-1LL);        //A bit board with the least significant horse, incremently moves to the next horse etc.
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);                //Counts traliing zeros, ie gives position of piece, 0 is top left bottom right is 63
        possibleMoves = knightMasks[iLocation] & Empty & protectKing;
        possibleMove = possibleMoves & ~(possibleMoves - 1LL);        //To loop over the possible moves bitboard, and take one bit at a time
        while(possibleMove != 0){
            placeholder = board;
            placeholder.bH &= ~onePiece;                               //Remove the place were the horse was moving from
            placeholder.bH |= possibleMove;                            //Add the move
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                
            possibleMove = possibleMoves&~(possibleMoves - 1LL);
        }
        horses&=~onePiece;                                        //erases the onePiece from horses for which all moves were calculated
        onePiece=horses&~(horses-1LL);                            //Gets the next least significant bit after removing the last one, if last one was the last, is 0
    }
}

void generateBBishopMoves(uint64_t &bishops, const uint64_t &Empty, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                          uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &pinnedDiagonalBishops, const uint64_t &pinnedOtherDiagonalBishops,
                          bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    bishops = board.bB & ~pinnedPieces;

    bishops |= pinnedDiagonalBishops;              //Add diagonally pinned bishops 
    onePiece = bishops&~(bishops-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = diagMoves(iLocation, occupied) & Empty & protectKing;       //Diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bB &= ~onePiece;
            placeholder.bB |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        bishops &= ~onePiece;
        onePiece = bishops&~(bishops-1);
    }
    bishops = board.bB & ~pinnedPieces;                          //Reset bishops and remove all pinned bishops
    bishops |= pinnedOtherDiagonalBishops;                       //Add other diagonally pinned bishops
    onePiece = bishops&~(bishops-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = otherDiagMoves(iLocation, occupied) & Empty & protectKing;        //Diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bB &= ~onePiece;
            placeholder.bB |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        bishops &= ~onePiece;
        onePiece = bishops&~(bishops-1);
    }
}

void generateBRookMoves(uint64_t &rooks, const uint64_t &Empty, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                        uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &horPinnedRooks, const uint64_t &verPinnedRooks,
                        bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    rooks = board.bR & ~pinnedPieces;

    rooks |= horPinnedRooks;              //Add horizontally pinned rooks
    onePiece = rooks&~(rooks-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = horMoves(iLocation, occupied) & Empty & protectKing;       
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bR &= ~onePiece;
            placeholder.bR |= possibleMove;
            if(iLocation == 7){
                placeholder.cBK = false;                        //Disallow future castling
            }else if(iLocation == 0){
                placeholder.cBQ = false;
            }
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        rooks &= ~onePiece;
        onePiece = rooks&~(rooks-1);
    }
    rooks = board.bR & ~pinnedPieces;                          //Reset rooks and remove all pinned rooks
    rooks |= verPinnedRooks;              //Add vertically pinned rooks
    onePiece = rooks&~(rooks-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = verMoves(iLocation, occupied) & Empty & protectKing;       //vermoves generates all horizontal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bR &= ~onePiece;
            placeholder.bR |= possibleMove;
            if(iLocation == 7){
                placeholder.cBK = false;                        //Disallow future castling
            }else if(iLocation == 0){
                placeholder.cBQ = false;
            }
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        rooks &= ~onePiece;
        onePiece = rooks&~(rooks-1);
    }
}

void generateBQueenMoves(uint64_t &queens, const uint64_t &Empty, const uint64_t &protectKing, uint64_t &possibleMoves, uint64_t &possibleMove, const uint64_t &pinnedPieces, 
                           uint64_t &onePiece,  const uint64_t &occupied, int &iLocation, const uint64_t &horPinnedQueens, const uint64_t &verPinnedQueens, const uint64_t &diagPinnedQueens, const uint64_t &diagOtherPinnedQueens,
                           bitBoard &placeholder, const bitBoard&board, vector<bitBoard> &returnVector){

    queens = board.bQ & ~pinnedPieces;

    queens |= horPinnedQueens;              //Add horizontally pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = horMoves(iLocation, occupied) & Empty & protectKing;       
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bQ &= ~onePiece;
            placeholder.bQ |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &=~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.bQ & ~pinnedPieces;
    //Diagonal queen moves
    queens |= diagPinnedQueens;              //Add diagonally pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = diagMoves(iLocation, occupied) & Empty & protectKing;       //diagmoves generates all diagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bQ &= ~onePiece;
            placeholder.bQ |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &= ~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.bQ & ~pinnedPieces;  
    //OtherDiagonal queen moves
    queens |= diagOtherPinnedQueens;              //Add otherdiag pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = otherDiagMoves(iLocation, occupied) & Empty & protectKing;       //otherdiagmoves generates all otherdiagonal moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bQ &= ~onePiece;
            placeholder.bQ |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &= ~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
    queens = board.bQ & ~pinnedPieces;  
    //Vertical queen moves
    queens |= verPinnedQueens;              //Add ver pinned queens
    onePiece = queens&~(queens-1);
    while(onePiece != 0){
        iLocation = __builtin_ctzll(onePiece);
        possibleMoves = verMoves(iLocation, occupied) & Empty & protectKing;       //vermoves generates all vertical moves from a square
        possibleMove = possibleMoves & ~(possibleMoves - 1);                    //Takes the least significant bit
        while(possibleMove != 0){                                               //Loop over all the moves for one piece
            placeholder = board;
            placeholder.bQ &= ~onePiece;
            placeholder.bQ |= possibleMove;
            returnVector.push_back(placeholder);
            possibleMoves &= ~possibleMove;                                      //Remove the least significant bit
            possibleMove = possibleMoves&~(possibleMoves - 1);                  //Take the least significant bit
        }
        queens  &= ~onePiece;
        onePiece = queens &~(queens -1);
    }
}

void generateBKingMoves(const uint64_t &blackKing, const uint64_t &Empty, const uint64_t &unSafe, uint64_t &possibleMoves,
                           uint64_t &possibleMove, int &iLocation, bitBoard &placeholder, const bitBoard &board, vector<bitBoard> &returnVector){

    iLocation = __builtin_ctzll(blackKing);
    possibleMoves = kingMasks[iLocation] & Empty & ~unSafe;
    possibleMove = possibleMoves&~(possibleMoves-1);
    while(possibleMove != 0){
        placeholder = board;
        placeholder.bK = possibleMove;              //Add the move to the placeholder board, normally we would have to add it with | and remove the original, but since only one king we can replace
        placeholder.cBK = false;                    //King move, castling privilidge revoked
        placeholder.cBQ = false;
        returnVector.push_back(placeholder);
        possibleMoves &= ~possibleMove;                         
        possibleMove = possibleMoves&~(possibleMoves-1);
    }
}

void generateBCastling(const bitBoard &board, const uint64_t &Empty, const uint64_t &unSafe, bitBoard &placeholder, vector<bitBoard> &returnVector){
    if(board.cBK){
        if((Empty>>5 & 1 == 1) && (Empty>>6 & 1 == 1) && (~unSafe>>4 & 1 == 1) && (~unSafe>>5 & 1 == 1) && (~unSafe>>6 & 1 == 1) && (board.bR>>7 & 1 == 1)){
            placeholder = board;
            placeholder.bK = 1LL<<6;
            placeholder.bR &= ~(1LL<<7);
            placeholder.bR |= 1LL<<5;
            placeholder.cBK = false;
            placeholder.cBQ = false;
            returnVector.push_back(placeholder);
        }
    }
    if(board.cBQ){
        if((Empty>>3 & 1 == 1) && (Empty>>2 & 1 == 1) && (Empty>>1 & 1 == 1) && (~unSafe>>4 & 1 == 1) && (~unSafe>>3 & 1 == 1) && (~unSafe>>2 & 1 == 1) && (board.bR & 1 == 1)){
            placeholder = board;
            placeholder.bK = 1LL<<2;
            placeholder.bR &= ~1LL;
            placeholder.bR |= 1LL<<3;
            placeholder.cBK = false;
            placeholder.cBQ = false;
            returnVector.push_back(placeholder);
        }
    }
}

//Generates all possible moves for black if <2 attackers
void blackMoves(const bitBoard &board, vector<bitBoard> &returnVector, const uint64_t &pinnedHorizontal, const uint64_t &pinnedVertical, 
                        const uint64_t &pinnedDiagonal, const uint64_t &pinnedOtherDiagonal, const uint64_t &pinnedPieces, const uint64_t &EnemyPieces,
                        const uint64_t &protectKing, const uint64_t &Empty, const uint64_t &unSafe, const uint64_t &occupied){

    //Initialize all needed variables

    bitBoard placeholder;           //A copy of this is pushed into returnVector for each possible move
    uint64_t possibleMoves;         //Used to store were a onePiece can move to, has multiple set bits as possible move locations
    uint64_t possibleMove;          //Used to loop over all possibleMoves
    uint64_t onePiece;              //Used to loop over all the pieces in for example horses etc. Note pawns have no need for this since their moves are generated by just bitshifting all of them
    int iLocation;                  //Used in locating a certain piece, used to call 

    //List of all types of pinned pieces that are needed
    const uint64_t horPinnedPawns = board.bP & pinnedHorizontal;
    const uint64_t verPinnedPawns = board.bP & pinnedVertical;
    const uint64_t diagPinnedPawns = board.bP & pinnedDiagonal;
    const uint64_t diagOtherPinnedPawns = board.bP & pinnedOtherDiagonal;

    const uint64_t horPinnedQueens = board.bQ & pinnedHorizontal;
    const uint64_t verPinnedQueens = board.bQ & pinnedVertical;
    const uint64_t diagPinnedQueens = board.bQ & pinnedDiagonal;
    const uint64_t diagOtherPinnedQueens = board.bQ & pinnedOtherDiagonal;

    const uint64_t pinnedDiagonalBishops = board.bB & pinnedDiagonal;
    const uint64_t pinnedOtherDiagonalBishops = board.bB & pinnedOtherDiagonal;

    const uint64_t horPinnedRooks = board.bR & pinnedHorizontal;
    const uint64_t verPinnedRooks = board.bR & pinnedVertical;

    //Use these variables since board is a reference, NOTE, these variables are changed, and need to be "reset" if wanted to be used again
    uint64_t pawns;
    uint64_t horses;
    uint64_t bishops;
    uint64_t rooks;
    uint64_t queens;
    
    //Enemypieces
    const uint64_t enemy359     = board.wH | board.wB | board.wR | board.wQ;
    const uint64_t enemy59      = board.wR | board.wQ;
    const uint64_t enemy9       = board.wQ;
    const uint64_t enemy135     = board.wP | board.wH | board.wB | board.wR;
    const uint64_t enemy13      = board.wP | board.wH | board.wB;
    const uint64_t enemy1       = board.wP;
    const uint64_t enemy3       = board.wH | board.wB;
    const uint64_t enemy5       = board.wR;
    

    //Pawn promotions
    generateBPawnPromotions(pawns, EnemyPieces, protectKing, diagPinnedPawns, diagOtherPinnedPawns, Empty, possibleMoves, possibleMove, placeholder, 
                            board, pinnedPieces, returnVector);
    
    //King captures
    generateBKingCaptures(board.bK, EnemyPieces, unSafe, possibleMoves, possibleMove, iLocation, placeholder, board, returnVector);
    
    //Pawn captures
    generateBPawnCaptures(pawns, EnemyPieces, protectKing, diagPinnedPawns, diagOtherPinnedPawns, Empty, possibleMoves, possibleMove, 
                            placeholder, board, pinnedPieces, occupied, returnVector);

    //Horse moves capturing enemy pieces, note a pinned horse can NEVER move
    generateBHorseCaptures(horses, EnemyPieces, protectKing, possibleMoves, pinnedPieces, possibleMove, onePiece, iLocation, placeholder, board, returnVector);

    //Bishop captures
    generateBBishopCaptures(bishops, EnemyPieces, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, 
                            pinnedDiagonalBishops, pinnedOtherDiagonalBishops, placeholder, board, returnVector);

    //All rook captures
    generateBRookCaptures(rooks, EnemyPieces, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, horPinnedRooks, 
                            verPinnedRooks, placeholder, board, returnVector);

    //All queen captures
    generateBQueenCaptures(queens, EnemyPieces, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, horPinnedQueens, 
                            verPinnedQueens, diagPinnedQueens, diagOtherPinnedQueens, placeholder, board, returnVector);

    //Castling
    generateBCastling(board, Empty, unSafe, placeholder, returnVector);

    //Pawn moves, double push and normal push, no push promotion, that is handled in pawn promotions
    generateBPawnMoves(pawns, protectKing, verPinnedPawns, Empty, possibleMoves, possibleMove, placeholder, board, pinnedPieces, returnVector);

    //Horse moves, no captures
    generateBHorseMoves(horses, Empty, protectKing, possibleMoves, pinnedPieces, possibleMove, onePiece, iLocation, placeholder, board, returnVector);

    //Bishop moves, no captures
    generateBBishopMoves(bishops, Empty, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, pinnedDiagonalBishops, 
                            pinnedOtherDiagonalBishops, placeholder, board, returnVector);

    //Rook moves, no captures
    generateBRookMoves(rooks, Empty, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, horPinnedRooks, 
                            verPinnedRooks, placeholder, board, returnVector);

    //Queen moves, no captures
    generateBQueenMoves(queens, Empty, protectKing, possibleMoves, possibleMove, pinnedPieces, onePiece, occupied, iLocation, horPinnedQueens, 
                        verPinnedQueens, diagPinnedQueens, diagOtherPinnedQueens, placeholder, board, returnVector);

    //King moves, no castling or capturing
    generateBKingMoves(board.bK, Empty, unSafe, possibleMoves, possibleMove, iLocation, placeholder, board, returnVector);

}

/**************************************************************************************/

//Call this function to get all white moves from outside the class
void generateWhiteMoves(bitBoard &b, vector<bitBoard> &allWhiteMoves){
    
    b.eP &= ~Rank4;         //Clears all own enpassantable pawns
    
    //Set variables to be used in generating moves
    const uint64_t MyPieces = b.wP|b.wH|b.wB|b.wR|b.wQ|b.wK,
    EnemyPieces = b.bP|b.bH|b.bB|b.bR|b.bQ,
    MovableSquares = ~MyPieces,
    Occupied = MyPieces | EnemyPieces | b.bK,
    Empty = ~Occupied,
    UnSafe = unSafeWhite(b.bK, b.bQ, b.bB, b.bH, b.bR, b.bP, b.wK, Occupied),
    
    //Set pieces attacking own king
    attackingPieces = (((b.wK >> 7) & b.bP) & ~AFile) | (((b.wK >> 9) & b.bP) & ~HFile) |
                      threatHorses(b.wK, b.bH) |
                      threatDiag(b.wK, b.bB, b.bQ, Occupied) | 
                      threatHorVer(b.wK, b.bR, b.bQ, Occupied),
    
    //Set pinned pieces
    pinnedDiagonal = pinnedDiag(b.wK, MyPieces, b.bQ, b.bB, Occupied),
    pinnedOtherDiagonal = pinnedOtherDiag(b.wK, MyPieces, b.bQ, b.bB, Occupied),
    pinnedHorizontal = pinnedHor(b.wK, MyPieces, b.bQ, b.bR, Occupied),
    pinnedVertical = pinnedVer(b.wK, MyPieces, b.bQ, b.bR, Occupied),
    pinnedPieces = pinnedHorizontal | pinnedVertical | pinnedDiagonal | pinnedOtherDiagonal;
    
    const int numberOfAttackes = __builtin_popcountll(attackingPieces);       //Counts the amout of attackers

    if(numberOfAttackes > 1){
        generateWhiteKingMoves(b, allWhiteMoves, MovableSquares, UnSafe);
    }else if(numberOfAttackes == 1){
        int attackLoc = __builtin_ctzll(attackingPieces);
        int kingLoc = __builtin_ctzll(b.wK);
        //All the possible locations were a move can be made to uncheck the king, ie either capture the attacking piece or block it,
        const uint64_t protectKing = (diagMoves(attackLoc, Occupied) & diagMoves(kingLoc, Occupied)) | (otherDiagMoves(attackLoc, Occupied) & otherDiagMoves(kingLoc, Occupied)) |
                      (horMoves(attackLoc, Occupied) & horMoves(kingLoc, Occupied)) | (verMoves(attackLoc, Occupied) & verMoves(kingLoc, Occupied)) | attackingPieces;
        whiteMoves(b, allWhiteMoves, pinnedHorizontal, pinnedVertical, pinnedDiagonal, pinnedOtherDiagonal, pinnedPieces, EnemyPieces, protectKing, Empty, UnSafe, Occupied);
    }else{
        const uint64_t protectKing = ~0LL;
        whiteMoves(b, allWhiteMoves, pinnedHorizontal, pinnedVertical, pinnedDiagonal, pinnedOtherDiagonal, pinnedPieces, EnemyPieces, protectKing, Empty, UnSafe, Occupied);
    }
}

//Call this function to get all black moves from outside the class
void generateBlackMoves(bitBoard &b, vector<bitBoard> &allBlackMoves){
    
    b.eP &= ~Rank5;         //Clears all own enpassantable pawns
    
    //Set variables to be used in generating moves
    const uint64_t MyPieces = b.bP|b.bH|b.bB|b.bR|b.bQ|b.bK,
    EnemyPieces = b.wP|b.wH|b.wB|b.wR|b.wQ,
    MovableSquares = ~MyPieces,
    Occupied = MyPieces | EnemyPieces | b.wK,
    Empty = ~Occupied,
    UnSafe = unSafeBlack(b.wK, b.wQ, b.wB, b.wH, b.wR, b.wP, b.bK, Occupied),
    
    //Set pieces attacking own king
    attackingPieces = (((b.bK << 7) & b.wP) & ~HFile) | (((b.bK << 9) & b.wP) & ~AFile) |
                      threatHorses(b.bK, b.wH) | 
                      threatDiag(b.bK, b.wB, b.wQ, Occupied) | 
                      threatHorVer(b.bK, b.wR, b.wQ, Occupied),
    
    //Set pinned pieces
    pinnedDiagonal = pinnedDiag(b.bK, MyPieces, b.wQ, b.wB, Occupied),
    pinnedOtherDiagonal = pinnedOtherDiag(b.bK, MyPieces, b.wQ, b.wB, Occupied),
    pinnedHorizontal = pinnedHor(b.bK, MyPieces, b.wQ, b.wR, Occupied),
    pinnedVertical = pinnedVer(b.bK, MyPieces, b.wQ, b.wR, Occupied),
    pinnedPieces = pinnedHorizontal | pinnedVertical | pinnedDiagonal | pinnedOtherDiagonal;

    const int numberOfAttackes = __builtin_popcountll(attackingPieces);       //Counts the amout of attackers

    if(numberOfAttackes > 1){
        generateBlackKingMoves(b, allBlackMoves, MovableSquares, UnSafe);
    }else if(numberOfAttackes == 1){
        int attackLoc = __builtin_ctzll(attackingPieces);
        int kingLoc = __builtin_ctzll(b.bK);
        //All the possible locations were a move can be made to uncheck the king, ie either capture the attacking piece or block it,
        const uint64_t protectKing = (diagMoves(attackLoc, Occupied) & diagMoves(kingLoc, Occupied)) | (otherDiagMoves(attackLoc, Occupied) & otherDiagMoves(kingLoc, Occupied)) |
                      (horMoves(attackLoc, Occupied) & horMoves(kingLoc, Occupied)) | (verMoves(attackLoc, Occupied) & verMoves(kingLoc, Occupied)) | attackingPieces;
        blackMoves(b, allBlackMoves, pinnedHorizontal, pinnedVertical, pinnedDiagonal, pinnedOtherDiagonal, pinnedPieces, EnemyPieces, protectKing, Empty, UnSafe, Occupied);
    }else{
        const uint64_t protectKing = ~0LL;
        blackMoves(b, allBlackMoves, pinnedHorizontal, pinnedVertical, pinnedDiagonal, pinnedOtherDiagonal, pinnedPieces, EnemyPieces, protectKing, Empty, UnSafe, Occupied);
    }
}

/**************************************************************************************/
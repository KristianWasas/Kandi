#pragma once

#include "bitboard.hpp"
#include <stdlib.h>

//Values are as, first 64 whitepawns, then white knights, then bishops, rooks, queens, king, then same for black
//Now we have 12*64 values, next we have side to move, castling rights and en passant file, thus ending with
//12*64+1+4+8 = 781 values, these will be used to generate a unique key
static uint64_t zobristValues[781];

//Global hashtable
static unordered_map<uint64_t, pair<int32_t, uint8_t>> table;

//Initialize random 64 bit numbers to zobrist values
void initializeZValues(){

    for(int i=0; i<781; i++){
        uint64_t randomNumber = 0LL;
        for(int j=0; j<8; j++){
            uint64_t a = rand() & 255;
            randomNumber |= a<<(j*8);      //Since rand() doesnt produce 64 bit numbers we create them like so
        }
        zobristValues[i] = randomNumber;
    }

}

//Function to create a unique key for each board state
uint64_t hashFunction(bitBoard &board, bool whiteTurn){
    
    uint64_t hashKey = 0LL;

    if(whiteTurn){
        hashKey ^= zobristValues[768];
    }

    for(int i=0; i<64; i++){
        if(board.wP>>i & 1){
            hashKey ^= zobristValues[i];
        }else if(board.wH>>i & 1){
            hashKey ^= zobristValues[i + 64];
        }else if(board.wB>>i & 1){
            hashKey ^= zobristValues[i + 64*2];
        }else if(board.wR>>i & 1){
            hashKey ^= zobristValues[i + 64*3];
        }else if(board.wQ>>i & 1){
            hashKey ^= zobristValues[i + 64*4];
        }else if(board.wK>>i & 1){
            hashKey ^= zobristValues[i + 64*5];
        }else if(board.bP>>i & 1){
            hashKey ^= zobristValues[i + 64*6];
        }else if(board.bH>>i & 1){
            hashKey ^= zobristValues[i + 64*7];
        }else if(board.bB>>i & 1){
            hashKey ^= zobristValues[i + 64*8];
        }else if(board.bR>>i & 1){
            hashKey ^= zobristValues[i + 64*9];
        }else if(board.bQ>>i & 1){
            hashKey ^= zobristValues[i + 64*10];
        }else if(board.bK>>i & 1){
            hashKey ^= zobristValues[i + 64*11];
        }
        if(board.eP>>i & 1){
            hashKey ^= zobristValues[i%8 + 12*64+1+4];
        }
    }
    if(board.cWK){
        hashKey ^= zobristValues[12*64+1];
    }
    if(board.cWQ){
        hashKey ^= zobristValues[12*64+2];
    }
    if(board.cBK){
        hashKey ^= zobristValues[12*64+3];
    }
    if(board.cBQ){
        hashKey ^= zobristValues[12*64+4];
    }

    return hashKey;
}
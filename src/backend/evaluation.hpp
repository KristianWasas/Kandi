#pragma once

#include "bitboard.hpp"
#include "lookuptable.hpp"
#include "testing/tests.hpp"

int evaluation(bitBoard board){

    const int gamePhase = __builtin_popcountll(board.wQ | board.bQ) * 4 + __builtin_popcountll(board.wR |board.bR) * 2 + __builtin_popcountll(board.wH | board.bH | board.wB | board.bB);

    int mgEval = 0;
    int egEval = 0;
    
    uint64_t onePiece = board.wP & ~(board.wP - 1);         //Variable to isolate one piece at a time and give an evaluation for said piece 
    //Eval pawns
    while(onePiece != 0){
        mgEval = mgEval + mg_pawn_table_w[__builtin_ctzll(onePiece)] + 100;
        egEval = egEval + eg_pawn_table_w[__builtin_ctzll(onePiece)] + 100;
        board.wP &= ~onePiece;
        onePiece = board.wP & ~(board.wP - 1);
    }
    onePiece = board.bP & ~(board.bP - 1);
    while(onePiece != 0){
        mgEval = mgEval - mg_pawn_table_w[63 - __builtin_ctzll(onePiece)] - 100;
        egEval = egEval - eg_pawn_table_w[63 - __builtin_ctzll(onePiece)] - 100;
        board.bP &= ~onePiece;
        onePiece = board.bP & ~(board.bP - 1);
    }
    //Eval knights
    onePiece = board.wH & ~(board.wH - 1);
    while(onePiece != 0){
        mgEval = mgEval + mg_knight_table_w[__builtin_ctzll(onePiece)] + 300;
        egEval = egEval + eg_knight_table_w[__builtin_ctzll(onePiece)] + 300;
        board.wH &= ~onePiece;
        onePiece = board.wH & ~(board.wH - 1);
    }
    onePiece = board.bH & ~(board.bH - 1);
    while(onePiece != 0){
        mgEval = mgEval - mg_knight_table_w[63 - __builtin_ctzll(onePiece)] - 300;
        egEval = egEval - eg_knight_table_w[63 - __builtin_ctzll(onePiece)] - 300;
        board.bH &= ~onePiece;
        onePiece = board.bH & ~(board.bH - 1);
    }
    //Eval Bishops
    onePiece = board.wB & ~(board.wB - 1);
    while(onePiece != 0){
        mgEval = mgEval + mg_bishop_table_w[__builtin_ctzll(onePiece)] + 320;
        egEval = egEval + eg_bishop_table_w[__builtin_ctzll(onePiece)] + 320;
        board.wB &= ~onePiece;
        onePiece = board.wB & ~(board.wB - 1);
    }
    onePiece = board.bB & ~(board.bB - 1);
    while(onePiece != 0){
        mgEval = mgEval - mg_bishop_table_w[63 - __builtin_ctzll(onePiece)] - 320;
        egEval = egEval - eg_bishop_table_w[63 - __builtin_ctzll(onePiece)] - 320;
        board.bB &= ~onePiece;
        onePiece = board.bB & ~(board.bB - 1);
    }
    //Eval Rooks
    onePiece = board.wR & ~(board.wR - 1);
    while(onePiece != 0){
        mgEval = mgEval + mg_rook_table_w[__builtin_ctzll(onePiece)] + 500;
        egEval = egEval + eg_rook_table_w[__builtin_ctzll(onePiece)] + 500;
        board.wR &= ~onePiece;
        onePiece = board.wR & ~(board.wR - 1);
    }
    onePiece = board.bR & ~(board.bR - 1);
    while(onePiece != 0){
        mgEval = mgEval - mg_rook_table_w[63 - __builtin_ctzll(onePiece)] - 500;
        egEval = egEval - eg_rook_table_w[63 - __builtin_ctzll(onePiece)] - 500;
        board.bR &= ~onePiece;
        onePiece = board.bR & ~(board.bR - 1);
    }
    //Eval queens
    onePiece = board.wQ & ~(board.wQ - 1);
    while(onePiece != 0){
        mgEval = mgEval + mg_queen_table_w[__builtin_ctzll(onePiece)] + 900;
        egEval = egEval + eg_queen_table_w[__builtin_ctzll(onePiece)] + 900;
        board.wQ &= ~onePiece;
        onePiece = board.wQ & ~(board.wQ - 1);
    }
    onePiece = board.bQ & ~(board.bQ - 1);
    while(onePiece != 0){
        mgEval = mgEval - mg_queen_table_b[63 - __builtin_ctzll(onePiece)] - 900;       //Note queens have separate mg tables for colors, since they are mirrored
        egEval = egEval - eg_queen_table_w[63 - __builtin_ctzll(onePiece)] - 900;       //eg tables are still identical
        board.bQ &= ~onePiece;
        onePiece = board.bQ & ~(board.bQ - 1);
    }
    //Eval kings   
    mgEval = mgEval + mg_king_table_w[__builtin_ctzll(board.wK)];                //only one king no need to loop
    egEval = egEval + eg_king_table_w[__builtin_ctzll(board.wK)];                //kings have only positional scores, no piece score since they are invaluable, ie you lose if eaten
    mgEval = mgEval - mg_king_table_b[63 - __builtin_ctzll(board.bK)];           //Note kings have separate mg tables for colors, since they are mirrored
    egEval = egEval - eg_king_table_w[63 - __builtin_ctzll(board.bK)];      

    int finalEval = (mgEval * gamePhase + egEval * (24 - gamePhase))/24;
    return finalEval;
}
#pragma once

#include "GUI/Game/piece.cpp"

#include <cstdint>
#include <iostream>
#include <bitset>
#include <map>

using namespace std;

//Has all kinds of helper functinos for bitBoard structs

typedef struct{
    //Board data is held in these 13 long longs and 4 bools, turn is handled outside the data struct
    uint64_t wP, wH, wB, wR, wQ, wK,        //White pieces
             bP, bH, bB, bR, bQ, bK,        //Black pieces
             eP;                            //En passsant
    bool cWQ, cWK, cBQ, cBK;                //Castling rights 
}bitBoard;

//Comparison operator overload for bitboard, returns true if the bitboards are equal
bool operator==(bitBoard board1, bitBoard board2){
    return (board1.wP == board2.wP) && (board1.wH == board2.wH) && (board1.wB == board2.wB) && (board1.wR == board2.wR) && (board1.wQ == board2.wQ) && (board1.wK == board2.wK) && 
           (board1.bP == board2.bP) && (board1.bH == board2.bH) && (board1.bB == board2.bB) && (board1.bR == board2.bR) && (board1.bQ == board2.bQ) && (board1.bK == board2.bK) && 
           (board1.eP == board2.eP) && 
           (board1.cWK == board2.cWK) && (board1.cWQ == board2.cWQ) && (board1.cBK == board2.cBK) && (board1.cBQ == board2.cBQ);
}

//Comparison again
bool operator!=(bitBoard board1, bitBoard board2){
    return (board1.wP != board2.wP) || (board1.wH != board2.wH) || (board1.wB != board2.wB) || (board1.wR != board2.wR) || (board1.wQ != board2.wQ) || (board1.wK != board2.wK) || 
           (board1.bP != board2.bP) || (board1.bH != board2.bH) || (board1.bB != board2.bB) || (board1.bR != board2.bR) || (board1.bQ != board2.bQ) || (board1.bK != board2.bK) || 
           (board1.eP != board2.eP) || 
           (board1.cWK != board2.cWK) || (board1.cWQ != board2.cWQ) || (board1.cBK != board2.cBK) || (board1.cBQ != board2.cBQ);
}

void draw64(uint64_t number, string output = " O "){               //Draws a single 64 bit number as a 8x8 square with set bits as 1 and nonset bits as .
    for(int rank = 0; rank<8; rank++){
        for(int file = 0; file<8; file++){
            if(number >> (rank * 8 + file) & 1 == 1){
                cout << output;
            }else{cout << " . ";}
        }
        cout << "\n";
    }
};

void drawBitBoard(bitBoard b){             //Draws a bitBoards all uint64_t 
    cout << "White pawns:" << "\n";
    draw64(b.wP);
    cout << "White horses:" << "\n";
    draw64(b.wH);
    cout << "White bishops:" << "\n";
    draw64(b.wB);
    cout << "White rooks:" << "\n";
    draw64(b.wR);
    cout << "White queens:" << "\n";
    draw64(b.wQ);
    cout << "White king:" << "\n";
    draw64(b.wK);
    cout << "Black pawns:" << "\n";
    draw64(b.bP);
    cout << "Black horses:" << "\n";
    draw64(b.bH);
    cout << "Black bishops:" << "\n";
    draw64(b.bB);
    cout << "Black rooks:" << "\n";
    draw64(b.bR);
    cout << "Black queens:" << "\n";
    draw64(b.bQ);
    cout << "Black king:" << "\n";
    draw64(b.bK);
    cout << "En passants:" << "\n";
    draw64(b.eP);
    cout << "Castle rights wk, wq, bk, bq: " << b.cWK << " " << b.cWQ << " " << b.cBK << " " << b.cBQ << "\n";
};

bitBoard FENtoBitBoard(string input){           //Converts a FEN string to a bitboard

    bitBoard r;
    r.wP = 0LL, r.wH = 0LL, r.wB = 0LL, r.wR = 0LL, r.wQ = 0LL, r.wK = 0LL, r.bP = 0LL, r.bH = 0LL, r.bB = 0LL, r.bR = 0LL, r.bQ = 0LL, r.bK = 0LL, r.eP = 0LL;
    r.cWQ = false, r.cWK = false, r.cBQ = false, r.cBK = false;
    
    map<char, int> enPassantMap = {{'3', 5}, {'6', 3}};
    int rank = 0;
    int file = 0;
    bool firstSpace = false;
    bool secondSpace = false;
    bool thirdSpace = false;
    for(int i = 0; i<input.size(); i++){
        if(input[i] == ' '){
            if(secondSpace){
                thirdSpace = true;
            }
            if(firstSpace){
                secondSpace = true;
            }
            firstSpace = true;
        }
        if(secondSpace && !thirdSpace){         //Castling
            if(input[i] == 'K'){
                r.cWK = true;
            }else if(input[i] == 'Q'){
                r.cWQ = true;
            }else if(input[i] == 'q'){
                r.cBQ = true;
            }else if(input[i] == 'k'){
                r.cBK = true;
            }
        }
        if(thirdSpace && input[i] != ' '){                     //Enpassant
            if(input[i] == '-'){
                break;
            }else{
                int square = input[i] - 'a' + enPassantMap[input[i+1]] * 8;
                r.eP |= 1LL<<square;
                break;
            }
        }
        if(!firstSpace){
            if(input[i] == '/'){                       //Board representation
                file = 0;
                rank = rank + 1;
            }else if(input[i] > 48 && input[i] < 57){
                file = file + input[i] - '0';
            }else if(input[i] == 'K'){
                r.wK |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'Q'){
                r.wQ |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'R'){
                r.wR |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'B'){
                r.wB |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'N'){
                r.wH |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'P'){
                r.wP |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'k'){
                r.bK |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'q'){
                r.bQ |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'r'){
                r.bR |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'b'){
                r.bB |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'n'){
                r.bH |= 1LL<<(rank*8+file);
                file = file + 1;
            }else if(input[i] == 'p'){
                r.bP |= 1LL<<(rank*8+file);
                file = file + 1;
            }
        }
    }
    return  r;
}

//Note that since the turn is evaluated outside the board, we always return white turn, also no turn counter
void boardToFEN(bitBoard board){
    string output = "";
    string enPassant = "-";
    int count = 0;
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            int a = j+i*8;
            if(board.wP>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "P";
            }else if(board.wH>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "N";
            }else if(board.wB>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "B";
            }else if(board.wR>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "R";
            }else if(board.wQ>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "Q";
            }else if(board.wK>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "K";
            }else if(board.bP>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "p";
            }else if(board.bH>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "n";
            }else if(board.bB>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "b";
            }else if(board.bR>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "r";
            }else if(board.bQ>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "q";
            }else if(board.bK>>a & 1){
                if(count){
                    output += to_string(count);
                    count = 0;
                }
                output += "K";
            }else{
                count += 1;
            }
            if(board.eP>>a & 1){
                enPassant = (char)('a' + j);
                string epRank = "";
                if(i==3){
                    epRank = "6";
                }else{
                    epRank = "3";
                }
                enPassant += epRank;
            }
            if((j == 7) && (count)){
                output += to_string(count);
                count = 0;
            }
        }
        if(i != 7){
            output += "/";
        }
    }
    output += " b ";
    string castling = "-";
    if(board.cWK|board.cWQ|board.cBK|board.cBQ){
        castling = "";
    }
    if(board.cWK){
        castling += "K";
    }
    if(board.cWQ){
        castling += "Q";
    }
    if(board.cBK){
        castling += "k";
    }
    if(board.cBQ){
        castling += "q";
    }
    output += castling;
    output += " ";
    output += enPassant;
    cout << output << "\n";
}

void createPieces(bitBoard board, vector<Piece*>& pieces){
    for(int rank = 0; rank < 8; ++rank){
        for(int file = 0; file < 8; ++file){
            if(board.wP>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("w_pawn", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.wH>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("w_knight", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.wB>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("w_bishop", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.wR>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("w_rook", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.wQ>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("w_queen", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.wK>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("w_king", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.bP>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("b_pawn", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.bH>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("b_knight", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.bB>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("b_bishop", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.bR>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("b_rook", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.bQ>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("b_queen", make_pair(rank, file));
                pieces.push_back(newPiece);
            }else if(board.bK>>(file+rank*8) & 1){
                Piece* newPiece = new Piece("b_king", make_pair(rank, file));
                pieces.push_back(newPiece);
            }
        }
    }
}

void drawBoard(bitBoard b){
    for(int i = 0; i<8; ++i){
        for(int j = 0; j<8; ++j){
            if((b.wP>>(j+8*i)) & 1){
                cout << " P ";
            }else if((b.wH>>(j+8*i)) & 1){
                cout << " N ";
            }else if((b.wB>>(j+8*i)) & 1){
                cout << " B ";
            }else if((b.wR>>(j+8*i)) & 1){
                cout << " R ";
            }else if((b.wQ>>(j+8*i)) & 1){
                cout << " Q ";
            }else if((b.wK>>(j+8*i)) & 1){
                cout << " K ";
            }else if((b.bP>>(j+8*i)) & 1){
                cout << " p ";
            }else if((b.bH>>(j+8*i)) & 1){
                cout << " n ";
            }else if((b.bB>>(j+8*i)) & 1){
                cout << " b ";
            }else if((b.bR>>(j+8*i)) & 1){
                cout << " r ";
            }else if((b.bQ>>(j+8*i)) & 1){
                cout << " q ";
            }else if((b.bK>>(j+8*i)) & 1){
                cout << " k ";
            }else{
                cout << " . ";
            }
        }
        cout << "\n";
    }
}

void drawMove(bitBoard first, bitBoard second, bool whiteTurn){

    if(whiteTurn){
        if(first.wP ^ second.wP){
            draw64(first.wP ^ second.wP, " P ");
        }else if(first.wH ^ second.wH){
            draw64(first.wH ^ second.wH, " N ");
        }else if(first.wB ^ second.wB){
            draw64(first.wB ^ second.wB, " B ");
        }else if(first.wK ^ second.wK){
            draw64(first.wK ^ second.wK, " K ");
        }else if(first.wR ^ second.wR){
            draw64(first.wR ^ second.wR, " R ");
        }else if(first.wQ ^ second.wQ){
            draw64(first.wQ ^ second.wQ, " Q ");
        }
    }else{
        if(first.bP ^ second.bP){
            draw64(first.bP ^ second.bP, " p ");
        }else if(first.bH ^ second.bH){
            draw64(first.bH ^ second.bH, " n ");
        }else if(first.bB ^ second.bB){
            draw64(first.bB ^ second.bB, " b ");
        }else if(first.bK ^ second.bK){
            draw64(first.bK ^ second.bK, " k ");
        }else if(first.bR ^ second.bR){
            draw64(first.bR ^ second.bR, " r ");
        }else if(first.bQ ^ second.bQ){
            draw64(first.bQ ^ second.bQ, " q ");
        }
    }
}
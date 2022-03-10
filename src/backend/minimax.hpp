#pragma once

#include "evaluation.hpp"
#include "testing/tests.hpp"
#include "movegenerator.hpp"
#include "quiescencesearch.hpp"
#include "transpositiontable.hpp"

//Values for checkmates
#define INF (int32_t)(999999)
#define NEGINF -INF
//The null move reduction in depth
#define NMDEPTHREDUCTION 2
//Toggles for versions of the algorithm
#define AB true
#define NULLMOVE (true && AB)   //Null move requires AB pruning to work
#define QSEARCH true
#define TT !true

//The null move reduction in depth for minimax2
#define NMDEPTHREDUCTION2 1
//Toggles for versions of the algorithm minimax2
#define AB2 true
#define NULLMOVE2 (!true && AB)   //Null move requires AB pruning to work
#define QSEARCH2 true
#define TT2 !true

//returns neginf if the white king is in check, ie really bad, if not returns 0, stalemate,
//used in searchMove to return an evaluation for checkmates and stalemates, ie a position were white has no more moves
int32_t whiteKingCheck(bitBoard& board){

    const uint64_t occ = board.bK | board.bQ | board.bB | board.bH | board.bR | board.bP | board.wK | board.wQ | board.wB | board.wH | board.wR | board.wP;
    if((board.wK & unSafeWhite(board.bK, board.bQ, board.bB, board.bH, board.bR, board.bP, board.wK, occ)) != 0){
        return NEGINF;
    }
    return 0;
}
//same for black
int32_t blackKingCheck(bitBoard& board){

    const uint64_t occ = board.bK | board.bQ | board.bB | board.bH | board.bR | board.bP | board.wK | board.wQ | board.wB | board.wH | board.wR | board.wP;
    if((board.bK & unSafeBlack(board.wK, board.wQ, board.wB, board.wH, board.wR, board.wP, board.bK, occ)) != 0){
        return INF;
    }
    return 0;
}

int32_t qSearch(bitBoard &board, uint8_t depth, bool whiteTurn, int32_t alpha, int32_t beta, bool nullTree){

    //Note that qSearch should be self collapsing, ie. it will stop on its own when it finds a quiet position
    //nullTree is only for mapping tree structure, ie. if true, then its a nullmove search, so we dont add it to the tree structure
    
    if(!nullTree){
        quiescenceNodesAtDepth[depth-1] += 1;        
    }      

    //Transposition table check
    #if TT
        uint64_t posHash = hashFunction(board, whiteTurn);
        if(table.count(posHash)){
            if(table[posHash].second == depth){
                leafNodes += 1;                             //If the current depth is larger or equal to the depth the position was originally searched in, return the value
                return table[posHash].first;             
            }
        }
    #endif

    int32_t standingEval = evaluation(board);                       //The static evaluation of current position

    //AB pruning
    #if AB
        if(whiteTurn){                                          //We assume that there exists a move that keeps the current standing eval
            if(standingEval >= beta){                           //ie. a move that doesnt weaken the position
                if(!nullTree){                                  //if beta (for white, alpha for black) is smaller than current eval, we can 
                    leafNodes += 1;                             //exit, since we know the opposition will never choose to come here, since by assumption, 
                }                                               //there exists a move that is atleast equal to standing eval, and return the standingEval
                return standingEval;                            
            }
            alpha = max(standingEval, alpha);                   //If we dont fail high, we update alpha
        }else{                                                  
            if(standingEval <= alpha){                         
                if(!nullTree){
                    leafNodes += 1;       
                } 
                return standingEval;
            }
            beta = min(standingEval, beta);
        }
    #endif

    vector<bitBoard> moves;                             //Initialize needed variables
    int32_t eval = standingEval;                        //Here the minimum eval is the standing eval, since now we only search captures, we assume there is a move that preserves the eval
    moves.reserve(10);

    if(whiteTurn){                                      //White ie. maximizing player
        generateWhiteNonQuietMoves(board, moves);       //Generate all possible moves for white that could impact the eval, ie. "non-quiet", and store them in moves
        if(moves.empty()){                              //If no moves possible, we have reached a quiet position, and can return the evaluation        
            if(!nullTree){
                leafNodes += 1;                         //Since if there are no more moves, we have reached a leafnode
            }                              
            return standingEval;
        }                               
        for(bitBoard &i : moves){                       //Loop over all non-quiet moves, and recursively call qsearch, if found move is better than existing one, replace
            int32_t moveEval = qSearch(i, depth + 1, false, alpha, beta, nullTree);
            eval = max(eval, moveEval);     
            
            #if AB
                if(beta <= eval){                           //If there exists a beta (beta being the best option for the minimizer in a parent node so far)
                    return eval;                            //That is smaller than the current value of the node we are in, we know that the minimizing player
                }                                           //Would never choose this path, since a better one is available, and we can terminate
                alpha = max(alpha, eval);                   //We update alpha to be the best value
            #endif

        }
    }
    else{                                               //Minimizing player, same process as for white, but we minimize and return INF if in checkmate
        generateBlackNonQuietMoves(board, moves);
        if(moves.empty()){         
            if(!nullTree){
                leafNodes += 1;                         
            }                             
            return standingEval;
        }                        
        for(bitBoard &i : moves){
            int32_t moveEval = qSearch(i, depth + 1, true, alpha, beta, nullTree);
            eval = min(eval, moveEval);                 //Same process as for the maximizer, but mirrored and using alpha (best option for a maximizing parent node)
            
            #if AB
                if(alpha >= eval){
                    return eval;
                }
                beta = min(beta, eval);
            #endif

        }
    }
    
    #if TT      //Insert value into table
        table.emplace(posHash, make_pair(eval, depth));
    #endif
        
    return eval;                                    //We return the value of the best move/child node, OR the if not capturing is better, we return standing eval
}

//Search were we dont conduct null moves, this is called when searching a null move, so we dont search nullmove into null move into null move etc
int32_t minimaxNoNullMove(bitBoard &board, uint8_t depth, bool whiteTurn, uint8_t maxDepth, int32_t alpha, int32_t beta){
    
    //Check if the position has already been searched by looking it up from the table
    
    #if TT
        uint64_t posHash = hashFunction(board, whiteTurn);
        if(table.count(posHash)){
            if(table[posHash].second == depth){
                leafNodes += 1;                             //If the current depth is larger or equal to the depth the position was originally searched in, return the value
                return table[posHash].first;             
            }
        }
    #endif

    if(depth >= maxDepth){                              //Once weve reached the desired depth, we just return the evaluation/Qsearch
        
        #if QSEARCH
            return qSearch(board, depth+1, !whiteTurn, alpha, beta, true);    //Call the qSearch once reached the 
        #endif

        return evaluation(board);
    }

    vector<bitBoard> moves;                             //Initialize needed variables
    int32_t eval;
    moves.reserve(50);

    if(whiteTurn){                                      //White ie. maximizing player
        generateWhiteMoves(board, moves);               //Generate all possible moves for white, and store them in moves
        if(moves.empty()){                              //If no moves possible, check is the white king in check and return NEGINF if yes or 0 if in stalemate
            return whiteKingCheck(board);
        }
        eval = NEGINF;                                  
        for(bitBoard &i : moves){                       //Loop over all possible moves, and recursively call minimax, if found move is better than existing one, replace
            int32_t moveEval = minimaxNoNullMove(i, depth + 1, false, maxDepth, alpha, beta);
            eval = max(eval, moveEval);     
            
            #if AB
                if(beta <= eval){                           //If there exists a beta (beta being the best option for the minimizer in a parent node so far)
                    break;                                  //That is smaller than the current value of the node we are in, we know that the minimizing player
                }                                           //Would never choose this path, since a better one is available, and we can terminate, the search 
                alpha = max(alpha, eval);                   //We update alpha to be the best value
            #endif

        }
    }
    else{                                               //Minimizing player, same process as for white, but we minimize and return INF if in checkmate
        generateBlackMoves(board, moves);
        if(moves.empty()){
            return blackKingCheck(board);
        }
        eval = INF;                                     //Again we initialize, but as INF, since black tries to minimize the score
        for(bitBoard &i : moves){
            int32_t moveEval = minimaxNoNullMove(i, depth + 1, true, maxDepth, alpha, beta);
            eval = min(eval, moveEval);                 //Same process as for the maximizer, but mirrored and using alpha (best option for a maximizing parent node)
            
            #if AB
                if(alpha >= eval){
                    return alpha;
                }
                beta = min(beta, eval);
            #endif

        }
    }

    #if TT      //Insert value into table
        table.emplace(posHash, make_pair(eval, depth));
    #endif

    return eval;                                    //We return the value of the best move/child node
}

//Primary search algorithm
int32_t minimax(bitBoard &board, uint8_t depth, bool whiteTurn, uint8_t maxDepth, int32_t alpha, int32_t beta){

    nodesAtDepth[depth] += 1; 
    
    //Transpostion table lookup 
    #if TT
        uint64_t posHash = hashFunction(board, whiteTurn);
        if(table.count(posHash)){
            //if((table[posHash].second) <= depth){    //Check that the value that is found was searched higher up in the tree or same depth
                leafNodes += 1;
                return table[posHash].first;
            //}
        }
    #endif

    if(depth >= maxDepth){                              //Once weve reached the desired depth, we just return the evaluation
        
        #if QSEARCH
            return qSearch(board, depth+1, !whiteTurn, alpha, beta, false);    //Call the qSearch 
        #endif

        leafNodes += 1;
        return evaluation(board);
    }

    vector<bitBoard> moves;                             //Initialize needed variables
    int32_t eval;
    moves.reserve(50);

    if(whiteTurn){                                      //White ie. maximizing player

        #if NULLMOVE
            //Nullmove pruning
            //Try a null move if ones desired, and aslong as we are not in Check
            if((maxDepth > depth + 1 + NMDEPTHREDUCTION) && (whiteKingCheck(board) == 0)){                           
                int32_t nullMove2alue = minimaxNoNullMove(board, depth + 1 + NMDEPTHREDUCTION, false, maxDepth, alpha, beta);
                if(nullMove2alue >= beta){                  //If the null move value fails high, return value
                    leafNodes += 1;
                    return nullMove2alue;
                }                                         
                //alpha = max(alpha, nullMove2alue);          //Assume that exists a move that is atleast as good as nullmove                                                      
            }
        #endif

        generateWhiteMoves(board, moves);               //Generate all possible moves for white, and store them in moves
        
        if(moves.empty()){                              //If no moves possible, check is the white king in check and return NEGINF if yes or 0 if in stalemate
            leafNodes += 1;                             //Since if there are no more moves, we have reached a leafnode
            return whiteKingCheck(board);               //Here there is no need to add the position to the transposition table, 
        }                                               //since searching the position normally would yield in no increase of the tree size

        eval = NEGINF;                                  
        for(bitBoard &i : moves){                       //Loop over all possible moves, and recursively call minimax, if found move is better than existing one, replace
            int32_t moveEval = minimax(i, depth + 1, false, maxDepth, alpha, beta);
            if((depth == 0) && (moveEval > eval)){      //Just to keep track of the best move from the original position
                bestMove.first = i;
                bestMove.second = moveEval;
            }
            eval = max(eval, moveEval);     
            
            #if AB
                if(beta <= eval){                           //If there exists a beta (beta being the best option for the minimizer in a parent node so far)
                    break;                                  //That is smaller than the current value of the node we are in, we know that the minimizing player
                }                                           //Would never choose this path, since a better one is available, and we can terminate, the search 
                alpha = max(alpha, eval);                   //We update alpha to be the best value
            #endif

        }
    }
    else{                                               //Minimizing player, same process as for white, but we minimize and return INF if in checkmate
        
        #if NULLMOVE
            //Nullmove pruning
            //Try a null move if ones desired, and aslong as we are not in Check
            if((maxDepth > depth + 1 + NMDEPTHREDUCTION) && (blackKingCheck(board) == 0)){                           
                int32_t nullMove2alue = minimaxNoNullMove(board, depth + 1 + NMDEPTHREDUCTION, true, maxDepth, alpha, beta);
                if(nullMove2alue <= alpha){                  //If the null move value fails low, return value
                    return nullMove2alue;
                }                                           
                //beta = min(beta, nullMove2alue); 
            }
        #endif
        
        generateBlackMoves(board, moves);

        if(moves.empty()){
            leafNodes += 1;                             //Since if there are no more moves, we have reached a leafnode
            return blackKingCheck(board);
        }

        eval = INF;                                     //Again we initialize, but as INF, since black tries to minimize the score
        for(bitBoard &i : moves){
            int32_t moveEval = minimax(i, depth + 1, true, maxDepth, alpha, beta);
            if((depth == 0) && (moveEval < eval)){
                bestMove.first = i;
                bestMove.second = moveEval;
            }
            eval = min(eval, moveEval);                 //Same process as for the maximizer, but mirrored and using alpha (best option for a maximizing parent node)     
            
            #if AB
                if(alpha >= eval){
                    return alpha;
                }
                beta = min(beta, eval);
            #endif

        }
    }

    #if TT      //Insert value into table
        table.emplace(posHash, make_pair(eval, depth));
    #endif

    return eval;        //Return the value of the best child node
}

int32_t qSearch2(bitBoard &board, uint8_t depth, bool whiteTurn, int32_t alpha, int32_t beta, bool nullTree){

    //Note that qSearch should be self collapsing, ie. it will stop on its own when it finds a quiet position
    //nullTree is only for mapping tree structure, ie. if true, then its a nullmove search, so we dont add it to the tree structure
    
    if(!nullTree){
        quiescenceNodesAtDepth[depth-1] += 1;        
    }      

    //Transposition table check
    #if TT2
        uint64_t posHash = hashFunction(board, whiteTurn);
        if(table.count(posHash)){
            if(table[posHash].second == depth){
                leafNodes += 1;                             //If the current depth is larger or equal to the depth the position was originally searched in, return the value
                return table[posHash].first;             
            }
        }
    #endif

    int32_t standingEval = evaluation(board);                       //The static evaluation of current position

    //AB pruning
    #if AB2
        if(whiteTurn){                                          //We assume that there exists a move that keeps the current standing eval
            if(standingEval >= beta){                           //ie. a move that doesnt weaken the position
                if(!nullTree){                                  //if beta (for white, alpha for black) is smaller than current eval, we can 
                    leafNodes += 1;                             //exit, since we know the opposition will never choose to come here, since by assumption, 
                }                                               //there exists a move that is atleast equal to standing eval, and return the standingEval
                return standingEval;                            
            }
            alpha = max(standingEval, alpha);                   //If we dont fail high, we update alpha
        }else{                                                  
            if(standingEval <= alpha){                         
                if(!nullTree){
                    leafNodes += 1;       
                } 
                return standingEval;
            }
            beta = min(standingEval, beta);
        }
    #endif

    vector<bitBoard> moves;                             //Initialize needed variables
    int32_t eval = standingEval;                        //Here the minimum eval is the standing eval, since now we only search captures, we assume there is a move that preserves the eval
    moves.reserve(10);

    if(whiteTurn){                                      //White ie. maximizing player
        generateWhiteNonQuietMoves(board, moves);       //Generate all possible moves for white that could impact the eval, ie. "non-quiet", and store them in moves
        if(moves.empty()){                              //If no moves possible, we have reached a quiet position, and can return the evaluation        
            if(!nullTree){
                leafNodes += 1;                         //Since if there are no more moves, we have reached a leafnode
            }                              
            return standingEval;
        }                               
        for(bitBoard &i : moves){                       //Loop over all non-quiet moves, and recursively call qsearch, if found move is better than existing one, replace
            int32_t moveEval = qSearch2(i, depth + 1, false, alpha, beta, nullTree);
            eval = max(eval, moveEval);     
            
            #if AB2
                if(beta <= eval){                           //If there exists a beta (beta being the best option for the minimizer in a parent node so far)
                    return eval;                            //That is smaller than the current value of the node we are in, we know that the minimizing player
                }                                           //Would never choose this path, since a better one is available, and we can terminate, the search 
                alpha = max(alpha, eval);                   //We update alpha to be the best value
            #endif

        }
    }
    else{                                               //Minimizing player, same process as for white, but we minimize and return INF if in checkmate
        generateBlackNonQuietMoves(board, moves);
        if(moves.empty()){         
            if(!nullTree){
                leafNodes += 1;                         
            }                             
            return standingEval;
        }                        
        for(bitBoard &i : moves){
            int32_t moveEval = qSearch2(i, depth + 1, true, alpha, beta, nullTree);
            eval = min(eval, moveEval);                 //Same process as for the maximizer, but mirrored and using alpha (best option for a maximizing parent node)
            
            #if AB2
                if(alpha >= eval){
                    return eval;
                }
                beta = min(beta, eval);
            #endif

        }
    }
    
    #if TT2      //Insert value into table
        table.emplace(posHash, make_pair(eval, depth));
    #endif
        
    return eval;                                    //We return the value of the best move/child node, OR the if not capturing is better, we return standing eval
}

//Search were we dont conduct null moves, this is called when searching a null move, so we dont search nullmove into null move into null move etc
int32_t minimaxNoNullMove2(bitBoard &board, uint8_t depth, bool whiteTurn, uint8_t maxDepth, int32_t alpha, int32_t beta){
    
    //Check if the position has already been searched by looking it up from the table
    
    #if TT2
        uint64_t posHash = hashFunction(board, whiteTurn);
        if(table.count(posHash)){
            if(table[posHash].second == depth){
                leafNodes += 1;                             //If the current depth is larger or equal to the depth the position was originally searched in, return the value
                return table[posHash].first;             
            }
        }
    #endif

    if(depth >= maxDepth){                              //Once weve reached the desired depth, we just return the evaluation/Qsearch
        
        #if QSEARCH2
            return qSearch2(board, depth+1, !whiteTurn, alpha, beta, true);    //Call the qSearch once reached the 
        #endif

        return evaluation(board);
    }

    vector<bitBoard> moves;                             //Initialize needed variables
    int32_t eval;
    moves.reserve(50);

    if(whiteTurn){                                      //White ie. maximizing player
        generateWhiteMoves(board, moves);               //Generate all possible moves for white, and store them in moves
        if(moves.empty()){                              //If no moves possible, check is the white king in check and return NEGINF if yes or 0 if in stalemate
            return whiteKingCheck(board);
        }
        eval = NEGINF;                                  
        for(bitBoard &i : moves){                       //Loop over all possible moves, and recursively call minimax, if found move is better than existing one, replace
            int32_t moveEval = minimaxNoNullMove2(i, depth + 1, false, maxDepth, alpha, beta);
            eval = max(eval, moveEval);     
            
            #if AB2
                if(beta <= eval){                           //If there exists a beta (beta being the best option for the minimizer in a parent node so far)
                    break;                                  //That is smaller than the current value of the node we are in, we know that the minimizing player
                }                                           //Would never choose this path, since a better one is available, and we can terminate, the search 
                alpha = max(alpha, eval);                   //We update alpha to be the best value
            #endif

        }
    }
    else{                                               //Minimizing player, same process as for white, but we minimize and return INF if in checkmate
        generateBlackMoves(board, moves);
        if(moves.empty()){
            return blackKingCheck(board);
        }
        eval = INF;                                     //Again we initialize, but as INF, since black tries to minimize the score
        for(bitBoard &i : moves){
            int32_t moveEval = minimaxNoNullMove2(i, depth + 1, true, maxDepth, alpha, beta);
            eval = min(eval, moveEval);                 //Same process as for the maximizer, but mirrored and using alpha (best option for a maximizing parent node)
            
            #if AB2
                if(alpha >= eval){
                    return alpha;
                }
                beta = min(beta, eval);
            #endif

        }
    }

    #if TT2      //Insert value into table
        table.emplace(posHash, make_pair(eval, depth));
    #endif

    return eval;                                    //We return the value of the best move/child node
}

//Primary search algorithm
int32_t minimax2(bitBoard &board, uint8_t depth, bool whiteTurn, uint8_t maxDepth, int32_t alpha, int32_t beta){

    nodesAtDepth[depth] += 1; 
    
    //Transpostion table lookup 
    #if TT2
        uint64_t posHash = hashFunction(board, whiteTurn);
        if(table.count(posHash)){
            //if((table[posHash].second) <= depth){    //Check that the value that is found was searched higher up in the tree or same depth
                leafNodes += 1;
                return table[posHash].first;
            //}
        }
    #endif

    if(depth >= maxDepth){                              //Once weve reached the desired depth, we just return the evaluation
        
        #if QSEARCH2
            return qSearch2(board, depth+1, !whiteTurn, alpha, beta, false);    //Call the qSearch 
        #endif

        leafNodes += 1;
        return evaluation(board);
    }

    vector<bitBoard> moves;                             //Initialize needed variables
    int32_t eval;
    moves.reserve(50);

    if(whiteTurn){                                      //White ie. maximizing player

        #if NULLMOVE2
            //Nullmove pruning
            //Try a null move if ones desired, and aslong as we are not in Check
            if((maxDepth > depth + 1 + NMDEPTHREDUCTION2) && (whiteKingCheck(board) == 0)){                           
                int32_t nullMove2alue = minimaxNoNullMove2(board, depth + 1 + NMDEPTHREDUCTION2, false, maxDepth, alpha, beta);
                if(nullMove2alue >= beta){                  //If the null move value fails high, return value
                    leafNodes += 1;
                    return nullMove2alue;
                }                                         
                //alpha = max(alpha, nullMove2alue);          //Assume that exists a move that is atleast as good as nullmove                                                      
            }
        #endif

        generateWhiteMoves(board, moves);               //Generate all possible moves for white, and store them in moves
        
        if(moves.empty()){                              //If no moves possible, check is the white king in check and return NEGINF if yes or 0 if in stalemate
            leafNodes += 1;                             //Since if there are no more moves, we have reached a leafnode
            return whiteKingCheck(board);               //Here there is no need to add the position to the transposition table, 
        }                                               //since searching the position normally would yield in no increase of the tree size

        eval = NEGINF;                                  
        for(bitBoard &i : moves){                       //Loop over all possible moves, and recursively call minimax, if found move is better than existing one, replace
            int32_t moveEval = minimax2(i, depth + 1, false, maxDepth, alpha, beta);
            if((depth == 0) && (moveEval > eval)){      //Just to keep track of the best move from the original position
                bestMove.first = i;
                bestMove.second = moveEval;
            }
            eval = max(eval, moveEval);     
            
            #if AB2
                if(beta <= eval){                           //If there exists a beta (beta being the best option for the minimizer in a parent node so far)
                    break;                                  //That is smaller than the current value of the node we are in, we know that the minimizing player
                }                                           //Would never choose this path, since a better one is available, and we can terminate, the search 
                alpha = max(alpha, eval);                   //We update alpha to be the best value
            #endif

        }
    }
    else{                                               //Minimizing player, same process as for white, but we minimize and return INF if in checkmate
        
        #if NULLMOVE2
            //Nullmove pruning
            //Try a null move if ones desired, and aslong as we are not in Check
            if((maxDepth > depth + 1 + NMDEPTHREDUCTION2) && (blackKingCheck(board) == 0)){                           
                int32_t nullMove2alue = minimaxNoNullMove2(board, depth + 1 + NMDEPTHREDUCTION2, true, maxDepth, alpha, beta);
                if(nullMove2alue <= alpha){                  //If the null move value fails low, return value
                    return nullMove2alue;
                }                                           
                //beta = min(beta, nullMove2alue); 
            }
        #endif
        
        generateBlackMoves(board, moves);

        if(moves.empty()){
            leafNodes += 1;                             //Since if there are no more moves, we have reached a leafnode
            return blackKingCheck(board);
        }

        eval = INF;                                     //Again we initialize, but as INF, since black tries to minimize the score
        for(bitBoard &i : moves){
            int32_t moveEval = minimax2(i, depth + 1, true, maxDepth, alpha, beta);
            if((depth == 0) && (moveEval < eval)){
                bestMove.first = i;
                bestMove.second = moveEval;
            }
            eval = min(eval, moveEval);                 //Same process as for the maximizer, but mirrored and using alpha (best option for a maximizing parent node)     
            
            #if AB2
                if(alpha >= eval){
                    return alpha;
                }
                beta = min(beta, eval);
            #endif

        }
    }

    #if TT2      //Insert value into table
        table.emplace(posHash, make_pair(eval, depth));
    #endif

    return eval;        //Return the value of the best child node
}
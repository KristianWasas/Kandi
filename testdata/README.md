The data that each algorithm produced is in these folders

Each folder has a file with

    1 - the minimax version plaincode, located in source.text
    2 - an explanation of what kind of algorithm has been implemented, ie. null move pruning, transpositiontable,
        combination/specific implementation values etc., located in source.text
    3 - the raw data, in a dataX.text file
        a - if there are more than one data files, it means that there are different depth searches for that one version
            ie. if version2.x has a data1 and data2 file, all version2.x have same depths in data1 and data2 respecfully
            but data1 and data2 have different search depths
        b - version3.x dataY and version2.x dataY might have different search depths

Each datax.text contains the data of done on X unique positions for that version of the algorithm

    1 - Total amount of nodes
        a - Nodes at each depth in the primary search
        b - Nodes at each depth in the quiescence search
        c - Total amount of nodes at each search
        d - Total amount of nodes
        e - Total amount of leafnodes
    2 - The best move found for each position
    3 - The evaluation of the best move (which is also the evaluation of the original position)
    4 - Time it took
    5 - The depth of the primary search
    6 - Branching factor = childnodes/parentnodes => (totalnodes - 1)/(totalnodes-leafnodes)

The dataX.text files, the X stands for a certain depth, if two files are datax.text, they have the same primary search depth, 
but if there is dataX.text and dataY.text, they have differing primary search depths, this is standard across all verions

Each versionX is built upon a previous implementation, ie. version2.x is version 1.0 with a-b pruning implemented
version3.x is version2.0 with quiescence search implemented. version4.x is version3.0 with null move pruning

The choice on which to version to build upon, is done by analyzing the data results, time and tree size being the 
main concerns.
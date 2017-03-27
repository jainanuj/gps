//
//  createGrid.c
//  gps-generate-xcode
//
//  Created by Anuj Jain on 3/27/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//

#include "createGrid.h"

int gen_grid_to_index( int *pos )
{
    int i, index;
    int extent[2];
    extent[0] = maxX;
    extent[1] = maxY;
    
    index = 0;
    for ( i=1; i>=0; i-- ) {
        index *= extent[i];
        index += pos[i];
    }
    //fprintf(stderr, "gridPOS:x:%d;y:%d - index=%d\n", pos[0], pos[1], index);
    return index;
}

void getIndextoCoord(int index, int* coord)
{
    int y = index / maxX;
    int x = index % maxX;
    coord[0] = x;
    coord[1] = y;
    
}

int getNextState(int curState, int action)
{
    int curCoord[2];        //X, Y
    getIndextoCoord(curState, curCoord);
    if (action == 0)            //NORTH
    {
        if (curCoord[1] != 0)           //Y
            curCoord[1]--;
    }
    else if (action == 1)       //EAST
    {
        if (curCoord[0] != maxX - 1)   //X
            curCoord[0]++;
    }
    else if (action == 2)       //SOUTH
    {
        if (curCoord[1] != maxY-1)     //Y
            curCoord[1]++;
    }
    else if (action == 3)       //WEST
    {
        if (curCoord[0] != 0)           //X
            curCoord[0]--;
        
    }
    //fprintf(stderr, "curState: %d; action: %d; result::x:%d;y:%d\n", curState, action, curCoord[0], curCoord[1]);
    
    return gen_grid_to_index(curCoord);
}

void createMDP()
{
    int totalStates = maxX * maxY;
    int finalState[2];
    
    finalState[0] = endX;
    finalState[1] = endY;
    int goalState = gen_grid_to_index(finalState);
    FILE *fp;
    float reward = 0.0;
    
    int i = 0;
    int action = 0;
    
    fp = stdout;
    
    fprintf(fp, "%d\n", totalStates );
    
    for (i =0; i < totalStates; i++)
    {
        fprintf(fp, "%d 4\n", i);
        for (action = 0; action < 4; action++)
        {
            int nextState = getNextState(i, action);
            if (nextState == goalState)
                reward = 1.0;
            else
                reward = 0.0;
            fprintf(fp, "%.2f 1 %d 0.99\n", reward, nextState);
        }
    }
}

int main( int argc, char *argv[] )
{
    char* opt;
    if (argc > 1)
    {
        opt = argv[1];
        maxX = atoi(opt);
        maxY = maxX;
        if (argc > 2)
        {
            opt = argv[2];
            endX = atoi(opt);
            opt = argv[3];
            endY = atoi(opt);
        }
    }
    fprintf(stderr, "MaxX & MaxY=%d. EndX=%d & EndY=%d\n", maxX, endX, endY);
    fprintf(stderr, "Starting up\n");
    createMDP();
    return 0;
}

#ifndef __E_ASTAR_H__
#define __E_ASTAR_H__
#include "E.H"


typedef struct {
    uint8 x;
    uint8 y;
} AStarPathNode;
typedef struct {
    uint8* far nodes; // bit field
    AStarPathNode* far path;
    uint16 pathNodeCount;
} AStar;

void AStar_Cleanup(AStar* astar);
void AStar_Create(AStar* astar);
void AStar_Trace(AStar* astar, uint8 x, uint8 y, uint8 goalX, uint8 goalY);

#endif

#include "E.H"
#include "E_MAP.H"
#include "E_ASTAR.H"
#include <malloc.h>
#include <string.h>


// @TODO: Perhaps do only a handful of iterations during traversal to speed up
//        algorithm, sacrificing accuracy, which makes the monster dumber.

#define IS_TRAVERSABLE(x, y) (!g_Map[g_MapWidth * (y) + (x)])
#define NODE_AT(x, y) me->nodes[g_MapWidth * (y) + (x)]
#define STATE_OPEN 0x01
#define STATE_CLOSED 0x02
#define STATE_LEFT 0x04
#define STATE_RIGHT 0x08
#define STATE_ABOVE 0x10
#define STATE_BELOW 0x20


#define CALC_COST(ax,ay,bx,by) (ABS((int16)(ax)-(int16)(bx))+ABS((int16)(ay)-(int16)(by)))


// static void pickCheapestNodeASM(
// 	AStar* me, uint8* retX, uint8* retY, uint8 startX, uint8 startY, uint8 goalX, uint8 goalY)
// {
// 	UNUSED_PARAMETER(me);
// 	UNUSED_PARAMETER(retX);
// 	UNUSED_PARAMETER(retY);
// 	UNUSED_PARAMETER(startX);
// 	UNUSED_PARAMETER(startY);
// 	UNUSED_PARAMETER(goalX);
// 	UNUSED_PARAMETER(goalY);

	
// }

static void pickCheapestNode(
	AStar* me, uint8* retX, uint8* retY, uint8 startX, uint8 startY, uint8 goalX, uint8 goalY)
{
	uint16 i = 0, x, y, fcost, hcost, cheapestIndex;
	uint16 lowestFCost = -1;
	uint16 lowestHCost = -1;
	for (y = 0; y < g_MapHeight; y++) {
		for (x = 0; x < g_MapWidth; x++, i++) {
			// Make sure we got a node that is marked as open
			if (!(me->nodes[i] & STATE_OPEN)) {
				continue;
			}
			
			// Check for cheapest node
			hcost = CALC_COST(x, y, goalX, goalY);
			fcost = CALC_COST(x, y, startX, startY) + hcost;
			if ((fcost <= lowestFCost) && (hcost  < lowestHCost)) {
				lowestFCost = fcost;
				lowestHCost = hcost;
				cheapestIndex = i;

				// Set coords of cheapest node
				*retX = x;
				*retY = y;
			}
		}
	}

	// Mark the picked node as closed, but not open
	me->nodes[cheapestIndex] |= STATE_CLOSED;
	me->nodes[cheapestIndex] &= ~STATE_OPEN;

}


static void retracePath(AStar* me, uint8 startX, uint8 startY, uint8 goalX, uint8 goalY) {
	uint8 currentX = goalX;
	uint8 currentY = goalY;
	uint8 state;
    uint16 low, high;
    uint16 pathNodeCount;
    AStarPathNode temp;

	for (pathNodeCount = 0; (currentX != startX || currentY != startY); pathNodeCount++) {
		
		me->path[pathNodeCount].x = currentX;
		me->path[pathNodeCount].y = currentY;
		state = NODE_AT(currentX, currentY);
		if (state & STATE_LEFT ) currentX--;
		if (state & STATE_RIGHT) currentX++;
		if (state & STATE_ABOVE) currentY--;
		if (state & STATE_BELOW) currentY++;
	}
	
    // Reverse path array so the start position's neighbour comes first
    // @TODO: This is only here to make it more user friendly
	if (pathNodeCount > 0) {
		for (low = 0, high = pathNodeCount - 1; low < high; low++, high--) {
			temp = me->path[low];
			me->path[low] = me->path[high];
			me->path[high] = temp;
		}
	}
    me->pathNodeCount = pathNodeCount;
}


void AStar_Trace(AStar* me, uint8 startX, uint8 startY, uint8 goalX, uint8 goalY) {
	uint16 newCost;
	uint8 currentX = startX;
	uint8 currentY = startY;
	memset(me->nodes, 0, g_MapWidth * g_MapHeight);
	NODE_AT(startX, startY) |= STATE_OPEN;
	
	// @TODO: use indices rather than calculate index from XY coords for speed
	for(;;) {
		
		pickCheapestNode(me, &currentX, &currentY, startX, startY, goalX, goalY);

		if (currentX == goalX && currentY == goalY) {
	        retracePath(me, startX, startY, goalX, goalY);
            return;
        }

		newCost = CALC_COST(currentX, currentY, startX, startY) + 1;


		// Traverse in each cardinal direction
		#define NX currentX - 1
		#define NY currentY
		if (IS_TRAVERSABLE(NX, NY) && !(NODE_AT(NX, NY) & STATE_CLOSED)) {
			if (newCost < CALC_COST(NX, NY, startX, startY) ||
				!(NODE_AT(NX, NY) & STATE_OPEN)) {
				NODE_AT(NX, NY) |= STATE_OPEN | STATE_RIGHT;
			}
		}
		#undef NX
		#undef NY

		#define NX currentX + 1
		#define NY currentY
		if (IS_TRAVERSABLE(NX, NY) && !(NODE_AT(NX, NY) & STATE_CLOSED)) {
			if (newCost < CALC_COST(NX, NY, startX, startY) ||
				!(NODE_AT(NX, NY) & STATE_OPEN)) {
				NODE_AT(NX, NY) |= STATE_OPEN | STATE_LEFT;
			}
		}
		#undef NX
		#undef NY

		#define NX currentX
		#define NY currentY - 1
		if (IS_TRAVERSABLE(NX, NY) && !(NODE_AT(NX, NY) & STATE_CLOSED)) {
			if (newCost < CALC_COST(NX, NY, startX, startY) ||
				!(NODE_AT(NX, NY) & STATE_OPEN)) {
				NODE_AT(NX, NY) |= STATE_OPEN | STATE_BELOW;
			}
		}
		#undef NX
		#undef NY

		#define NX currentX
		#define NY currentY + 1
		if (IS_TRAVERSABLE(NX, NY) && !(NODE_AT(NX, NY) & STATE_CLOSED)) {
			if (newCost < CALC_COST(NX, NY, startX, startY) ||
				!(NODE_AT(NX, NY) & STATE_OPEN)) {
				NODE_AT(NX, NY) |= STATE_OPEN | STATE_ABOVE;
			}
		}
		#undef NX
		#undef NY

	}

}


void AStar_Cleanup(AStar* me) {
	SAFE_DELETE_FAR_PTR(me->nodes);
    SAFE_DELETE_FAR_PTR(me->path);
}

void AStar_Create(AStar* me) {
	me->nodes = _fcalloc(g_MapWidth * g_MapHeight, 1);

    // Approximate a large enough but safe array.
    me->path = _fmalloc(g_MapWidth * g_MapHeight / 2 * sizeof(AStarPathNode));
}

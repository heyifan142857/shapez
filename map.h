#ifndef MAP_H
#define MAP_H


#include <vector>

#define BLANK 0
#define REDMINE 1
#define BLUEMINE 2
#define GREENMINE 3
#define DELIVER 4
#define CYCLEMINE 5
#define SQUAREMINE 6
#define RHOMBUSMINE 7

class gameMap{
    int m,n;
    int deliver_x,deliver_y;
    std::vector<std::vector<int>> map;
public:
    gameMap(int m,int n);
};


#endif // MAP_H

#include "map.h"

gameMap::gameMap(int m, int n) :m(m), n(n), deliver_x(m/2), deliver_y(n/2){
    map.resize(m,std::vector<int>(n,BLANK));
    for(int i=0;i<3;i++) {
        for(int j=0;j<3;j++) {
            map[deliver_x+i][deliver_y+j] = DELIVER;
        }
    }
}

#ifndef HASH_H
#define HASH_H

#include <QVector>
#include <cstdlib>
#include <cmath>
#include <algorithm>

class Hash {
public:
    Hash(double spacing_var, unsigned int maxNumObjects){
        spacing = spacing_var;
        tableSize = 2 * maxNumObjects;
        cellStart.resize(tableSize+1);
        cellEntries.resize(maxNumObjects);
        queryIds.resize(maxNumObjects);
        querySize = 0;
    }

    unsigned int hasCoords(unsigned int xi, unsigned int yi, unsigned int zi){
        int h = (xi * 92837111) ^ (yi * 689289499) ^ (xi * 283923481);
        return std::abs(h) % tableSize;
    }

    unsigned int intCoord(int coord){
        return std::floor(coord / spacing);
    }

    unsigned int hashPos(QVector<unsigned int> pos, unsigned int nr){
        return hasCoords(
                    intCoord(pos[3 * nr]),
                    intCoord(pos[3 * nr]),
                    intCoord(pos[3 * nr])
                    );
    }

    void create(QVector<unsigned int> pos){
        unsigned int numObjects = std::min(pos.size()/3,cellEntries.size());

        // determine cell sizes

        cellStart.fill(0);
        cellEntries.fill(0);

        for(unsigned int i=0; i < numObjects; i++){
            int h = hashPos(pos,i);
            cellStart[h]++;
        }

        // determine cells starts

        unsigned int start = 0;
        for(unsigned int i=0; i < tableSize; i++){
            start += cellStart[i];
            cellStart[i] = start;
        }
        cellStart[tableSize] = start; //guard

        // fill in objects ids

        for(unsigned int i=0; i < numObjects; i++){
            int h = hashPos(pos,i);
            cellStart[h]--;
            cellEntries[cellStart[h]] = i;
        }
    }

    void query(QVector<unsigned int> pos, unsigned int nr, unsigned int maxDist){
        unsigned int x0 = intCoord(pos[3 * nr] - maxDist);
        unsigned int y0 = intCoord(pos[3 * nr + 1] - maxDist);
        unsigned int z0 = intCoord(pos[3 * nr + 2] - maxDist);

        unsigned int x1 = intCoord(pos[3 * nr] + maxDist);
        unsigned int y1 = intCoord(pos[3 * nr + 1] + maxDist);
        unsigned int z1 = intCoord(pos[3 * nr + 2] + maxDist);

        querySize = 0;

        for(unsigned int  xi=x0; xi<=x1; xi++){
            for(unsigned int  yi=y0;yi<=y1; yi++){
                for(unsigned int  zi=z0;zi<=z1; zi++){
                    int h = hasCoords(xi,yi,zi);
                    unsigned int start = cellStart[h];
                    unsigned int end = cellStart[h+1];

                    for(unsigned int i=start; i<end; i++){
                        queryIds[querySize] = cellEntries[i];
                        querySize++;
                    }
                }
            }
        }
    }

    double spacing;
    unsigned int tableSize, querySize;
    QVector<unsigned int> cellStart, cellEntries, queryIds;
};



#endif // HASH_H

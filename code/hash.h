#ifndef HASH_H
#define HASH_H

#include <QVector>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include "particle.h"

class Hash {
public:
    Hash(double spacing_var, unsigned int maxNumObjects){
        spacing = spacing_var;
        tableSize = 50 * maxNumObjects;
        cellStart.resize(tableSize+1);
        cellEntries.resize(maxNumObjects);
        queryIds.resize(maxNumObjects);
        querySize = 0;
    }

    unsigned int hashCoords(int xi, int yi, int zi){
        int h = (xi * 92837111) ^ (yi * 689289499) ^ (xi * 283923481);
        return std::abs(h) % tableSize;
    }

    int intCoord(int coord){
        return std::floor(coord/ spacing);
    }

    unsigned int hashPos(QVector<Particle *> parts, unsigned int nr){
        return hashCoords(
                    intCoord(int(parts[nr]->pos.x())),
                    intCoord(int(parts[nr]->pos.y())),
                    intCoord(int(parts[nr]->pos.z()))
                    );
    }

    void create(QVector<Particle *> parts){
        unsigned int numObjects = std::min(int(parts.size()),cellEntries.size());

        // determine cell sizes

        cellStart.fill(0);
        cellEntries.fill(0);

        for(unsigned int i=0; i < numObjects; i++){
            int h = hashPos(parts,i);
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
            int h = hashPos(parts,i);
            cellStart[h]--;
            cellEntries[cellStart[h]] = i;
        }
    }

    void query(QVector<Particle *> parts, unsigned int nr, unsigned int maxDist){
        int x0 = intCoord(int(parts[nr]->pos[0]) - maxDist);
        int y0 = intCoord(int(parts[nr]->pos[1]) - maxDist);
        int z0 = intCoord(int(parts[nr]->pos[2]) - maxDist);

        int x1 = intCoord(int(parts[nr]->pos[0]) + maxDist);
        int y1 = intCoord(int(parts[nr]->pos[1]) + maxDist);
        int z1 = intCoord(int(parts[nr]->pos[2]) + maxDist);

        querySize = 0;

        for(int  xi=x0; xi<=x1; xi++){
            for(int  yi=y0;yi<=y1; yi++){
                for(int  zi=z0;zi<=z1; zi++){
                    int h = hashCoords(xi,yi,zi);
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

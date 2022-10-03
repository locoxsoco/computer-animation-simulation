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
        tableSize = 2 * maxNumObjects;
        cellStart.resize(tableSize+1);
        cellEntries.resize(maxNumObjects);
        queryIds.resize(maxNumObjects);
        querySize = 0;
    }

    unsigned int hashCoords(int xi, int yi, int zi){
        int h = (xi * 92837111) ^ (yi * 689287499) ^ (zi * 283923481); // fantasy function
        return std::abs(h) % tableSize;
    }

    int intCoord(double coord){
        return std::floor(coord/ spacing);
    }

    unsigned int hashPos(QVector<Particle *> parts, unsigned int nr){
        return hashCoords(
                    intCoord(parts[nr]->pos.x()),
                    intCoord(parts[nr]->pos.y()),
                    intCoord(parts[nr]->pos.z())
                    );
    }

    void create(QVector<Particle *> parts){
        unsigned int numObjects = std::min(parts.size(),cellEntries.size());

        // determine cell sizes

        cellStart.fill(0);
        cellEntries.fill(0);

        for(unsigned int i=0; i < numObjects; i++){
            unsigned int h = hashPos(parts,i);
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
            unsigned int h = hashPos(parts,i);
            cellStart[h]--;
            cellEntries[cellStart[h]] = i;
        }
    }

    void query(QVector<Particle *> parts, unsigned int nr, double maxDist){
        int x0 = intCoord(parts[nr]->pos.x() - maxDist);
        int y0 = intCoord(parts[nr]->pos.y() - maxDist);
        int z0 = intCoord(parts[nr]->pos.z() - maxDist);

        int x1 = intCoord(parts[nr]->pos.x() + maxDist);
        int y1 = intCoord(parts[nr]->pos.y() + maxDist);
        int z1 = intCoord(parts[nr]->pos.z() + maxDist);

        querySize = 0;

        for(int  xi=x0; xi<=x1; xi++){
            for(int  yi=y0;yi<=y1; yi++){
                for(int  zi=z0;zi<=z1; zi++){
                    unsigned int h = hashCoords(xi,yi,zi);
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

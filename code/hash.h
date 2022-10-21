#ifndef HASH_H
#define HASH_H

#include <QVector>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include "particle.h"

class Hash {
public:
    Hash(double spacing_var, unsigned int maxNObjects){
        spacing = spacing_var;
        tableSize = 5 * maxNObjects;
        cellStart.resize(tableSize+1);
        cellEntries.resize(maxNObjects);
        queryIds.resize(maxNObjects);
        querySize = 0;

        maxNumObjects = maxNObjects;
        firstAdjId.resize(maxNumObjects + 1);
        adjIds.resize(10 * maxNumObjects);
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
    unsigned int hashPos(Vec3 pos){
        return hashCoords(
                    intCoord(pos.x()),
                    intCoord(pos.y()),
                    intCoord(pos.z())
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

    void query(QVector<Particle *> parts, unsigned int nr, float maxDist){
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

    void query(Vec3 pos, float maxDist){
        int x0 = intCoord(pos.x() - maxDist);
        int y0 = intCoord(pos.y() - maxDist);
        int z0 = intCoord(pos.z() - maxDist);

        int x1 = intCoord(pos.x() + maxDist);
        int y1 = intCoord(pos.y() + maxDist);
        int z1 = intCoord(pos.z() + maxDist);

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

    void queryAll(QVector<Particle *> parts, float maxDist){
        int num = 0;
        float maxDist2 = maxDist * maxDist;

        for (int i = 0; i < maxNumObjects; i++) {
            int id0 = i;
            firstAdjId[id0] = num;
            query(parts, id0, maxDist);

            for (int j = 0; j < querySize; j++) {
                int id1 = queryIds[j];
                if (id1 >= id0)
                    continue;
                float dist2 = (parts[id0]->pos-parts[id1]->pos).squaredNorm();//vecDistSquared(parts, id0, parts, id1);
                if (dist2 > maxDist2)
                    continue;

                if (num >= adjIds.size()) {
                    QVector<int> newIds(2 * num);  // dynamic array
                    newIds.append(adjIds);
                    adjIds = newIds;
                }
                adjIds[num++] = id1;
            }
        }

        firstAdjId[maxNumObjects] = num;
    }

    double spacing;
    unsigned int tableSize, querySize;
    QVector<unsigned int> cellStart, cellEntries, queryIds;

    unsigned int maxNumObjects;
    QVector<int> firstAdjId;
    QVector<int> adjIds;
};



#endif // HASH_H

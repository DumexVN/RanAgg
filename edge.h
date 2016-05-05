#ifndef EDGE_H
#define EDGE_H

#include <QtGlobal>

class Vertex;

class Edge
{

public:
    Edge(Vertex *fromVertex, Vertex *toVertex, quint32 index);
    ~Edge();

    Vertex *fromVertex() const;
    Vertex *toVertex() const;

    void removeAll();

    quint32 getIndex() const;

protected:
    Vertex *myFromVertex;
    Vertex *myToVertex;
    quint32 index;
};

#endif

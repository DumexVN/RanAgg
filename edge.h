#ifndef EDGE_H
#define EDGE_H

#include <QtGlobal>

class Vertex;

class Edge
{

public:
    Edge(Vertex *fromVertex, Vertex *toVertex, quint64 index);
    ~Edge();

    Vertex *fromVertex() const;
    Vertex *toVertex() const;

    void removeAll();

    quint64 getIndex() const;

protected:
    Vertex *myFromVertex;
    Vertex *myToVertex;
    quint64 index;
};

#endif

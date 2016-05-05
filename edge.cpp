#include "edge.h"
#include "vertex.h"

Edge::Edge(Vertex *fromVertex, Vertex *toVertex, quint64 index)
{
    myFromVertex = fromVertex;
    myToVertex = toVertex;

    myFromVertex->addEdge(this);
    myToVertex->addEdge(this);

    myFromVertex->addAdj(toVertex->getIndex());
    myToVertex->addAdj(fromVertex->getIndex());

    this->index = index;

}

Edge::~Edge()
{
    myFromVertex->removeEdge(this);
    myToVertex->removeEdge(this);
    myFromVertex->removeAdj(myToVertex->getIndex());
    myToVertex->removeAdj(myFromVertex->getIndex());
}

Vertex *Edge::fromVertex() const
{
    return myFromVertex;
}

Vertex *Edge::toVertex() const
{
    return myToVertex;
}

void Edge::removeAll()
{
    myFromVertex->removeEdge(this);
    myToVertex->removeEdge(this);
    myFromVertex->removeAdj(myToVertex->getIndex());
    myToVertex->removeAdj(myFromVertex->getIndex());
}

quint64 Edge::getIndex() const
{
    return index;
}




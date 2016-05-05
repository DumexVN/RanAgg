#ifndef VERTEX_H
#define VERTEX_H

#include <QSet>
#include <QDebug>

#include "edge.h"

class Vertex
{
public:
    Vertex();
    ~Vertex();
    void setIndex(const quint64 &number);
    quint64 getIndex() const;

    void addAdj(const quint64 &index);
    quint64 getNumAdj() const;

    int getOneNeighbourIndex(const quint64 &index);
    void removeAdj(const quint64 &index);
    void removeAll();

    void setWeight(const quint64 &w);
    void setWeightAsNumberOfAbsorbed();
    quint64 getWeight() const;

    void addEdge(Edge *edge);
    void removeEdge(Edge *edge);
    quint64 getNumberEdge() const;
    void remove_all_edges();

    void absorb_removeEdge(quint64 edge_index);
    void absorb_removeEdge(Edge * e);
    void absorb_removeVertex_retainEdge(Edge * e);
    void absorb_retainEdge(Edge * e);
    void absorb_retainEdge_setParentPointer(Edge * e);
    void absorb_singleton(Vertex * v);

    Edge * getEdge(int edgeIndex) const;
    Edge * getWeightedProbabilisticEdge();
    Edge * getDegreeProbabilisticEdge();
    Edge * getEdgeFromVertex(Vertex * v2);
    Edge * getSmallestCurrentDegreeNeighbour();
    Edge * getSmallestCurrentWeightNeighbour();
    Edge * getHighestDegreeNeighbour();
    Edge * getMostMutualVertex();
    Edge * getHighestTriangulateCluster();
    Edge * getProbabilisticTriangulationCoeffVertex();
    Edge * getProbabilisticTriangulationAndWeightVertex();
    QList<Edge*> getAllEdge() const;

    Vertex * aggregate_get_degree_biased_neighbour();
    Vertex *get_neighbour_fromEdge(int edge_index);
    Vertex *get_neighbour_fromEdge(Edge * e);


    void setParent(Vertex * v);
    void setParentPointerOnly(Vertex * v);
    Vertex * getParent() const;

    void incrementNoChild();
    void setExtraWeight(const quint64 &w);
    quint64 getExtraWeight() const;
    int getNoChild() const;

    QList<Vertex*> getAbsorbedList();
    QList<int> getNeighbourIndexes();

    void set_vertex_as_absorbed(bool val);
    bool is_vertex_absorbed() const;
    bool is_vertex_dragged_along() const;

    int getNoOfTriangles(Vertex * v);
    QList<Vertex*> getMyCluster();
    void addMemberToCluster(Vertex * v);
    void addMemberToCluster(QList<Vertex*> v);
    void clearCluster();
    void clearAbsorbed();

    void setTruthCommunity(const int &p);
    int getTruthCommunity() const;

    void resetClusterRelevant();

private:

    Vertex * parent;
    QList<int> myNeighbours;
    QList<Vertex*> absorbed;

protected:
    QList<Edge *> myEdge;
    QList<Vertex*> myCluster;

    int myIndex;
    quint64 myWeight;
    bool isDraggedAlong;
    bool isAbsorbed;
    int noOfChild;
    quint64 ExtraWeight;
    int myRealCommunity;
};

#endif // VERTEX_H

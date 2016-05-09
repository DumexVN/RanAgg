#include "vertex.h"
#include "edge.h"

#include <QTime>
#include <QDebug>
#include <QTime>

std::default_random_engine gen;


Vertex::Vertex()
{
    myWeight = 1;
    parent = 0;
    isDraggedAlong = false;
    isAbsorbed = false;
    noOfChild = 0;
    ExtraWeight = 0;
    myRealCommunity = -1;
    gen.seed(QTime::currentTime().msec());

}

Vertex::~Vertex()
{
    foreach (Edge *edge, myEdge)
        delete edge;
}

void Vertex::setIndex(const quint32 &number)
{
    myIndex = number;
}

quint32 Vertex::getIndex() const
{
    return myIndex;
}


void Vertex::addAdj(const quint32 &index)
{
    if (!myNeighbours.contains(index))
        myNeighbours.append(index);
    else
        qDebug() << "NEIGHBOUR ALREADY EXISTS, SKIPPING";
}

quint32 Vertex::getNumAdj() const
{
    return myNeighbours.size();
}

quint32 Vertex::getOneNeighbourIndex(const quint32 &index)
{
    if (myNeighbours.size()+1 > index)
        return myNeighbours.at(index);
    else
        qDebug() << "Out of Bound While Getting A Neighbour";
}


void Vertex::removeAdj(const quint32 &index)
{
    if (myNeighbours.contains(index))
        myNeighbours.removeAll(index);
    else
    {}
}

void Vertex::removeAll()
{
    foreach (Edge *edge, myEdge)
       edge->removeAll();
}

void Vertex::setWeight(const quint64 &w)
{
    myWeight = w;
}

void Vertex::setWeightAsNumberOfAbsorbed()
{
    if (absorbed.size() == 0)
        return;
    else
        myWeight = absorbed.size();
}

quint64 Vertex::getWeight() const
{
    return myWeight;
}



void Vertex::setParent(Vertex *v)
{
    if (v == 0)
        qDebug() << "NULL POINTER PARENT";
    else
    {
        parent = v;
        isAbsorbed = true;
        if (v == this)
            return;
        v->incrementNoChild();
        v->setExtraWeight(this->getWeight());

        //readjustcluter
        v->addMemberToCluster(this);
        v->addMemberToCluster(myCluster);
        myCluster.clear();
    }
}


void Vertex::setParentPointerOnly(Vertex *v)
{
    if (v == 0)
        qDebug() << "NULL POINTER PARENT";
    else
    {
        parent = v;
        isAbsorbed = true;
        if (v == this)
            return;
        v->incrementNoChild();
        v->setExtraWeight(this->getWeight());

        //readjustcluter
    }
}


void Vertex::incrementNoChild()
{
    noOfChild++;
}

void Vertex::setExtraWeight(const quint64 &w)
{
    ExtraWeight = w;
}

quint64 Vertex::getExtraWeight() const
{
    return ExtraWeight;
}

quint32 Vertex::getNoChild() const
{
    return noOfChild;
}

Vertex *Vertex::getParent() const
{
    return parent;
}

QList<Vertex *> Vertex::getAbsorbedList()
{
    return absorbed;
}

QList<quint32> Vertex::getNeighbourIndexes()
{
    QList<quint32> indexes;
    for (int i = 0; i < myEdge.size(); i++)
    {
        Vertex * neighbour = this->get_neighbour_fromEdge(myEdge[i]);
        indexes.append(neighbour->getIndex());
    }
    return indexes;
}

void Vertex::addEdge(Edge *edge)
{
    if (myEdge.contains(edge))
    {
        //DUP
    }
    else
        myEdge.append(edge);
}

void Vertex::removeEdge(Edge *edge)
{
    myEdge.removeOne(edge);
    Vertex * neighbour = this->get_neighbour_fromEdge(edge);
    myNeighbours.removeOne(neighbour->getIndex());
}

quint32 Vertex::getNumberEdge() const
{
    return myEdge.size();
}

void Vertex::remove_all_edges()
{
    foreach (Edge *edge, myEdge)
        delete edge;
}

Edge *Vertex::getEdgeFromVertex(Vertex * v2)
{
    for (int i = 0; i < myEdge.size(); i++)
    {
        Edge * e = myEdge.at(i);
        if (e->fromVertex() == v2 || e->toVertex() == v2)
            return e;
    }

    qDebug() << "WARNING ! CANNOT FIND NEIGHBOUR FROM AN EDGE! TERMINATING";
    return 0;
}

Edge *Vertex::getSmallestCurrentDegreeNeighbour()
{
    QList<quint32> indexes;
    quint32 smallest = 999999;
    for (int i = 0; i < myEdge.size(); i++)
    {
        Edge * e = myEdge.at(i);
        Vertex * neighbour = this->get_neighbour_fromEdge(e);
        quint32 w = neighbour->getNumberEdge();
        if (w < smallest)
        {
            indexes.clear();
            indexes.append(i);
            smallest = w;
        }
        else if (w == smallest)
        {
            indexes.append(i);
        }
    }

    if (indexes.size() == 1)
        return myEdge[indexes[0]];
    else
    {
        std::uniform_int_distribution<int> distribution(0,indexes.size()-1);
        int ran = distribution(gen);
        return myEdge.at(indexes[ran]);
    }
}

Edge *Vertex::getSmallestCurrentWeightNeighbour()
{
    QList<quint32> indexes;
    quint64 smallest = 99999999;
    for (int i = 0; i < myEdge.size(); i++)
    {
        Edge * e = myEdge.at(i);
        Vertex * neighbour = this->get_neighbour_fromEdge(e);
        quint64 w = neighbour->getWeight();
        if (w < smallest)
        {
            indexes.clear();
            indexes.append(i);
            smallest = w;
        }
        else if (w == smallest)
        {
            indexes.append(i);
        }
    }

    if (indexes.size() == 1)
        return myEdge[indexes[0]];
    else
    {
        std::uniform_int_distribution<int> distribution(0,indexes.size()-1);
        int ran = distribution(gen);
        return myEdge.at(indexes[ran]);
    }
}


void Vertex::absorb_removeEdge(quint32 edge_index)
{
    Vertex * neighbour = 0;
    Edge * edge = myEdge.at(edge_index);
    if (edge->fromVertex() == this)
        neighbour = edge->toVertex();
    else
        neighbour = edge->fromVertex();
    neighbour->remove_all_edges();
    absorbed.append(neighbour);
    absorbed.append(neighbour->getAbsorbedList());
    neighbour->setParent(this);
}

void Vertex::absorb_removeEdge(Edge *e)
{
    if (!myEdge.contains(e))
    {
        qDebug() << "Absorb Edge NOT FOUND!";
        return;
    }
    Vertex * neighbour = 0;
    if (e->fromVertex() == this)
        neighbour = e->toVertex();
    else
        neighbour = e->fromVertex();
   // neighbour->loser_drag_vertex_with_degree_one(e);
    neighbour->remove_all_edges();
    absorbed.append(neighbour);
    absorbed.append(neighbour->getAbsorbedList());
    neighbour->setParent(this);
}

void Vertex::absorb_removeVertex_retainEdge(Edge *e)
{
    if (!myEdge.contains(e))
    {
        qDebug() << "Absorb Edge NOT FOUND!";
        return;
    }
    Vertex * neighbour = 0;
    if (e->fromVertex() == this)
        neighbour = e->toVertex();
    else
        neighbour = e->fromVertex();
    absorbed.append(neighbour);
    absorbed.append(neighbour->getAbsorbedList());
    neighbour->setParent(this);
}

void Vertex::absorb_retainEdge(Edge *e)
{
    if (!myEdge.contains(e))
    {
        qDebug() << "Absorb Edge NOT FOUND!";
        return;
    }
    Vertex * neighbour = 0;
    if (e->fromVertex() == this)
        neighbour = e->toVertex();
    else
        neighbour = e->fromVertex();

    absorbed.append(neighbour);
    absorbed.append(neighbour->getAbsorbedList());
    neighbour->setParent(this);
}

void Vertex::absorb_retainEdge_setParentPointer(Edge *e)
{
    if (!myEdge.contains(e))
    {
        qDebug() << "Absorb Edge NOT FOUND!";
        return;
    }
    Vertex * neighbour = 0;
    if (e->fromVertex() == this)
        neighbour = e->toVertex();
    else
        neighbour = e->fromVertex();

    absorbed.append(neighbour);
    absorbed.append(neighbour->getAbsorbedList());
    neighbour->setParentPointerOnly(this);
}

void Vertex::absorb_singleton(Vertex *v)
{
    absorbed.append(v->getAbsorbedList());
    absorbed.append(v);
    v->setParent(this);
    v->remove_all_edges();
}

Vertex *Vertex::get_neighbour_fromEdge(quint32 edge_index)
{
    Vertex * neighbour = 0;
    Edge * edge = myEdge.at(edge_index);
    if (edge->fromVertex() == this)
        neighbour = edge->toVertex();
    else
        neighbour = edge->fromVertex();
    return neighbour;
}

Vertex *Vertex::get_neighbour_fromEdge(Edge *edge)
{
    Vertex * neighbour = 0;
    if (edge->toVertex() == this || edge->fromVertex() == this)
    {
        if (edge->fromVertex() == this)
            neighbour = edge->toVertex();
        else
            neighbour = edge->fromVertex();
    }
    else
        qDebug() << "ERROR: EITHER END OF THE EDGE IS NOT THE QUERIED VERTEX!";
    return neighbour;
}

Edge *Vertex::getHighestDegreeNeighbour()
{
    QList<Edge*> edge;
    Edge * final;
    quint32 highest = 0;
    for (int i = 0; i < myEdge.size(); i++)
    {
        Edge * e = myEdge.at(i);
        Vertex * v = get_neighbour_fromEdge(e);
        quint32 d = v->getNumberEdge();
        if (d > highest)
        {
            highest = d;
            edge.clear();
            edge.append(e);
        }
        else if (d == highest)
        {
            edge.append(e);
        }
    }
    if (edge.size() > 1)
    {
        std::uniform_int_distribution<int> distribution(0,edge.size()-1);
        int ran = distribution(gen);
        final = edge.at(ran);
    }
    else
        final = edge.at(0);
    return final;
}

QList<Edge *> Vertex::getAllEdge() const
{
    return myEdge;
}

Edge *Vertex::getEdge(quint32 edgeIndex) const
{
    return myEdge.at(edgeIndex);
}

/**  Return a neigbour vertex which was selected with uniform selectiong with a given bias to one's weight
 * @brief Vertex::getWeightedProbabilisticEdge
 * @return
 */
Edge *Vertex::getWeightedProbabilisticEdge()
{
    QList<Edge*> edge;
    for (int i = 0; i < myEdge.size(); i++)
    {
        Edge * e = myEdge.at(i);
        Vertex * v = get_neighbour_fromEdge(e);
        quint64 w = v->getWeight();
        for (quint64 j = 0; j < w; j++)
            edge.append(e);
    }
    std::uniform_int_distribution<quint64> distribution(0,edge.size()-1);
    quint64 ran = distribution(gen);
    return edge.at(ran);
}

/** Get Edege with Pr(e) = d(e)/sum d
 * @brief Vertex::getDegreeProbabilisticEdge
 * @return
 */
Edge *Vertex::getDegreeProbabilisticEdge()
{
    QList<Edge*> edge;
    for (int i = 0; i < myEdge.size(); i++)
    {
        Edge * e = myEdge.at(i);
        Vertex * v = get_neighbour_fromEdge(e);
        quint32 w = v->getNumberEdge();
        for (quint32 j = 0; j < w; j++)
            edge.append(e);
    }

    std::uniform_int_distribution<quint32> distribution(0,edge.size()-1);
    quint32 ran = distribution(gen);
    return edge.at(ran);
}


/** For aggregate with degree bias:
 * Return a neigbour vertex which was selected with uniform selectiong with a given bias to one's weight
 * @brief Vertex::aggregate_get_degree_biased_neighbour
 * @return
 */
Vertex *Vertex::aggregate_get_degree_biased_neighbour()
{
    QList<Vertex*> neighbours;
    for (int i = 0; i < myEdge.size(); i++)
    {
        Vertex * neighbour = this->get_neighbour_fromEdge(myEdge[i]);
        quint64 weight = neighbour->getWeight();
        for (quint64 j = 0; j < weight; j++)
            neighbours.append(neighbour);
    }
    std::uniform_int_distribution<quint64> distribution(0,neighbours.size()-1);
    quint64 ran = distribution(gen);
    return neighbours.at(ran);
}


void Vertex::set_vertex_as_absorbed(bool val)
{
    isAbsorbed = val;
}

bool Vertex::is_vertex_absorbed() const
{
    return isAbsorbed;
}

bool Vertex::is_vertex_dragged_along() const
{
    return isDraggedAlong;
}



/** GET THE NEIGHBOUR VERTEX THAT HAS THE HIGHEST NUMBER OF MUTUAL TRIANGULATION
 * @brief Vertex::getMostMutualVertex
 */
Edge * Vertex::getMostMutualVertex()
{
    QList<Edge*> ran_list;
    quint32 highest = 0;
    for (int i = 0; i < myEdge.size(); i++)
    {
        Vertex * neighbour = this->get_neighbour_fromEdge(myEdge[i]);
        quint64 similar = this->getNoOfTriangles(neighbour);
        if (similar > highest)
        {
            highest = similar;
            ran_list.clear();
            ran_list.append(myEdge[i]);
        }
        else if (similar == highest)
        {
            ran_list.append(myEdge[i]);
        }
    }
    if (ran_list.size() == 0)
    {
        std::uniform_int_distribution<int> distribution(0,myEdge.size()-1);
        int ran = distribution(gen);
        return myEdge.at(ran);
    }
    else if (ran_list.size() > 1)
    {
        std::uniform_int_distribution<int> distribution(0,ran_list.size()-1);
        int ran = distribution(gen);
        return myEdge.at(ran);
    }
    else
        return ran_list[0];
}



/** Get the Highest Triangulate Cluster
 * @brief Vertex::getHighestTriangulateCluster
 * @return
 */

Edge *Vertex::getHighestTriangulateCluster()
{
    //first get all neighbour cluster
    QList<Vertex*> centroids;
    QList<quint32> queried_edge;
    for (quint32 i = 0; i < myEdge.size(); i++)
    {
        Edge * e = myEdge.at(i);
        Vertex * neighbour = this->get_neighbour_fromEdge(e);
        //get the neighbour cluster
        Vertex * par = neighbour->getParent();
        QList<Vertex*> tree;
        while (par != 0)
        {
            neighbour = par;
            if (tree.contains(neighbour))
                break;
            else
                tree.append(neighbour);
            par = neighbour->getParent();
        }
        if (centroids.contains(neighbour))
            continue;
        else
        {
                centroids.append(neighbour);
                queried_edge.append(i);
        }

    }

    quint64 highest_score = 0;
    QList<quint32> index;

    for (quint32 i = 0 ; i < queried_edge.size(); i++)
    {
        quint32 edge_index = queried_edge.at(i);
        Edge * e = myEdge.at(edge_index);
        Vertex * adjacent = this->get_neighbour_fromEdge(e);

        Vertex * queried_centroid = centroids[i];
        quint32 score = 0;
        // count number of real triangles between V and This
        QList<quint32> current_iteration_neighbour = adjacent->getNeighbourIndexes();
        //get current centroid indexes
        QList<Vertex*> current_iteration_cluster = queried_centroid->getMyCluster();
        QList<quint32> current_iteration_cluster_indexes;
        for (quint32 j = 0; j < current_iteration_cluster.size(); j++ )
        {
            Vertex * v = current_iteration_cluster.at(j);
            current_iteration_cluster_indexes.append(v->getIndex());

        }

        //counting score
        for (quint32 j = 0; j < myNeighbours.size(); j++)
        {
            quint32 adj = myNeighbours[j];
            if (adj == adjacent->getIndex())
                continue;
            if (current_iteration_neighbour.contains(adj))
            {
                score++;
            }
            if (current_iteration_cluster_indexes.contains(adj))
            {
                score++;
            }
        }

        if (queried_centroid->getIndex() == this->getIndex())
            score -= myCluster.size();
        if (score < highest_score)
        {
            if (index.size() == 0)
            {
                highest_score = score;
                index.append(edge_index);
            }
        }
        else if (score == highest_score)
        {
            index.append(edge_index);
        }
        else if (score > highest_score)
        {
            highest_score = score;
            index.clear();
            index.append(edge_index);
        }
    }


    quint32 selected_index = 0;
    if (index.size() > 1)
    {
        std::uniform_int_distribution<int> distribution(0,index.size()-1);
        int ran = distribution(gen);
        selected_index = index.at(ran);
    }
    else
    {
        selected_index = index.at(0);
    }

    Edge * final = myEdge.at(selected_index);
    return final;
}


/** A vertex is selected with a probability toward its number of triangulation
 * Sample using uniform-simulate
 * @brief Vertex::getProbabilisticTriangulationCoeffVertex
 * @return selected Edge
 */

Edge *Vertex::getProbabilisticTriangulationCoeffVertex()
{
    QList<Edge*> sample;
    for (quint32 i = 0; i < myEdge.size(); i++)
    {
        Vertex * neighbour = this->get_neighbour_fromEdge(myEdge[i]);
        quint32 similar = this->getNoOfTriangles(neighbour);

        sample.append(myEdge[i]);
        for (quint32 j =0; j < similar; j++)
            sample.append(myEdge[i]);
    }
    if(!sample.empty())
    {
        std::uniform_int_distribution<quint32> distribution(0,sample.size()-1);
        quint32 ran = distribution(gen);
        return sample.at(ran);
    }
    else
    {
        std::uniform_int_distribution<int> distribution(0,myEdge.size()-1);
        int ran = distribution(gen);
        return myEdge.at(ran);
    }
}


/** GET A NEIGHBOUR VERTEX FOR ABSORPTION. THE RULE IS AS FOLLOWS:
 * For a vertex v, the probability of selecting a neighbour vertex u is:
 * Pr(u) = ((number of triangulation) * (sum/weight) ) / (sum over all neighbour)
 * @brief Vertex::getProbabilisticTriangulationAndWeightVertex
 * @return the selected vertex
 */
Edge *Vertex::getProbabilisticTriangulationAndWeightVertex()
{
    QList<Edge*> sample;
    if (this->getNumberEdge() == 1)
        return myEdge.at(0);

    for (int i = 0; i < myEdge.size(); i++)
    {
        Vertex * neighbour = this->get_neighbour_fromEdge(myEdge[i]);
        quint32 similar = this->getNoOfTriangles(neighbour);
        quint64 normalise_w = 0;
        if (neighbour->getNoChild() > 0)
            normalise_w = neighbour->getExtraWeight() / neighbour->getNoChild();
        quint64 new_w = (similar*2) * (neighbour->getWeight() + normalise_w);
        for (quint64 j =0; j < new_w; j++)
            sample.append(myEdge[i]);
    }
    if (sample.size() == 0)
        return 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    gen.seed(QTime::currentTime().msec());
    std::uniform_int_distribution<quint64> distribution(0, sample.size() - 1);
    quint64 ran = distribution(gen);
    return sample.at(ran);
}



quint32 Vertex::getNoOfTriangles(Vertex *v)
{
    QSet<quint32> thisAdj = this->getNeighbourIndexes().toSet();
    QSet<quint32> neighbourAdj = v->getNeighbourIndexes().toSet();
    quint32 similar = thisAdj.intersect(neighbourAdj).size();
    return similar;
}


QList<Vertex *> Vertex::getMyCluster()
{
    return myCluster;
}


void Vertex::addMemberToCluster(Vertex *v)
{
    if (myCluster.contains(v))
        qDebug() << "ERR: MEMBER ALREADY IN CLUSTER";
    else
        myCluster.append(v);
}


void Vertex::addMemberToCluster(QList<Vertex *> v)
{
    if (v.size() == 0)
        return;
    for (quint32 i = 0; i < v.size(); i++)
    {
        if (!myCluster.contains(v[i]))
            myCluster.append(v[i]);
    }
}


void Vertex::clearCluster()
{
    myCluster.clear();
}


void Vertex::clearAbsorbed()
{
    absorbed.clear();
}

void Vertex::setTruthCommunity(const int &p)
{
    myRealCommunity = p;
}

int Vertex::getTruthCommunity() const
{
    if (myRealCommunity == -1)
        qDebug() << "Have No Truth Community";
    return myRealCommunity;
}


void Vertex::resetClusterRelevant()
{
    myCluster.clear();
    absorbed.clear();
    myWeight = 1;
    parent = 0;
    isDraggedAlong = false;
    isAbsorbed = false;
    noOfChild = 0;
    ExtraWeight = 0;
    myNeighbours.clear();
    myEdge.clear();
}

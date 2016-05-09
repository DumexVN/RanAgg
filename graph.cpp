#include "graph.h"

#include <limits>
#include <random>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/random_layout.hpp>
#include <boost/graph/topology.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/clustering_coefficient.hpp>
#include <boost/graph/exterior_property.hpp>

#include <QTime>

std::random_device rd;
std::mt19937 generator(rd());


QString globalDirPath;
qint32 global_e = 0;
qint32 global_v = 0;
static int no_run = 0;

Graph::Graph()
{   //set up graphic scenes to display all kinds of stuff
    graphIsReady = false;
    generator.seed(sqrt(QTime::currentTime().msec()*QTime::currentTime().msec()));
}


// ----------------------- GRAPH GENERATOR -------------------------------------------

/** READ GML FILE AND PARSE FOR EDGE FILE
 * Otherwise GML takes way too long to parse
 * @brief Graph::read_GML_file
 * @param filePath
 */
void Graph::read_GML_file(QString filePath)
{
    QFile file(filePath);
    if (!file.exists())
    {
        qDebug() << "File Not Found";
        return;
    }
    GMLpath = filePath;
    QFileInfo fileInfo(file);
    QDir dir = fileInfo.absoluteDir();
    if (!file.exists())
    {
        qDebug() << "FILE NOT FOUND";
        return;
    }

    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    QString all = in.readAll();
    QStringList split = all.split("\n");
    QString directed = split[3];
    if (!directed.contains("0"))
    {
        qDebug() << "DIRECTED GRAPH";
        return; //directed graph
    }
    QSet<int> v;
    QList<QPair<int,int> > my_edge;
    QStringListIterator iter(split);
    while(iter.hasNext())
    {
        QString str = iter.next();
        if (str.contains("node") || str.contains("edge"))
        {
            bool node = str.contains(" node");
            bool edge = str.contains(" edge");
            if (!node && !edge)
            {
                qDebug() << "FILE FORMATE ERROR";
                return;
            }
            else
                iter.next();
            if (node)
            {
                while (iter.peekNext() != "  ]")
                {
                    QStringList cur = iter.next().split(" ");
                    if (cur.contains("id"))
                    {
                        bool ok;
                        int i = cur.last().toInt(&ok);
                        if (ok)
                        {
                            Vertex * v = new Vertex;
                            v->setIndex(i);
                            myVertexList.append(v);
                        }
                        else
                            qDebug() << "Error While Parsing GML: id not recognised!";
                    }
                    else if (cur.contains("label"))
                    {
                        int index = cur.indexOf("label");
                        index++;
                        QString label;
                        for (index; index < cur.size(); index++)
                        {
                            label.append(cur.at(index));
                        }
                    }
                    else if (cur.contains("value"))
                    {
                        bool ok;
                        //vertex_weight.append(cur.last().toInt(&ok));
                        if (!ok)
                        {
                            QString c = cur.last();
                            //QString name = vertex_label.last();
                            QString parsed_name = "";
                           // for (int i = 1; i < name.size()-1;i++) parsed_name += name[i];
                        }
                    }
                }
            }
            else if (edge)
            {
                int source = -1, des = -1;
                while (iter.peekNext() != "  ]")
                {
                    QStringList cur = iter.next().split(" ");
                    if (cur.contains("source"))
                    {
                        source = cur.last().toInt();
                    }
                    else if (cur.contains("target"))
                    {
                        des = cur.last().toInt();
                    }
                    else if (cur.contains("value"))
                    {
                      //  edge_weight.append(cur.last().toInt());
                    }
                }
                if (source != -1 && des != -1)
                {
                    QPair<int,int> e = qMakePair(source,des);
                    QPair<int,int> reverse = qMakePair(des,source);
                    if (!my_edge.contains(e) && !my_edge.contains(reverse))
                    {
                        my_edge.append(e);
                        my_edge.append(reverse);
                    }
                    else
                        qDebug() << "Error While Parsing GML: Duplicate Edge!";
                }
                else
                {
                    qDebug() << "EDGE ERROR" << source << des;
                    return;
                }
            }
            else
            {
                qDebug() << "ERROR: UNKNOWN ERROR WHILE PARSING! RETURNING";
                return;
            }

        }
        else
            continue;
    }

    for (int i = 0; i < my_edge.size(); i+= 2)
    {
        QPair<int,int> e = my_edge[i];
        int v = e.first, u = e.second;
        Vertex * from = myVertexList[v];
        Vertex * to = myVertexList[u];
        Edge * newe = new Edge(from,to,i/2);
        myEdgeList.append(newe);
    }

    qDebug() << "GML Parsed Successfully!";
    qDebug() << "V: " << myVertexList.size() << "; E: " << myEdgeList.size();
    save_edge_file_from_GML();

}


/** Save GML file as Simple .txt File for faster parsing
 * @brief Graph::save_edge_file_from_GML
 */
void Graph::save_edge_file_from_GML()
{
    qDebug() << "No Problem Detected! Saving Edge File";
    if (GMLpath.size() == 0)
    {
        qDebug() << "GML Path Has Not Been Set";
    }
    else
    {
        QFileInfo info(GMLpath);
        QDir dir = info.absoluteDir();
        QFile vFile(dir.absolutePath() + "/vertex_file.txt");
        vFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream os(&vFile);
        os << QString::number(myVertexList.size()) << '\t' << QString::number(myEdgeList.size()) << endl;
        vFile.close();

        QFile outFile(dir.absolutePath() + "/edge_file.txt");
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream ts(&outFile);
        for (int i = 0; i < myEdgeList.size(); i++)
        {
            int from = myEdgeList[i]->fromVertex()->getIndex(), to = myEdgeList[i]->toVertex()->getIndex();
            ts << QString::number(from) << '\t' << QString::number(to) << endl;
        }
        outFile.close();
    }
}

/** Just Save the Edge to current Dir
 * @brief Graph::save_current_run_as_edge_file
 */
void Graph::save_current_run_as_edge_file(QString fileName)
{
    QFile outFile(fileName);
    if (outFile.exists())
        outFile.remove();
    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream ts(&outFile);
    ts << "Source\tTarget" << endl;
    for (int i = 0; i < myEdgeList.size(); i++)
    {
        quint32 from = myEdgeList[i]->fromVertex()->getIndex(), to = myEdgeList[i]->toVertex()->getIndex();
        ts << QString::number(from) << '\t' << QString::number(to) << endl;
    }
    outFile.close();
}

/**
 * @brief Graph::read_simple_edge
 * @param dirPath
 */
void Graph::read_simple_edge(QString dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
    {
        qDebug() << "DIR NOT EXISTS! Terminating ...";
        return;
    }
    globalDirPath = dirPath;
    QStringList filters;
    filters << "*.txt";
    QFileInfoList file = dir.entryInfoList(filters);
    QString v_file, e_file;
    for (int i = 0; i < file.size(); i++)
    {
        QFileInfo f = file.at(i);
        QString name = f.fileName();
        if (name.contains("edge"))
            e_file = f.absoluteFilePath();
        else if (name.contains("vertex"))
            v_file = f.absoluteFilePath();
        else
        {
            qDebug() << "While READING FILES: File not Recognised!";
            qDebug() << file[i].fileName() << " Skipping this File";
        }
    }

    //reload original vertices
    //Parsing
    QFile efile(e_file), vfile(v_file);
    if (!efile.exists() || !vfile.exists())
    {
        qDebug() << "FILE NOT FOUND! Recheck! Terminating ...";
        return;
    }
    //else
    vfile.open(QFile::ReadOnly | QFile::Text);
    QTextStream vin(&vfile);
    QStringList str = vin.readLine().split('\t');
    bool load;
    global_v = str[0].toUInt(&load);
    global_e = str[1].toUInt(&load);
    if (!load)
    {
        qDebug() << "ERROR LOADING V FILE";
        return;
    }
    qDebug() << "Graph: " << "V: " <<  global_v << "; E: " << global_e;
    vfile.close();
    //READ E FILE
    efile.open(QFile::ReadOnly | QFile::Text);
    QTextStream ein(&efile);
    QList<QPair<quint32,quint32> > edge;
    while (!ein.atEnd())
    {
        QStringList str = ein.readLine().split('\t');
        bool ok;
        quint32 v1 = str[0].toUInt(&ok), v2 = str[1].toUInt(&ok);
        if (ok)
        {
            edge.append(qMakePair(v1,v2));
        }
    }
    efile.close();
    qDebug() << "Generating Vertex and Edges ...";
    // adding ve edge independent of global file
    for (int i = 0; i < global_v; i++ )
    {
        Vertex * v = new Vertex;
        v->setIndex(i);
        myVertexList.append(v);
    }

    for (int i = 0; i < edge.size(); i++)
    {
        QPair<int,int> e = edge[i];
        int v = e.first, u = e.second;
        Vertex * from = myVertexList[v];
        Vertex * to = myVertexList[u];
        Edge * newe = new Edge(from,to,i/2);
        myEdgeList.append(newe);
    }


    bool fit = false;
    if (myVertexList.size() == global_v && myEdgeList.size() == global_e)
        fit = true;
    qDebug() << "Check Sum" << fit;
    if (fit)
    {
        graphIsReady = true;
    }
    else
    {
        qDebug() << "Preset V: " << global_v << "; E: " << global_e;
        qDebug() << "Load V: " << myVertexList.size() << "; E: " << myEdgeList.size();
    }

}

/*
 * RECONNECT THE GRAPH AFTER AN AGGREGATION HAS BEEN DONE
 */
void Graph::reConnectGraph()
{
    LARGE_reload();
    return;
}

// -----------------------------RANDOM AGGREGATE CLUSTERING -------------------------
// ----------------------------------------------------------------------------------
/** Type I.a - Uniform (Everything is Uniformly at Random)
 * @brief Graph::random_aggregate
 */
void Graph::random_aggregate()
{
    qDebug() << "CHECKING CONDITION || RECONNECTING GRAPH";
    qDebug() << "Graph Condition: " << graphIsReady <<";"<<myVertexList.size();
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;

    quint32 t = 0;
    QTime t0;
    t0.start();
    qDebug() << "STARTING...";
    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        if (selected->is_vertex_absorbed() || selected->getParent() != 0)
        {
            qDebug() << "Candidate Has Been Clustered!!!!! ";
            return;
        }
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else // else absorb
        {
            std::uniform_int_distribution<quint32> distribution2(0,no_neighbour-1);
            quint32 selected_edge_index = distribution2(generator);
            Edge * e = selected->getEdge(selected_edge_index);
            Vertex * neighbour = selected->get_neighbour_fromEdge(e); //get the neighbour (not clean)
            hierarchy.append(qMakePair(neighbour->getIndex(), selected->getIndex()));
            selected->absorb_removeEdge(e);
            players.removeOne(neighbour);
            t++;
        }
    }
    centroids = winners;

    qDebug("I.a - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}

/** Type I.b - Uniformly and Comparing the CURRENT DEGREE
 * Pr(v) = u.a.r
 * Pr(u) = u.a.r
 * Graph Type: Destructive
 * @brief Graph::random_aggregate_with_degree_comparison
 */
void Graph::random_aggregate_with_degree_comparison()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }

    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else // else absorb
        {
            std::uniform_int_distribution<quint32> distribution2(0,no_neighbour-1);
            quint32 selected_edge_index = distribution2(generator);
            Edge * e = selected->getEdge(selected_edge_index);
            Vertex * neighbour = selected->get_neighbour_fromEdge(selected_edge_index); //get the neighbour (not clean)
            Vertex * winner, * loser;
            quint32 selected_d = selected->getNumberEdge(), neighbour_d = neighbour->getNumberEdge();
            if (selected_d >= neighbour_d)
            {
                winner = selected;
                loser = neighbour;
            }
            else
            {
                winner = neighbour;
                loser = selected;
            }

            //abosbr
            hierarchy.append(qMakePair(loser->getIndex(), winner->getIndex()));
            winner->absorb_removeEdge(e);
            players.removeOne(loser);
            t++;
        }
    }
    centroids = winners;
    qDebug("I.b - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}


/** Type I.c - Uniformly and Comparing the ORIGINAL DEGREE
 * Pr(v) = u.a.r
 * Pr(u) = u.a.r
 * Graph Type: Destructive
 * @brief Graph::random_aggregate_with_weight_comparison
 */
void Graph::random_aggregate_with_weight_comparison()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    for (quint32 i = 0; i < myVertexList.size(); i++)
    {
        Vertex * v = myVertexList.at(i);
        v->setWeight(v->getNumberEdge());
    }

    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else // else absorb
        {

            std::uniform_int_distribution<quint32> distribution2(0,no_neighbour-1);
            quint32 selected_edge_index = distribution2(generator);
            Edge * e = selected->getEdge(selected_edge_index);
            Vertex * neighbour = selected->get_neighbour_fromEdge(selected_edge_index); //get the neighbour (not clean)
            Vertex * winner, * loser;
            quint64 selected_w = selected->getWeight(), neighbour_w = neighbour->getWeight();
            if (selected_w >= neighbour_w)
            {
                winner = selected;
                loser = neighbour;
            }
            else
            {
                winner = neighbour;
                loser = selected;
            }
            hierarchy.append(qMakePair(loser->getIndex(), winner->getIndex()));
            winner->absorb_removeEdge(e);
            players.removeOne(loser);
            t++;
        }
    }
    centroids = winners;
    qDebug("I.c - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}

/** Type II.a - Select Neighbour With the ORIGINAL DEGREE BIAS
 * Pr(v) = u.a.r
 * Pr(u) = w(u) / w(i) forall i in adj(v)
 * @brief Graph::random_aggregate_with_neighbour_degree_bias
 */
void Graph::random_aggregate_with_neighbour_initial_degree_bias()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    for (int i = 0; i < myVertexList.size(); i++)
    {
        Vertex * v = myVertexList.at(i);
        v->setWeight(v->getNumberEdge());
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;
  //  QSequentialAnimationGroup * group_anim = new QSequentialAnimationGroup;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else // else absorb
        {
            Vertex * neighbour = selected->aggregate_get_degree_biased_neighbour();
            Edge * e = selected->getEdgeFromVertex(neighbour);
            //create the animation
            selected->absorb_removeEdge(e);
            hierarchy.append(qMakePair(neighbour->getIndex(), selected->getIndex()));
            players.removeOne(neighbour);
            t++;
        }
    }
    centroids = winners;
    qDebug("II.a - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}

/** Type II.b - Select Neighbour With the CURRENT Degree Bias
 * Pr(v) = u.a.r
 * Pr(u) = d(u) / sum d(i) forall i in adj(v)
 * u -> v
 * @brief Graph::random_aggregate_with_neighbour_CURRENT_degree_bias
 */
void Graph::random_aggregate_with_neighbour_CURRENT_degree_bias()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;

    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else // else absorb
        {
            Edge * e = selected->getDegreeProbabilisticEdge();
            Vertex * neighbour = selected->get_neighbour_fromEdge(e);
            selected->absorb_removeEdge(e);
            hierarchy.append(qMakePair(neighbour->getIndex(), selected->getIndex()));
            players.removeOne(neighbour);
            t++;
        }
    }
    centroids = winners;
    qDebug("II.b - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}


/** Type II.c - Aggregate HIGHEST DEGREE neighbour
 * Pr(v) = u.a.r
 * Select u: arg max d(u)
 * if d(v) < d(u) ...
 * @brief Graph::random_aggregate_highest_CURRENT_degree_neighbour
 */
void Graph::random_aggregate_highest_CURRENT_degree_neighbour()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;
   // QSequentialAnimationGroup * group_anim = new QSequentialAnimationGroup;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else // else absorb
        {
            Edge * e = selected->getHighestDegreeNeighbour();
            Vertex * neighbour = selected->get_neighbour_fromEdge(e);
            quint32 dv = selected->getNumberEdge(), du = neighbour->getNumberEdge();
            Vertex * winner, * loser;
            if (dv >= du)
            {
                winner = selected;
                loser = neighbour;
            }
            else
            {
                winner = neighbour;
                loser = selected;
            }

            winner->absorb_removeEdge(e);
            hierarchy.append(qMakePair(neighbour->getIndex(), selected->getIndex()));
            players.removeOne(loser);
            t++;
        }
    }
    centroids = winners;
    qDebug("II.c - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}


/** Type II.d - Select Min DEGREE Neighbour
 * Pr(v) = u.a.r
 * Select u: arg min d(u): u in adj(v)
 * @brief Graph::random_aggregate_with_minimum_weight_neighbour
 */
void Graph::random_aggregate_with_minimum_weight_neighbour()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    for (int i = 0; i < myVertexList.size(); i++)
    {
        Vertex * v = myVertexList.at(i);
        v->setWeight(v->getNumberEdge());
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;
   // QSequentialAnimationGroup * group_anim = new QSequentialAnimationGroup;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else // else absorb
        {
            Edge * e = selected->getSmallestCurrentDegreeNeighbour();
            Vertex * neighbour = selected->get_neighbour_fromEdge(e);
            selected->absorb_removeEdge(e);
            hierarchy.append(qMakePair(neighbour->getIndex(), selected->getIndex()));
            players.removeOne(neighbour);
            t++;
        }
    }
    centroids = winners;
    qDebug("II.d - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}


/** Type II.e Probabilistic Minimum Degree Neighbour (Destructive, Greedy)
 * Pr(v) = d(v)/ sum d(i) forall
 * Select u: arg min u forall u in adj(v)
 * @brief Graph::random_aggregate_probabilistic_lowest_degree_neighbour_destructive
 */
void Graph::random_aggregate_probabilistic_lowest_degree_neighbour_destructive()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;

    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        QList<Vertex*> ran_list;
        for (int i = 0; i < players.size(); i++)
        {
            Vertex * v = players.at(i);
            for (quint32 j = 0; j < v->getNumberEdge(); j++)
                ran_list.append(v);
        }
        if (ran_list.size() == 0)
        {
            quint32 size = players.size();
            std::uniform_int_distribution<quint32> distribution(0,size-1);
            quint32 selected_index = distribution(generator);
            Vertex * selected = players.at(selected_index);
            winners.append(selected);
            players.removeOne(selected);
        }
        else
        {
            quint64 size = ran_list.size();
            std::uniform_int_distribution<quint64> distribution(0,size-1);
            quint64 selected_index = distribution(generator);
            Vertex * selected = ran_list.at(selected_index);
            //get a neighbour
            quint64 no_neighbour = selected->getNumberEdge();
            if (no_neighbour == 0) // if there is no neighbour, declare a winner
            {
                winners.append(selected);
                players.removeOne(selected);
            }
            else // else absorb
            {
                Edge * e = selected->getSmallestCurrentDegreeNeighbour();
                Vertex * neighbour = selected->get_neighbour_fromEdge(e); //get the neighbour (not clean)
                Vertex * winner, * loser;
                winner = selected;
                loser = neighbour;
                hierarchy.append(qMakePair(winner->getIndex(), loser->getIndex()));

                winner->absorb_removeEdge(e);
                players.removeOne(loser);
            }
        }
        t++;
    }
    centroids = winners;
    qDebug("II.e - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}


/** Type II.f - Probabilistic Min DEGREE Neighbour (RETENTIVE)
 * Pr(v) = w(v)/ sum w(i) forall i in V
 * Select u: arg min d(u): u in adj(v)
 * u -> v: w(v) += w(u)
 * @brief Graph::random_aggregate_probabilistic_candidate_with_minimum_weight_neighbour
 */
void Graph::random_aggregate_probabilistic_candidate_with_minimum_weight_neighbour()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    for (int i = 0; i < myVertexList.size(); i++)
    {
        Vertex * v = myVertexList.at(i);
        v->setWeight(v->getNumberEdge());
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;

    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        QList<Vertex*> ran_list;
        for (quint32 i = 0 ;i < players.size(); i++)
        {
            Vertex * v = players.at(i);
            quint64 w = v->getWeight();
            for (quint64 j = 0; j < w; j++)
                ran_list.append(v);
        }
        if (ran_list.size() == 0)
        {
            quint32 size = players.size();
            std::uniform_int_distribution<quint32> distribution(0,size-1);
            quint32 selected_index = distribution(generator);
            Vertex * selected = players.at(selected_index);
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else
        {
            quint64 size = ran_list.size();
            std::uniform_int_distribution<quint64> distribution(0,size-1);
            quint64 selected_index = distribution(generator);
            Vertex * selected = ran_list.at(selected_index);
            //get a neighbour
            quint32 no_neighbour = selected->getNumberEdge();
            if (no_neighbour == 0) // if there is no neighbour, declare a winner
            {
                winners.append(selected);
                players.removeOne(selected);
                t++;
            }
            else // else absorb
            {
                Edge * e = selected->getSmallestCurrentDegreeNeighbour();
                Vertex * neighbour = selected->get_neighbour_fromEdge(e); //get the neighbour (not clean)
                Vertex * winner, * loser;
                winner = selected;
                loser = neighbour;
                hierarchy.append(qMakePair(winner->getIndex(), loser->getIndex()));
                winner->absorb_removeEdge(e);
                winner->setWeight(loser->getWeight() + winner->getWeight());
                players.removeOne(loser);
                t++;
            }
        }
    }
    centroids = winners;
    qDebug("II.f - Time elapsed: %d ms", t0.elapsed());
    // draw_dense_graph_aggregation_result();
    // group_anim->start();
    // connect(group_anim, SIGNAL(finished()), this, SLOT(large_graph_parse_result()));
    large_graph_parse_result();
}


/** Type II.g - Deterministic Max Degree Candidate (Destructive)
 * Select candidate: v: arg max d(v)
 * Select u: arg min d(u)
 * @brief Graph::random_aggregate_greedy_max_degree
 */
void Graph::random_aggregate_greedy_max_degree()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;
   // QSequentialAnimationGroup * group_anim = new QSequentialAnimationGroup;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        QList<Vertex*> ran_list;
        quint32 max_d = 0;
        for (int i = 0; i < players.size(); i++)
        {
            Vertex * v = players.at(i);
            quint32 dv = v->getNumberEdge();
            if (dv > max_d)
            {
                max_d = dv;
                ran_list.clear();
                ran_list.append(v);
            }
            else if (dv == max_d)
            {
                ran_list.append(v);
            }
        }

        int size = ran_list.size();
        std::uniform_int_distribution<int> distribution(0,size-1);
        int selected_index = distribution(generator);
        Vertex * selected = ran_list.at(selected_index);
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
        }
        else // else absorb
        {
            Edge * e = selected->getSmallestCurrentDegreeNeighbour();
            Vertex * neighbour = selected->get_neighbour_fromEdge(e); //get the neighbour (not clean)
            Vertex * winner, * loser;
            winner = selected;
            loser = neighbour;
            hierarchy.append(qMakePair(winner->getIndex(), loser->getIndex()));
            winner->absorb_removeEdge(e);
            players.removeOne(loser);
        }

        t++;
    }
    centroids = winners;
    qDebug("II.g - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}


/** Type II.h (Retentive) Greedy Max Weight (candidate selection)
 * Select Candidate: v = arg max w(v)
 * Select Neighbour: u = arg min w(u)
 * @brief Graph::random_aggregate_greedy_max_weight
 */
void Graph::random_aggregate_greedy_max_weight()
{
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    for (int i = 0; i < myVertexList.size(); i++)
    {
        Vertex * v = myVertexList.at(i);
        v->setWeight(v->getNumberEdge());
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;

    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        QList<Vertex*> ran_list;
        quint64 max_w = 0;
        for (int i = 0; i < players.size(); i++)
        {
            Vertex * v = players[i];
            quint64 wv = v->getWeight();
            if (wv > max_w)
            {
                max_w = wv;
                ran_list.clear();
                ran_list.append(v);
            }
            else if (wv == max_w)
            {
                ran_list.append(v);
            }
        }

        int size = ran_list.size();
        std::uniform_int_distribution<int> distribution(0,size-1);
        int selected_index = distribution(generator);
        Vertex * selected = ran_list.at(selected_index);
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else // else absorb
        {
            Edge * e = selected->getSmallestCurrentWeightNeighbour();
            Vertex * neighbour = selected->get_neighbour_fromEdge(e); //get the neighbour (not clean)
            Vertex * winner, * loser;
            winner = selected;
            loser = neighbour;
            hierarchy.append(qMakePair(winner->getIndex(), loser->getIndex()));
            winner->absorb_removeEdge(e);
            winner->setWeight(loser->getWeight() + winner->getWeight());
            players.removeOne(loser);
            t++;
        }

    }
    centroids = winners;
    qDebug("II.h - Time elapsed: %d ms", t0.elapsed());
    // draw_dense_graph_aggregation_result();
    // group_anim->start();
    // connect(group_anim, SIGNAL(finished()), this, SLOT(large_graph_parse_result()));
    large_graph_parse_result();
}


/** Type III.c - Select Highest Triangles Neighbour Destructive
 * Pr(v) = u.a.r
 * Select u: arg max tri(u) forall u in adj(v)
 * if w(v) > w(u) then u -> v and vice versa
 * SELECTED VERTEX V ABSORBS THE HIGHEST-TRIANGULATED VERTEX U
 * @brief Graph::random_aggregate_with_highest_triangulated_vertex
 */
void Graph::random_aggregate_with_highest_triangulated_vertex()
{
    hierarchy.clear();
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    for (int i = 0; i < myVertexList.size(); i++)
    {
        Vertex * v = myVertexList.at(i);
        v->setWeight(v->getNumberEdge());
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;
   // QSequentialAnimationGroup * group_anim = new QSequentialAnimationGroup;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        //get a neighbour
        quint32 no_neighbour = selected->getNumberEdge();
        if (no_neighbour == 0) // if there is no neighbour, declare a winner
        {
            winners.append(selected);
            players.removeOne(selected);
            t++;
        }
        else // else absorb
        {
            Edge * e = selected->getMostMutualVertex();
            Vertex * neighbour, * winner, * loser;
            if (e->toVertex() == selected)
                neighbour = e->fromVertex();
            else
                neighbour = e->toVertex();

            if (selected == neighbour)
            {   qDebug() << "BUG CHECK" << "SELECTED POINTER == NEIGHBOUR POINTER";
                return;
            }
            quint64 u_w = selected->getWeight(), v_w = neighbour->getWeight();
            if (u_w >= v_w)
            {
                winner = selected;
                loser = neighbour;
            }
            else
            {
                winner = neighbour;
                loser = selected;
            }
            winner->absorb_removeEdge(e);
            hierarchy.append(qMakePair(loser->getIndex(), winner->getIndex()));
            players.removeOne(loser);
            t++;
        }
    }
    centroids = winners;
    qDebug("III.c - Time elapsed: %d ms", t0.elapsed());
    large_graph_parse_result();
}

// ---------------------------------------- AGGREGATION WHITE RETAINING VERTEX -----------------------------
/** The absorbed vertices are now retained in the graph.
 * Type III.a - Highest Triangles Neighbour
 * Pr(v) = u.a.r
 * Selet u: arg max tri(u)
 * Stationary
 * @brief Graph::random_aggregate_retain_vertex_using_triangulation_and_weight_comparison
 */
void Graph::random_aggregate_retain_vertex_using_triangulation()
{
    hierarchy.clear();
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        if (selected->getNumberEdge() == 0)
            players.removeOne(selected);
        else
        {
            Edge * e = selected->getMostMutualVertex();
            Vertex * neighbour, * winner, * loser;
            if (e->toVertex() == selected)
                neighbour = e->fromVertex();
            else
                neighbour = e->toVertex();

            winner = neighbour;
            loser = selected;
            //create the animation
            winner->absorb_retainEdge(e);
            hierarchy.append(qMakePair(loser->getIndex(), winner->getIndex()));
            players.removeOne(loser);
        }
        t++;
    }
    qDebug("III.a - Time elapsed: %d ms", t0.elapsed());
    large_parse_retain_result();

}


/** Type III.b - Probabilistic Triangle Neighbour
 * Select candidate u.a.r
 * Select neighbour with Pr = tri(u)/sum_tri(u)
 * @brief Graph::random_aggregate_retain_vertex_using_probabilistic_triangulation
 */
void Graph::random_aggregate_retain_vertex_using_probabilistic_triangulation()
{
    hierarchy.clear();
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }

    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);

        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        if (selected->getNumberEdge() == 0)
            players.removeOne(selected);
        else
        {
            Edge * e = selected->getProbabilisticTriangulationCoeffVertex();
            Vertex * neighbour, * winner, * loser;
            if (e->toVertex() == selected)
                neighbour = e->fromVertex();
            else
                neighbour = e->toVertex();
            winner = neighbour;
            loser = selected;
            winner->absorb_retainEdge(e);
            hierarchy.append(qMakePair(loser->getIndex(), winner->getIndex()));
            players.removeOne(loser);
        }
        t++;
    }
    qDebug("III.b - Time elapsed: %d ms", t0.elapsed());
    large_parse_retain_result();
}


/** The absorbed vertices are now retained in the graph.
 * Type III.d - Probabilistic Triangles Emphaised Cluster
 * Pr(v) = u.a.r
 * f(u) = (tri(u)*2) * (extra_w(u)/no_absorbed(u))
 * Pr(u) = f(u) / sum f(i) forall i in adj(v)
 * @brief Graph::random_aggregate_retain_vertex_using_triangulation_and_weight_comparison
 */
void Graph::random_aggregate_retain_vertex_using_triangulation_times_weight()
{
    hierarchy.clear();
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    for (int i = 0; i < myVertexList.size(); i++)
    {
        Vertex * v = myVertexList.at(i);
       // v->setWeight(v->getNumberEdge());
        v->setWeight(1);
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    QList<Vertex*> winners;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);

        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        if (selected->getNumberEdge() == 0)
            players.removeOne(selected);
        else
        {
            Edge * e = selected->getProbabilisticTriangulationAndWeightVertex();
            if (e == 0)
            {
                selected->setParent(selected);
                players.removeOne(selected);
                continue;
            }
            Vertex * neighbour, * winner, * loser;
            if (e->toVertex() == selected)
                neighbour = e->fromVertex();
            else
                neighbour = e->toVertex();

            winner = neighbour;
            loser = selected;
            //create the animation
            winner->absorb_retainEdge(e);
            hierarchy.append(qMakePair(loser->getIndex(), winner->getIndex()));
            players.removeOne(loser);
        }
        t++;
    }
    qDebug("III.d - Time elapsed: %d ms", t0.elapsed());
    large_parse_retain_result();

}

/** Type III.e - Highest Tri(Cluster)
 * Pr(v) = u.a.r
 * LEt C(u) be the set of vertex in u cluster:
 * Select u: arg max tri(C(u)) forall u in adj(v)
 * @brief Graph::random_aggregate_retain_vertex_using_triangulation_of_cluster
 */
void Graph::random_aggregate_retain_vertex_using_triangulation_of_cluster()
{
    hierarchy.clear();
    if (!checkGraphCondition())
    {
        reConnectGraph();
    }
    for (int i = 0; i < myVertexList.size(); i++)
    {
        Vertex * v = myVertexList.at(i);
        v->setWeight(v->getNumberEdge());
    }
    //initialise arrays
    QList<Vertex*> players = myVertexList;
    quint32 t = 0;
    QTime t0;
    t0.start();

    while(!players.empty()) //start
    {
        //select a vertex uniformly at random
        quint32 size = players.size();
        std::uniform_int_distribution<quint32> distribution(0,size-1);
        quint32 selected_index = distribution(generator);
        Vertex * selected = players.at(selected_index);
        if (selected->getNumberEdge() == 0)
            players.removeOne(selected);
        else
        {
            Edge * e = selected->getHighestTriangulateCluster();
            Vertex * neighbour, * winner, * loser;
            if (e->toVertex() == selected)
                neighbour = e->fromVertex();
            else
                neighbour = e->toVertex();

            winner = neighbour;
            loser = selected;
            //create the animation
            winner->absorb_retainEdge(e);
            hierarchy.append(qMakePair(loser->getIndex(), winner->getIndex()));
            players.removeOne(loser);
        }
        t++;
    }

    qDebug("III.e - Time elapsed: %d ms", t0.elapsed());
    large_parse_retain_result();

}



bool Graph::checkGraphCondition()
{
    if (myVertexList.empty())
    {
        qDebug() << "V is empty, GENERATE A GRAPH FIRST!";
        return false;
    }
    if (!graphIsReady)
    {
       // qDebug() << "Graph is DISCONNECTED!";
        return false;
    }
    else return true;
}


// ------------------------- FOR LARGE GRAPH ----------------------------------------
// ------------ TOO LAZY TO SEPERATE TO A DIFFRENT PROJECT --------------------------
/** READ LARGE GRAPH WITH GROUND TRUTH COMMUNITIES
 * PARSE AND REINDEXING SO IT FOLLOWS A CONSISTENT INDICES
 * FILES ARE FROM SNAP - STANFORD
 * @brief Graph::read_large_graph_with_ground_truth_communities
 */
void Graph::read_large_graph_with_ground_truth_communities()
{
    //testing
    //q
    /*
    for (int i = 0; i < 6; i++)
    {
        Vertex * v = new Vertex;
        v->setIndex(i);
        myVertexList.append(v);
    }
    Edge * e = new Edge(myVertexList[0],myVertexList[1],0);
    Edge * e1 = new Edge(myVertexList[0],myVertexList[2],1);
    Edge * e2 = new Edge(myVertexList[1],myVertexList[2],2);
    Edge * e3 = new Edge(myVertexList[2],myVertexList[3],3);
    Edge * e4 = new Edge(myVertexList[3],myVertexList[4],4);
    Edge * e5 = new Edge(myVertexList[3],myVertexList[5],5);
    Edge * e6 = new Edge(myVertexList[4],myVertexList[5],6);
    myEdgeList << e << e1 << e2 << e3 << e4 << e5 << e6;
    QList<int> a,b;
    a << 0 << 1 << 2 << 3;
    b << 4 << 5;
    large_result << a << b;
    qDebug() << LARGE_compute_modularity();
    /*
    qDebug() << std::numeric_limits<qreal>::max();
    QList<int> a,b,c;
    a << 1 << 2 << 3;
    b << 4 << 5 << 6;
    c << 7 << 8 << 9;
    ground_truth_communities << a << b << c;
    QList<QList<int> > res;
    QList<int> e,f,g,h;
    e << 1 << 2;
    f << 3 << 4 << 5 << 6;
    g << 7 << 8;
    h << 9;
    res << e << f << g << h;
    large_result = res;
    LARGE_compute_Pairwise_efficient(count_unique_element());*/
    /*
    //READING ...
    //parsing, file are edges file with \t seperator
    //NOTE: Vertices are not uniquely label in increasing order
    QString filePath = QFileDialog::getOpenFileName(this,
                                                     ("Load Vertex")
                                                     , "C:/Users/Dumex/Desktop"
                                                     , ("Text Files (*.txt *csv)"));
    QFile file(filePath);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    QList<QPair<int,int> > edge;
    QSet<QPair<int,int> > check;
    while (!in.atEnd())
    {
        QStringList str = in.readLine().split('\t');
        if (str.size() == 2 && !str[0].startsWith("#"))
        {
            bool ok;
            int v1 = str[0].toInt(&ok), v2 = str[1].toInt(&ok);
            if (!ok)
            {
                qDebug() << "ERROR WHEN PARSING: Data not int! Terminating";
                return;
            }
            edge.append(qMakePair(v1,v2));
        }
    }
    file.close();

    qDebug() << "HERE";
    qDebug() << "FILE CLOSED \nPARSING NOW";
    QMap<int, Vertex*> v_list;
    qDebug() << edge.size();
    for (int i = 0; i < edge.size(); i++)
    {
        QPair<int,int> p = edge[i];
        int v1 = p.first, v2 = p.second;
        if (!v_list.contains(v1))
        {
            Vertex * v = new Vertex;
            v->setIndex(v1);
            v_list.insert(v1,v);
            myVertexList.append(v);
        }
        if (!v_list.contains(v2))
        {
            Vertex * v = new Vertex;
            v->setIndex(v2);
            v_list.insert(v2,v);
            myVertexList.append(v);
        }
    }

    for (int i = 0; i < edge.size(); i++)
    {
        QPair<int,int> p = edge[i];
        int v1 = p.first, v2 = p.second;
        Vertex * from = v_list.value(v1);
        Vertex * to = v_list.value(v2);
        Edge *edge = new Edge(from, to, i);
        myEdgeList.append(edge);
    }

    graphIsReady = true;
    qDebug() << "DONE HASHING" << myVertexList.size() << myEdgeList.size();
    reindexing();
    reindexing_ground_truth();*/
}

/** REINDEXING THE SNAP GRAPH (THE PROBLEM IS THAT THE INDEX IS NOT CONTINOUS)
 *  SO A VERTEX HAS 2 INDICES: SNAP INDICIES and DUMEX INDICES
 *  ALGORITHM RUNS USING DUMEX INDICES
 *  CONTENT MATCHING USING SNAP INDICIES (FOR GROUND TRUTH)
 * @brief Graph::reindexing
 */
void Graph::reindexing()
{
    qDebug() << "Reindexing Started ...";
    qDebug() << "Writing Edge!";
    QString edgePath = "C:/Users/Dumex/Desktop/SocialNetworksCollection/SNAP_DumexTemplate/edge_file.txt";
    QString originalIndexPath = "C:/Users/Dumex/Desktop/SocialNetworksCollection/SNAP_DumexTemplate/vertex_file.txt";
    QFile file(edgePath);
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream out(&file);
    //edge are Source Target Seperated by tab \t
    //begin writing edge
    for (quint32 i = 0; i < myEdgeList.size(); i++)
    {
        Edge * e = myEdgeList.at(i);
        Vertex * from = e->fromVertex();
        Vertex * to = e->toVertex();
        quint32 dumex_v = myVertexList.indexOf(from);
        quint32 dumex_u = myVertexList.indexOf(to);
        if (dumex_v == dumex_u)
        {
            qDebug() << "Error: Self Loop Edge";
            return;
        }
        out << dumex_v << "\t" << dumex_u << endl;
    }
    file.close();
    qDebug() << "Now Writing Vertex!";
    //write the node
    //Dumex Index - Original Index seperate by tab \t
    QFile vFile(originalIndexPath);
    vFile.open(QFile::WriteOnly | QFile::Text);
    QTextStream out2(&vFile);
    for (quint32 i = 0; i < myVertexList.size(); i++)
    {
        Vertex * v = myVertexList.at(i);
        quint32 origin_index = v->getIndex();
        out2 << i << "\t" << origin_index << endl;
    }
    vFile.close();
    qDebug() << "FINISHED! Check Files and Move to Correct Location";
}

/** REINDEXING THE GROUND TRUTH
 * @brief Graph::reindexing_ground_truth
 */
void Graph::reindexing_ground_truth()
{
    read_large_ground_truth_communities();
    // REINDEXING THE GROUND TRUTH FILE
    QString truthPath = "C:/Users/Dumex/Desktop/SocialNetworksCollection/SNAP_DumexTemplate/truth_file.txt";
    QFile truthfile(truthPath);
    truthfile.open(QFile::WriteOnly | QFile::Text);
    QTextStream out(&truthfile);
    qDebug() << "Reindexing the SNAP Ground Truth";
    //community are seperate by \n
    //memebrs of community are seperated by \t
    QMap<quint32, quint32> map;
    for (quint32 i = 0; i < myVertexList.size(); i++)
        map.insert(myVertexList[i]->getIndex(), i); //snap_id  -> dumex_id
    //precheck ground_truth

    for (quint32 i = 0; i < ground_truth_communities.size(); i++)
    {
        QList<quint32> c = ground_truth_communities[i]; //SNAP index
        for (quint32 j = 0; j < c.size(); j++)
        {
            //find the appropriate dumex index
            quint32 snap_id = c[j];
            if (map.contains(snap_id))
            {
                out << map.value(snap_id) << "\t";
            }
            else
            {
                qDebug() << "KEY NOT FOUND; TERMINATING!";
                return;
            }
        }
        out << endl;
    }
    truthfile.close();
    qDebug() << "DONE!";
}


void Graph::read_large_ground_truth_communities()
{
    qDebug() << "PARSING GROUND TRUTH COMMUNITIES";
    ground_truth_communities.clear();
    QString filePath;
    QFile file(filePath);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    quint32 max = 0;
    while (!in.atEnd())
    {
        QStringList str = in.readLine().split('\n'); //a community
        QStringList sub_str = str[0].split('\t'); //community split by '\t'
        QList<quint32> community;
        for (quint32 i = 0; i < sub_str.size(); i++)
        {
            bool ok;
            quint32 indx = sub_str[i].toUInt(&ok); //get vertex index
            if (ok)
            {
                community.append(indx);
                if (indx > max)
                    max = indx;
            }
        }
        ground_truth_communities.append(community);
    }
    //readjust if index does not start from 0
    if (max > myVertexList.size())
        qDebug() << "Did you load the SNAP or DUMEX file?";
    //check sum
    quint32 n = 0;
    for (int i = 0; i < ground_truth_communities.size(); i++)
        n+=ground_truth_communities[i].size();

    if (n == myVertexList.size())
    {
        qDebug() << "OK!";
    }
    qDebug() << "FINISHED! Number of Comm: " << ground_truth_communities.size();
    file.close();
}



/** Require Clarification
 * We Process The OverLap vertices
 * for now, for overlap vertices, retain each vertex in the largest community
 * @brief Graph::large_process_overlap
 */
void Graph::large_process_overlap()
{
    QMap<quint32,quint32> map;
    for (int i = 0; i < ground_truth_communities.size(); i++)
    {
        QList<quint32> c = ground_truth_communities[i];
        for(int j = 0; j < c.size(); j++)
        {
            quint32 id = c[j];
            if (!map.contains(id))
                map.insert(id,i);
            else
                map.insertMulti(id, i);
        }
    }
    QList<quint32> keys = map.keys();
    for (int k = 0; k < keys.size(); k++)
    {
        quint32 id = keys[k];
        QList<quint32> comms = map.values(id);
        if (comms.size() > 1) //belong to more than 1 community
        {
            quint32 largest_comm_size = 0, chosen_comm = 0;
            for (int i = 0; i < comms.size(); i++)
            {
                quint32 cid = comms[i];
                quint32 c_size = ground_truth_communities[cid].size();
                if (c_size > largest_comm_size)
                {
                    largest_comm_size = c_size;
                    chosen_comm = i;
                }
            }

            for(int i = 0; i < comms.size(); i++)
            {
                if (i != chosen_comm)
                {
                    ground_truth_communities[comms[i]].removeOne(id);
                }
            }
        }
    }
    //final check
    quint32 n = 0;
    for (int i = 0; i < ground_truth_communities.size(); i++)
        n += ground_truth_communities[i].size();
    qDebug() << n;
}

/** Preprocess Ground-Truth Communities
 * Let X be the communities: X = {X1,X2,..}
 * for every pair of communities
 * if X1 \cap X2 >= 1/2 of X1 then merge
 * @brief Graph::large_process_overlap_by_merge_intersection
 */
void Graph::large_process_overlap_by_merge_intersection()
{

}


/** RELOAD SNAP FILE IN DUMEX FORMAT
 * @brief Graph::read_DUMEX_input
 */

void Graph::read_DUMEX_input(QString dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
    {
        qDebug() << "DIR NOT EXISTS! Terminating ...";
        return;
    }
    globalDirPath = dirPath;
    QStringList filters;
    filters << "*.txt";
    QFileInfoList file = dir.entryInfoList(filters);
    QString v_file, e_file, t_file;
    for (int i = 0; i < file.size(); i++)
    {
        QFileInfo f = file.at(i);
        QString name = f.fileName();
        if (name.contains("edge"))
            e_file = f.absoluteFilePath();
        else if (name.contains("vertex"))
            v_file = f.absoluteFilePath();
        else if (name.contains("truth"))
            t_file = f.absoluteFilePath();
        else
            qDebug() << "ERROR READING FILES: File not Found!";
    }

    //reload original vertices
    //Parsing
    QFile efile(e_file), vfile(v_file), tfile(t_file);
    if (!efile.exists() || !vfile.exists() || !tfile.exists())
    {
        qDebug() << "FILE NOT FOUND! Recheck! Terminating ...";
        return;
    }
    //else
    vfile.open(QFile::ReadOnly | QFile::Text);
    QTextStream vin(&vfile);
    QStringList str = vin.readLine().split('\t');
    bool load;
    global_v = str[0].toUInt(&load);
    global_e = str[1].toUInt(&load);
    if (!load)
    {
        qDebug() << "ERROR LOADING V FILE";
        return;
    }
    vfile.close();
    //READ E FILE
    efile.open(QFile::ReadOnly | QFile::Text);
    QTextStream ein(&efile);
    QList<QPair<quint32,quint32> > edge;
    while (!ein.atEnd())
    {
        QStringList str = ein.readLine().split('\t');
        bool ok;
        quint32 v1 = str[0].toUInt(&ok), v2 = str[1].toUInt(&ok);
        if (ok)
        {
            edge.append(qMakePair(v1,v2));
        }
    }
    efile.close();

    qDebug() << "FINISHED LOADING DUMEX_TEMPLATE GRAPH!";
    qDebug() << "V:" << global_v << "; E:" << global_e;
    /*
    //reload original vertices
    vfile.open(QFile::ReadOnly | QFile::Text);
    QTextStream vin(&vfile);
    QList<int> origin_v;
    for (int i = 0; i < boost::num_vertices(g); i++)
        origin_v.append(-1);

    while (!vin.atEnd())
    {
        QStringList str = vin.readLine().split('\t');
        if (str.size() == 2 && !str.startsWith("#"))
        {
            int dumex_index = str[0].toInt(),
                SNAP_index = str[1].toInt();
            origin_v.replace(dumex_index, SNAP_index);
        }
    }
    vfile.close();
    //checking
    for (int i = 0; i < origin_v.size(); i++)
    {
        if (origin_v[i] == -1)
        {
            qDebug() << "ERROR: INDEX MISSING";
            return;
        }
    }
    qDebug() << "FINISHED RELOAD SNAP INDICES!";
    */
    //create Vertex and Edge object DECAPREATED
    for (quint32 i = 0; i < global_v; i++)
    {
        Vertex * v = new Vertex;
        v->setIndex(i);
        myVertexList.append(v);
    }

    for (quint32 i = 0; i < global_e; i++)
    {
        QPair<quint32,quint32> p = edge[i];
        quint32 from = p.first, to = p.second;
        Vertex * vfrom = myVertexList.at(from);
        Vertex * vto = myVertexList.at(to);
        Edge * e = new Edge(vfrom,vto,i);
        myEdgeList.append(e);
    }
    //check sum
    bool fit = false;
    if (myVertexList.size() == global_v && myEdgeList.size() == global_e)
        fit = true;
    qDebug() << "Check Sum" << fit;
    if(fit)
    {
        graphIsReady = true;
        qDebug() << "PREQUISITE: OK! READING TRUTH FILES";
        ground_truth_communities.clear();
        QFile tfile(t_file);
        tfile.open(QFile::ReadOnly | QFile::Text);
        QTextStream tin(&tfile);
        quint32 max = 0;
        while (!tin.atEnd())
        {
            QStringList str = tin.readLine().split('\n'); //a community
            QStringList sub_str = str[0].split('\t'); //community split by '\t'
            QList<quint32> community;
            for (int i = 0; i < sub_str.size(); i++)
            {
                bool ok;
                quint32 indx = sub_str[i].toUInt(&ok); //get vertex index
                if (ok)
                {
                    community.append(indx);
                    if (indx > max)
                        max = indx;
                }
            }
            ground_truth_communities.append(community);
        }
        //readjust if index does not start from 0
        if (max > myVertexList.size())
            qDebug() << "Did you load the SNAP or DUMEX file?";
        //check sum
        quint32 n = 0;
        for (int i = 0; i < ground_truth_communities.size(); i++)
            n+=ground_truth_communities[i].size();

        if (n == myVertexList.size())
        {
            qDebug() << "OK!";
        }
        qDebug() << "FINISHED! Number of Comm: " << ground_truth_communities.size();
        tfile.close();
        //check,
        QSet<quint32> clustered;

        for (int i = 0; i < ground_truth_communities.size(); i++)
        {
            QList<quint32> c = ground_truth_communities[i];
            for (int j = 0; j < c.size(); j++)
            {
                if (!clustered.contains(c[j]))
                    clustered.insert(c[j]);
                if (c[j] > myVertexList.size())
                    qDebug() << "ERROR: index > size";
            }
        }
        for (quint32 i = 0; i < myVertexList.size(); i++)
        {
            if (!clustered.contains(i))
                large_excluded.insert(i);
        }
        //reset index
        for (quint32 i = 0; i < myVertexList.size(); i++)
        {
            myVertexList[i]->setIndex(i);
        }
        qDebug() << "Number of Vertex Excluded From SNAP Community:" << large_excluded.size();
        qDebug() << "Removing Overlap (By Assigning each vertex to the largest)";
        large_process_overlap();
        graphIsReady = true;
    }
    else
    {
        qDebug() << "Check Sum FAILS!";
    }
}

/** PARSE AGGREGATING RESULT FROM LARGE GRAPH
 * @brief Graph::large_graph_parse_result
 */
void Graph::large_graph_parse_result()
{
    graphIsReady = false;
    qDebug() << "PARSING RESULT";
    if (centroids.empty())
    {
        qDebug() << "WINNER SET IS EMPTY";
        return;
    }
    QList<QList<quint32> > C;
    //qDebug() << "C: " << centroids.size();
    for (int i = 0; i < centroids.size(); i++)
    {
        QList<quint32> c;
        Vertex * v = centroids.at(i);
        quint32 v_i = v->getIndex();
        if (!large_excluded.contains(v_i))
            c.append(v_i);
        QList<Vertex*> absorbed = v->getAbsorbedList();
        for (int j = 0; j < absorbed.size(); j++)
        {
            quint32 u_i = absorbed[j]->getIndex();
            if (!large_excluded.contains(u_i))
                c.append(u_i); //get the SNAP index
        }
        //exclude vertices that is not included in SNAP
        if (c.size() > 0)
            C.append(c);
    }
    /** Prepare data for indicies matching
      */
    large_result = C;
    C.clear();
    print_result_stats();
    qDebug() << "- Number of Clusters: " << large_result.size();
    if (ground_truth_communities.empty()) //for non ground truth parsing
    {
        qDebug() << "GROUND TRUTH COMMUNITIES HAS NOT BEEN LOADED OR GRAPH HAS NOT BEEN CLUSTERED";
        qDebug() << "Only Modularity Can Be Calculated:";
        LARGE_reload_edges();
        qDebug() << "Q: " << LARGE_compute_modularity();
    }
    else
    {   //count number of unique elements in RESULT and in Ground_truth
        quint32 uniq = count_unique_element();
        if (uniq > 0)
            LARGE_compute_cluster_matching(uniq);
        else
            qDebug() << "ERROR: NUMBER OF UNIQUE ELEMTNS IN RESULT DIFF IN GROUND TRUTH (AFTER CHECKING EXCLUDED)";
    }
}

/** Parse result of retain
 * @brief Graph::large_parse_retain_result
 */
void Graph::large_parse_retain_result()
{
    graphIsReady = false;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> NormalGraph;
    NormalGraph g;
    for(int i = 0; i < myVertexList.size(); i++)
        boost::add_vertex(g);

    for (int i = 0; i < hierarchy.size(); i++)
    {
        QPair<quint32,quint32> p = hierarchy.at(i);
        boost::add_edge(p.first, p.second, g);
    }

    std::vector<quint32> component(boost::num_vertices(g));
    quint32 num = boost::connected_components(g, &component[0]);
    qDebug() << "Number of Clusters:" << num;
    //compute indices
    QList<QList<quint32> > clusters;
    for (int i = 0 ; i < num; i++)
    {
        QList<quint32> c;
        clusters.append(c);
    }

    for (int i = 0; i < component.size(); i++)
    {
        if (!large_excluded.contains(i))
        {
            QList<quint32> c = clusters[component[i]];
            c.append(i);
            clusters.replace(component[i],c);
        }
    }
    large_result = clusters;
    clusters.clear();
    print_result_stats();
    if (ground_truth_communities.empty()) //for non ground truth parsing
    {
        qDebug() << "GROUND TRUTH COMMUNITIES HAS NOT BEEN LOADED OR GRAPH HAS NOT BEEN CLUSTERED";
        qDebug() << "Only Modularity Can Be Calculated:";
        qDebug() << "Q: " << LARGE_compute_modularity();
    }
    else
    {   //count number of unique elements in RESULT and in Ground_truth
        quint32 uniq = count_unique_element();
        if (uniq > 0)
            LARGE_compute_cluster_matching(uniq);
        else
            qDebug() << "ERROR: NUMBER OF UNIQUE ELEMTNS IN RESULT DIFF IN GROUND TRUTH (AFTER CHECKING EXCLUDED)";
    }
}

void Graph::print_result_stats()
{
    qDebug() << "- Writing Log ...";
    QString logPath = globalDirPath + "/log.txt";
    QFile outFile(logPath);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream out(&outFile);
    //print size of clusters
    quint32 small = 0, isolated = 0;
    out << "****************** 1 RUN ***********************" << endl;
    out << "Cluster#\tSize" << endl;
    for (int i = 0; i < large_result.size(); i++)
    {
        quint32 size = large_result[i].size();
        if (size == 1)
            isolated++;
        else if (size > 1 && size <= 3)
            small++;
        else
            out << i <<'\t' << large_result[i].size() << endl;
    }
    out << "********************************" << endl;
    out << "Summary: Total C:" << large_result.size() << endl
        << "Small Size C ( < 3): " << small << "; Isolated (== 1): " << isolated;
    outFile.close();
    qDebug() << " - DONE!!!";
}

/** Compare NUmber of Unique Element
 * @brief Graph::count_unique_element
 * @return True if match
 */
quint32 Graph::count_unique_element()
{
    QSet<quint32> res, truth;
    quint32 sum = 0;
    for (int i = 0 ; i < large_result.size(); i++)
    {
        for (int j = 0; j < large_result[i].size(); j++)
        {
            quint32 v = large_result[i][j];
            sum++;
            if (!res.contains(v))
                res.insert(v);
        }
    }
    for (int i = 0 ; i < ground_truth_communities.size(); i++)
    {
        for (int j = 0; j < ground_truth_communities[i].size(); j++)
        {
            quint32 v = ground_truth_communities[i][j];
            if (!truth.contains(v))
                truth.insert(v);
        }
    }

    if (res.size() != truth.size())
    {
        qDebug() << "Number of Element in Result:" << sum;
        qDebug() << "Number of Unique Elements in RESULT: " << res.size();
        qDebug() << "Number of Unique Elements in TRUTH: " << truth.size();
        return 0;
    }
    else
        return res.size();
}




/**
 * @brief Graph::LARGE_compute_cluster_matching
 */
void Graph::LARGE_compute_cluster_matching(quint32 n)
{
    //check sum not available since overlap
    //checking ground truth
    if (ground_truth_communities.empty())
    {
        qDebug() << "GROUND TRUTH COMMUNITIES HAS NOT BEEN LOADED OR GRAPH HAS NOT BEEN CLUSTERED";
        return;
    }

    double RAND = LARGE_compute_Pairwise_efficient(n);
           // JAcc = LARGE_compute_Jaccard_index(),
           // Newman = LARGE_compute_Newman_fraction_of_classified();
    //calculate modularity
    LARGE_reload_edges();
    double Q = LARGE_compute_modularity();
    qDebug() << "Q:" << Q;
    return;
}


/** Compare result using RAND index
 * ground truth is X = {x1, x2 ..., xr }
 * result is Y = {y1, y2, .., ys}
 * with r != s
 * then, count:
    a, the number of pairs of elements in S that are in the same set in X and in the same set in Y
    b, the number of pairs of elements in S that are in different sets in X and in different sets in Y
    c, the number of pairs of elements in S that are in the same set in X and in different sets in Y
    d, the number of pairs of elements in S that are in different sets in X and in the same set in Y
 *  R = {a+b}/{a+b+c+d} = {a+b}/{n \choose 2 }
 * a,b,c,d is computed using the contigency table
 * Let the ground truth X = {X1, X2 ..., XR}
 * Let the result be Y = {Y1, Y2 ... YC}
 * the table is then
 * /    X1 X2 ... Xr
 * Y1   n11 n22...n1r
 * Y2   n12 n22 ... n2r
 * .    .
 * .    .
 * YC   X1C X2C ....
 * in which n11 = X1 \intersect Y1
 * a, b, c, d is then have formula as in Hubert paper
 * NAIVE IMPLEMENTATION: PREFERABLY FOR SMALL COMMUNITIES SET
 * @brief Graph::compare_using_RAND_index
 * @return
 */
/** Optimise SPACE
 * @brief Graph::LARGE_compute_Pairwise_efficient
 * @param result
 * @return
 */
double Graph::LARGE_compute_Pairwise_efficient(quint32 n)
{
    qDebug() << "STARTING Pairwise Indices ...";
    qDebug() << "Clusters:" << large_result.size();
    quint64 nij = 0, nij_minus = 0, nij_square = 0, nij_choose_2 = 0;
    int row = ground_truth_communities.size(), column = large_result.size();
  //  qDebug() << "R:" << row << "; C:" << column;
    QList<quint64> ni, nj; //ni: sum row, nj: sum column
    for(int j = 0; j < column; j++)
        nj.append(0);

    for (int i = 0; i < row; i++)
    {
        quint64 sum_row = 0;
        QSet<quint32> X = ground_truth_communities[i].toSet();
        for (int j = 0; j < column; j++)
        {
            QSet<quint32> Y = large_result[j].toSet();
            QSet<quint32> copy = X;
            quint32 entry = copy.intersect(Y).size();
            nij_minus += (quint64)entry*(entry-1); // nij(nij-1)
            nij_square += (quint64)entry*entry; // nij^2
            nij_choose_2 += (quint64) entry*(entry-1)/2; //(nij choose 2) for adjust rand
            nij += (quint64)entry;
            sum_row += (quint64)entry;
            quint64 sum_col = nj[j];
            sum_col += entry;
            nj.replace(j, sum_col);
        }
        ni.append(sum_row);
    }

    quint64 n_square = 0,
            n_choose_2 = n*(n-1)/2,
            ni_sum = 0, //sum row
            nj_sum =0, // sum column
            ni_choose_2 = 0, //bionomial row
            nj_choose_2 = 0, //bionomial column
            ni_square = 0,  // sum each row square
            nj_square = 0; // sum each column square

    for (int i = 0; i < ni.size(); i++)
    {
        int entry = ni[i];
        quint64 entry_square = entry*entry;
        quint64 entry_choose_2 = entry*(entry-1)/2;
        ni_square += entry_square;
        ni_choose_2 += entry_choose_2;
        ni_sum+=  entry;
    }
    ni.clear();
    for (int i = 0; i < nj.size(); i++)
    {
        int entry = nj[i];
        quint64 entry_square = entry*entry;
        quint64 entry_choose_2 = entry*(entry-1)/2;
        nj_square += entry_square;
        nj_choose_2 += entry_choose_2;
        nj_sum+=  entry;
    }
    nj.clear();

    qDebug() << "Sheck sum Pairwise Indicies:" << ni_sum << nj_sum <<(ni_sum == nj_sum);
//    qDebug() << "ELEMENT:";
    n_square = qPow(n,2);
//    qDebug() << "n:" << n << "n^2" << n_square <<"; nij^2:" << nij_square <<"; ni_square" << ni_square << "; nj_square:" << nj_square;

    QList<quint64> param_a,param_b,param_c,param_d;
    param_a << nij_minus;
    param_b << ni_square << nij_square;
    param_c << nj_square << nij_square;
    param_d << n_square << nij_square << ni_square << nj_square;

    quint64 a = calA(param_a);
    quint64 d = calD(param_d);
    quint64 c = calC(param_c); // type iii: same diff
    quint64 b = calB(param_b); //type iv: diff same

    double RAND = (double) (a+d)/(a+b+c+d);
    double Jaccard = (double) a/(a+b+c);
    //
    QList<quint64> param_ARI;
    param_ARI << ni_choose_2 << nj_choose_2 << n_choose_2 << nij_choose_2;
    double ARI = calAdRand(param_ARI);

    qDebug() << "RAND:" << RAND
             << "Jaccard: " << Jaccard
             << "Adjusted Rand Index: " << ARI;
    return 0;
}

quint64 Graph::calA(QList<quint64> param)
{
    quint64 a = param[0];
    a = a/2;
    qDebug() << "a:" << a;
    return a;
}

quint64 Graph::calB(QList<quint64> param)
{
    quint64 ai = param[0], nij_square = param[1];
    quint64 b = (ai-nij_square)/2;
    qDebug() << "b:" << b;
    return b;
}

quint64 Graph::calC(QList<quint64> param)
{
    quint64 ni_square = param[0], nij_square = param[1];
    quint64 c = (ni_square - nij_square)/2;
    qDebug() << "c:" << c;
    return c;
}

quint64 Graph::calD(QList<quint64> param)
{
    quint64 n_square = param[0],
            nij_square = param[1],
            ni_square = param[2],
            nj_square = param[3];
    quint64 d = n_square + nij_square - ni_square - nj_square; //type ii: diff diff
    d /= 2;
    qDebug() << "d:" << d;
    return d;
}

double Graph::calAdRand(QList<quint64> param)
{
    quint64 ni_choose_2 = param[0],
            nj_choose_2 = param[1],
            n_choose_2 = param[2],
            nij_choose_2 = param[3];
    double nc = (double)ni_choose_2*nj_choose_2/n_choose_2;
    double nom = (double)(nij_choose_2 - nc);
    double sum = (double) (ni_choose_2 + nj_choose_2)/2;
    double denom = sum - nc;
    double ARI = nom/denom;
    return ARI;
}

/** Calculate the Clustering Coeffficient, which is the average over all v
 * Watts Algorithm
 * @brief Graph::cal_average_clustering_coefficient
 * @return
 */
double Graph::cal_average_clustering_coefficient()
{
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> NormalGraph;
    NormalGraph g;
    for (int i = 0; i < myVertexList.size(); i++)
    {
        boost::add_vertex(g);
    }
    for(int i = 0;i  < myEdgeList.size(); i++)
    {
        Edge * e = myEdgeList.at(i);
        int from = e->fromVertex()->getIndex(), to = e->toVertex()->getIndex();
        boost::add_edge(from,to,g);
    }
    // Compute the clustering coefficients of each vertex in the graph
    // and the mean clustering coefficient which is returned from the
    // computation.
    typedef boost::exterior_vertex_property<NormalGraph, float> ClusteringProperty;
    typedef ClusteringProperty::container_type ClusteringContainer;
    typedef ClusteringProperty::map_type ClusteringMap;

    ClusteringContainer coefs(boost::num_vertices(g));
    ClusteringMap cm(coefs, g);
    double cc = boost::all_clustering_coefficients(g, cm);
    return cc;
}


/** Reset to prepare for next run
 * @brief Graph::LARGE_reset
 */
void Graph::LARGE_reset()
{
    qDebug() << "Reseting ... ";
    for (int i = 0; i < myVertexList.size(); i++)
    {
        myVertexList[i]->resetClusterRelevant();
    }
    myEdgeList.clear();
    hierarchy.clear();
    centroids.clear();
    large_result.clear();
    graphIsReady = false;
}

/** Reload
 * @brief Graph::LARGE_reload
 */
bool Graph::LARGE_reload()
{
    qDebug() << "RELOADING";
    LARGE_reset();
    if (globalDirPath.size() == 0)
    {
        qDebug() << "GLOBAL DIR PATH HAS NOT BEEN SET!";
        return false;
    }
    //read edge only
    myEdgeList.clear();
    if (no_run == 0 )
        LARGE_reload_edges();
    else
    {
        qDebug() << "- ** Reaggregation Detected! Following are results for super graph...";
        LARGE_reload_superEdges();
    }

    graphIsReady = true;
    return graphIsReady;
}

/** RELOAD EDGES ONLY
 * This is good enough
 * @brief Graph::LARGE_reload_edges
 */
void Graph::LARGE_reload_edges()
{
    qDebug() << "- Reloading Edges ...";
    QDir dir(globalDirPath);
    QStringList filters;
    filters << "*.txt";
    QFileInfoList file = dir.entryInfoList(filters);
    QString e_file;
    for (int i = 0; i < file.size(); i++)
    {
        QFileInfo f = file.at(i);
        QString name = f.fileName();
        if (name.contains("edge"))
            e_file = f.absoluteFilePath();
    }

    //Parsing
    QFile efile(e_file);
    if (!efile.exists())
    {
        qDebug() << "FILE NOT FOUND! Recheck! Terminating ...";
        return;
    }
    //else
    efile.open(QFile::ReadOnly | QFile::Text);
    QTextStream ein(&efile);
    QList<QPair<quint32,quint32> > edge;
    ein.readLine(); //skip first line
    while (!ein.atEnd())
    {
        QStringList str = ein.readLine().split('\t');
        bool ok;
        quint32 v1 = str[0].toUInt(&ok), v2 = str[1].toUInt(&ok);
        if (ok)
        {
            edge.append(qMakePair(v1,v2));
        }
    }
    efile.close();
    //reload original vertices
    //create Vertex and Edge object
    qDebug() << "- Now Loading Edges ...";
    for (int i = 0; i < edge.size(); i++)
    {
        QPair<quint32,quint32> p = edge[i];
        quint32 from = p.first, to = p.second;
        Vertex * vfrom = myVertexList.at(from);
        Vertex * vto = myVertexList.at(to);
        Edge * e = new Edge(vfrom,vto,i);
        myEdgeList.append(e);
    }
    edge.clear();
}

void Graph::LARGE_reload_superEdges()
{
    qDebug() << "- Reloading Edges ...";
    QDir dir(globalDirPath);
    QStringList filters;
    filters << "*.txt";
    QFileInfoList file = dir.entryInfoList(filters);
    QString e_file;
    for (int i = 0; i < file.size(); i++)
    {
        QFileInfo f = file.at(i);
        QString name = f.fileName();
        if (name.contains("superGraph"))
            e_file = f.absoluteFilePath();
    }

    //Parsing
    QFile efile(e_file);
    if (!efile.exists())
    {
        qDebug() << "FILE NOT FOUND! Recheck! Terminating ...";
        return;
    }
    //else
    efile.open(QFile::ReadOnly | QFile::Text);
    QTextStream ein(&efile);
    QList<QPair<quint32,quint32> > edge;
    ein.readLine(); //skip first line
    while (!ein.atEnd())
    {
        QStringList str = ein.readLine().split('\t');
        bool ok;
        quint32 v1 = str[0].toUInt(&ok), v2 = str[1].toUInt(&ok);
        if (ok)
        {
            edge.append(qMakePair(v1,v2));
        }
    }
    efile.close();
    //reload original vertices
    //create Vertex and Edge object
    qDebug() << "- Now Loading Edges ...";
    for (int i = 0; i < edge.size(); i++)
    {
        QPair<quint32,quint32> p = edge[i];
        quint32 from = p.first, to = p.second;
        Vertex * vfrom = myVertexList.at(from);
        Vertex * vto = myVertexList.at(to);
        Edge * e = new Edge(vfrom,vto,i);
        myEdgeList.append(e);
    }
    edge.clear();
}



/** Rerun
 * @brief Graph::LARGE_rerun
 */
void Graph::LARGE_rerun()
{
    int per_agg = 5;
    for (int i = 50; i < 17*per_agg; i++)
    {
        int agg = i/per_agg;
        qDebug() << "********* NEW RUN START BELOWS ******** ";
        if (agg == 0){}
        else if (agg == 1)
        {
            qDebug() << "Type I.a:";
            random_aggregate();
        }
        else if (agg == 2)
        {
            qDebug() << "Type I.b:";
            random_aggregate_with_degree_comparison();
        }
        else if (agg == 3)
        {
            qDebug() << "Type I.c:";
            random_aggregate_with_weight_comparison();
        }
        else if (agg == 4)
        {
             qDebug() << "Type II.a:";
            random_aggregate_with_neighbour_initial_degree_bias();
        }
        else if (agg == 5)
        {
            qDebug() << "Type II.b:";
            random_aggregate_with_neighbour_CURRENT_degree_bias();
        }
        else if (agg == 6)
        {
            qDebug() << "Type II.c:";
            random_aggregate_highest_CURRENT_degree_neighbour();
        }
        else if (agg == 7)
        {
            qDebug() << "Type II.d:";
            random_aggregate_with_minimum_weight_neighbour();
        }
        else if (agg == 8)
        {
            qDebug() << "Type II.e:";
            random_aggregate_probabilistic_lowest_degree_neighbour_destructive();
        }
        else if (agg == 9)
        {
            qDebug() << "Type II.f:";
            random_aggregate_probabilistic_candidate_with_minimum_weight_neighbour();
        }
        else if (agg == 10)
        {
            qDebug() << "Type II.g:";
            random_aggregate_greedy_max_degree();
        }
        else if (agg == 11)
        {
            qDebug() << "Type II.h:";
            random_aggregate_greedy_max_weight();
        }
        else if (agg == 12)
        {
            qDebug() << "Type III.a:";
            random_aggregate_retain_vertex_using_triangulation();
        }
        else if (agg == 13)
        {
            qDebug() << "Type III.b:";
            random_aggregate_retain_vertex_using_probabilistic_triangulation();
        }
        else if (agg == 14)
        {
            qDebug() << "Type III.c:";
            random_aggregate_with_highest_triangulated_vertex();
        }
        else if (agg == 15)
        {
            qDebug() << "Type III.d:";
            random_aggregate_retain_vertex_using_triangulation_times_weight();
        }
        else if (agg == 16)
        {
            qDebug() << "Type III.e:";
            random_aggregate_retain_vertex_using_triangulation_of_cluster();
        }
    }
}

/** Select Type of Aggregation to Run
 * @brief Graph::run_aggregation_on_selection
 * @param n
 */
void Graph::run_aggregation_on_selection(int n)
{
    switch (n) {
    case 0:
        qDebug() << "Type I.a:";
        random_aggregate();
        break;
    case 1:
        qDebug() << "Type I.b:";
        random_aggregate_with_degree_comparison();
        break;
    case 2:
        qDebug() << "Type I.c:";
        random_aggregate_with_weight_comparison();
        break;
    case 3:
         qDebug() << "Type II.a:";
        random_aggregate_with_neighbour_initial_degree_bias();
        break;
    case 4:
        qDebug() << "Type II.b:";
        random_aggregate_with_neighbour_CURRENT_degree_bias();
        break;
    case 5:
        qDebug() << "Type II.c:";
        random_aggregate_highest_CURRENT_degree_neighbour();
        break;
    case 6:
        qDebug() << "Type II.d:";
        random_aggregate_with_minimum_weight_neighbour();
        break;
    case 7:
        qDebug() << "Type II.e:";
        random_aggregate_probabilistic_lowest_degree_neighbour_destructive();
        break;
    case 8:
        qDebug() << "Type II.f:";
        random_aggregate_probabilistic_candidate_with_minimum_weight_neighbour();
        break;
    case 9:
        qDebug() << "Type II.g:";
        random_aggregate_greedy_max_degree();
        break;
    case 10:
        qDebug() << "Type II.h:";
        random_aggregate_greedy_max_weight();
        break;
    case 11:
        qDebug() << "Type III.a:";
        random_aggregate_retain_vertex_using_triangulation();
        break;
    case 12:
        qDebug() << "Type III.b:";
        random_aggregate_retain_vertex_using_probabilistic_triangulation();
        break;
    case 13:
        qDebug() << "Type III.c:";
        random_aggregate_with_highest_triangulated_vertex();
        break;
    case 14:
        qDebug() << "Type III.d:";
        random_aggregate_retain_vertex_using_triangulation_times_weight();
        break;
    case 15:
        qDebug() << "Type III.e:";
        random_aggregate_retain_vertex_using_triangulation_of_cluster();
        break;
    default:
        qDebug() << "Select A Number Between 0 - 15";
      break;
    }
}


/** Calculate Modularity
 * @brief Graph::LARGE_compute_modularity
 * @return
 */
double Graph::LARGE_compute_modularity()
{
    //firstly reload the edges
    if (global_e == 0)
    {
        qDebug() << "Graph Has Not Been Initialised Properly: E = 0 ! Trying to Probe Again;";
        global_e = myEdgeList.size();
    }
    //go through result
    double Q = 0.0;
    for (int i = 0; i < large_result.size(); i++)
    {
        QList<quint32> c = large_result[i];
        QSet<quint32> vi = c.toSet();
        quint64 intra = 0, inter = 0;
        for (int j = 0; j < c.size(); j++)
        {
            quint32 id = c[j];
            Vertex * v = myVertexList.at(id);
            QList<Edge*> adj = v->getAllEdge();
            for (int k = 0; k < adj.size(); k++)
            {
                Vertex * other = v->get_neighbour_fromEdge(adj[k]);
                quint32 other_id = other->getIndex();
                if (vi.contains(other_id))
                    intra++;
                else
                    inter++;
            }
        }

        quint64 m = 2*global_e;
        double e = (double)intra/m;
        double a = (double)(intra + inter)/m;
        double Qi = e - qPow(a,2);
        Q += Qi;
    }
    return Q;

}

// --------------------------- POST AGGREGATION -----------------------------------------
// --------------------------------------------------------------------------------------
/** For each cluster in the results from previous aggregation,
 * Each cluster is now collapsed into a super vertex, which is then used for further aggregation
 * @brief Graph::PostAgg_generate_super_vertex
 */
void Graph::PostAgg_generate_super_vertex()
{
    if(large_result.empty())
    {
        qDebug() << "- Aggregation Result is Empty! Terminating ...";
        return;
    }
    if (myEdgeList.empty()) //reload edge if this is not retain type
    {
        LARGE_reload_edges();
    }
    //first create super vertices
    QList<Vertex*> superV;
    QMap<int,Vertex*> superMap; //map from old vertex to new vertex
    for(int i = 0; i < large_result.size(); i++)
    {
        Vertex * v = new Vertex;
        v->setIndex(superV.size());
        superV.append(v);
        QList<quint32> c = large_result[i];
        for (int j = 0; j < c.size(); j++)
        {
            quint32 id = c[j];
            superMap.insert(id,v);
        }
    }

    QList<Edge*> superE;
    QList<QPair<quint32,quint32> > newE;
    //conencting super vertices
    for(int i = 0; i < myEdgeList.size(); i++)
    {
        quint32 from = myEdgeList[i]->fromVertex()->getIndex(), to = myEdgeList[i]->toVertex()->getIndex();
        if (!superMap.contains(from) || !superMap.contains(to))
        {
            qDebug() << "- Post Aggregation Error! Vertices Has Not Been Assigned To A Super Vertex";
            qDebug() << "- Terminating ...";
            return;
        }
        Vertex * superFrom = superMap.value(from);
        Vertex * superTo = superMap.value(to);
        if (superFrom->getIndex() == superTo->getIndex()){
        }
        else
        {
            QPair<quint32,quint32> s = qMakePair(superFrom->getIndex(), superTo->getIndex());
            QPair<quint32,quint32> r_s = qMakePair(superTo->getIndex(), superFrom->getIndex());
            if (!newE.contains(s) && !newE.contains(r_s))
            {
                newE.append(s);
                newE.append(r_s);
                Edge * e = new Edge(superFrom,superTo,superE.size());
                superE.append(e);
            }
        }
    }

    //clearing the old list
    for (int i = 0; i < myVertexList.size(); i++)
        delete myVertexList[i];
    myVertexList = superV;
    myEdgeList = superE;
    qDebug() << "- Post Aggregation Finished! After collapsing: SuperV: " << myVertexList.size()
             << "SuperE: " << myEdgeList.size();
    global_e = myEdgeList.size();
    global_v = myVertexList.size();
    no_run++;
    qDebug() << "After Clustering Coefficient:" << cal_average_clustering_coefficient();
    qDebug() << "Saving to the dir";
    save_current_run_as_edge_file(QString(globalDirPath + "superGraph" + QString::number(no_run) + ".txt"));
}

/** Save the current run to stich back later
 * @brief Graph::save_current_clusters
 */
void Graph::save_current_clusters()
{

}

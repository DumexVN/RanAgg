#ifndef GRAPH_H
#define GRAPH_H

#include <QtCore>
#include <QTimer>

#include "vertex.h"
#include "edge.h"


class Graph
{
public:
    Graph();

    void read_GML_file(QString filePath);
    void save_edge_file_from_GML();
    void save_current_run_as_edge_file(QString fileName);
    void read_DUMEX_input(QString dirPath);
    void read_simple_edge(QString dirPath);
    void load_ground_truth_communities();
    void read_large_graph_with_ground_truth_communities();
    //investigate bridges
    void get_bridge_stats();
    void LARGE_rerun();
    //run
    void run_aggregation_on_selection(int n);
    //stats
    double cal_average_clustering_coefficient();
    //aggregation
    void random_aggregate();
    void random_aggregate_with_degree_comparison();
    void random_aggregate_with_weight_comparison();
    void random_aggregate_with_neighbour_initial_degree_bias();
    void random_aggregate_with_neighbour_CURRENT_degree_bias();
    void random_aggregate_highest_CURRENT_degree_neighbour();
    void random_aggregate_with_minimum_weight_neighbour();
    void random_aggregate_probabilistic_lowest_degree_neighbour_destructive();
    void random_aggregate_probabilistic_candidate_with_minimum_weight_neighbour();
    void random_aggregate_with_highest_edge_weight_and_weight_comparison();
    void random_aggregate_with_edge_weight_bias_and_weight_comparison();
    void random_aggregate_with_highest_triangulated_vertex();
    void random_aggregate_greedy_max_degree();
    void random_aggregate_greedy_max_weight();
    //agg without 'removing' vertices
    void random_aggregate_retain_vertex_using_triangulation();
    void random_aggregate_retain_vertex_using_probabilistic_triangulation();
    void random_aggregate_retain_vertex_using_triangulation_times_weight();
    void random_aggregate_retain_vertex_using_triangulation_of_cluster();
    //post aggregation
    void PostAgg_generate_super_vertex();
    void PostAgg_adjust_variables();

private:
    void read_ground_truth_communities();
    bool checkGraphCondition();
    void reConnectGraph();
    // for large graph
    void reindexing();
    void reindexing_ground_truth();
    void read_large_ground_truth_communities();
    void large_process_overlap();
    void large_process_overlap_by_merge_intersection();
    void large_graph_parse_result();
    void large_parse_retain_result();
    void print_result_stats();
    void LARGE_compute_cluster_matching(quint32 n);
    double LARGE_compute_Pairwise_efficient(quint32 n);
    double LARGE_compute_Newman_fraction_of_classified();
    double LARGE_compute_modularity();
    void LARGE_reset();
    bool LARGE_reload();
    void LARGE_reload_edges();
    void LARGE_reload_superEdges();
    void save_current_clusters();

    quint32 count_unique_element();
    quint64 calA(QList<quint64> param);
    quint64 calB(QList<quint64> param);
    quint64 calC(QList<quint64> param);
    quint64 calD(QList<quint64> param);
    double calAdRand(QList<quint64> param);

    //
    QString GMLpath;
    //

    QList<Vertex*> myVertexList;
    QList<Edge*> myEdgeList;
    QList<Vertex*> centroids;
    //
    QList<QList<quint32> > ground_truth_communities;
    QList<QPair<quint32,quint32> > hierarchy;
    QList<QList<quint32> > large_result;
    QSet<quint32> large_excluded;
    //
    bool graphIsReady;
};
#endif // GRAPH_H

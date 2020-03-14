#pragma once

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>
#include <assert.h>

using namespace boost;

const int BIG_INTEGER = std::numeric_limits<int>::max();
// const int BIG_INTEGER = 10000;

/// Node properties for the PR algorithm
struct PRPotential {
    /// Excess flow at the node.
    int excess_flow = 0;
    /// Label d for the PR algorithm : height of the node
    int labeling = 0;
    /// Class of the node after min cut : 0 if in class of source, 1 otherwise
    int cut_class = 0;
};

/// Edge property, with both flow value and edge capacity.
struct EdgeProperties {
    int capacity = 0;
    int flow = 0;
};

/// 

int get_residual(EdgeProperties e) {
    return (e.capacity - e.flow);
}

typedef adjacency_list<vecS, vecS, bidirectionalS,
                       PRPotential, EdgeProperties> Graph;

typedef property_map<Graph, PRPotential> vertex_prop_map;
typedef property_map<Graph, EdgeProperties> edge_prop_map;

void push_flow(const Graph::vertex_descriptor& v, const Graph::vertex_descriptor& w, Graph& g, int to_push)
{
    auto edge = boost::edge(v, w, g).first;
    auto rev_edge = boost::edge(w, v, g).first;

    assert (g[edge].flow + to_push <= g[edge].capacity);
    
    g[edge].flow     += to_push;
    // get negative flow with capacity >= 0 so we can push back
    g[rev_edge].flow -= to_push;
    
    g[v].excess_flow -= to_push;
    g[w].excess_flow += to_push;

}

void symmetrize_graph(Graph& g)
{
    auto edges = boost::edges(g);
    for(auto &it = edges.first; it != edges.second; it++)
    {
        auto v(source(*it, g)), w(target(*it, g));
        if (boost::edge(w, v, g).second == false)
        {
            boost::add_edge(w, v, {0, 0}, g);
        }
    }
}

void split_edges(Graph& g)
{
    // 1) Iterate on all edges. If we have two non zero antiparallel edges : 
    // we split one of them into two edges w/ same capacity

    // Rmk : the graph g must be symmetric (cf. symmetrize_graph)

    const int n_verts = boost::num_vertices(g);
    int z = n_verts;

    for (Graph::vertex_descriptor i = 0; i < n_verts; i++) 
    {
        auto neighs = boost::adjacent_vertices(i, g);
        for (auto &it = neighs.first; it != neighs.second; it++)
        {
            // only consider cases where j < i
            if (*it >= i)
                continue;
            auto j = *it;
            auto edge = boost::edge(i, j, g).first;
            auto rev_edge = boost::edge(j, i, g).first;
            if (g[edge].capacity != 0 && g[rev_edge].capacity != 0)
            {
                // split the edge going 'upstream' i.e from j < i to i
                auto capacity = g[rev_edge].capacity;
                boost::add_edge(j, z, {capacity, 0}, g);
                boost::add_edge(z, i, {capacity, 0}, g);
                boost::remove_edge(rev_edge, g);
            }
        }
    }

    // symmetrize the graph once again

    symmetrize_graph(g);
}

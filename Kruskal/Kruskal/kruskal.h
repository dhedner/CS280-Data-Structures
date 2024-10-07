#ifndef KRUSKAL_H
#define KRUSKAL_H
#include "disjoint_sets.h"
#include "graph.h"

template <typename Vertex, typename Edge>
std::vector<Edge> kruskal(Graph<Vertex, Edge> const& g)
{
    std::vector<Edge> mst;

    // sort edges
    const std::list<Edge>& edges = g.GetEdges();

    std::vector<Edge> edgesVector{edges.begin(), edges.end()};
    std::sort(edgesVector.begin(), edgesVector.end());

    // create disjoint sets
    DisjointSets ds(g.Size()); // g.Size() is the number of vertices
    for (size_t i = 0; i < g.Size(); i++)
    {
        ds.Make();
    }

    // iterate over edges
    for (auto const& e : edgesVector)
    {
        size_t u = e.ID1();
        size_t v = e.ID2();

        // are they in different blobs?
        if (ds.GetRepresentative(u) != ds.GetRepresentative(v))
        {
            mst.push_back(e);
            ds.Join(u, v);
        }
    }

    return mst;
}

#endif

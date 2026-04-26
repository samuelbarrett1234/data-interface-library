#include <bit>
#include <new>
#include <vector>

#include "data_interface.hpp"
#include "graph/dfs/linked_stack.ipp"
// #include "graph/dfs/sequential_stack.ipp"


enum class VertexColour{ WHITE, GREY, BLACK };
struct DenseGraphVertex
{
    size_t outdegree, index;
};
struct DenseGraphVertexCold
{
    VertexColour colour;
    DenseGraphVertex* parent;
    DenseGraphVertex* stack;
};


struct DenseGraph
{
    DenseGraph(const size_t num_vertices, const size_t num_edges)
    {
        next_vertex_index = 0;

        const size_t allocation_size
            = sizeof(DenseGraphVertex) * num_vertices
                + sizeof(DenseGraphVertex*) * num_edges;

        memory_begin = reinterpret_cast<std::byte*>(malloc(
            allocation_size
        ));
        memory_logical_end = memory_begin;
        memory_physical_end = memory_begin + allocation_size;

        colds.resize(num_vertices);
    }
    ~DenseGraph()
    {
        free(memory_begin);
    }

    DenseGraphVertex* create_vertex(const size_t num_edges)
    {
        DenseGraphVertex* const vertex
            = new (memory_logical_end) DenseGraphVertex{
                .outdegree = num_edges,
                .index = next_vertex_index++ };

        memory_logical_end += sizeof(DenseGraphVertex);

        new (memory_logical_end) DenseGraphVertex*[num_edges](nullptr);

        memory_logical_end += sizeof(DenseGraphVertex*) * num_edges;

        return vertex;
    }

    size_t next_vertex_index;
    std::byte *memory_begin, *memory_logical_end, *memory_physical_end;
    std::vector<DenseGraphVertexCold> colds;
};


struct OutEdgeBeginFieldTag {};
struct OutEdgeEndFieldTag {};
struct VertexColourTag {};
struct VertexParentTag {};
struct VertexStackTag {};
struct DenseEdgeTargetTag {};


inline DenseGraphVertex* next_impl(
    DenseGraphVertex* const vertex)
{
    return std::launder(
        reinterpret_cast<
            DenseGraphVertex*>(
                reinterpret_cast<std::byte*>(vertex)
                    + sizeof(DenseGraphVertex)
                    + sizeof(DenseGraphVertex*)
                        * vertex->outdegree));
}
inline std::ptrdiff_t diff_impl(
    DenseGraphVertex* const v1,
    DenseGraphVertex* const v2)
{
    /*
    * This is an interesting example of
    * why we might be able to supply a
    * `difference` but not an `advance`!
    *
    * Obviously, with the head-tail layout,
    * taking the difference in the pointers
    * `v1, v2` would not be correct here
    * (and would be UB anyway.)
    */
    const auto
        i = static_cast<std::ptrdiff_t>(v1->index),
        j = static_cast<std::ptrdiff_t>(v2->index);

    return j - i;
}
inline std::ptrdiff_t less_impl(
    DenseGraphVertex* const v1,
    DenseGraphVertex* const v2)
{
    /*
    * This is an interesting example of
    * why we might be able to supply a
    * `less` but not an `advance`!
    *
    * Note that it would be undefined
    * behaviour to compare the raw
    * pointers `v1, v2` here, even though
    * it would produce the same result.
    */
    return v1->index < v2->index;
}
inline DenseGraphVertex** next_impl(
    DenseGraphVertex** const edge)
{
    return edge + 1;
}
inline DenseGraphVertex** prev_impl(
    DenseGraphVertex** const edge)
{
    return edge - 1;
}
inline DenseGraphVertex** advance_impl(
    DenseGraphVertex** const edge,
    const std::ptrdiff_t d)
{
    return edge + d;
}
inline std::ptrdiff_t diff_impl(
    DenseGraphVertex** const e1,
    DenseGraphVertex** const e2)
{
    return e2 - e1;
}
inline bool less_impl(
    DenseGraphVertex** const e1,
    DenseGraphVertex** const e2)
{
    return e1 < e2;
}


namespace di
{


template<>
struct FieldTypeImpl<OutEdgeBeginFieldTag, DenseGraph>
{
    using Type = DenseGraphVertex**;
};
template<>
struct FieldTypeImpl<OutEdgeEndFieldTag, DenseGraph>
{
    using Type = DenseGraphVertex**;
};
template<>
struct FieldTypeImpl<VertexColourTag, DenseGraph>
{
    using Type = VertexColour;
};
template<>
struct FieldTypeImpl<VertexParentTag, DenseGraph>
{
    using Type = DenseGraphVertex*;
};
template<>
struct FieldTypeImpl<VertexStackTag, DenseGraph>
{
    using Type = DenseGraphVertex*;
};
template<>
struct FieldTypeImpl<DenseEdgeTargetTag, DenseGraph>
{
    using Type = DenseGraphVertex*;
};


}  // namespace di


inline di::FieldType<OutEdgeBeginFieldTag, DenseGraph> load_impl(
        const DenseGraph&,
        DenseGraphVertex* const vertex,
        OutEdgeBeginFieldTag)
    noexcept
{
    return std::launder(
        reinterpret_cast<
            DenseGraphVertex**>(
                reinterpret_cast<std::byte*>(vertex)
                    + sizeof(DenseGraphVertex)));
}


inline di::FieldType<OutEdgeEndFieldTag, DenseGraph> load_impl(
        const DenseGraph& graph,
        DenseGraphVertex* const vertex,
        OutEdgeEndFieldTag)
    noexcept
{
    return di::load<OutEdgeBeginFieldTag>(graph, vertex)
        + vertex->outdegree;
}


inline di::FieldType<VertexColourTag, DenseGraph> load_impl(
        const DenseGraph& graph,
        DenseGraphVertex* const vertex,
        VertexColourTag)
    noexcept
{
    return graph.colds[vertex->index].colour;
}
inline void store_impl(
        DenseGraph& graph,
        DenseGraphVertex* const vertex,
        const di::FieldType<VertexColourTag, DenseGraph> colour,
        VertexColourTag)
    noexcept
{
    graph.colds[vertex->index].colour = colour;
}


inline di::FieldType<VertexParentTag, DenseGraph> load_impl(
        const DenseGraph& graph,
        DenseGraphVertex* const vertex,
        VertexParentTag)
    noexcept
{
    return graph.colds[vertex->index].parent;
}
inline void store_impl(
        DenseGraph& graph,
        DenseGraphVertex* const vertex,
        const di::FieldType<VertexParentTag, DenseGraph> parent,
        VertexParentTag)
    noexcept
{
    graph.colds[vertex->index].parent = parent;
}


inline di::FieldType<VertexStackTag, DenseGraph> load_impl(
        const DenseGraph& graph,
        DenseGraphVertex* const vertex,
        VertexStackTag)
    noexcept
{
    return graph.colds[vertex->index].stack;
}
inline void store_impl(
        DenseGraph& graph,
        DenseGraphVertex* const vertex,
        const di::FieldType<VertexStackTag, DenseGraph> stack,
        VertexStackTag)
    noexcept
{
    graph.colds[vertex->index].stack = stack;
}


inline di::FieldType<DenseEdgeTargetTag, DenseGraph> load_impl(
        const DenseGraph&,
        DenseGraphVertex** edge,
        DenseEdgeTargetTag)
    noexcept
{
    return *edge;
}


void dfs_linked_stack(DenseGraph& graph)
{
    DenseGraphVertex* const vertex_begin
        = std::launder(
            reinterpret_cast<
                DenseGraphVertex*>(
                    graph.memory_begin));

    DenseGraphVertex* const vertex_end
        = std::launder(
            reinterpret_cast<
                DenseGraphVertex*>(
                    graph.memory_logical_end));

    di::graph::dfs_linked_stack<
        DenseGraph,
        DenseGraphVertex*,
        DenseGraphVertex**,
        OutEdgeBeginFieldTag,
        OutEdgeEndFieldTag,
        DenseEdgeTargetTag,
        VertexParentTag,
        VertexStackTag,
        VertexColourTag,
        VertexColour::WHITE,
        VertexColour::GREY,
        VertexColour::BLACK
    >(graph, vertex_begin, vertex_end);
}

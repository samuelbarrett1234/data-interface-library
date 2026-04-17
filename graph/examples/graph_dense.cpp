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


struct DenseVertexTag {};
struct DenseEdgeTag {};
struct OutEdgeBeginFieldTag {};
struct OutEdgeEndFieldTag {};
struct VertexColourTag {};
struct VertexParentTag {};
struct VertexStackTag {};
struct DenseEdgeTargetTag {};


namespace di
{
template<>
struct EntityTypeImpl<DenseVertexTag, DenseGraph>
{
    using Type = DenseGraphVertex*;
};
template<>
inline EntityType<DenseVertexTag, DenseGraph>
    next<DenseVertexTag, DenseGraph>(
        const DenseGraph&,
        EntityType<DenseVertexTag, DenseGraph> vertex)
    noexcept
{
    return std::launder(
        reinterpret_cast<
            DenseGraphVertex*>(
                reinterpret_cast<std::byte*>(vertex)
                    + sizeof(DenseGraphVertex)
                    + sizeof(DenseGraphVertex*)
                        * vertex->outdegree));
}
template<>
inline std::ptrdiff_t
    difference<DenseVertexTag, DenseGraph>(
        const DenseGraph&,
        const EntityType<DenseVertexTag, DenseGraph> v1,
        const EntityType<DenseVertexTag, DenseGraph> v2)
    noexcept
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
template<>
inline bool
    less<DenseVertexTag, DenseGraph>(
        const DenseGraph&,
        const EntityType<DenseVertexTag, DenseGraph> v1,
        const EntityType<DenseVertexTag, DenseGraph> v2)
    noexcept
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


template<>
struct EntityTypeImpl<DenseEdgeTag, DenseGraph>
{
    using Type = DenseGraphVertex**;
};
template<>
inline EntityType<DenseEdgeTag, DenseGraph>
    next<DenseEdgeTag, DenseGraph>(
        const DenseGraph&,
        EntityType<DenseEdgeTag, DenseGraph> edge)
    noexcept
{
    return edge + 1;
}
template<>
inline EntityType<DenseEdgeTag, DenseGraph>
    prev<DenseEdgeTag, DenseGraph>(
        const DenseGraph&,
        EntityType<DenseEdgeTag, DenseGraph> edge)
    noexcept
{
    return edge - 1;
}
template<>
inline EntityType<DenseEdgeTag, DenseGraph>
    advance<DenseEdgeTag, DenseGraph>(
        const DenseGraph&,
        EntityType<DenseEdgeTag, DenseGraph> edge,
        const std::ptrdiff_t delta)
    noexcept
{
    return edge + delta;
}
template<>
inline std::ptrdiff_t
    difference<DenseEdgeTag, DenseGraph>(
        const DenseGraph&,
        const EntityType<DenseEdgeTag, DenseGraph> e1,
        const EntityType<DenseEdgeTag, DenseGraph> e2)
    noexcept
{
    return e2 - e1;
}
template<>
inline bool
    less<DenseEdgeTag, DenseGraph>(
        const DenseGraph&,
        const EntityType<DenseEdgeTag, DenseGraph> e1,
        const EntityType<DenseEdgeTag, DenseGraph> e2)
    noexcept
{
    return e1 < e2;
}


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
inline FieldType<OutEdgeBeginFieldTag, DenseGraph>
    load<OutEdgeBeginFieldTag, DenseVertexTag, DenseGraph>(
        const DenseGraph&,
        EntityType<DenseVertexTag, DenseGraph> vertex)
    noexcept
{
    return std::launder(
        reinterpret_cast<
            DenseGraphVertex**>(
                reinterpret_cast<std::byte*>(vertex)
                    + sizeof(DenseGraphVertex)));
}
template<>
inline FieldType<OutEdgeEndFieldTag, DenseGraph>
    load<OutEdgeEndFieldTag, DenseVertexTag, DenseGraph>(
        const DenseGraph& data_interface,
        EntityType<DenseVertexTag, DenseGraph> vertex)
    noexcept
{
    return load<OutEdgeBeginFieldTag, DenseVertexTag, DenseGraph>(
        data_interface, vertex)
            + vertex->outdegree;
}


template<>
struct FieldTypeImpl<VertexColourTag, DenseGraph>
{
    using Type = VertexColour;
};
template<>
inline FieldType<VertexColourTag, DenseGraph>
    load<VertexColourTag, DenseVertexTag, DenseGraph>(
        const DenseGraph& graph,
        EntityType<DenseVertexTag, DenseGraph> vertex)
    noexcept
{
    return graph.colds[vertex->index].colour;
}
template<>
inline void
    store<VertexColourTag, DenseVertexTag, DenseGraph>(
        DenseGraph& graph,
        EntityType<DenseVertexTag, DenseGraph> vertex,
        FieldType<VertexColourTag, DenseGraph> colour)
    noexcept
{
    graph.colds[vertex->index].colour = colour;
}


template<>
struct FieldTypeImpl<VertexParentTag, DenseGraph>
{
    using Type = DenseGraphVertex*;
};
template<>
inline FieldType<VertexParentTag, DenseGraph>
    load<VertexParentTag, DenseVertexTag, DenseGraph>(
        const DenseGraph& graph,
        EntityType<DenseVertexTag, DenseGraph> vertex)
    noexcept
{
    return graph.colds[vertex->index].parent;
}
template<>
inline void
    store<VertexParentTag, DenseVertexTag, DenseGraph>(
        DenseGraph& graph,
        EntityType<DenseVertexTag, DenseGraph> vertex,
        FieldType<VertexParentTag, DenseGraph> parent)
    noexcept
{
    graph.colds[vertex->index].parent = parent;
}


template<>
struct FieldTypeImpl<VertexStackTag, DenseGraph>
{
    using Type = DenseGraphVertex*;
};
template<>
inline FieldType<VertexStackTag, DenseGraph>
    load<VertexStackTag, DenseVertexTag, DenseGraph>(
        const DenseGraph& graph,
        EntityType<DenseVertexTag, DenseGraph> vertex)
    noexcept
{
    return graph.colds[vertex->index].stack;
}
template<>
inline void
    store<VertexStackTag, DenseVertexTag, DenseGraph>(
        DenseGraph& graph,
        EntityType<DenseVertexTag, DenseGraph> vertex,
        FieldType<VertexStackTag, DenseGraph> stack)
    noexcept
{
    graph.colds[vertex->index].stack = stack;
}


template<>
struct FieldTypeImpl<DenseEdgeTargetTag, DenseGraph>
{
    using Type = DenseGraphVertex*;
};
template<>
inline FieldType<DenseEdgeTargetTag, DenseGraph>
    load<DenseEdgeTargetTag, DenseEdgeTag, DenseGraph>(
        const DenseGraph&,
        EntityType<DenseEdgeTag, DenseGraph> edge)
    noexcept
{
    return *edge;
}


}  // namespace di


void dfs_linked_stack(DenseGraph& G)
{
    DenseGraphVertex* const vertex_begin
        = std::launder(
            reinterpret_cast<
                DenseGraphVertex*>(
                    G.memory_begin));

    DenseGraphVertex* const vertex_end
        = std::launder(
            reinterpret_cast<
                DenseGraphVertex*>(
                    G.memory_logical_end));

    di::graph::dfs_linked_stack<
        DenseGraph,
        DenseVertexTag,
        DenseEdgeTag,
        OutEdgeBeginFieldTag,
        OutEdgeEndFieldTag,
        DenseEdgeTargetTag,
        VertexParentTag,
        VertexStackTag,
        VertexColourTag,
        VertexColour::WHITE,
        VertexColour::GREY,
        VertexColour::BLACK
    >(G, vertex_begin, vertex_end);
}

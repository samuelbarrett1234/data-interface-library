#include <bit>
#include <new>
#include <vector>

#include "data_interface.hpp"
#include "graph/dfs/linked_stack.ipp"
// #include "graph/dfs/sequential_stack.ipp"


enum class VertexColour{ WHITE, GREY, BLACK };
struct DenseGraphVertexColdIndex
{
    size_t index;
};
struct DenseGraphVertex
{
    size_t outdegree;
    DenseGraphVertexColdIndex cold_index;
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

        // colds.resize(num_vertices);
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
                .cold_index = DenseGraphVertexColdIndex{
                    .index = next_vertex_index++ } };

        memory_logical_end += sizeof(DenseGraphVertex);

        new (memory_logical_end) DenseGraphVertex*[num_edges](nullptr);

        memory_logical_end += sizeof(DenseGraphVertex*) * num_edges;

        return vertex;
    }

    size_t next_vertex_index;
    std::byte *memory_begin, *memory_logical_end, *memory_physical_end;
};


struct OutEdgeBeginFieldTag {};
struct OutEdgeEndFieldTag {};
struct VertexColdIndexTag {};
struct DenseEdgeTargetTag {};
struct VertexColourTag {};
struct VertexParentTag {};
struct VertexStackTag {};


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
        i = static_cast<std::ptrdiff_t>(v1->cold_index.index),
        j = static_cast<std::ptrdiff_t>(v2->cold_index.index);

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
    return v1->cold_index.index < v2->cold_index.index;
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


/*
* Stateless DI for expressing the head-tail
* allocation of vertices and edges.
*/
struct HeadTailBasicsDI{};


namespace di
{


template<>
struct FieldTypeImpl<OutEdgeBeginFieldTag, HeadTailBasicsDI>
{
    using Type = DenseGraphVertex**;
};
template<>
struct FieldTypeImpl<OutEdgeEndFieldTag, HeadTailBasicsDI>
{
    using Type = DenseGraphVertex**;
};
template<>
struct FieldTypeImpl<VertexColdIndexTag, HeadTailBasicsDI>
{
    using Type = DenseGraphVertexColdIndex;
};
template<>
struct FieldTypeImpl<DenseEdgeTargetTag, HeadTailBasicsDI>
{
    using Type = DenseGraphVertex*;
};


// template<>
// struct FieldTypeImpl<VertexColourTag, DenseGraph>
// {
//     using Type = VertexColour;
// };
// template<>
// struct FieldTypeImpl<VertexParentTag, DenseGraph>
// {
//     using Type = DenseGraphVertex*;
// };
// template<>
// struct FieldTypeImpl<VertexStackTag, DenseGraph>
// {
//     using Type = DenseGraphVertex*;
// };


}  // namespace di


inline DenseGraphVertex** load_impl(
        const HeadTailBasicsDI&,
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


inline DenseGraphVertex** load_impl(
        const HeadTailBasicsDI& di,
        DenseGraphVertex* const vertex,
        OutEdgeEndFieldTag)
    noexcept
{
    return di::load<OutEdgeBeginFieldTag>(di, vertex)
        + vertex->outdegree;
}


inline DenseGraphVertexColdIndex load_impl(
        const HeadTailBasicsDI&,
        DenseGraphVertex* const vertex,
        VertexColdIndexTag)
    noexcept
{
    return vertex->cold_index;
}


inline DenseGraphVertex* load_impl(
        const HeadTailBasicsDI&,
        DenseGraphVertex** const edge,
        DenseEdgeTargetTag)
    noexcept
{
    return *edge;
}


// inline VertexColour load_impl(
//         const GraphVertexColdData& graph,
//         const DenseGraphVertexColdIndex index,
//         VertexColourTag)
//     noexcept
// {
//     return graph.colds[index.index].colour;
// }
// inline void store_impl(
//         GraphVertexColdData& graph,
//         const DenseGraphVertexColdIndex index,
//         const VertexColour colour,
//         VertexColourTag)
//     noexcept
// {
//     graph.colds[index.index].colour = colour;
// }


// inline DenseGraphVertex* load_impl(
//         const GraphVertexColdData& graph,
//         const DenseGraphVertexColdIndex index,
//         VertexParentTag)
//     noexcept
// {
//     return graph.colds[index.index].parent;
// }
// inline void store_impl(
//         GraphVertexColdData& graph,
//         const DenseGraphVertexColdIndex index,
//         DenseGraphVertex* const parent,
//         VertexParentTag)
//     noexcept
// {
//     graph.colds[index.index].parent = parent;
// }


// inline DenseGraphVertex* load_impl(
//         const GraphVertexColdData& graph,
//         const DenseGraphVertexColdIndex index,
//         VertexStackTag)
//     noexcept
// {
//     return graph.colds[index.index].stack;
// }
// inline void store_impl(
//         GraphVertexColdData& graph,
//         const DenseGraphVertexColdIndex index,
//         DenseGraphVertex* const stack,
//         VertexStackTag)
//     noexcept
// {
//     graph.colds[index.index].stack = stack;
// }


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

    struct EmptyDI{};

    /*
    * Firstly, define a DI which understands
    * the fields of the
    * `DenseGraphVertexCold` struct.
    */
    auto cold_ptrs
        = di::make_with_pointer_to_member_data_interface<
            DenseGraphVertexCold,
            &DenseGraphVertexCold::parent,
            VertexParentTag>(
        di::make_with_pointer_to_member_data_interface<
            DenseGraphVertexCold,
            &DenseGraphVertexCold::stack,
            VertexStackTag>(
        di::make_with_pointer_to_member_data_interface<
            DenseGraphVertexCold,
            &DenseGraphVertexCold::colour,
            VertexColourTag>(EmptyDI{})));

    /*
    * Now define a DI which can convert an
    * index `DenseGraphVertexColdIndex` to
    * a pointer `DenseGraphVertexCold*` and
    * then dispatches to `cold_ptrs` above.
    */
    std::vector<DenseGraphVertexCold> colds;
    colds.resize(di::diff(vertex_begin, vertex_end));
    auto cold_array
        = di::make_entity_mapper_data_interface<
            DenseGraphVertexColdIndex,
            DenseGraphVertexCold*>(
            cold_ptrs,
            [&](const DenseGraphVertexColdIndex index)
                -> DenseGraphVertexCold*
            {
                return &colds[index.index];
            });

    /*
    * The `HeadTailBasicsDI` has no state
    * and is what understands the
    * `DenseGraphVertex*` vertex entities and
    * `DenseGraphVertex**` edge entities.
    *
    * Let's take the disjoint union with
    * the cold fields here.
    */
    auto head_tails_and_colds
        = di::make_disjoint_union_data_interface(
            HeadTailBasicsDI{},
            cold_array);

    /*
    * Finally, we make the connection between
    * the "hot" vertex entities
    * `DenseGraphVertex*` and the cold entities
    * `DenseGraphVertexCold*` via the connection
    * index `DenseGraphVertexColdIndex`.
    */
    auto with_hot_and_cold_connection
        = di::make_with_indirection_lookup<
            VertexColdIndexTag,
            DenseGraphVertex*,
            true /* bijection */>(
            head_tails_and_colds);

    di::graph::dfs_linked_stack<
        decltype(with_hot_and_cold_connection),
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
    >(with_hot_and_cold_connection, vertex_begin, vertex_end);
}

#ifndef DATA_INTERFACE_GRAPH_DFS_SEQUENTIAL_STACK_IPP
#define DATA_INTERFACE_GRAPH_DFS_SEQUENTIAL_STACK_IPP


#include <cassert>

#include "data_interface.hpp"


namespace di
{
namespace graph
{


/*
* This function implements depth-first-search
* using colour values and using a
* singly-linked-list stack of vertices.
*/
template<
    /* what are the entities? */
    typename VertexEntityTag,
    typename EdgeEntityTag,
    typename TrailEntityTag,
    /* what are the fields? */
    typename OutEdgeBeginFieldTag,
    typename OutEdgeEndFieldTag,
    typename EdgeTargetFieldTag,
    typename DfsTreeParentFieldTag,
    typename TrailVertexLinkFieldTag,
    /* and data interface: */
    typename DataInterface
>
void dfs_sequential_stack(
        DataInterface& data_interface,
        const EntityType<VertexEntityTag, DataInterface> vertex_begin,
        const EntityType<VertexEntityTag, DataInterface> vertex_end,
        const EntityType<TrailEntityTag, DataInterface> trail_begin,
        const EntityType<TrailEntityTag, DataInterface> trail_end)
    noexcept
{
    /*
    * Type aliases for brevity.
    */
    using VertexType = EntityType<VertexEntityTag, DataInterface>;
    using EdgeType = EntityType<EdgeEntityTag, DataInterface>;
    using TrailType = EntityType<TrailEntityTag, DataInterface>;

    /*
    * Enforce our requirements on the data.
    * Anything we use, rely on or assume, should
    * be asserted here.
    */
    static_assert(
        is_nextable<VertexEntityTag, DataInterface>,
        "Must be able to increment vertex entities."
    );
    static_assert(
        is_nextable<EdgeEntityTag, DataInterface>,
        "Must be able to increment edge entities."
    );
    static_assert(
        is_nextable<TrailEntityTag, DataInterface>,
        "Must be able to increment trail entities."
    );
    static_assert(
        is_prevable<TrailEntityTag, DataInterface>,
        "Must be able to decrement trail entities."
    );
    static_assert(
        is_comparable<TrailEntityTag, DataInterface>,
        "Trail elements must be comparable."
    );
    static_assert(
        is_loadable<OutEdgeBeginFieldTag, VertexEntityTag, DataInterface>,
        "Must be able to read out-edge begin field "
        "from vertex entities."
    );
    static_assert(
        is_loadable<OutEdgeEndFieldTag, VertexEntityTag, DataInterface>,
        "Must be able to read out-edge end field "
        "from vertex entities."
    );
    static_assert(
        is_loadable<EdgeTargetFieldTag, EdgeEntityTag, DataInterface>,
        "Must be able to read target field "
        "from edge entities."
    );
    static_assert(
        is_storeable<DfsTreeParentFieldTag, VertexEntityTag, DataInterface>,
        "Must be able to store DFS tree parent "
        "field to vertex entities."
    );
    static_assert(
        is_loadable<TrailVertexLinkFieldTag, VertexEntityTag, DataInterface>,
        "Must be able to load the trail-vertex "
        "link field from vertex entities."
    );
    static_assert(
        is_storeable<TrailVertexLinkFieldTag, VertexEntityTag, DataInterface>,
        "Must be able to store the trail-vertex "
        "link field to vertex entities."
    );
    static_assert(
        is_loadable<TrailVertexLinkFieldTag, TrailEntityTag, DataInterface>,
        "Must be able to load the trail-vertex "
        "link field from trail entities."
    );
    static_assert(
        is_storeable<TrailVertexLinkFieldTag, TrailEntityTag, DataInterface>,
        "Must be able to store the trail-vertex "
        "link field to trail entities."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                OutEdgeBeginFieldTag,
                VertexEntityTag,
                DataInterface>,
            EdgeType>,
        "Out-edge-begin field must be edge "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                OutEdgeEndFieldTag,
                VertexEntityTag,
                DataInterface>,
            EdgeType>,
        "Out-edge-end field must be edge "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                EdgeTargetFieldTag,
                EdgeEntityTag,
                DataInterface>,
            VertexType>,
        "Edge target field must be vertex "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                DfsTreeParentFieldTag,
                VertexEntityTag,
                DataInterface>,
            VertexType>,
        "DFS tree parent field must be vertex "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                TrailVertexLinkFieldTag,
                VertexEntityTag,
                DataInterface>,
            EntityType<
                TrailEntityTag,
                DataInterface>>,
        "The trail link field should be the "
        "trail entity type on vertex entities."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                TrailVertexLinkFieldTag,
                TrailEntityTag,
                DataInterface>,
            EntityType<
                VertexEntityTag,
                DataInterface>>,
        "The trail link field should be the "
        "vertex entity type on trail entities."
    );

    #ifndef NDEBUG
    if constexpr (
        is_diffable<TrailEntityTag, DataInterface>
        && is_diffable<VertexEntityTag, DataInterface>)
    {
        /*
        * Assert there is enough space in the
        * trail to store all of the vertices.
        */
        assert(
            difference(trail_begin, trail_end)
                >= difference(vertex_begin, vertex_end)
        );
    }

    for (VertexType v = vertex_begin;
        v != vertex_end;
        v = next(data_interface, v))
    {
        /*
        * PRECONDITION:
        *
        * Assert that the trail array is already
        * a permutation of the vertex array.
        */
        assert(
            load<TrailVertexLinkFieldTag>(
                data_interface,
                load<TrailVertexLinkFieldTag>(
                    data_interface, v))
                        == v
        );
    }

    #else  // NDEBUG

    ((void)trail_end);  // unused

    #endif  // NDEBUG

    /*
    * We'll use the trail to represent 2
    * stacks. The sum of the sizes of these
    * stacks will never exceed the total vertex
    * count. At termination, one of these
    * stacks will be empty, and the other will
    * contain all of the graph's vertices in
    * preorder.
    */
    TrailType
        trail_done_stack_end = trail_begin,
        trail_working_stack_begin = trail_end;

    /*
    * Loop through vertices in the
    * graph.
    */
    for (VertexType root = vertex_begin;
        root != vertex_end;
        root = next(data_interface, root))
    {
        assert(
            trail_working_stack_begin
                == trail_end,
            "Invariant failed - expected the "
            "working stack to be empty between "
            "runs."
        );

        /*
        * Check prior status.
        */
        {
            const TrailType trail_other
                = load<TrailVertexLinkFieldTag>(
                    data_interface, root);

            /*
            * Check permutation invariant.
            */
            assert(
                load<TrailVertexLinkFieldTag>(
                    data_interface, trail_other)
                        == root
            );

            if (less(data_interface, root, trail_done_stack_end))
            {
                /*
                * Already explored.
                */
                continue;
            }

            /*
            * Else bring it under the
            * working stack.
            */
            trail_working_stack_begin
                = prev(data_interface, trail_working_stack_begin);

            const VertexType vertex_other
                = load<TrailVertexLinkFieldTag>(
                    data_interface, trail_working_stack_begin);

            /*
            * Check permutation invariant.
            */
            assert(
                load<TrailVertexLinkFieldTag>(
                    data_interface, vertex_other)
                        == trail_working_stack_begin
            );

            store<TrailVertexLinkFieldTag>(
                data_interface, trail_working_stack_begin, root);
            store<TrailVertexLinkFieldTag>(
                data_interface, trail_other, vertex_other);
            store<TrailVertexLinkFieldTag>(
                data_interface, root, trail_working_stack_begin);
            store<TrailVertexLinkFieldTag>(
                data_interface, vertex_other, trail_other);
        }

        /*
        * Roots of the DFS trees are their
        * own parents.
        */
        store<DfsTreeParentFieldTag>(data_interface, root, root);

        while (trail_working_stack_begin
            != trail_end)
        {
            /*
            * Assert trail-stacks invariant.
            */
            assert(less_equal(
                data_interface,
                trail_done_stack_end,
                trail_working_stack_begin
            ));

            const VertexType vertex_source
                = load<TrailVertexLinkFieldTag>(
                    data_interface, trail_working_stack_begin);

            /*
            * Check permutation invariant.
            */
            assert(
                load<TrailVertexLinkFieldTag>(
                    data_interface, vertex_source)
                        == trail_working_stack_begin
            );

            /*
            * Move this vertex from the working
            * stack to the done stack.
            */
            {
                const VertexType vertex_other
                    = load<TrailVertexLinkFieldTag>(
                        data_interface, trail_done_stack_end);

                /*
                * Check permutation invariant.
                */
                assert(
                    load<TrailVertexLinkFieldTag>(
                        data_interface, vertex_other)
                            == trail_done_stack_end
                );

                store<TrailVertexLinkFieldTag>(
                    data_interface, trail_working_stack_begin, vertex_other);
                store<TrailVertexLinkFieldTag>(
                    data_interface, trail_done_stack_end, vertex_source);
                store<TrailVertexLinkFieldTag>(
                    data_interface, vertex_other, trail_working_stack_begin);
                store<TrailVertexLinkFieldTag>(
                    data_interface, vertex_source, trail_done_stack_end);

                trail_working_stack_begin
                    = next(data_interface, trail_working_stack_begin);
                trail_done_stack_end
                    = next(data_interface, trail_done_stack_end);
            }

            const EdgeType
                out_begin = load<OutEdgeBeginFieldTag>(data_interface, stack),
                out_end = load<OutEdgeEndFieldTag>(data_interface, stack);

            /*
            * Loop through the out-edges of
            * this vertex.
            */
            for (EdgeType e = out_begin;
                e != out_end;
                e = next(data_interface, e))
            {
                const VertexType vertex_target
                    = load<EdgeTargetFieldTag>(data_interface, e);

                const TrailType trail
                    = load<TrailVertexLinkFieldTag>(data_interface, vertex_target);

                /*
                * Check permutation invariant.
                */
                assert(
                    load<TrailVertexLinkFieldTag>(
                        data_interface, trail)
                            == vertex_target
                );

                /*
                * If this vertex is in EITHER stack
                * (corresponding to cases where it has
                * already been fully explored or where
                * it is currently in exploration) then
                * skip it.
                */
                if (less(trail, trail_done_stack_end)
                    || less_equal(trail_working_stack_begin, trail))
                {
                    continue;
                }

                /*
                * NOTE: `DfsTreeParentFieldTag` is not readable,
                * so the caller can ignore it if they wish,
                * making this write a no-op.
                */
                store<DfsTreeParentFieldTag>(data_interface, vertex_target, source);

                /*
                * Move this vertex from the middle
                * to the working stack.
                */
                {
                    trail_working_stack_begin
                        = prev(data_interface, trail_working_stack_begin);

                    const VertexType vertex_other
                        = load<TrailVertexLinkFieldTag>(
                            data_interface, trail_working_stack_begin);

                    /*
                    * Check permutation invariant.
                    */
                    assert(
                        load<TrailVertexLinkFieldTag>(
                            data_interface, vertex_other)
                                == trail_working_stack_begin
                    );

                    store<TrailVertexLinkFieldTag>(
                        data_interface, trail_working_stack_begin, vertex_target);
                    store<TrailVertexLinkFieldTag>(
                        data_interface, trail, vertex_other);
                    store<TrailVertexLinkFieldTag>(
                        data_interface, vertex_target, trail_working_stack_begin);
                    store<TrailVertexLinkFieldTag>(
                        data_interface, vertex_other, trail);
                }
            }
        }
    }
}


}  // namespace graph
}  // namespace di


#endif  // DATA_INTERFACE_GRAPH_DFS_SEQUENTIAL_STACK_IPP

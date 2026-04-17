#ifndef DATA_INTERFACE_GRAPH_DFS_LINKED_STACK_IPP
#define DATA_INTERFACE_GRAPH_DFS_LINKED_STACK_IPP


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
    typename DataInterface,
    /* what are the entities? */
    typename VertexEntityTag,
    typename EdgeEntityTag,
    /* what are the fields? */
    typename OutEdgeBeginFieldTag,
    typename OutEdgeEndFieldTag,
    typename EdgeTargetFieldTag,
    typename DfsTreeParentFieldTag,
    typename StackNextFieldTag,
    typename ColourFieldTag,
    /* constants */
    FieldType<ColourFieldTag, DataInterface> UNEXPLORED,
    FieldType<ColourFieldTag, DataInterface> EXPLORING,
    FieldType<ColourFieldTag, DataInterface> EXPLORED
>
void dfs_linked_stack(
        DataInterface& data_interface,
        const EntityType<VertexEntityTag, DataInterface> vertex_begin,
        const EntityType<VertexEntityTag, DataInterface> vertex_end)
    noexcept
{
    /*
    * Type aliases for brevity.
    */
    using ColourType = FieldType<ColourFieldTag, DataInterface>;
    using VertexType = EntityType<VertexEntityTag, DataInterface>;
    using EdgeType = EntityType<EdgeEntityTag, DataInterface>;

    /*
    * Enforce our requirements on the data.
    * Anything we use, rely on or assume, should
    * be asserted here.
    */
    static_assert(
        is_loadable<ColourFieldTag, VertexEntityTag, DataInterface>,
        "Must be able to read vertex colour from vertex entity."
    );
    static_assert(
        is_storeable<ColourFieldTag, VertexEntityTag, DataInterface>,
        "Must be able to write vertex colour to vertex entity."
    );
    static_assert(
        is_nextable<VertexEntityTag, DataInterface>,
        "Must be able to increment vertex entities."
    );
    static_assert(
        is_nextable<EdgeEntityTag, DataInterface>,
        "Must be able to increment edge entities."
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
        is_loadable<StackNextFieldTag, VertexEntityTag, DataInterface>,
        "Must be able to load the stack next field "
        "from vertex entities."
    );
    static_assert(
        is_storeable<StackNextFieldTag, VertexEntityTag, DataInterface>,
        "Must be able to store the stack next field "
        "to vertex entities."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                OutEdgeBeginFieldTag,
                DataInterface>,
            EdgeType>,
        "Out-edge-begin field must be edge "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                OutEdgeEndFieldTag,
                DataInterface>,
            EdgeType>,
        "Out-edge-end field must be edge "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                EdgeTargetFieldTag,
                DataInterface>,
            VertexType>,
        "Edge target field must be vertex "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                DfsTreeParentFieldTag,
                DataInterface>,
            VertexType>,
        "DFS tree parent field must be vertex "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                StackNextFieldTag,
                DataInterface>,
            VertexType>,
        "The stack next field must be vertex "
        "type."
    );

    /*
    * Reset all colours.
    */
    for (VertexType v = vertex_begin;
        v != vertex_end;
        v = next<VertexEntityTag>(data_interface, v))
    {
        store<ColourFieldTag, VertexEntityTag>(
            data_interface, v, UNEXPLORED);
    }

    /*
    * Loop through vertices in the
    * graph.
    */
    for (VertexType root = vertex_begin;
        root != vertex_end;
        root = next<VertexEntityTag>(data_interface, root))
    {
        const ColourType root_old_colour
            = load<ColourFieldTag, VertexEntityTag>(
                data_interface, root);

        /*
        * Nodes should be fully explored or not
        * by now.
        */
        assert(
            root_old_colour != EXPLORING
        );

        /*
        * Roots of the DFS trees are their
        * own parents.
        *
        * Do the same with the stack root
        * so that it wraps back around.
        */
        store<DfsTreeParentFieldTag, VertexEntityTag>(
            data_interface, root, root);
        store<StackNextFieldTag, VertexEntityTag>(
            data_interface, root, root);

        /*
        * Begin depth-first search from this
        * vertex.
        */
        VertexType stack = root;
        store<ColourFieldTag, VertexEntityTag>(
            data_interface, stack, EXPLORING);

        while (load<ColourFieldTag, VertexEntityTag>(
                data_interface, stack)
            != EXPLORED)
        {
            const VertexType source = stack;

            /*
            * Expected the stack to consist of precisely
            * the set of `EXPLORING` vertices.
            */
            #ifndef NDEBUG
            const ColourType source_colour
                = load<ColourFieldTag, VertexEntityTag>(
                    data_interface, source);
            assert(
                source_colour == EXPLORING
            );
            #endif  // NDEBUG

            /*
            * Pop from stack.
            */
            stack = load<StackNextFieldTag, VertexEntityTag>(
                data_interface, stack);

            const EdgeType
                out_begin = load<OutEdgeBeginFieldTag, VertexEntityTag>(
                    data_interface, stack),
                out_end = load<OutEdgeEndFieldTag, VertexEntityTag>(
                    data_interface, stack);

            /*
            * Loop through the out-edges of
            * this vertex.
            */
            for (EdgeType e = out_begin;
                e != out_end;
                e = next<EdgeEntityTag>(data_interface, e))
            {
                const VertexType target
                    = load<EdgeTargetFieldTag, EdgeEntityTag>(
                        data_interface, e);

                const ColourType old_colour
                    = load<ColourFieldTag, VertexEntityTag>(
                        data_interface, target);

                if (old_colour != UNEXPLORED)
                {
                    continue;
                }

                /*
                * Mark as `EXPLORING`, set the DFS tree parent
                * value, then push onto the stack.
                *
                * NOTE: `DfsTreeParentFieldTag` is not readable,
                * so the caller can ignore it if they wish,
                * making this write a no-op.
                */
                store<ColourFieldTag, VertexEntityTag>(
                    data_interface, target, EXPLORING);
                store<DfsTreeParentFieldTag, VertexEntityTag>(
                    data_interface, target, source);
                store<StackNextFieldTag, VertexEntityTag>(
                    data_interface, target, stack);
                stack = target;
            }
        }
    }
}


}  // namespace graph
}  // namespace di


#endif  // DATA_INTERFACE_GRAPH_DFS_LINKED_STACK_IPP

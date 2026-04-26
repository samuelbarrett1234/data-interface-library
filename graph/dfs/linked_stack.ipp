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
    /* what are the entities? */
    typename VertexEntityType,
    typename EdgeEntityType,
    /* what are the fields? */
    typename OutEdgeBeginFieldTag,
    typename OutEdgeEndFieldTag,
    typename EdgeTargetFieldTag,
    typename DfsTreeParentFieldTag,
    typename StackNextFieldTag,
    typename ColourFieldTag,
    /* constants */
    FieldType<ColourFieldTag> UNEXPLORED,
    FieldType<ColourFieldTag> EXPLORING,
    FieldType<ColourFieldTag> EXPLORED,
    /* and the DI (this template arg is deducible) */
    typename DataInterface
>
void dfs_linked_stack(
        DataInterface& data_interface,
        const VertexEntityType vertex_begin,
        const VertexEntityType vertex_end)
    noexcept
{
    /*
    * Type aliases for brevity.
    */
    using ColourType = FieldType<ColourFieldTag>;

    /*
    * Enforce our requirements on the data.
    * Anything we use, rely on or assume, should
    * be asserted here.
    */
    static_assert(
        is_loadable<ColourFieldTag, DataInterface, VertexEntityType>,
        "Must be able to read vertex colour from vertex entity."
    );
    static_assert(
        is_storeable<ColourFieldTag, DataInterface, VertexEntityType>,
        "Must be able to write vertex colour to vertex entity."
    );
    static_assert(
        is_nextable<VertexEntityType>,
        "Must be able to increment vertex entities."
    );
    static_assert(
        is_nextable<EdgeEntityType>,
        "Must be able to increment edge entities."
    );
    static_assert(
        is_loadable<OutEdgeBeginFieldTag, DataInterface, VertexEntityType>,
        "Must be able to read out-edge begin field "
        "from vertex entities."
    );
    static_assert(
        is_loadable<OutEdgeEndFieldTag, DataInterface, VertexEntityType>,
        "Must be able to read out-edge end field "
        "from vertex entities."
    );
    static_assert(
        is_loadable<EdgeTargetFieldTag, DataInterface, EdgeEntityType>,
        "Must be able to read target field "
        "from edge entities."
    );
    static_assert(
        is_storeable<DfsTreeParentFieldTag, DataInterface, VertexEntityType>,
        "Must be able to store DFS tree parent "
        "field to vertex entities."
    );
    static_assert(
        is_loadable<StackNextFieldTag, DataInterface, VertexEntityType>,
        "Must be able to load the stack next field "
        "from vertex entities."
    );
    static_assert(
        is_storeable<StackNextFieldTag, DataInterface, VertexEntityType>,
        "Must be able to store the stack next field "
        "to vertex entities."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                OutEdgeBeginFieldTag>,
            EdgeEntityType>,
        "Out-edge-begin field must be edge "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                OutEdgeEndFieldTag>,
            EdgeEntityType>,
        "Out-edge-end field must be edge "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                EdgeTargetFieldTag>,
            VertexEntityType>,
        "Edge target field must be vertex "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                DfsTreeParentFieldTag>,
            VertexEntityType>,
        "DFS tree parent field must be vertex "
        "type."
    );
    static_assert(
        std::is_same_v<
            FieldType<
                StackNextFieldTag>,
            VertexEntityType>,
        "The stack next field must be vertex "
        "type."
    );

    /*
    * Reset all colours.
    */
    for (VertexEntityType v = vertex_begin;
        v != vertex_end;
        v = next(v))
    {
        store<ColourFieldTag>(
            data_interface, v, UNEXPLORED);
    }

    /*
    * Loop through vertices in the
    * graph.
    */
    for (VertexEntityType root = vertex_begin;
        root != vertex_end;
        root = next(root))
    {
        const ColourType root_old_colour
            = load<ColourFieldTag>(
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
        store<DfsTreeParentFieldTag>(
            data_interface, root, root);
        store<StackNextFieldTag>(
            data_interface, root, root);

        /*
        * Begin depth-first search from this
        * vertex.
        */
        VertexEntityType stack = root;
        store<ColourFieldTag>(
            data_interface, stack, EXPLORING);

        while (load<ColourFieldTag>(
                data_interface, stack)
            != EXPLORED)
        {
            const VertexEntityType source = stack;

            /*
            * Expected the stack to consist of precisely
            * the set of `EXPLORING` vertices.
            */
            #ifndef NDEBUG
            const ColourType source_colour
                = load<ColourFieldTag>(
                    data_interface, source);
            assert(
                source_colour == EXPLORING
            );
            #endif  // NDEBUG

            /*
            * Pop from stack.
            */
            stack = load<StackNextFieldTag>(
                data_interface, stack);

            const EdgeEntityType
                out_begin = load<OutEdgeBeginFieldTag>(
                    data_interface, stack),
                out_end = load<OutEdgeEndFieldTag>(
                    data_interface, stack);

            /*
            * Loop through the out-edges of
            * this vertex.
            */
            for (EdgeEntityType e = out_begin;
                e != out_end;
                e = next(e))
            {
                const VertexEntityType target
                    = load<EdgeTargetFieldTag>(
                        data_interface, e);

                const ColourType old_colour
                    = load<ColourFieldTag>(
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
                store<ColourFieldTag>(
                    data_interface, target, EXPLORING);
                store<DfsTreeParentFieldTag>(
                    data_interface, target, source);
                store<StackNextFieldTag>(
                    data_interface, target, stack);
                stack = target;
            }
        }
    }
}


}  // namespace graph
}  // namespace di


#endif  // DATA_INTERFACE_GRAPH_DFS_LINKED_STACK_IPP

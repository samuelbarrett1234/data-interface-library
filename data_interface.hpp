#ifndef DATA_INTERFACE_HPP
#define DATA_INTERFACE_HPP


#include <cstddef>
#include <type_traits>


namespace di
{


/*
* "Detection idiom" helper.
*/
template<typename ...>
using void_t = void;


/*
* Users MUST overload this to contain a
* `Type` type which maps to the type of
* this field in the context of this data
* interface.
*/
template<typename FieldTag, typename DataInterface>
struct FieldTypeImpl
{
    static_assert(
        false,
        "This type of data interface does not "
        "support this type of field."
    );
};


/*
* Users MUST overload this to contain a
* `Type` type which maps to the type of
* this entity in the context of this data
* interface.
*/
template<typename EntityTag, typename DataInterface>
struct EntityTypeImpl
{
    static_assert(
        false,
        "This type of data interface does not "
        "support this type of entity."
    );
};


/*
* Helper to extract the field type.
*/
template<typename FieldTag, typename DataInterface>
using FieldType = typename FieldTypeImpl<FieldTag, DataInterface>::Type;


/*
* Helper to extract the entity value type.
*/
template<typename EntityTag, typename DataInterface>
using EntityType = typename EntityTypeImpl<EntityTag, DataInterface>::Type;


/*
* Load the given field from the given entity
* in the given data interface. This should
* ALWAYS be inlined (or inlinable.) It should
* also be `noexcept`, but feel free to write
* assertions which check that the load is
* valid.
*/
template<typename FieldTag, typename EntityTag, typename DataInterface>
inline FieldType<FieldTag, DataInterface>
    load(
        const DataInterface&,
        EntityType<EntityTag, DataInterface>)
    noexcept
{
    static_assert(
        false,
        "This type of entity does not support "
        "reading of this field in the context "
        "of this data interface."
    );
}


/*
* Store the given value for the given field
* to the given entity in the given data
* interface. This should ALWAYS be inlined
* (or inlinable.) It should also be `noexcept`,
* but feel free to write assertions which
* check that the load is valid.
*/
template<typename FieldTag, typename EntityTag, typename DataInterface>
inline void
    store(
        DataInterface&,
        EntityType<EntityTag, DataInterface>,
        FieldType<FieldTag, DataInterface>)
    noexcept
{
    static_assert(
        false,
        "This type of entity does not support "
        "writing of this field in the context "
        "of this data interface."
    );
}


/*
* Get the next entity ("increment".)
*/
template<typename EntityTag, typename DataInterface>
inline EntityType<EntityTag, DataInterface>
    next(
        const DataInterface&,
        EntityType<EntityTag, DataInterface>)
    noexcept
{
    static_assert(
        false,
        "This type of entity does not support "
        "increment in the context of this data "
        "interface."
    );
}


/*
* Get the previous entity ("increment".)
*/
template<typename EntityTag, typename DataInterface>
inline EntityType<EntityTag, DataInterface>
    prev(
        const DataInterface&,
        EntityType<EntityTag, DataInterface>)
    noexcept
{
    static_assert(
        false,
        "This type of entity does not support "
        "decrement in the context of this data "
        "interface."
    );
}


/*
* Advance the entity by some value ("random
* access".)
*
* NOTE: `advance` by 1 must be equivalent
* to `next`. Likewise, `advance` by -1
* must be equivalent to `prev`.
*/
template<typename EntityTag, typename DataInterface>
inline EntityType<EntityTag, DataInterface>
    advance(
        const DataInterface&,
        EntityType<EntityTag, DataInterface>,
        std::ptrdiff_t)
    noexcept
{
    static_assert(
        false,
        "This type of entity does not support "
        "random access in the context of this data "
        "interface."
    );
}


/*
* Take the difference between two entities.
*
* NOTE: this is not just any old distance,
* but is specifically the inverse of
* `advance`: what would you have to advance
* the former by to get the latter?
*
* Note that the return value of this can
* be negative.
*/
template<typename EntityTag, typename DataInterface>
inline std::ptrdiff_t
    difference(
        const DataInterface&,
        EntityType<EntityTag, DataInterface>,
        EntityType<EntityTag, DataInterface>)
    noexcept
{
    static_assert(
        false,
        "This type of entity does not support "
        "taking differences."
    );
}


/*
* Compare a pair of entities. This is
* typically only possible when
* `advance` is also implemented.
*
* NOTE: this is NOT comparison by an
* arbitrary key. It is specifically
* comparison with respect to the order
* induced by `advance`/`next`/`prev`.
*/
template<typename EntityTag, typename DataInterface>
inline bool
    less(
        const DataInterface&,
        EntityType<EntityTag, DataInterface>,
        EntityType<EntityTag, DataInterface>)
    noexcept
{
    static_assert(
        false,
        "This type of entity does not support "
        "comparison."
    );
}


/*
* What follows is a helper using the
* "detection idiom" which checks whether
* a given field can be loaded from a given
* entity in the context of a given data
* interface.
*/
template<typename FieldTag, typename EntityTag, typename DataInterface, typename = void>
struct IsLoadableImpl : std::false_type {};


template<typename FieldTag, typename EntityTag, typename DataInterface>
struct IsLoadableImpl<
        FieldTag, EntityTag, DataInterface,
        void_t<decltype(load<FieldTag>(
            std::declval<DataInterface&>(),
            std::declval<EntityType<EntityTag, DataInterface>>()))>>
    : std::true_type
{ };


template<typename FieldTag, typename EntityTag, typename DataInterface>
inline constexpr bool is_loadable = IsLoadableImpl<FieldTag, EntityTag, DataInterface>::value;


/*
* Corresponding version for storing.
*/
template<typename FieldTag, typename EntityTag, typename DataInterface, typename = void>
struct IsStoreableImpl : std::false_type {};


template<typename FieldTag, typename EntityTag, typename DataInterface>
struct IsStoreableImpl<
        FieldTag, EntityTag, DataInterface,
        void_t<decltype(store<FieldTag>(
            std::declval<DataInterface&>(),
            std::declval<EntityType<EntityTag, DataInterface>>(),
            std::declval<FieldType<FieldTag, DataInterface>>()))>>
    : std::true_type
{ };


template<typename FieldTag, typename EntityTag, typename DataInterface>
inline constexpr bool is_storeable = IsStoreableImpl<FieldTag, EntityTag, DataInterface>::value;


/*
* Corresponding version for `next`.
*/
template<typename EntityTag, typename DataInterface, typename = void>
struct IsNextableImpl : std::false_type {};


template<typename EntityTag, typename DataInterface>
struct IsNextableImpl<
        EntityTag, DataInterface,
        void_t<decltype(next(
            std::declval<DataInterface&>(),
            std::declval<EntityType<EntityTag, DataInterface>>()))>>
    : std::true_type
{ };


template<typename EntityTag, typename DataInterface>
inline constexpr bool is_nextable = IsNextableImpl<EntityTag, DataInterface>::value;


/*
* Corresponding version for `prev`.
*/
template<typename EntityTag, typename DataInterface, typename = void>
struct IsPrevableImpl : std::false_type {};


template<typename EntityTag, typename DataInterface>
struct IsPrevableImpl<
        EntityTag, DataInterface,
        void_t<decltype(prev(
            std::declval<DataInterface&>(),
            std::declval<EntityType<EntityTag, DataInterface>>()))>>
    : std::true_type
{ };


template<typename EntityTag, typename DataInterface>
inline constexpr bool is_prevable = IsPrevableImpl<EntityTag, DataInterface>::value;


/*
* Corresponding version for `advance`.
*/
template<typename EntityTag, typename DataInterface, typename = void>
struct IsAdvanceableImpl : std::false_type {};


template<typename EntityTag, typename DataInterface>
struct IsAdvanceableImpl<
        EntityTag, DataInterface,
        void_t<decltype(advance(
            std::declval<DataInterface&>(),
            std::declval<EntityType<EntityTag, DataInterface>>(),
            std::declval<std::ptrdiff_t>()))>>
    : std::true_type
{ };


template<typename EntityTag, typename DataInterface>
inline constexpr bool is_advanceable = IsAdvanceableImpl<EntityTag, DataInterface>::value;


/*
* Corresponding version for `difference`.
*/
template<typename EntityTag, typename DataInterface, typename = void>
struct IsDiffableImpl : std::false_type {};


template<typename EntityTag, typename DataInterface>
struct IsDiffableImpl<
        EntityTag, DataInterface,
        void_t<decltype(difference(
            std::declval<DataInterface&>(),
            std::declval<EntityType<EntityTag, DataInterface>>(),
            std::declval<EntityType<EntityTag, DataInterface>>()))>>
    : std::true_type
{ };


template<typename EntityTag, typename DataInterface>
inline constexpr bool is_diffable = IsDiffableImpl<EntityTag, DataInterface>::value;


/*
* Corresponding version for `less`.
*/
template<typename EntityTag, typename DataInterface, typename = void>
struct IsComparableImpl : std::false_type {};


template<typename EntityTag, typename DataInterface>
struct IsComparableImpl<
        EntityTag, DataInterface,
        void_t<decltype(less(
            std::declval<DataInterface&>(),
            std::declval<EntityType<EntityTag, DataInterface>>(),
            std::declval<EntityType<EntityTag, DataInterface>>()))>>
    : std::true_type
{ };


template<typename EntityTag, typename DataInterface>
inline constexpr bool is_comparable = IsComparableImpl<EntityTag, DataInterface>::value;


/*
* And of course, we can implement all of
* the other comparators in terms of this
* one.
*/
template<typename EntityTag, typename DataInterface>
inline bool
    greater(
        const DataInterface& data_interface,
        EntityType<EntityTag, DataInterface> a,
        EntityType<EntityTag, DataInterface> b)
    noexcept
{
    static_assert(
        is_comparable<EntityTag, DataInterface>,
        "This entity, in the context of this data "
        "interface, must have `less` implemented "
        "in order to access any of the other "
        "comparators."
    );
    return less(data_interface, b, a);
}
template<typename EntityTag, typename DataInterface>
inline bool
    greater_equal(
        const DataInterface& data_interface,
        EntityType<EntityTag, DataInterface> a,
        EntityType<EntityTag, DataInterface> b)
    noexcept
{
    static_assert(
        is_comparable<EntityTag, DataInterface>,
        "This entity, in the context of this data "
        "interface, must have `less` implemented "
        "in order to access any of the other "
        "comparators."
    );
    return !less(data_interface, a, b);
}
template<typename EntityTag, typename DataInterface>
inline bool
    less_equal(
        const DataInterface& data_interface,
        EntityType<EntityTag, DataInterface> a,
        EntityType<EntityTag, DataInterface> b)
    noexcept
{
    static_assert(
        is_comparable<EntityTag, DataInterface>,
        "This entity, in the context of this data "
        "interface, must have `less` implemented "
        "in order to access any of the other "
        "comparators."
    );
    return !greater(data_interface, a, b);
}


}  // namespace di


#endif  // DATA_INTERFACE_HPP

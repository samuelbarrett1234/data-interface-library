#ifndef DATA_INTERFACE_HPP
#define DATA_INTERFACE_HPP


#include <cstddef>
#include <type_traits>
#include <utility>


namespace di
{


/*
* Users' field tag types should have a nested
* `Type` specifier which says what the type is
* that _actually_ represents a value for that
* field.
*/
template<typename FieldTag>
using FieldType = typename FieldTag::Type;


/*
* Concept: a field is loadable if an unqualified
* call `load_impl(di, entity, FieldTag{})` is
* valid via ADL and returns exactly
* `FieldType<FieldTag>`.
*/
template<typename FieldTag, typename DataInterface, typename Entity>
concept is_loadable
    = requires(const DataInterface& di, Entity e)
{
    { load_impl(di, e, FieldTag{}) }
        -> std::same_as<FieldType<FieldTag>>;
};


/*
* Used for loading a field from an entity in the
* context of a given data interface.
*
* Usage: `auto v = load<FieldTag>(di, entity);`
*/
template<
    typename FieldTag,
    typename DataInterface,
    typename Entity>
requires (
    is_loadable<FieldTag, DataInterface, Entity>)
constexpr inline auto load(
    const DataInterface& di, Entity e)
noexcept(
    noexcept(load_impl(di, e, FieldTag{})))
-> FieldType<FieldTag>
{
    return load_impl(di, e, FieldTag{});
}


/*
* Concept: a field is storable if an unqualified
* call `store_impl(di, entity, value, FieldTag{})`
* is valid via ADL and returns `void`.
*/
template<typename FieldTag, typename DataInterface, typename Entity>
concept is_storeable
    = requires(DataInterface& di, Entity e, FieldType<FieldTag> v)
{
    { store_impl(di, e, v, FieldTag{}) } -> std::same_as<void>;
};


/*
* Used for storing a value for a field into an
* entity in the context of a given data interface.
*
* Usage: `store<FieldTag>(di, entity, value);`
*/
template<
    typename FieldTag,
    typename DataInterface,
    typename Entity>
requires (
    is_storeable<FieldTag, DataInterface, Entity>)
constexpr inline void store(
    DataInterface& di,
    Entity e,
    FieldType<FieldTag> value)
noexcept(
    noexcept(store_impl(di, e, value, FieldTag{})))
{
    store_impl(di, e, value, FieldTag{});
}


template<typename E>
concept is_nextable
    = requires(E e) {
        { next_impl(e) } -> std::same_as<E>; };


/*
* Function for incrementing an entity.
*
* Usage: `e2 = next(e1)`.
*/
template<typename E>
requires (is_nextable<E>)
constexpr inline auto next(E e)
noexcept(noexcept(next_impl(e)))
-> E
{
    return next_impl(e);
}


template<typename E>
concept is_prevable
    = requires(E e) {
        { prev_impl(e) } -> std::same_as<E>; };


/*
* Function for decrementing an entity.
*
* Usage: `e2 = prev(e1)`.
*/
template<typename E>
requires (is_prevable<E>)
constexpr inline auto prev(E e)
noexcept(noexcept(prev_impl(e)))
-> E
{
    return prev_impl(e);
}


template<typename E>
concept is_advanceable
    = requires(E e, std::ptrdiff_t d) {
        { advance_impl(e, d) } -> std::same_as<E>; };


/*
* Function for advancing an entity. Corresponds
* to zero or more `next`s or `prev`s, but in
* a single O(1) call.
*
* Usage: `e2 = advance(e1, d)`.
*/
template<typename E>
requires (is_advanceable<E>)
constexpr inline auto advance(E e, std::ptrdiff_t d)
noexcept(noexcept(advance_impl(e, d)))
-> E
{
    return advance_impl(e, d);
}


template<typename E>
concept is_diffable
    = requires(E e1, E e2) {
        { diff_impl(e1, e2) } -> std::same_as<std::ptrdiff_t>; };


/*
* Function for taking the difference in position
* of two entities.
*
* Usage: `d = diff(e1, e2)`.
*
* If the return value is 0, it must be that `e1 == e2`.
* Else if the return value is `n > 0`, then `n` calls
* to `next` on `e1` must result in `e2. Else if the
* return value is `-n < 0`, then `n` calls to `prev`
* on `e1` must result in `e2`.
*/
template<typename E>
requires (is_diffable<E>)
constexpr inline auto diff(E e1, E e2)
noexcept(noexcept(diff_impl(e1, e2)))
-> std::ptrdiff_t
{
    return diff_impl(e1, e2);
}


template<typename E>
concept is_comparable
    = requires(E e1, E e2) {
        { less_impl(e1, e2) } -> std::same_as<bool>; };


/*
* Function for comparing two entities.
* If `e1` is less than `e2` it means
* that 1 or more calls to `next` on `e1`
* will eventually result in `e2`.
*
* Usage: `if (less(e1, e2)) {}`.
*/
template<typename E>
requires (is_comparable<E>)
constexpr inline auto less(E e1, E e2)
noexcept(noexcept(less_impl(e1, e2)))
-> bool
{
    return less_impl(e1, e2);
}


/*
* And of course, we can implement all of
* the other comparators in terms of this
* one.
*/
template<typename EntityType>
requires (is_comparable<EntityType>)
inline bool greater(EntityType e1, EntityType e2)
    noexcept(noexcept(less(e2, e1)))
{
    return less(e2, e1);
}
template<typename EntityType>
requires (is_comparable<EntityType>)
inline bool greater_equal(EntityType e1, EntityType e2)
    noexcept(noexcept(less(e1, e2)))
{
    return !less(e1, e2);
}
template<typename EntityType>
requires (is_comparable<EntityType>)
inline bool less_equal(EntityType e1, EntityType e2)
    noexcept(noexcept(greater(e1, e2)))
{
    return !greater(e1, e2);
}


/*****************************************
* MEMBER WRAPPER FOR POINTERS TO STRUCTS *
*                                        *
* Example usage:                         *
*                                        *
* ```                                    *
* struct AgeTag{};                       *
* struct HeightTag{};                    *
*                                        *
* struct Person                          *
* {                                      *
*     int age;                           *
*     float height;                      *
* };                                     *
*                                        *
* using DI = MemberFieldDI<              *
*         MemberFieldDI<                 *
*         SomeUnderlyingDI,              *
*         Person,                        *
*         &Person::age,                  *
*         AgeTag                         *
*     >,                                 *
*     Person,                            *
*     &Person::height,                   *
*     HeightTag                          *
* >;                                     *
* ```                                    *
*****************************************/

template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename NewFieldTag
>
struct MemberFieldDI
{
    UnderlyingDI underlying;
};

template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename NewFieldTag
>
auto load_impl(
    const MemberFieldDI<UnderlyingDI, T, member_ptr, NewFieldTag>&,
    T* obj,
    NewFieldTag)
-> FieldType<NewFieldTag>
{
    return obj->*member_ptr;
}
template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename NewFieldTag,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (is_loadable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
auto load_impl(
    const MemberFieldDI<UnderlyingDI, T, member_ptr, NewFieldTag>& di,
    QueryEntityType e,
    QueryFieldTag tag)
-> FieldType<QueryFieldTag>
{
    return load_impl(di.underlying, e, tag);
}
template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename NewFieldTag
>
void store_impl(
    MemberFieldDI<UnderlyingDI, T, member_ptr, NewFieldTag>&,
    T* obj,
    FieldType<NewFieldTag> value,
    NewFieldTag)
{
    obj->*member_ptr = value;
}
template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename NewFieldTag,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (is_storeable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
void store_impl(
    MemberFieldDI<UnderlyingDI, T, member_ptr, NewFieldTag>& di,
    QueryEntityType e,
    FieldType<QueryFieldTag> value,
    QueryFieldTag tag)
{
    store_impl(di.underlying, e, value, tag);
}


/*****************************************
*      ENTITY MAPPER DATA INTERFACE      *
*                                        *
* Used to map an entity of type E1 to    *
* an entity of type E2 using a possibly- *
* stateful functor. This could be useful *
* for example with arrays, where E1 is   *
* an index type and E2 is a pointer type *
* and the functor holds a pointer to the *
* first array element in order to do     *
* this translation.                      *
*****************************************/


template<
    typename UnderlyingDI,
    typename FromEntityType,
    typename ToEntityType,
    typename EntityMapFunctor
>
struct EntityMapperDataInterface
{
    static_assert(
        std::is_invocable_r_v<ToEntityType, EntityMapFunctor, FromEntityType>,
        "If `EntityMapFunctor` is called with a "
        "`FromEntityType` it should return a `ToEntityType`."
    );

    UnderlyingDI underlying;
    EntityMapFunctor entity_map;
};


template<
    typename UnderlyingDI,
    typename FromEntityType,
    typename ToEntityType,
    typename EntityMapFunctor,
    typename QueryFieldTag
>
requires (is_loadable<QueryFieldTag, UnderlyingDI, ToEntityType>)
auto load_impl(
    const EntityMapperDataInterface<UnderlyingDI, FromEntityType, ToEntityType, EntityMapFunctor>& di,
    FromEntityType e1,
    QueryFieldTag tag)
-> FieldType<QueryFieldTag>
{
    const ToEntityType e2 = di.entity_map(e1);
    return load_impl(di.underlying, e2, tag);
}
template<
    typename UnderlyingDI,
    typename FromEntityType,
    typename ToEntityType,
    typename EntityMapFunctor,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (is_loadable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
auto load_impl(
    const EntityMapperDataInterface<UnderlyingDI, FromEntityType, ToEntityType, EntityMapFunctor>& di,
    QueryEntityType e,
    QueryFieldTag tag)
-> FieldType<QueryFieldTag>
{
    return load_impl(di.underlying, e, tag);
}
template<
    typename UnderlyingDI,
    typename FromEntityType,
    typename ToEntityType,
    typename EntityMapFunctor,
    typename QueryFieldTag
>
requires (is_storeable<QueryFieldTag, UnderlyingDI, ToEntityType>)
auto store_impl(
    EntityMapperDataInterface<UnderlyingDI, FromEntityType, ToEntityType, EntityMapFunctor>& di,
    FromEntityType e1,
    FieldType<QueryFieldTag> v,
    QueryFieldTag tag)
{
    const ToEntityType e2 = di.entity_map(e1);
    return store_impl(di.underlying, e2, v, tag);
}
template<
    typename UnderlyingDI,
    typename FromEntityType,
    typename ToEntityType,
    typename EntityMapFunctor,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (is_storeable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
auto store_impl(
    EntityMapperDataInterface<UnderlyingDI, FromEntityType, ToEntityType, EntityMapFunctor>& di,
    QueryEntityType e,
    FieldType<QueryFieldTag> v,
    QueryFieldTag tag)
{
    return store_impl(di.underlying, e, v, tag);
}


/*****************************************
*      DISJOINT UNION DATA INTERFACE     *
*                                        *
* This allows you to merge 2 underlying  *
* data interfaces, provided they don't   *
* _both_ provide definitions of a given  *
* field (that'd be ambiguous.)           *
*****************************************/


template<typename DI1, typename DI2>
struct DisjointUnionDI
{
    DI1 di1;
    DI2 di2;
};


template<
    typename DI1,
    typename DI2,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (
    is_loadable<QueryFieldTag, DI1, QueryEntityType>
    || is_loadable<QueryFieldTag, DI2, QueryEntityType>)
auto load_impl(
    const DisjointUnionDI<DI1, DI2>& di,
    QueryEntityType e,
    QueryFieldTag tag)
-> FieldType<QueryFieldTag>
{
    if constexpr (is_loadable<QueryFieldTag, DI1, QueryEntityType>)
    {
        static_assert(
            !is_loadable<QueryFieldTag, DI2, QueryEntityType>,
            "This field should be managed by "
            "only 1 underlying DI."
        );
        static_assert(
            !is_storeable<QueryFieldTag, DI2, QueryEntityType>,
            "This field should be managed by "
            "only 1 underlying DI."
        );

        return load_impl(di.di1, e, tag);
    }
    else
    {
        static_assert(
            !is_storeable<QueryFieldTag, DI1, QueryEntityType>,
            "This field should be managed by "
            "only 1 underlying DI."
        );

        return load_impl(di.di2, e, tag);
    }
}


template<
    typename DI1,
    typename DI2,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (
    is_storeable<QueryFieldTag, DI1, QueryEntityType>
    || is_storeable<QueryFieldTag, DI2, QueryEntityType>)
void store_impl(
    DisjointUnionDI<DI1, DI2>& di,
    QueryEntityType e,
    FieldType<QueryFieldTag> value,
    QueryFieldTag tag)
{
    if constexpr (is_storeable<QueryFieldTag, DI1, QueryEntityType>)
    {
        static_assert(
            !is_loadable<QueryFieldTag, DI2, QueryEntityType>,
            "This field should be managed by "
            "only 1 underlying DI."
        );
        static_assert(
            !is_storeable<QueryFieldTag, DI2, QueryEntityType>,
            "This field should be managed by "
            "only 1 underlying DI."
        );

        return store_impl(di.di1, e, value, tag);
    }
    else
    {
        static_assert(
            !is_loadable<QueryFieldTag, DI1, QueryEntityType>,
            "This field should be managed by "
            "only 1 underlying DI."
        );

        return store_impl(di.di2, e, value, tag);
    }
}


/*****************************************
*              TRIVIAL FIELDS            *
*                                        *
* Here are two trivial data interfaces:  *
* one where loading a field always       *
* returns a given constant; and one      *
* storing a field is a no-op.            *
*****************************************/


template<
    typename UnderlyingDI,
    typename EntityForNewFieldType,
    typename NewFieldTag
>
struct ConstantLoadDI
{
    static_assert(
        !is_storeable<
            NewFieldTag,
            UnderlyingDI,
            EntityForNewFieldType
        >,
        "It's probably best you don't have a constant "
        "load value if the field is also storeable, "
        "as this will lead to inconsistent results."
    );

    UnderlyingDI underlying;
    FieldType<NewFieldTag> constant;
};

template<
    typename UnderlyingDI,
    typename EntityForNewFieldType,
    typename NewFieldTag
>
auto load_impl(
    const ConstantLoadDI<UnderlyingDI, EntityForNewFieldType, NewFieldTag>& di,
    EntityForNewFieldType,
    NewFieldTag)
-> FieldType<NewFieldTag>
{
    return di.constant;
}
template<
    typename UnderlyingDI,
    typename EntityForNewFieldType,
    typename NewFieldTag,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (is_loadable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
auto load_impl(
    const ConstantLoadDI<UnderlyingDI, EntityForNewFieldType, NewFieldTag>& di,
    QueryEntityType e,
    QueryFieldTag tag)
-> FieldType<QueryFieldTag>
{
    return load_impl(di.underlying, e, tag);
}

template<
    typename UnderlyingDI,
    typename EntityForNewFieldType,
    typename NewFieldTag
>
struct NoopStoreDI
{
    static_assert(
        !is_loadable<
            NewFieldTag,
            UnderlyingDI,
            EntityForNewFieldType
        >,
        "It's probably best you don't have a no-op "
        "store value if the field is also loadable, "
        "as this will lead to inconsistent results."
    );

    UnderlyingDI underlying;
};

template<
    typename UnderlyingDI,
    typename EntityForNewFieldType,
    typename NewFieldTag
>
void store_impl(
    NoopStoreDI<UnderlyingDI, EntityForNewFieldType, NewFieldTag>&,
    EntityForNewFieldType,
    FieldType<NewFieldTag>,
    NewFieldTag)
{
    /* no-op! */
}
template<
    typename UnderlyingDI,
    typename EntityForNewFieldType,
    typename NewFieldTag,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (is_storeable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
void store_impl(
    NoopStoreDI<UnderlyingDI, EntityForNewFieldType, NewFieldTag>& di,
    QueryEntityType e,
    FieldType<QueryFieldTag> value,
    QueryFieldTag tag)
{
    store_impl(di.underlying, e, value, tag);
}


/*****************************************
*     FIELD TRANSFORM DATA INTERFACE     *
*                                        *
* This makes it appear as though an      *
* entity has a new field, based on a     *
* function of one of its other fields.   *
* If you also have a corresponding       *
* inverse function, you can store as     *
* well!                                  *
*****************************************/


template<
    typename UnderlyingDI,
    typename OldFieldTag,
    typename NewFieldTag,
    typename ForwardFunction,
    typename BackwardFunction = void
>
struct TransformDI
{
    static_assert(
        std::is_invocable_r_v<FieldType<NewFieldTag>, ForwardFunction, FieldType<OldFieldTag>>,
        "Forward function must be callable with the "
        "value of the old field and must return a "
        "value of the new field."
    );
    static_assert(
        std::is_same_v<BackwardFunction, void>
        ||
        std::is_invocable_r_v<FieldType<OldFieldTag>, BackwardFunction, FieldType<NewFieldTag>>,
        "Backward function must be callable with the "
        "value of the new field and must return a "
        "value of the old field."
    );

    UnderlyingDI underlying;
    ForwardFunction forward_function;
    BackwardFunction backward_function;
};

template<
    typename UnderlyingDI,
    typename OldFieldTag,
    typename NewFieldTag,
    typename ForwardFunction,
    typename BackwardFunction,
    typename QueryEntityType
>
requires (is_loadable<OldFieldTag, UnderlyingDI, QueryEntityType>)
auto load_impl(
    const TransformDI<UnderlyingDI, OldFieldTag, NewFieldTag, ForwardFunction, BackwardFunction>& di,
    QueryEntityType e,
    NewFieldTag)
-> FieldType<NewFieldTag>
{
    static_assert(
        !is_loadable<NewFieldTag, UnderlyingDI, QueryEntityType>,
        "This entity already had a loader for this field! "
        "Replacing it with this new one is ambiguous."
    );
    static_assert(
        !is_storeable<NewFieldTag, UnderlyingDI, QueryEntityType>,
        "This entity already had a storer for this field! "
        "Replacing it with this new one is ambiguous."
    );

    /*
    * Load old field then use forward function.
    */
    return di.forward_function(load<OldFieldTag>(di.underlying, e));
}
template<
    typename UnderlyingDI,
    typename OldFieldTag,
    typename NewFieldTag,
    typename ForwardFunction,
    typename BackwardFunction,
    typename QueryFieldTag,
    typename QueryEntityType
>
requires (is_loadable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
auto load_impl(
    const TransformDI<UnderlyingDI, OldFieldTag, NewFieldTag, ForwardFunction, BackwardFunction>& di,
    QueryEntityType e,
    QueryFieldTag tag)
-> FieldType<QueryFieldTag>
{
    return load_impl(di.underlying, e, tag);
}

template<
    typename UnderlyingDI,
    typename OldFieldTag,
    typename NewFieldTag,
    typename ForwardFunction,
    typename BackwardFunction,
    typename QueryEntityType
>
requires (
    !std::is_same_v<BackwardFunction, void>
    && is_storeable<OldFieldTag, UnderlyingDI, QueryEntityType>)
void store_impl(
    TransformDI<UnderlyingDI, OldFieldTag, NewFieldTag, ForwardFunction, BackwardFunction>& di,
    QueryEntityType e,
    FieldType<NewFieldTag> value,
    NewFieldTag)
{
    static_assert(
        !is_loadable<NewFieldTag, UnderlyingDI, QueryEntityType>,
        "This entity already had a loader for this field! "
        "Replacing it with this new one is ambiguous."
    );
    static_assert(
        !is_storeable<NewFieldTag, UnderlyingDI, QueryEntityType>,
        "This entity already had a storer for this field! "
        "Replacing it with this new one is ambiguous."
    );

    /*
    * Use backwards function then store old field.
    */
    store<OldFieldTag>(di.underlying, e, di.backward_function(value));
}
template<
    typename UnderlyingDI,
    typename OldFieldTag,
    typename NewFieldTag,
    typename ForwardFunction,
    typename BackwardFunction,
    typename QueryFieldTag,
    typename QueryEntityType
>
requires (is_storeable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
void store_impl(
    TransformDI<UnderlyingDI, OldFieldTag, NewFieldTag, ForwardFunction, BackwardFunction>& di,
    QueryEntityType e,
    FieldType<QueryFieldTag> value,
    QueryFieldTag tag)
{
    store_impl(di.underlying, e, value, tag);
}


/*****************************************
*    INDIRECTION-LOOKUP DATA INTERFACE   *
*                                        *
* Suppose an entity E1 has a field which *
* references an entity E2. This foreign  *
* key could be followed, allowing any    *
* field on E2 to 'appear as though' it   *
* was a field on E1. This data interface *
* extends the fields of E1 accordingly.  *
* If a field exists on both E1 and E2,   *
* the E1 one takes precedence (i.e.      *
* there is no error.)                    *
*                                        *
* If there is a bijection between E1     *
* and E2 - namely the E2 is unique for   *
* each E1 - then you can also store to   *
* E2's fields, too!                      *
*****************************************/


template<
    typename UnderlyingDI,
    typename ForeignKeyFieldTag,
    typename EntityWithForeignKeyType,
    bool bijection = false
>
struct IndirectionLookupDI
{
    static_assert(
        is_loadable<ForeignKeyFieldTag, UnderlyingDI, EntityWithForeignKeyType>,
        "This entity does not have the given foreign "
        "key field."
    );

    UnderlyingDI underlying;
};

// Case 1: loads on `EntityWithForeignKeyType`s
template<
    typename UnderlyingDI,
    typename ForeignKeyFieldTag,
    typename EntityWithForeignKeyType,
    bool bijection,
    typename QueryFieldTag
>
requires (
    is_loadable<QueryFieldTag, UnderlyingDI, EntityWithForeignKeyType>
    || is_loadable<QueryFieldTag, UnderlyingDI, FieldType<ForeignKeyFieldTag>>)
auto load_impl(
    const IndirectionLookupDI<UnderlyingDI, ForeignKeyFieldTag, EntityWithForeignKeyType, bijection>& di,
    EntityWithForeignKeyType e,
    QueryFieldTag tag)
    -> FieldType<QueryFieldTag>
{
    if constexpr (is_loadable<QueryFieldTag, UnderlyingDI, EntityWithForeignKeyType>)
    {
        /*
        * Always give preference to original entity,
        * even if both entity and the foreign entity
        * can load this field - don't throw an error
        * in that case.
        */
        return load_impl(di.underlying, e, tag);
    }
    else
    {
        /*
        * Note: if the caller is trying to load several
        * fields on the foreign entity (transparently),
        * we are relying on the compiler to eliminate
        * duplicate loads here to be efficient, otherwise
        * we'd be loading the same `foreign_entity`
        * several times.
        */
        const auto foreign_entity
            = load<ForeignKeyFieldTag>(di.underlying, e);

        return load_impl(di.underlying, foreign_entity, tag);
    }
}

// Case 2: other loads
template<
    typename UnderlyingDI,
    typename ForeignKeyFieldTag,
    typename EntityWithForeignKeyType,
    bool bijection,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (is_loadable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
auto load_impl(
    const IndirectionLookupDI<UnderlyingDI, ForeignKeyFieldTag, EntityWithForeignKeyType, bijection>& di,
    QueryEntityType e,
    QueryFieldTag tag)
    -> FieldType<QueryFieldTag>
{
    return load_impl(di.underlying, e, tag);
}

// Case 1: stores on `EntityWithForeignKeyType`s
template<
    typename UnderlyingDI,
    typename ForeignKeyFieldTag,
    typename EntityWithForeignKeyType,
    bool bijection,
    typename QueryFieldTag
>
requires (
    /* ONLY implement if bijection! */
    bijection
    && (
        is_storeable<QueryFieldTag, UnderlyingDI, EntityWithForeignKeyType>
        || is_storeable<QueryFieldTag, UnderlyingDI, FieldType<ForeignKeyFieldTag>>))
auto store_impl(
    IndirectionLookupDI<UnderlyingDI, ForeignKeyFieldTag, EntityWithForeignKeyType, bijection>& di,
    EntityWithForeignKeyType e,
    FieldType<QueryFieldTag> v,
    QueryFieldTag tag)
{
    if constexpr (is_storeable<QueryFieldTag, UnderlyingDI, EntityWithForeignKeyType>)
    {
        static_assert(
            is_loadable<QueryFieldTag, UnderlyingDI, EntityWithForeignKeyType>
            || !is_loadable<QueryFieldTag, UnderlyingDI, FieldType<ForeignKeyFieldTag>>,
            "Field is storeable on `EntityWithForeignKeyType` but "
            "is only loadable on the foreign entity. This could "
            "lead to ambiguities, so is a compiler error."
        );

        /*
        * Always give preference to original entity,
        * even if both entity and the foreign entity
        * can store this field - don't throw an error
        * in that case.
        */
        return store_impl(di.underlying, e, v, tag);
    }
    else
    {
        static_assert(
            is_loadable<QueryFieldTag, UnderlyingDI, FieldType<ForeignKeyFieldTag>>
            || !is_loadable<QueryFieldTag, UnderlyingDI, EntityWithForeignKeyType>,
            "Field is storeable on the foreign entity but "
            "is only loadable on the `EntityWithForeignKeyType`. "
            "This could lead to ambiguities, so is a compiler error."
        );

        /*
        * Note: if the caller is trying to store several
        * fields on the foreign entity (transparently),
        * we are relying on the compiler to eliminate
        * duplicate loads here to be efficient, otherwise
        * we'd be loading the same `foreign_entity`
        * several times.
        */
        const auto foreign_entity
            = load<ForeignKeyFieldTag>(di.underlying, e);

        return store_impl(di.underlying, foreign_entity, v, tag);
    }
}

// Case 2: other stores
template<
    typename UnderlyingDI,
    typename ForeignKeyFieldTag,
    typename EntityWithForeignKeyType,
    bool bijection,
    typename QueryEntityType,
    typename QueryFieldTag
>
requires (is_storeable<QueryFieldTag, UnderlyingDI, QueryEntityType>)
auto store_impl(
    IndirectionLookupDI<UnderlyingDI, ForeignKeyFieldTag, EntityWithForeignKeyType, bijection>& di,
    QueryEntityType e,
    FieldType<QueryFieldTag> v,
    QueryFieldTag tag)
{
    return store_impl(di.underlying, e, v, tag);
}


/*****************************************
*        HELPER 'MAKER' FUNCTIONS        *
*                                        *
* You may find the functions below a     *
* more user-friendly way to construct    *
* data interfaces than using the raw     *
* structs above.                         *
*****************************************/


template<
    typename T,
    auto member_ptr,
    typename NewFieldTag,
    typename UnderlyingDI>
auto make_with_pointer_to_member_data_interface(
    UnderlyingDI di)
{
    return MemberFieldDI<
        UnderlyingDI, T, member_ptr, NewFieldTag
    >{
        .underlying = di
    };
}


template<
    typename FromEntityType,
    typename ToEntityType,
    typename UnderlyingDI,
    typename EntityMapFunctor
>
auto make_entity_mapper_data_interface(
    UnderlyingDI underlying, EntityMapFunctor entity_map)
{
    return EntityMapperDataInterface<UnderlyingDI, FromEntityType, ToEntityType, EntityMapFunctor>{
        .underlying = underlying,
        .entity_map = entity_map
    };
}


template<typename DI1, typename DI2>
auto make_disjoint_union_data_interface(
    DI1 di1, DI2 di2)
{
    return DisjointUnionDI<DI1, DI2>{ .di1 = di1, .di2 = di2 };
}


template<
    typename NewFieldTag,
    typename EntityForNewFieldType,
    typename NewFieldValueType,
    typename UnderlyingDI
>
auto make_with_constant_field(
    UnderlyingDI di,
    NewFieldValueType value)
{
    return ConstantLoadDI<
        UnderlyingDI,
        EntityForNewFieldType,
        NewFieldTag,
        NewFieldValueType
    >{
        .underlying = di,
        .constant = value
    };
}
template<
    typename NewFieldTag,
    typename EntityForNewFieldType,
    typename NewFieldValueType,
    typename UnderlyingDI
>
auto make_with_no_op_store(
    UnderlyingDI di)
{
    return NoopStoreDI<
        UnderlyingDI,
        EntityForNewFieldType,
        NewFieldTag,
        NewFieldValueType
    >{
        .underlying = di
    };
}


template<
    typename OldFieldTag,
    typename NewFieldTag,
    typename ForwardFunction,
    typename UnderlyingDI
>
auto make_with_mapped_field(
    UnderlyingDI di,
    ForwardFunction field_map)
{
    return TransformDI<
        UnderlyingDI,
        OldFieldTag,
        NewFieldTag,
        ForwardFunction
    >{
        .underlying = di,
        .forward_function = field_map
    };
}


template<
    typename OldFieldTag,
    typename NewFieldTag,
    typename ForwardFunction,
    typename BackwardFunction,
    typename UnderlyingDI
>
auto make_with_bijection_field(
    UnderlyingDI di,
    ForwardFunction field_map,
    BackwardFunction inverse_field_map)
{
    return TransformDI<
        UnderlyingDI,
        OldFieldTag,
        NewFieldTag,
        ForwardFunction,
        BackwardFunction
    >{
        .underlying = di,
        .forward_function = field_map,
        .backward_function = inverse_field_map
    };
}


template<
    typename ForeignKeyFieldTag,
    typename EntityWithForeignKeyType,
    bool bijection,
    typename UnderlyingDI>
auto make_with_indirection_lookup(
    UnderlyingDI di)
{
    return IndirectionLookupDI<
        UnderlyingDI,
        ForeignKeyFieldTag,
        EntityWithForeignKeyType,
        bijection
    >{
        .underlying = di
    };
}


}  // namespace di


#endif  // DATA_INTERFACE_HPP

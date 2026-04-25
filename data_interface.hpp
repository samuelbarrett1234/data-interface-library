#ifndef DATA_INTERFACE_HPP
#define DATA_INTERFACE_HPP


#include <cstddef>
#include <type_traits>
#include <utility>


namespace di
{


/*
* Users should specialise this struct
* for their fields, providing a nested
* `Type`, representing the actual type
* (e.g. `int`) of the underlying field.
*/
template<typename FieldTag, typename DataInterface>
struct FieldTypeImpl;


template<typename FieldTag, typename DataInterface>
using FieldType = typename FieldTypeImpl<FieldTag, DataInterface>::Type;


/*
* Concept: a field is loadable if an unqualified
* call `load_impl(di, entity, FieldTag{})` is
* valid via ADL and returns exactly
* `FieldType<FieldTag, DataInterface>`.
*/
template<typename FieldTag, typename DataInterface, typename Entity>
concept is_loadable
    = requires(const DataInterface& di, Entity e)
{
    { load_impl(di, e, FieldTag{}) }
        -> std::same_as<FieldType<FieldTag, DataInterface>>;
};


/*
* Customization Point Object (CPO) for loading fields.
*
* Usage: `auto v = load<FieldTag>(di, entity);`
*/
inline constexpr struct load_fn
{
    template<typename FieldTag, typename DataInterface, typename Entity>
        requires is_loadable<FieldTag, DataInterface, Entity>
    constexpr auto operator()(const DataInterface& di, Entity e) const
        noexcept(noexcept(load_impl(di, e, FieldTag{})))
        -> FieldType<FieldTag, DataInterface>
    {
        return load_impl(di, e, FieldTag{});
    }

    template<typename FieldTag, typename DataInterface, typename Entity>
    constexpr auto operator()(const DataInterface&, Entity) const
    {
        static_assert(
            is_loadable<FieldTag, DataInterface, Entity>,
            "`load<FieldTag>(di, entity)`: no matching "
            "`load_impl(di, entity, FieldTag{})` found via ADL, or "
            "return type does not match "
            "`FieldType<FieldTag, DataInterface>`."
        );
    }

} load;


/*
* Concept: a field is storable if an unqualified
* call `store_impl(di, entity, value, FieldTag{})`
* is valid via ADL and returns `void`.
*/
template<typename FieldTag, typename DataInterface, typename Entity>
concept is_storeable
    = requires(DataInterface& di, Entity e, FieldType<FieldTag, DataInterface> v)
{
    { store_impl(di, e, v, FieldTag{}) } -> std::same_as<void>;
};


/*
* Customization Point Object (CPO) for storing fields.
*
* Usage: `store<FieldTag>(di, entity, value);`
*/
inline constexpr struct store_fn
{
    template<typename FieldTag, typename DataInterface, typename Entity>
    requires is_storeable<FieldTag, DataInterface, Entity>
    constexpr void operator()(DataInterface& di,
                              Entity e,
                              FieldType<FieldTag, DataInterface> value) const
        noexcept(noexcept(store_impl(di, e, value, FieldTag{})))
    {
        store_impl(di, e, value, FieldTag{});
    }

    template<typename FieldTag, typename DataInterface, typename Entity>
    constexpr void operator()(
        DataInterface&, Entity, FieldType<FieldTag, DataInterface>) const
    {
        static_assert(
            is_storeable<FieldTag, DataInterface, Entity>,
            "store<FieldTag>(di, entity, value): no matching "
            "store_impl(di, entity, value, FieldTag{}) found via ADL, or "
            "signature/return type is incorrect."
        );
    }

} store;


/*
* Override this traits implementation
* providing any of the functionality you
* like, setting the corresponding
* `_exists` bools to true where you have
* done so.
*/
template<typename EntityType>
struct EntityImpl
{
    static inline constexpr bool next_exists = false;
    // static inline EntityType next_impl(EntityType e);

    static inline constexpr bool prev_exists = false;
    // static inline EntityType prev_impl(EntityType e);

    static inline constexpr bool advance_exists = false;
    // static inline EntityType advance_impl(EntityType e, std::ptrdiff_t d);

    static inline constexpr bool difference_exists = false;
    // static inline std::ptrdiff_t difference_impl(EntityType e1, EntityType e2);

    static inline constexpr bool less_exists = false;
    // static inline bool less_impl(EntityType e1, EntityType e2);
};


template<typename FieldTag, typename DataInterface>
using FieldType = typename FieldTypeImpl<FieldTag, DataInterface>::Type;


template<typename FieldTag, typename EntityType, typename DataInterface>
static inline constexpr bool is_loadable
    = LoadImpl<FieldTag, EntityType, DataInterface>::exists;


template<typename FieldTag, typename EntityType, typename DataInterface>
requires is_loadable<FieldTag, EntityType, DataInterface>
inline FieldType<FieldTag, DataInterface>
    load(DataInterface& di, EntityType e)
{
    return LoadImpl<FieldTag, EntityType, DataInterface>::impl(di, e);
}


template<typename FieldTag, typename EntityType, typename DataInterface>
static inline constexpr bool is_storeable
    = StoreImpl<FieldTag, EntityType, DataInterface>::exists;


template<typename FieldTag, typename EntityType, typename DataInterface>
requires is_storeable<FieldTag, EntityType, DataInterface>
inline void
    store(DataInterface& di, EntityType e, FieldType<FieldTag, DataInterface> f)
{
    return StoreImpl<FieldTag, EntityType, DataInterface>::impl(di, e, f);
}


template<typename EntityType>
static inline constexpr bool is_nextable
    = EntityImpl<EntityType>::next_exists;
template<typename EntityType>
static inline constexpr bool is_prevable
    = EntityImpl<EntityType>::prev_exists;
template<typename EntityType>
static inline constexpr bool is_advanceable
    = EntityImpl<EntityType>::advance_exists;
template<typename EntityType>
static inline constexpr bool is_diffable
    = EntityImpl<EntityType>::difference_exists;
template<typename EntityType>
static inline constexpr bool is_comparable
    = EntityImpl<EntityType>::less_exists;


template<typename EntityType>
requires is_nextable<EntityType>
inline EntityType next(EntityType e)
{
    return EntityImpl<EntityType>::next_impl(e);
}
template<typename EntityType>
requires is_prevable<EntityType>
inline EntityType prev(EntityType e)
{
    return EntityImpl<EntityType>::prev_impl(e);
}
template<typename EntityType>
requires is_advanceable<EntityType>
inline EntityType advance(EntityType e, std::ptrdiff_t d)
{
    return EntityImpl<EntityType>::advance_impl(e, d);
}
template<typename EntityType>
requires is_diffable<EntityType>
inline std::ptrdiff_t difference(EntityType e1, EntityType e2)
{
    return EntityImpl<EntityType>::difference_impl(e1, e2);
}
template<typename EntityType>
requires is_comparable<EntityType>
inline bool less(EntityType e1, EntityType e2)
{
    return EntityImpl<EntityType>::less_impl(e1, e2);
}


/*
* And of course, we can implement all of
* the other comparators in terms of this
* one.
*/
template<typename EntityType>
requires is_comparable<EntityType>
inline bool greater(EntityType e1, EntityType e2)
{
    return less(e2, e1);
}
template<typename EntityType>
requires is_comparable<EntityType>
inline bool greater_equal(EntityType e1, EntityType e2)
{
    return !less(e1, e2);
}
template<typename EntityType>
requires is_comparable<EntityType>
inline bool less_equal(EntityType e1, EntityType e2)
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
template<typename>
struct member_pointer_traits;
template<typename T, typename M>
struct member_pointer_traits<M T::*>
{
    using class_type  = T;
    using member_type = M;
};

template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename FieldTag
>
struct MemberFieldDI
{
    UnderlyingDI underlying;
};

// Case 1: the new field this wrapper introduces
template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename FieldTag
>
struct FieldTypeImpl<
    FieldTag,
    MemberFieldDI<UnderlyingDI, T, member_ptr, FieldTag>
>
{
    using Type
        = typename member_pointer_traits<decltype(member_ptr)>::member_type;
};

// Case 2: all other fields forward to underlying
template<
    typename OtherFieldTag,
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename FieldTag
>
struct FieldTypeImpl<
    OtherFieldTag,
    MemberFieldDI<UnderlyingDI, T, member_ptr, FieldTag>
>
{
    using Type = FieldType<OtherFieldTag, UnderlyingDI>;
};

template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename FieldTag
>
auto load_impl(
    const MemberFieldDI<UnderlyingDI, T, member_ptr, FieldTag>&,
    T* obj,
    FieldTag)
-> typename FieldType<
        FieldTag,
        MemberFieldDI<UnderlyingDI, T, member_ptr, FieldTag>
   >
{
    return obj->*member_ptr;
}
template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename FieldTag,
    typename Entity,
    typename OtherFieldTag
>
auto load_impl(
    const MemberFieldDI<UnderlyingDI, T, member_ptr, FieldTag>& wrapper,
    Entity e,
    OtherFieldTag tag)
-> FieldType<OtherFieldTag, UnderlyingDI>
{
    return load_impl(wrapper.underlying, e, tag);
}
template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename FieldTag
>
void store_impl(
    MemberFieldDI<UnderlyingDI, T, member_ptr, FieldTag>&,
    T* obj,
    FieldType<FieldTag, MemberFieldDI<UnderlyingDI, T, member_ptr, FieldTag>> value,
    FieldTag)
{
    obj->*member_ptr = value;
}
template<
    typename UnderlyingDI,
    typename T,
    auto member_ptr,
    typename FieldTag,
    typename Entity,
    typename OtherFieldTag
>
void store_impl(
    MemberFieldDI<UnderlyingDI, T, member_ptr, FieldTag>& wrapper,
    Entity e,
    FieldType<OtherFieldTag, UnderlyingDI> value,
    OtherFieldTag tag)
{
    store_impl(wrapper.underlying, e, value, tag);
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


template<typename FieldTag, typename DI1, typename DI2>
struct FieldTypeImpl<FieldTag, DisjointUnionDI<DI1, DI2>>
{
private:
    static constexpr bool has1
        = requires { typename FieldType<FieldTag, DI1>; };

    static constexpr bool has2
        = requires { typename FieldType<FieldTag, DI2>; };

    static_assert(
        has1 || has2,
        "Could not deduce type of this field based "
        "on underlyings."
    );
    static_assert(
        !(has1 && has2)
        || std::is_same_as_v<
            FieldType<FieldTag, DI1>,
            FieldType<FieldTag, DI2>
        >,
        "If both underlying DIs have a field type for "
        "this field, it must be consistent."
    );

public:
    using Type = std::conditional_t<
        has1,
        FieldType<FieldTag, DI1>,
        FieldType<FieldTag, DI2>
    >;
};


template<
    typename DI1,
    typename DI2,
    typename Entity,
    typename FieldTag
>
requires (is_loadable<FieldTag, DI1, Entity> || is_loadable<FieldTag, DI2, Entity>)
auto load_impl(
    const DisjointUnionDI<DI1, DI2>& di,
    Entity e,
    FieldTag tag)
-> FieldType<FieldTag, DisjointUnionDI<DI1, DI2>>
{
    if constexpr (is_loadable<FieldTag, DI1, Entity>)
    {
        static_assert(
            !is_loadable<FieldTag, DI2, Entity>,
            "This field should be managed by "
            "only 1 underlying DI."
        );
        static_assert(
            !is_storeable<FieldTag, DI2, Entity>,
            "This field should be managed by "
            "only 1 underlying DI."
        );

        return load_impl(di.di1, e, tag);
    }
    else
    {
        static_assert(
            !is_storeable<FieldTag, DI1, Entity>,
            "This field should be managed by "
            "only 1 underlying DI."
        );

        return load_impl(di.di2, e, tag);
    }
}


template<
    typename DI1,
    typename DI2,
    typename Entity,
    typename FieldTag
>
requires (is_storeable<FieldTag, DI1, Entity> || is_storeable<FieldTag, DI2, Entity>)
void store_impl(
    DisjointUnionDI<DI1, DI2>& di,
    Entity e,
    FieldType<FieldTag, DisjointUnionDI<DI1, DI2>> value,
    FieldTag tag)
{
    if constexpr (is_storeable<FieldTag, DI1, Entity>)
    {
        static_assert(
            !is_loadable<FieldTag, DI2, Entity>,
            "This field should be managed by "
            "only 1 underlying DI."
        );
        static_assert(
            !is_storeable<FieldTag, DI2, Entity>,
            "This field should be managed by "
            "only 1 underlying DI."
        );

        return store_impl(di.di1, e, value, tag);
    }
    else
    {
        static_assert(
            !is_loadable<FieldTag, DI1, Entity>,
            "This field should be managed by "
            "only 1 underlying DI."
        );

        return store_impl(di.di2, e, value, tag);
    }
}


}  // namespace di


#endif  // DATA_INTERFACE_HPP

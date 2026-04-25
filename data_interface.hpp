#ifndef DATA_INTERFACE_HPP
#define DATA_INTERFACE_HPP


#include <cstddef>


namespace di
{


/*
* Users MUST overload this to contain a
* `Type` type which maps to the type of
* this field in the context of this data
* interface. They must also set
* `exists = true`.
*/
template<typename FieldTag, typename DataInterface>
struct FieldTypeImpl
{
    static inline constexpr bool exists = false;

    // using Type = YourTypeGoesHere;
};


/*
* Load the given field from the given entity
* in the given data interface. This should
* ALWAYS be inlined (or inlinable.) Template
* specialisations should also make sure to
* set `exists = true`.
*/
template<typename FieldTag, typename EntityType, typename DataInterface>
struct LoadImpl
{
    static inline constexpr bool exists = false;

    // static inline FieldType<FieldTag, DataInterface>
    //     impl(DataInterface&, EntityType);
};


/*
* Store the given value for the given field
* to the given entity in the given data
* interface. This should ALWAYS be inlined
* (or inlinable.) Template specialisations
* should also make sure to set `exists = true`.
*/
template<typename FieldTag, typename EntityType, typename DataInterface>
struct StoreImpl
{
    static inline constexpr bool exists = false;

    // static inline void
    //     impl(DataInterface&, EntityType, FieldType<FieldTag, DataInterface>);
};


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


}  // namespace di


#endif  // DATA_INTERFACE_HPP

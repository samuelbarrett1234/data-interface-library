# Data Interface Example

This repository illustrates a "data interface design" for C++ applications.
The aim is to separate data layout from algorithms to the maximum extent possible.
This is difficult because it means that, for example, you cannot use pointers or structs, because both of those imply a particular data layout.

Why do this?
Given that loading and storing from and to memory is usually the slowest part of a modern CPU, it is also the bottleneck of many applications, and the way to "speed your code up" is often not rewriting an algorithm but is fundamentally restructuring your data.
This is well documented - cf. Mike Acton's "Data Oriented Design" talk for a starting point.
Reorganising your entire data model to achieve a performance speedup is a costly refactoring process, and the hope is that by separating data layout from algorithms, this is made far easier.

## Terminology

This "data interface design" is centred around three concepts:
1. Data interfaces,
2. Entities,
3. Fields.

When you're implementing a function, you'd likely write it _templated on the type of data interface_.
The data interface encapsulates all data available to this function, and its layout.

In the context of a given data interface, an entity has zero or more fields, and some may be available for only reading or only writing.
This may change depending on different data interfaces.

Moreover - following the old databases adage "every table can have at most one clustered index" - every entity can be part of up to 1 "clustered index" in the context of a data interface.
For intuition, this is like saying that an entity can be in an array of similar entities - it is not possible for an entity to be in two arrays at once (if you take the view that overlapping segments of one array is still just one array.)

But the term "array" here is not quite true to its generality.
The entity could be, for example, stored in a "head-tail buffer", where you can 'increment an entity' using some pointer calculations.
Interestingly, there are parallels here to the C++ iterator types: entities can be incremented, decremented, or advanced using random access.
But entities are not iterators, because they cannot be dereferenced - all data must be loaded or stored through the `load` and `store` APIs.

**Warning:** just because entities can be in at most one clustered index, does not mean that all instances of a given entity are in the _same_ "clustered index".
There can be lots of "clustered segments" of entities of a given type lying about.
For example, consider a "head-tail" buffer - the tail elements will be a particular kind of entity, and they will support some random access, but only within the tail of a given head.
As a rule of thumb, only move entites within a range which the caller has already established for you.

## TODO (documentation)

1. Explain why this is good for unit testing.
2. Why separate load and store instructions? Think: bit-packing integers; using the bits of a pointer; SIMD values.

## Requirements

C++20 or later for the "data_interface.hpp" header due to concepts usage.

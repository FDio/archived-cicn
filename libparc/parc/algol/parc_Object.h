/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file parc_Object.h
 * @ingroup memory
 * @brief Reference Counted Object Memory
 *
 * An arbitrary C structure stored in allocated memory with a reference counter.
 *
 * When a PARC Object is created, via `parcObject_Create`, or `parcObject_Copy` it has reference count of 1.
 *
 * References to a PARC Object are acquired by calling `parcObject_Acquire`
 * and once a reference is no longer needed it is released via parcObject_Release`.
 * When the last reference is released, the memory storing the PARC Object is deallocated.
 * Any further reference to that object or its memory is undefined.
 *
 * When creating a PARCObject, the caller may supply an instance of `PARCObjectDescriptor` containing
 * configuration information used and pointers to functions that are invoked during the lifecycle of the PARC Object.
 * Notable functions that are a finalization cleanup function for an object that will be deallocated,
 * a clone function for an object that is being cloned,
 * and a string generator for an object that is implementing the `ToString` function.
 * Implementors of modules that use PARCObject supply a specification of callback
 * functions that implement specific behaviours for interfaces using PARC Object.
 *
 */
#ifndef libparc_parc_Object_h
#define libparc_parc_Object_h

#include <stdint.h>
#include <time.h>

#include <LongBow/runtime.h>
#include <LongBow/longBow_Compiler.h>

#include <parc/algol/parc_CMacro.h>
//#include <parc/algol/parc_JSON.h>
//#include <parc/algol/parc_HashCode.h>
#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcObject_OptionalAssertValid(_instance_)
#else
#  define parcObject_OptionalAssertValid(_instance_) parcObject_AssertValid(_instance_)
#endif

//Switch to strong type after subclasses are converted
//typedef struct {int unused;} PARCObject;
typedef void PARCObject;

#include <stdint.h>
#include <stdbool.h>

typedef struct PARCObjectDescriptor PARCObjectDescriptor;

/**
 * @define parcObject_DescriptorName(_type)
 *
 * Compose the token for a subtype specific name for a subtype's `PARCObjectDescriptor`
 * which is a parameter to `parcObject_Create`.
 *
 * For example
 * @code
 * parcObject_DescriptorName(MyType)
 * @endcode
 *
 * generates the token `MyType_Descriptor`
 *
 * Implementations should use this macro, rather than hardcode the format in their code.
 */
#define parcObject_DescriptorName(_type) parcCMacro_Cat(_type, _Descriptor)

/**
 * @define parcObjectDescriptor_Declaration(_type_)
 *
 * Create a declaration of a `PARCObjectDescriptor` implementation.
 * To define the actual implementation, use `parcObject_Override`
 */
#define parcObjectDescriptor_Declaration(_type_) const PARCObjectDescriptor parcObject_DescriptorName(_type_)

/**
 * @define parcObject_Declare(_type_)
 *
 * Create a declaration of a `PARCObject` implementation.
 * This causes the corresponding `typedef` to be defined
 * and the global PARCObjectDescriptor corresponding to the declared type.
 */
#define parcObject_Declare(_type_) \
    typedef struct _type_ _type_; \
    extern parcObjectDescriptor_Declaration(_type_)

#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_JSON.h>

/**
 * A Function that performs the final cleanup and resource deallocation when
 * a PARC Object has no more references.
 *
 * This is deprecated and will be removed.
 * Use `PARCObjectDestructor`
 */
typedef void (PARCObjectDestroy)(PARCObject **);

/**
 * A Function that performs the final cleanup and resource deallocation when
 * a PARC Object has no more references.
 *
 * If the function returns `true` the object is automatically deallocated and destroyed.
 * If the function returns `false` the object is not automatically deallocated and destroyed,
 * and the responsibility for the object's state and memory are the responsibility of
 * the `PARCObjectDestructor` function.
 */
typedef bool (PARCObjectDestructor)(PARCObject **);

/**
 * A Function that releases one reference to the given PARC Object.
 * On the final release, where the number of references becomes 0,
 * the PARCObjectDestroy function is invoked.
 */
typedef void (PARCObjectRelease)(PARCObject **objectPointer);

/**
 * A function that produces a deep-copy of the given PARC Object instance.
 */
typedef PARCObject *(PARCObjectCopy)(const PARCObject *object);

/**
 * A function that invokes the proper _Equals() function for two PARC Object instances.
 */
typedef bool (PARCObjectEquals)(const PARCObject *objectX, const PARCObject *objectY);

/**
 * A function that invokes the proper _Compare() functions for two PARC Object instances.
 */
typedef int (PARCObjectCompare)(const PARCObject *, const PARCObject *);

/**
 * A function that computes the 32-bit hashcode of the given PARC Object instance.
 */
typedef PARCHashCode (PARCObjectHashCode)(const PARCObject *object);

/**
 * A function that produces a C style nul-terminated string representation of the given PARC Object instance.
 */
typedef char *(PARCObjectToString)(const PARCObject *object);

/**
 * A function that displays a human readable representation of the given PARCObject.
 */
typedef void (PARCObjectDisplay)(const PARCObject *object, const int indentation);

/**
 * A function that generates a JSON representation of the given PARC Object instance.
 */
typedef PARCJSON *(PARCObjectToJSON)(const PARCObject *);

/**
 * Every PARCObject descriptor has a pointer to a `PARCObjectState`
 * containing arbitrary data related to all instances sharing the descriptor.
 */
typedef void PARCObjectTypeState;

/**
 * Every PARC Object instance contains a pointer to an instance of this structure defining
 * the canonical meta-data for the object.
 */
struct PARCObjectDescriptor {
    char name[64];
    PARCObjectDestroy *destroy;
    PARCObjectDestructor *destructor;
    PARCObjectRelease *release;
    PARCObjectCopy *copy;
    PARCObjectToString *toString;
    PARCObjectEquals *equals;
    PARCObjectCompare *compare;
    PARCObjectHashCode *hashCode;
    PARCObjectToJSON *toJSON;
    PARCObjectDisplay *display;
    const struct PARCObjectDescriptor *super;
    size_t objectSize;
    unsigned objectAlignment;
    bool isLockable;
    PARCObjectTypeState *typeState;
};


/**
 * Create an allocated instance of `PARCObjectDescriptor`.
 *
 * @param [in] name A nul-terminated, C string containing the name of the object descriptor.
 * @param [in] objectSize The number of bytes necessary to contain the object.
 * @param [in] objectAlignment The alignment boundary necessary for the object, a power of 2 greater than or equal to `sizeof(void *)`
 * @param [in] isLockable True, if this object implementation supports locking.
 * @param [in] destructor The callback function to call when the last `parcObject_Release()` is invoked (replaces @p destroy).
 * @param [in] release The callback function to call when `parcObject_Release()` is invoked.
 * @param [in] copy The callback function to call when parcObject_Copy() is invoked.
 * @param [in] toString The callback function to call when `parcObject_ToString()` is invoked.
 * @param [in] equals The callback function to call when `parcObject_Equals()` is invoked.
 * @param [in] compare The callback function to call when `parcObject_Compare()` is invoked.
 * @param [in] hashCode The callback function to call when `parcObject_HashCode()` is invoked.
 * @param [in] toJSON The callback function to call when `parcObject_ToJSON()` is invoked.
 * @param [in] display The callback function to call when `parcObject_Display()` is invoked.
 * @param [in] super A pointer to a `PARCObjectDescriptor` for the supertype of created `PARCObjectDescriptor`
 * @param [in] typeState A pointer to a `PARCObjectTypeState` for the per-type data for the created `PARCObjectDescriptor`
 *
 * @return NULL Memory could not be allocated to store the `PARCObjectDescriptor` instance.
 * @return non-NULL Successfully created the implementation
 */
PARCObjectDescriptor *parcObjectDescriptor_Create(const char *name,
                                                  size_t objectSize,
                                                  unsigned int objectAlignment,
                                                  bool isLockable,
                                                  PARCObjectDestructor *destructor,
                                                  PARCObjectRelease *release,
                                                  PARCObjectCopy *copy,
                                                  PARCObjectToString *toString,
                                                  PARCObjectEquals *equals,
                                                  PARCObjectCompare *compare,
                                                  PARCObjectHashCode *hashCode,
                                                  PARCObjectToJSON *toJSON,
                                                  PARCObjectDisplay *display,
                                                  const PARCObjectDescriptor *superType,
                                                  PARCObjectTypeState *typeState);

PARCObjectDescriptor *parcObjectDescriptor_CreateExtension(const PARCObjectDescriptor *superType, const char *name);

PARCObjectTypeState *parcObjectDescriptor_GetTypeState(const PARCObjectDescriptor *descriptor);

const PARCObjectDescriptor *parcObjectDescriptor_GetSuperType(const PARCObjectDescriptor *descriptor);

bool parcObjectDescriptor_Destroy(PARCObjectDescriptor **descriptorPointer);

/**
 * The globally available `PARCObject` descriptor.
 */
extern parcObjectDescriptor_Declaration(PARCObject);

typedef uint64_t PARCReferenceCount;

/**
 * Assert that an instance of PARC Object is valid.
 *
 * If the instance is not valid, terminate via an assertion.
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] object A pointer to a PARC Object instance.
 */
void parcObject_AssertValid(const PARCObject *object);

/**
 * Determine if an instance of PARCObject is valid.
 *
 * A valid PARCObject has non-NULL, it has a reference count > 0,
 * it is non-zero in length, and has a valid alignment.
 *
 * @param [in] object A pointer to a PARC Object instance.
 *
 * @return true The PARCObject is valid.
 * @return true The PARCObject is invalid.
 */
bool parcObject_IsValid(const PARCObject *object);

/**
 * Create a new reference counted object that is a deep copy of the specified object,
 * if possible, or, otherwise, a shallow copy of the object's total allocation memory.
 *
 * The reference count for the new object is 1.
 *
 * @param [in] object A pointer to the original object.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new reference counted object.
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     struct MyType *copy = parcObject_Copy(t);
 * }
 * @endcode
 *
 * @see parcObject_Release
 */
PARCObject *parcObject_Copy(const PARCObject *object);

/**
 * Compare two object instances.
 *
 * The comparison function in the first `PARCObjectDescriptor` parameter is used for comparison.
 * The objects are expected to be of the same type. Thus, if the comparison function
 * associated with the first `PARCObjectDescriptor` function is NULL, it is assumed the
 * same holds for the second parameter. In this case, the instance pointers are compared.
 *
 * @param [in] x An object.
 * @param [in] y An object.
 *
 * @return int The result of the comparison.
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     int compareResult = parcObject_Compare(t, t);
 *     printf("0? %d\n", compareResult);
 *
 *     parcObject_Release(&t);
 * }
 * @endcode
 */
int parcObject_Compare(const PARCObject *x, const PARCObject *y);

/**
 * Determine if two PARCObject instances are equal.
 *
 * Two PARCObject instances are equal if, and only if, the instance pointers are equal.
 *
 * The following equivalence relations on non-null `PARCObject` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `PARCObject_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `parcObject_Equals(x, y)` must return true if and only if
 *        `parcObject_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcObject_Equals(x, y)` returns true and
 *        `parcObject_Equals(y, z)` returns true,
 *        then  `parcObject_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `parcObject_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `parcObject_Equals(x, NULL)` must
 *      return false.
 *
 * @param x A pointer to a `PARCObject` instance.
 * @param y A pointer to a `PARCObject` instance.
 * @return true if the two `PARCObject` instances are equal.
 *
 * Example:
 * @code
 * {
 *     struct MyType *a = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *     struct MyType *b = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     if (parcObject_Equals(a, b)) {
 *        // true
 *     } else {
 *        // false
 *     }
 * }
 * @endcode
 */
bool parcObject_Equals(const PARCObject *x, const PARCObject *y);

/**
 * Retrieve the hashcode of the given object.
 *
 * If no object implementation is provided, the hashcode is the 32-bit address
 * of the base object pointer. Otherwise, the hashcode is computed by the
 * provided hashcode function.
 *
 * @param [in] object An object.
 *
 * @return uint32_t The object hashcode
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     PARCHashCode hashcode = parcObject_HashCode(t);
 *     printf("Hashcode = %" PRIXPARCHashCode "\n", hashcode);
 *
 *     parcObject_Release(&t);
 * }
 * @endcode
 */
PARCHashCode parcObject_HashCode(const PARCObject *object);

/**
 * Create a C string containing a human readable representation of the given object.
 *
 * @param [in] object The object from which a string representation will be generated.
 *
 * @return NULL Memory could not be allocated to contain the C string.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     char *string = parcObject_ToString(t);
 *     printf("%s\n", string);
 *     parcMemory_Deallocate((void **) &string);
 *
 *     parcObject_Release(&t);
 * }
 * @endcode
 */
char *parcObject_ToString(const PARCObject *object);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] object The object from which a JSON instance will be generated.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     PARCJSON *json = parcObject_ToJSON(t);
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcObject_Release(&t);
 * }
 * @endcode
 */
PARCJSON *parcObject_ToJSON(const PARCObject *object);

/**
 * Acquire a new reference to an object.
 *
 * The reference count to the object is incremented.
 *
 * @param [in] object The object to which to refer.
 *
 * @return The same value as the input parameter @p object
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     struct MyType *new = parcObject_Acquire(t);
 *
 *     parcObject_Release(&t);
 *     parcObject_Release(&new);
 * }
 * @endcode
 */
PARCObject *parcObject_Acquire(const PARCObject *object);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If an invocation of `parcObject_Release` causes the last reference to
 * the instance to be released, it calls the instance's `destructor` function
 * specified in the `PARCObjectDescriptor` structure supplied when the instance
 * was created (see `parcObject_Create`.
 *
 * The contents of the deallocated memory used for the PARC object are undefined.
 * Do not reference the object after the last release.
 *
 * @param [in] objectPointer A pointer to a pointer to the instance to release.
 *
 * @return The number of remaining references to the object.
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     parcObject_Release(&t);
 * }
 * @endcode
 *
 * @see parcObject_Create
 * @see parcObject_Acquire
 */
PARCReferenceCount parcObject_Release(PARCObject **objectPointer);

/**
 * Get the current `PARCReferenceCount` for the specified object.
 *
 * The reference count must always be greater than zero.
 *
 * @param [in] object A pointer to a valid `PARCObject` instance.
 *
 * @return The current reference count for the specified object.
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     PARCReferenceCount count = parcObject_GetReferenceCount(t);
 *
 *     parcObject_Release(&t);
 * }
 * @endcode
 *
 * @see parcObject_Acquire
 * @see parcObject_Release
 */
PARCReferenceCount parcObject_GetReferenceCount(const PARCObject *object);

/**
 * Print a human readable representation of the given `PARC Object.
 *
 * @param [in] object A pointer to the instance to display.
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     parcObject_Display(t, 0);
 *
 *     parcObject_Release(&t);
 * }
 * @endcode
 */
void parcObject_Display(const PARCObject *object, const int indentation);

/**
 * Get the `PARCObjectDescriptor` of the given `PARCObject`.
 *
 * @param [in] object A pointer to a valid `PARCObject` instance
 *
 * @return A pointer to the given PARCObject's `PARCObjectDescriptor`.
 *
 * Example:
 * @code
 * {
 *     struct MyType *t = parcObject_Create(sizeof(struct MyType), &MyType_Descriptor);
 *
 *     PARCObjectDescriptor *descriptor = parcObject_GetDescriptor(t);
 *
 *     parcObject_Release(&t);
 * }
 * @endcode
 */
const PARCObjectDescriptor *parcObject_GetDescriptor(const PARCObject *object);

/**
 * Set the `PARCObjectDescriptor` of the given `PARCObject`.
 *
 * @param [in] object A pointer to a valid `PARCObject` instance
 * @param [in] objectType A pointer to a valid `PARCObjectDescriptor` instance
 *
 * @return The previous value of the given PARCObject's `PARCObjectDescriptor`.
 */
const PARCObjectDescriptor *parcObject_SetDescriptor(PARCObject *object, const PARCObjectDescriptor *objectType);

/**
 * @def parcObject_MetaInitialize
 * @deprecated Use parcObject_Override instead;
 *
 * Initialize a PARCObjectDescriptor structure. Every function pointer is set to NULL.
 *
 * @param [in] objectType A pointer to the PARCObjectDescriptor structure to initialize.
 *
 * @return The pointer to the initialized PARCObjectDescriptor.
 */
#define parcObject_MetaInitialize(objectType) \
    (objectType)->destructor = NULL, \
    (objectType)->destroy = NULL, \
    (objectType)->release = NULL, \
    (objectType)->copy = NULL, \
    (objectType)->toString = NULL, \
    (objectType)->equals = NULL, \
    (objectType)->compare = NULL, \
    (objectType)->hashCode = NULL, \
    (objectType)->toJSON = NULL, \
    (objectType)->display = NULL, \
    (objectType)->super = NULL


/**
 * Create new `PARCObjectDescriptor` based on an existing `PARCObjectDescriptor.`
 * The new `PARCObjectDescriptor` uses the existing `PARCObjectDescriptor` as the super-type of the new descriptor.
 */
#define parcObject_Extends(_subtype, _superType, ...) \
    LongBowCompiler_IgnoreInitializerOverrides \
    parcObjectDescriptor_Declaration(_subtype) = { \
        .super           = &parcObject_DescriptorName(_superType), \
        .name            = #_subtype, \
        .objectSize      = 0, \
        .objectAlignment = 0, \
        .destroy         = NULL,    \
        .destructor      = NULL, \
        .release         = NULL,   \
        .copy            = NULL,   \
        .toString        = NULL,   \
        .equals          = NULL,   \
        .compare         = NULL,   \
        .hashCode        = NULL,   \
        .toJSON          = NULL,   \
        .display         = NULL,   \
        .isLockable      = true, \
        .typeState       = NULL, \
        __VA_ARGS__  \
    }; \
    LongBowCompiler_WarnInitializerOverrides \
    const PARCObjectDescriptor parcObject_DescriptorName(_subtype)

/**
 * Define a new PARC Object implementation, by composing a new PARC Object Descriptor referencing an old one.
 * The new PARC Object implementation must be accompanied by the corresponding `typedef` of the type containing the object's data.
 */
#define parcObject_Override(_subtype, _superType, ...) \
    parcObject_Extends(_subtype, _superType,           \
                       .objectSize = sizeof(_subtype),               \
                       .objectAlignment = sizeof(void *),                 \
                       __VA_ARGS__)


/// Helper MACROS for PARCObject Normalization
/** \cond */
/**
 * parcObject_DestroyWrapper builds the boiler plate wrapper for PARCObject type conversion in the
 * destroy operation. Intended for internal use.
 */
#define parcObject_DestroyWrapper(_type, _fname)                \
    static void _autowrap_destroy_ ## _type(PARCObject **object)  \
    {                                                           \
        _fname((_type **) object);                              \
    }

/**
 * parcObject_DestructorWrapper builds the boiler plate wrapper for PARCObject type conversion in the
 * destroy operation. Intended for internal use.
 */
#define parcObject_DestructorWrapper(_type, _fname)               \
    static bool _autowrap_destructor_ ## _type(PARCObject **object) \
    {                                                             \
        return _fname((_type **) object);                         \
    }

/**
 * `parcObject_CopyWrapper` builds the boiler plate wrapper for PARCObject type conversion in the
 * copy operation. Intended for internal use.
 */
#define parcObject_CopyWrapper(_type, _fname)                           \
    static PARCObject *_autowrap_copy_ ## _type(const PARCObject *object) \
    {                                                                   \
        return (PARCObject *) _fname((const _type *) object);           \
    }

/**
 * parcObject_ToStringWrapper builds the boiler plate wrapper for PARCObject type conversion in the
 * ToString operation. Intended for internal use.
 */
#define parcObject_ToStringWrapper(_type, _fname) \
    static char *_autowrap_toString_ ## _type(const PARCObject *object)  \
    {                                                                   \
        return _fname((const _type *) object);                          \
    }

/**
 * `parcObject_EqualsWrapper` builds the boiler plate wrapper for PARCObject type conversion in the
 * equals operation. Intended for internal use.
 */
#define parcObject_EqualsWrapper(_type, _fname)                         \
    static bool _autowrap_equals_ ## _type(const PARCObject *a, const PARCObject *b) \
    {                                                                              \
        return _fname((const _type *) a, (const _type *) b);                       \
    }

/**
 * parcObject_CompareWrapper builds the boiler plate wrapper for PARCObject type conversion in the
 * compare operation. Intended for internal use.
 */
#define parcObject_CompareWrapper(_type, _fname) \
    static int _autowrap_compare_ ## _type(const PARCObject *a, const PARCObject *b) \
    {                                                                              \
        return _fname((const _type *) a, (const _type *) b);                       \
    }

/**
 * `parcObject_HashCodeWrapper` builds the boiler plate wrapper for PARCObject type conversion in the
 * HashCode operation. Intended for internal use.
 */
#define parcObject_HashCodeWrapper(_type, _fname) \
    static PARCHashCode _autowrap_hashCode_ ## _type(const PARCObject *object) \
    {                                                                        \
        return _fname((const _type *) object);                               \
    }

/**
 * `parcObject_CopyWrapper` builds the boiler plate wrapper for PARCObject type conversion in the
 * ToJSON operation. Intended for internal use.
 */
#define parcObject_ToJSONWrapper(_type, _fname) \
    static PARCJSON *_autowrap_toJSON_ ## _type(const PARCObject *object) \
    {                                                                   \
        return _fname((const _type *) object);                          \
    }

/**
 * `parcObject_DisplayWrapper` builds the boiler plate wrapper for PARCObject type conversion in the
 * Display operation. Intended for internal use.
 */
#define parcObject_DisplayWrapper(_type, _fname) \
    static void _autowrap_Display_ ## _type(const PARCObject *object, const int indentation) \
    {                                                                                      \
        _fname((const _type *) object, indentation);                                       \
    }

/**
 * _autowrap_NULL is a part of the c-macro trick for implement a macro If-Else switch.
 * If included as a macro parameter it inserts a comma into the parameter list for that macro.
 * This can be used by a switching macro that always resolves to the nth element and the
 * presence of a comma generating macro changes which element is the nth. When NULL is used
 * as a parameter in a call to "ExtendPARCObject", _autowrap_NULL will be the name generated which
 * expands to a comma.
 */
#define _autowrap_NULL(...) ,

/** \endcond */


/**
 * @define parcObject_ExtendPARCObject
 * @deprecated Use parcObject_Override instead;
 *
 * @discussion parcObject_ExtendPARCObject is a helper macro for constructing a PARCObjectDescriptor Structure of
 * function pointers pointing to a subtype's overriding functions. This struct serves the same
 * purpose as a vTable in c++ and provides for simple polymorphism. The functions used as parameters
 * should NOT call through to the parcObject function they override as this will result in an infinite loop.
 * NULL should be used for functions where PARCObject's default implementation is desired.
 *
 * Note: It uses the parcCMacro_IfElse trickery to handle NULL parameters.
 *
 * @param [in] _destroy A pointer to the Destroy callback function.
 * @param [in] _copy A pointer to the Copy callback function.
 * @param [in] _toString A pointer to the ToString callback function.
 * @param [in] _equals A pointer to the Equals callback function.
 * @param [in] _compare A pointer to the Compare callback function.
 * @param [in] _hashCode A pointer to the HashCode callback function.
 * @param [in] _toJSON A pointer to the ToJSON callback function.
 */
#define parcObject_ExtendPARCObject(_type, _destroy, _copy, _toString, _equals, _compare, _hashCode, _toJSON) \
    parcCMacro_IfElse(, parcObject_DestroyWrapper(_type, _destroy), _autowrap_ ## _destroy()) \
    parcCMacro_IfElse(, parcObject_CopyWrapper(_type, _copy), _autowrap_ ## _copy()) \
    parcCMacro_IfElse(, parcObject_ToStringWrapper(_type, _toString), _autowrap_ ## _toString()) \
    parcCMacro_IfElse(, parcObject_EqualsWrapper(_type, _equals), _autowrap_ ## _equals()) \
    parcCMacro_IfElse(, parcObject_CompareWrapper(_type, _compare), _autowrap_ ## _compare()) \
    parcCMacro_IfElse(, parcObject_HashCodeWrapper(_type, _hashCode), _autowrap_ ## _hashCode()) \
    parcCMacro_IfElse(, parcObject_ToJSONWrapper(_type, _toJSON), _autowrap_ ## _toJSON()) \
    parcObject_Override(_type, PARCObject, \
                        .destroy = parcCMacro_IfElse(NULL, _autowrap_destroy_ ## _type, _autowrap_ ## _destroy()), \
                        .destructor = NULL, \
                        .release = NULL,                                               \
                        .copy = parcCMacro_IfElse(NULL, _autowrap_copy_ ## _type, _autowrap_ ## _copy()), \
                        .toString = parcCMacro_IfElse(NULL, _autowrap_toString_ ## _type, _autowrap_ ## _toString()), \
                        .equals = parcCMacro_IfElse(NULL, _autowrap_equals_ ## _type, _autowrap_ ## _equals()), \
                        .compare = parcCMacro_IfElse(NULL, _autowrap_compare_ ## _type, _autowrap_ ## _compare()), \
                        .hashCode = parcCMacro_IfElse(NULL, _autowrap_hashCode_ ## _type, _autowrap_ ## _hashCode()), \
                        .toJSON = parcCMacro_IfElse(NULL, _autowrap_toJSON_ ## _type, _autowrap_ ## _toJSON()), \
                        .display = NULL)

/**
 * @define parcObject_CreateInstance
 *
 * `parcObject_CreateInstance` is a helper C-macro that creates an instance of a PARCObject subtype
 * using `parcObject_CreateInstanceImpl` that is based on the PARCObjectDescriptor.
 *
 * @param [in] _subtype A subtype's type string (e.g. PARCBuffer)
 */
#define parcObject_CreateInstance(_subtype) \
    parcObject_CreateInstanceImpl(&parcObject_DescriptorName(_subtype))

PARCObject *parcObject_CreateInstanceImpl(const PARCObjectDescriptor *descriptor);

/**
 * @define parcObject_CreateAndClearInstance
 *
 * parcObject_CreateAndClearInstance is a helper C-macro that creates an instance of a PARCObject subtype
 * using parcObject_CreateAndClear that is based on the PARCObjectDescriptor struct created by the
 * parcObject_ExtendPARCObject macro.
 *
 * @param [in] _subtype A subtype's type string (e.g. PARCBuffer)
 */
#define parcObject_CreateAndClearInstance(_subtype) \
    (_subtype *) parcObject_CreateAndClearInstanceImpl(&parcObject_DescriptorName(_subtype))

/**
 * Create a reference counted segment of memory of at least @p objectLength long.
 *
 * The implementation pointer, is either NULL or points to a valid `PARCObjectDescriptor` structure
 * containing the callback functions that implement the object's life-cycle operations.
 *
 * The allocated memory is such that the memory's base address is aligned on a sizeof(void *) boundary,
 * and filled with zero bytes.
 *
 * If memory cannot be allocated, `errno` is set to ENOMEM.
 *
 * @param [in] descriptor A pointer to a valid `PARCObjectDescriptor` structure.
 *
 * @return NULL The memory could not be allocated.
 * @return non-NULL A pointer to reference counted memory of at least length bytes.
 *
 * Example:
 * @code
 * {
 *     struct timeval *t = parcObject_CreateAndClearInstanceImpl(sizeof(struct timeval), &PARCObject_Descriptor);
 * }
 * @endcode
 *
 * @see PARCObjectDescriptor
 * @see parcObject_Create
 */
PARCObject *parcObject_CreateAndClearInstanceImpl(const PARCObjectDescriptor *descriptor);

/**
 * Define a static PARCObject instance for the given type, alignment, per-object data.
 *
 * Once the instance has been defined, it must be initialised via `parcObject_InitInstance`
 * or `parcObject_InitAndClearInstance` before it is used.
 *
 * @return A pointer to an invalid `PARCObject` instance that must be initialised .
 */
#define parcObject_Instance(_type_, _alignment_, _size_) \
    (_type_ *) (& (char[parcObject_TotalSize(_alignment_, _size_)]) { 0 }[parcObject_PrefixLength(sizeof(void *))])

/**
 * @define parcObject_InitInstance
 *
 * `parcObject_InitInstance` is a helper C-macro that initializes a portion of memory to contain a `PARCObject` subtype
 * using `parcObject_InitInstanceImpl`.
 *
 * @param [in] _object_ A pointer to memory that will contain the object and its meta-data.
 * @param [in] _subtype A subtype's type name.
 */
#define parcObject_InitInstance(_object_, _subtype) \
    parcObject_InitInstanceImpl(_object_, &parcObject_DescriptorName(_subtype))

/**
 * @define parcObject_InitInstanceImpl
 *
 * Initialize a PARCObject instance given the `PARCObjectDescriptor`.
 * Any previous state of the given PARCObject is destroyed.
 *
 * @param [in] object A pointer to an existing valid or invalid `PARCObject` instance.
 * @param [in] descriptor A pointer to a valid `PARCObjectDescriptor` structure.
 */
PARCObject *parcObject_InitInstanceImpl(PARCObject *object, const PARCObjectDescriptor *descriptor);

/**
 * @define parcObject_InitAndClearInstance
 *
 * `parcObject_InitAndClearInstance` is a helper C-macro that initializes a portion of memory to contain a PARCObject subtype
 * using `parcObject_InitAndClearInstanceImpl`.
 *
 * @param [in] _object_ A pointer to memory that will contain the object and its meta-data.
 * @param [in] _subtype A subtype's type name.
 */
#define parcObject_InitAndClearInstance(_object_, _subtype) \
    parcObject_InitAndClearInstanceImpl(_object_, &parcObject_DescriptorName(_subtype))

/**
 * Create a reference counted segment of memory of at least @p objectLength long.
 *
 * The implementation pointer, is either NULL or points to a valid `PARCObjectDescriptor` structure
 * containing the callback functions that implement the object's life-cycle operations.
 *
 * The allocated memory is such that the memory's base address is aligned on a sizeof(void *) boundary,
 * and filled with zero bytes.
 *
 * If memory cannot be allocated, `errno` is set to `ENOMEM`.
 *
 * @param [in] object A pointer to an existing valid or invalid `PARCObject` instance.
 * @param [in] descriptor A pointer to a valid `PARCObjectDescriptor` structure.
 *
 * @return NULL The memory could not be allocated.
 * @return non-NULL A pointer to reference counted memory of at least length bytes.
 *
 * Example:
 * @code
 * {
 *
 * }
 * @endcode
 *
 * @see PARCObjectDescriptor
 * @see parcObject_Create
 */
PARCObject *parcObject_InitAndClearInstanceImpl(PARCObject *object, const PARCObjectDescriptor *descriptor);

/**
 * Compute the number of bytes necessary for a PARC Object prefix.
 *
 * The @p _alignment_ parameter specifies the required memory alignment of the object.
 *
 * @param [in] _alignment_ An unsigned integer value greater than `sizeof(void *)`
 *
 * @return The number of bytes necessary for a PARC Object header.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
// The constant value here must be greater than or equal to the size of the internal _PARCObjectHeader structure.
#define parcObject_PrefixLength(_alignment_) ((152 + (_alignment_ - 1)) & - _alignment_)

/**
 * Compute the number of bytes necessary for a PARC Object.
 *
 * The number of bytes consists of the number of bytes for the PARC Object header, padding, and the object's specific data.
 *
 * The @p _alignment_ parameter specifies the required memory alignment of the object.
 * The @p _size_ parameter specifies the number of bytes necessary for the object specific data.
 */
#define parcObject_TotalSize(_alignment_, _size_) (parcObject_PrefixLength(_alignment_) + _size_)

/**
 * Wrap a static, unallocated region of memory producing a valid pointer to a `PARCObject` instance.
 *
 * Note that the return value will not be equal to the value of @p origin.
 *
 * @param [in] memory A pointer to memory that will contain the object and its state.
 * @param [in] descriptor The subtype name that will be used to compose the name of the `PARCObjectDescriptor` for the object.
 *
 * @return NULL An error occured.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
#define parcObject_Wrap(_memory_, _subtype) \
    parcObject_WrapImpl(_memory_, &parcObject_DescriptorName(_subtype))

/**
 * Wrap a static, unallocated region of memory producing a valid pointer to a `PARCObject` instance.
 *
 * Note that the return value will not be equal to the value of @p origin.
 *
 * @param [in] memory A pointer to memory that will contain the object and its state.
 * @param [in] descriptor A pointer to a valid `PARCObjectDescriptor` for the object.
 *
 * @return NULL An error occured.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCObject *parcObject_WrapImpl(void *memory, const PARCObjectDescriptor *descriptor);

/**
 * @def parcObject_ImplementAcquire
 *
 * `parcObject_ImplementAcquire` is a helper C-macro that creates a canonical subtype specific
 * Acquire function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. parcBuffer)
 * @param [in] _type A subtype's type string (e.g. PARCBuffer)
 */
#define parcObject_ImplementAcquire(_namespace, _type)               \
    _type * _namespace ## _Acquire(const _type * pObject) {              \
        return (_type *) parcObject_Acquire((PARCObject *) pObject); \
    } extern _type *_namespace ## _Acquire(const _type * pObject)

/**
 * @def parcObject_ImplementRelease
 *
 * `parcObject_ImplementRelease` is a helper C-macro that creates a canonical subtype specific
 * Release function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. parcBuffer)
 * @param [in] _type A subtype's type string (e.g. PARCBuffer)
 */
#define parcObject_ImplementRelease(_namespace, _type) \
    inline void _namespace ## _Release(_type **pObject) { \
        parcObject_Release((PARCObject **) pObject);     \
    } extern void _namespace ## _Release(_type **pObject)

/**
 * `parcObject_ImplementationCheck` is a helper macro that will generate compile time warnings for
 * missing canonical functions or canonical functions with faulty signatures.
 *
 * @param _namespace A subtype's namespace string (e.g. parcBuffer)
 * @param _type A subtype's type string (e.g. PARCBuffer)
 */
#define parcObject_ImplementationCheck(_namespace, _type) \
    static void                                           \
    _impl_check() {                                       \
        _type *po;                                        \
        const _type co;                                   \
        const _type *pco;                                 \
        pco = _namespace ## _Copy(&co);                     \
        pco = _namespace ## _Acquire(&co);                  \
        pco = pco;                                        \
        _namespace ## _Release(&po);                        \
        bool b = _namespace ## _Equals(&co, &co);           \
        b = b;                                            \
        int i = _namespace ## _Compare(&co, &co);           \
        i = i;                                            \
        char *pc = _namespace ## _ToString(&co);            \
        pc = pc;                                          \
        uint32_t ui = _namespace ## _HashCode(&co);         \
        ui = ui;                                          \
        PARCJSON *pj = _namespace ## _ToJSON(&co);          \
        pj = pj;                                          \
    } typedef void parcCMacro_Cat (_type, _IC_NOOP)

/**
 * Obtain the lock on the given `PARCObject` instance.
 *
 * If the lock is already held by another thread, this function will block.
 * If the lock is aleady held by the current thread, this function will return `false`.
 *
 * Implementors must avoid deadlock by attempting to lock the object a second time within the same calling thread.
 *
 * @param [in] object A pointer to a valid `PARCObject` instance.
 *
 * @return true The lock was obtained successfully.
 * @return false The lock is already held by the current thread, the `PARCObject` is invalid, or does not support locking.
 *
 * Example:
 * @code
 * {
 *     if (parcObject_Lock(object)) {
 *
 *     }
 * }
 * @endcode
 */
bool parcObject_Lock(const PARCObject *object);

/**
 * @def parcObject_ImplementLock
 *
 * `parcObject_ImplementLock` is a helper C-macro that defines a static, inline facade for the `parcObject_Lock` function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. `parcBuffer`)
 * @param [in] _type A subtype's type string (e.g. `PARCBuffer`)
 */
#define parcObject_ImplementLock(_namespace, _type)              \
    static inline bool _namespace ## _Lock(const _type * pObject) { \
        return parcObject_Lock((PARCObject *) pObject);          \
    } typedef void parcCMacro_Cat (_type, _Lock_NOOP)

/**
 * Try to obtain the advisory lock on the given `PARCObject` instance.
 *
 * Once the lock is obtained, the caller must release the lock via `parcObject_Unlock`.
 *
 * @param [in] object A pointer to a valid `PARCObject` instance.
 *
 * @return true The `PARCObject` is locked.
 * @return false The `PARCObject` is unlocked, or does not support locking.
 *
 * Example:
 * @code
 * {
 *     while (parcObject_TryLock(object))
 *         ;
 * }
 * @endcode
 */
bool parcObject_TryLock(const PARCObject *object);

/**
 * @def parcObject_ImplementTryLock
 *
 * `parcObject_ImplementTryLock` is a helper C-macro that defines a static, inline facade for the `parcObject_TryLock` function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. `parcBuffer`)
 * @param [in] _type A subtype's type string (e.g. `PARCBuffer`)
 */
#define parcObject_ImplementTryLock(_namespace, _type)              \
    static inline bool _namespace ## _TryLock(const _type * pObject) { \
        return parcObject_TryLock((PARCObject *) pObject);          \
    } typedef void parcCMacro_Cat (_type, _TryLock_NOOP)

/**
 * Try to unlock the advisory lock on the given `PARCObject` instance.
 *
 * @param [in] object A pointer to a valid `PARCObject` instance.
 *
 * @return true The `PARCObject` was locked and now is unlocked.
 * @return false The `PARCObject` was not locked and remains unlocked.
 *
 * Example:
 * @code
 * {
 *     if (parcObject_Lock(Object)) {
 *         parcObject_Unlock(object);
 *     }
 * }
 * @endcode
 */
bool parcObject_Unlock(const PARCObject *object);

/**
 * @def parcObject_ImplementUnlock
 *
 * `parcObject_ImplementUnlock` is a helper C-macro that defines a static, inline facade for the `parcObject_Unlock` function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. `parcBuffer`)
 * @param [in] _type A subtype's type string (e.g. `PARCBuffer`)
 */
#define parcObject_ImplementUnlock(_namespace, _type)              \
    static inline bool _namespace ## _Unlock(const _type * pObject) { \
        return parcObject_Unlock((PARCObject *) pObject);          \
    } typedef void parcCMacro_Cat (_type, _Unlock_NOOP)

/**
 * Determine if the advisory lock on the given PARCObject instance is locked.
 *
 * @param [in] object A pointer to a valid PARCObject instance.
 *
 * @return true The `PARCObject` is locked.
 * @return false The `PARCObject` is unlocked.
 * Example:
 * @code
 * {
 *     if (parcObject_Lock(object)) {
 *         ...
 *         if (parcObject_IsLocked(object) {
 *             ....
 *         }
 *         ...
 *         parcObject_Unlock(object);
 *     }
 * }
 * @endcode
 */
bool parcObject_IsLocked(const PARCObject *object);

/**
 * @def parcObject_ImplementIsLocked
 *
 * parcObject_ImplementIsLocked is a helper C-macro that defines a static, inline facade for the `parcObject_IsLocked` function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. `parcBuffer`)
 * @param [in] _type A subtype's type string (e.g. `PARCBuffer`)
 */
#define parcObject_ImplementIsLocked(_namespace, _type)              \
    static inline bool _namespace ## _IsLocked(const _type * pObject) { \
        return parcObject_IsLocked((const PARCObject *) pObject);    \
    } typedef void parcCMacro_Cat (_type, _IsLocked_NOOP)

/**
 * Causes the calling thread to wait until another thread invokes the `parcObject_Notify()` function on the same object.
 *
 * The calling thread must own this object's lock.
 * The calling thread will release ownership of this lock and wait until another thread invokes `parcObject_Notify`
 * on the same object. The original calling thread then re-obtains ownership of the lock and resumes execution.
 *
 * This function must only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] object A pointer to a valid PARCObject instance.
 *
 * Example:
 * @code
 * {
 *     if (parcObject_Lock(object)) {
 *         ...
 *         if (parcObject_Wait(object) {
 *             ....
 *         }
 *         ...
 *         parcObject_Unlock(object);
 *     }
 * }
 * @endcode
 */
void parcObject_Wait(const PARCObject *object);

/**
 * @def parcObject_ImplementWait
 *
 * parcObject_ImplementWait is a helper C-macro that defines a static, inline facade for the `parcObject_Wait` function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. `parcBuffer`)
 * @param [in] _type A subtype's type string (e.g. `PARCBuffer`)
 * @see parcObject_Wait
 */
#define parcObject_ImplementWait(_namespace, _type)              \
    static inline void _namespace ## _Wait(const _type * pObject) { \
        parcObject_Wait((const PARCObject *) pObject);           \
    }  typedef void parcCMacro_Cat (_type, _Wait_NOOP)


/**
 * Causes the calling thread to wait until either another thread invokes the `parcObject_Notify()`
 * function on the same object or the system time equals or exceeds the specified time.
 *
 * The calling thread must own the object's lock.
 * The calling thread will release ownership of this lock and wait until another thread invokes
 * `parcObject_Notify` or the computer's system time equals or exceeds that specified by @p time.
 * on the same object.
 * The original calling thread then re-obtains ownership of the lock and resumes execution.
 *
 * This function must only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] object A pointer to a valid PARCObject instance.
 *
 * @returns false if the alloted time was exceeded.
 * @returns true if another thread invoked the `parcObject_Notify()` function
 *
 * Example:
 * @code
 * {
 *     struct timeval tv;
 *     gettimeofday(&tv, NULL);
 *
 *     struct timespec absoluteTime;
 *     absoluteTime.tv_sec = tv.tv_sec + 0;
 *     absoluteTime.tv_nsec = 0;
 *
 *     parcObject_WaitUntil(object, &absoluteTime);
 * }
 * @endcode
 */
bool parcObject_WaitUntil(const PARCObject *object, const struct timespec *time);

/**
 * @def parcObject_ImplementWaitUntil
 *
 * parcObject_ImplementWaitUntil is a helper C-macro that defines a static, inline facade for the
 * `parcObject_WaitUntil` function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. `parcBuffer`)
 * @param [in] _type A subtype's type string (e.g. `PARCBuffer`)
 * @see parcObject_WaitUntil
 */
#define parcObject_ImplementWaitUntil(_namespace, _type)                                           \
    static inline bool _namespace ## _WaitUntil(const _type * pObject, const struct timespec *time) { \
        return parcObject_WaitUntil((const PARCObject *) pObject, time);                           \
    }  typedef void parcCMacro_Cat (_type, _WaitUntil_NOOP)

/**
 * Causes the calling thread to wait until either another thread invokes the `parcObject_Notify()`
 * function on the same object or the given number of nanoseconds elapses.
 *
 * The calling thread must own the object's lock.
 *
 * The calling thread will release ownership of its lock and wait until another thread invokes
 * `parcObject_Notify` on the same object,
 * or the computer's system time equals or exceeds  the time specified by the
 * time of invocation plus nanoSeconds.
 * The original calling thread then re-obtains ownership of the lock and resumes execution.
 *
 * This function must only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] object A pointer to a valid PARCObject instance.
 * @param [in] nanoSeconds The number of nanoseconds to wait.
 *
 * @returns false if the allotted time was exceeded.
 * @returns true if another thread invoked the `parcObject_Notify()` function
 *
 * Example:
 * @code
 * {
 *     parcObject_WaitFor(object, 1000000000UL);
 * }
 * @endcode
 */
bool parcObject_WaitFor(const PARCObject *object, const uint64_t nanoSeconds);

/**
 * @def parcObject_ImplementWaitUntil
 *
 * parcObject_ImplementWaitUntil is a helper C-macro that defines a static, inline facade for the
 * `parcObject_WaitUntil` function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. `parcBuffer`)
 * @param [in] _type A subtype's type string (e.g. `PARCBuffer`)
 * @see parcObject_WaitUntil
 */
#define parcObject_ImplementWaitFor(_namespace, _type)                                               \
    static inline bool _namespace ## _WaitFor(const _type * pObject, const unsigned long nanoSeconds) { \
        return parcObject_WaitFor((const PARCObject *) pObject, nanoSeconds);                        \
    }  typedef void parcCMacro_Cat (_type, _WaitFor_NOOP)

/**
 * Wakes up a single thread that is waiting on this object (see `parcObject_Wait)`.
 * If any threads are waiting on this object, one of them is chosen to be awakened.
 * The choice is arbitrary and occurs at the discretion of the underlying implementation.
 *
 * The awakened thread will not be able to proceed until the current thread relinquishes the lock on this object.
 * The awakened thread will compete in the usual manner with any other threads that might be actively
 * competing to synchronize on this object;
 * for example,
 * the awakened thread enjoys no reliable privilege or disadvantage in being the next thread to lock this object.
 *
 * @param [in] object A pointer to a valid `PARCObject` instance.
 *
 * Example:
 * @code
 * {
 *     if (parcObject_Lock(object)) {
 *         parcObject_Notify(object);
 *         parcObject_Unlock(object);
 *     }
 * }
 * @endcode
 */
void parcObject_Notify(const PARCObject *object);

/**
 * @def parcObject_ImplementNotify
 *
 * parcObject_ImplementNotify is a helper C-macro that defines a static, inline facade for the `parcObject_Notify` function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. `parcBuffer`)
 * @param [in] _type A subtype's type string (e.g. `PARCBuffer`)
 */
#define parcObject_ImplementNotify(_namespace, _type)              \
    static inline void _namespace ## _Notify(const _type * pObject) { \
        parcObject_Notify((const PARCObject *) pObject);           \
    } typedef void parcCMacro_Cat (_type, _Notify_NOOP)


/**
 * Wakes up all threads that are waiting on the given object's lock.
 *
 * A thread waits on an object by calling one of the wait methods, `parcObject_Wait`, `parcObject_WaitFor`, `parcObject_WaitUntil`.
 * The awakened threads will proceed after the current thread relinquishes the lock on the given object.
 * The awakened threads will compete in the usual manner with any other threads that might be actively competing
 * to synchronize on this object.
 * Awakened threads have no priority between them in being the next thread to lock this object.
 *
 * This method can only be called by a thread that is the owner of this object's lock.
 *
 * @param [in] object A pointer to a valid `PARCObject` instance.
 *
 * Example:
 * @code
 * {
 *     if (parcObject_Lock(object)) {
 *         parcObject_NotifyAll(object);
 *         parcObject_Unlock(object);
 *     }
 * }
 * @endcode
 */
void parcObject_NotifyAll(const PARCObject *object);

/**
 * @def parcObject_ImplementNotifyAll
 *
 * parcObject_ImplementNotifyAll is a helper C-macro that defines a static, inline facade for the `parcObject_NotifyAll` function.
 *
 * @param [in] _namespace A subtype's namespace string (e.g. `parcBuffer`)
 * @param [in] _type A subtype's type  (e.g. `PARCBuffer`)
 */
#define parcObject_ImplementNotifyAll(_namespace, _type)       \
    static inline void _namespace ## _NotifyAll(const _type * pObject) { \
        parcObject_NotifyAll((const PARCObject *) pObject);               \
    } typedef void parcCMacro_Cat (_type, _NotifyAll_NOOP)

/**
 * @def parcObject_Mutex
 *
 * This macro uses the functions `parcObject_Lock` and `parcObject_Unlock`
 * to provide a simple syntax for implementing a mutual exclusion region of code.
 *
 * @param [in] _object_ A pointer to a valid PARCObject that implements locking.
 *
 * Example:
 * @code
 * {
 *      parcObject_Mutex(object) {
 *          ....
 *      }
 * }
 * @endcode
 * @see parcObject_Synchronize
 */
#define parcObject_Mutex(_object_) for (bool once = true; once && parcObject_Lock(_object_); parcObject_Unlock(_object_), once = false)

/**
 * Determine if a given `PARCObject` is and instance of the specified `PARCObjectDescriptor`.
 *
 * @param [in] object A pointer to a valid PARCObject instance.
 * @param [in] descriptor A pointer to a valid PARCObjectDescriptor instance.
 *
 * @return true @p object is an instance of @p descriptor.
 * @return false @p object is not an instance of @p descriptor.
 */
bool parcObject_IsInstanceOf(const PARCObject *object, const PARCObjectDescriptor *descriptor);

/**
 * Atomically set an object's barrier.
 *
 * If the barrier is not set, the barrier will be set and this function returns `true`.
 *
 * If the barrier is already set, any subsequent attempt to set the barrier will block until the barrier is unset
 * (see `parcObject_BarrierUnset`).
 * If there are multiple competitors to set the barrier,
 * only one will (indiscriminately) succeed and return and the remaining will continue to attempt to set the barrier.
 *
 * Barriers can be used in both threaded and non-threaded applications,
 * but are not a substitute for thread locking and do not interoperate the wait and notify operations.
 *
 * Barriers should be used in pairs within the same level of program abstraction to avoid confusion.
 * It is possible to set a barrier without ever unsetting the same barrier,
 * and as a consequence any other attempt to set the same barrier will hang the program.
 *
 * @param [in] object A pointer to a valid PARCObject
 *
 * @return true
 *
 * Example:
 * @code
 * {
 *     parcObject_BarrierSet(object);
 *
 *     ...
 *
 *     parcObject_BarrierUnset(object);
 * }
 * @endcode
 * @see parcObject_BarrierUnset
 * @see parcObject_Synchronize
 */
bool parcObject_BarrierSet(const PARCObject *object);

/**
 * Unset an objects' barrier.
 *
 * If a barrier is set (see `parcObject_BarrierSet`), the barrier is unset (see `parcObject_BarrierUnset`).
 *
 * If a barrier is not set, this function will block until the barrier is set,
 * whereupon it will be immediately unset the barrier and return.
 *
 * If there are multiple competitors attempting to unset the barrier,
 * only one will (indiscriminately) succeed and return and the remaining will continue to attempt to unset the barrier.
 *
 * Barriers are not a substitute for thread locking and do not interoperate the wait and notify operations.
 *
 *
 * @param [in] object A pointer to a valid `PARCObject`
 *
 * @return false
 *
 * Example:
 * @code
 * {
 *     parcObject_BarrierSet(object);
 *
 *     ...
 *
 *     parcObject_BarrierUnset(object);
 * }
 * @endcode
 */
bool parcObject_BarrierUnset(const PARCObject *object);

/**
 * Synchronize on a `PARCObject` instance to provide a simple mutual exclusion region of code.
 *
 * This macro uses the functions `parcObject_BarrierSet` and `parcObject_BarrierUnset`
 * to provide a simple syntax for implementing a mutual exclusion region of code.
 *
 * This defines and uses the local variable `_parcObjectSynchronize` which will always appear to be true to the calling function
 * and must never be defined by the calling function, or any module.
 *
 * @param [in] object A pointer to a valid `PARCObject`
 *
 * Example:
 * @code
 * {
 *     parcObject_Synchronize(_object_) {
 *         // Only one thread executes this code at a single time.
 *     }
 * }
 * @endcode
 * @see parcObject_Mutex
 */
#define parcObject_Synchronize(_object_) for (bool _parcObjectSynchronize = parcObject_BarrierSet(_object_); _parcObjectSynchronize; _parcObjectSynchronize = parcObject_BarrierUnset(_object_))

#endif // libparc_parc_Object_h

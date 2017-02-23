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
 */
#include <config.h>
#include <LongBow/runtime.h>
#include <inttypes.h>

#include <parc/testing/parc_ObjectTesting.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_Memory.h>

void
parcObjectTesting_AssertAcquireReleaseContractImpl(void *(acquireFunction)(const PARCObject *), const PARCObject *instance)
{
    void *reference = acquireFunction(instance);
    assertTrue(reference == instance, "Expected the acquire function to return the same instance pointer.");
    parcObject_Release(&reference);
    parcObjectTesting_AssertAcquireReleaseImpl(instance);
}

void
parcObjectTesting_AssertAcquireReleaseImpl(const PARCObject *instance)
{
    PARCReferenceCount originalReferences = parcObject_GetReferenceCount(instance);

    PARCObject *newReference = parcObject_Acquire(instance);

    assertTrue(newReference == instance, "Expected the acquire function to return the same instance pointer.");

    PARCReferenceCount currentReferences = parcObject_GetReferenceCount(instance);
    assertTrue(currentReferences == (originalReferences + 1),
               "Expected references to be %" PRIu64 ", actual %" PRIu64, (originalReferences + 1), currentReferences);

    parcObject_Release(&newReference);
    currentReferences = parcObject_GetReferenceCount(instance);
    assertTrue(currentReferences == originalReferences,
               "Expected references to be %" PRIu64 ", actual %" PRIu64, originalReferences, currentReferences);
}

static void
_parcObjectTesting_AssertEquals(bool (*equalsFunction)(const void *a, const void *b), const void *x, const void *y, const void *z, va_list ap)
{
    assertNotNull(x, "The value of x cannot be NULL.");
    assertNotNull(y, "The value of y cannot be NULL.");
    assertNotNull(z, "The value of z cannot be NULL.");

    assertFalse(x == y, "The value x cannot be the same as y");
    assertFalse(x == z, "The value x cannot be the same as z");
    assertFalse(y == z, "The value y cannot be the same as z");

    assertTrue(equalsFunction(NULL, NULL), "Equality failed: Equals(NULL, NULL) must be true");

    assertFalse(equalsFunction(x, NULL), "Equality failed: The value of x must not be Equal to NULL.");
    assertFalse(equalsFunction(NULL, x), "Equality failed: NULL must not be equal to the value of x.");

    assertTrue(equalsFunction(x, x), "Reflexive failed: for any non-null reference value x, equals(x, x) must return true.");

    assertTrue(equalsFunction(x, y), "Equality failed: The values of x and y must be Equal.");
    assertTrue(equalsFunction(x, z), "Equality failed: The values of x and z must be Equal.");

    assertTrue(equalsFunction(x, y) == equalsFunction(y, x), "Symmetric equality failed: equals(x, y) == equals(y, x) must true.");

    assertTrue((equalsFunction(x, y) == equalsFunction(y, z)) == equalsFunction(z, x),
               "Transitive equality failed: equals(x, y) == equals(y, z) == equals(z, x) must true.");

    int index = 0;
    for (void *value = va_arg(ap, void *); value != 0; value = va_arg(ap, void *)) {
        assertFalse(equalsFunction(x, value), "Value %d (@%p) must not be equal to x", index, value);
        assertTrue(equalsFunction(x, value) == equalsFunction(value, x),
                   "Symmetric equality failed: equals(x, value) == equals(value, x) must true.");
        index++;
    }
}

void
parcObjectTesting_AssertEquals(const PARCObject *x, const void *y, const void *z, ...)
{
    va_list ap;
    va_start(ap, z);

    _parcObjectTesting_AssertEquals((bool (*)(const void *, const void *))parcObject_Equals, x, y, z, ap);

    assertTrue(parcObject_HashCode(x) == parcObject_HashCode(y),
               "HashCode of x and y must be equal");
    assertTrue(parcObject_HashCode(x) == parcObject_HashCode(z),
               "HashCode of x and z must be equal");

    va_end(ap);
}

void
parcObjectTesting_AssertEqualsFunctionImpl(bool (*equalsFunction)(const void *a, const void *b), const void *x, const void *y, const void *z, ...)
{
    va_list ap;
    va_start(ap, z);
    _parcObjectTesting_AssertEquals(equalsFunction, x, y, z, ap);

    va_end(ap);
}

bool
parcObjectTesting_AssertCompareToImpl(int (*compareTo)(const void *a, const void *b),
                                      const void *exemplar,
                                      const void **equivalent,
                                      const void **lesser,
                                      const void **greater)
{
    assertNotNull(exemplar, "Parameter exemplar must not be NULL");
    assertNotNull(equivalent, "Parameter equivalent must not be NULL");
    assertNotNull(lesser, "Parameter less must not be NULL");
    assertNotNull(greater, "Parameter greater must not be NULL");

    assertTrue(compareTo(NULL, NULL) == 0, "Comparison of null values must be 0.");

    assertTrue(compareTo(exemplar, NULL) > 0, "Comparison of a non-null value to a null value must be > 0.");

    assertTrue(compareTo(NULL, exemplar) < 0, "Comparison of null value to a non-null value must be < 0.");

    assertTrue(compareTo(exemplar, exemplar) == 0, "Comparison of a value to itself must == 0");

    for (int i = 0; equivalent[i] != NULL; i++) {
        assertTrue(compareTo(exemplar, equivalent[i]) == 0,
                   "Comparison of the value to equivalent[%d] must == 0", i);
        assertTrue(compareTo(exemplar, equivalent[i]) == -compareTo(equivalent[i], exemplar),
                   "Requires sgn(compareTo(value, equivalent[%d])) == -sgn(equivalent[%d], value)", i, i);
    }
    for (int i = 0; lesser[i] != NULL; i++) {
        assertTrue(compareTo(exemplar, lesser[i]) > 0,
                   "Compare of value to lesser[%d] must be > 0", i);
        assertTrue(compareTo(exemplar, lesser[i]) == -compareTo(lesser[i], exemplar),
                   "Requires signum(compareTo(value, lesser[%d])) == -signum(lesser[%d], value)", i, i);
    }
    for (int i = 0; greater[i] != NULL; i++) {
        assertTrue(compareTo(exemplar, greater[i]) < 0, "Compare to greater[%d] must be > 0", i);
        assertTrue(compareTo(exemplar, greater[i]) == -compareTo(greater[i], exemplar),
                   "Requires compareTo(value, greater[%d]) == -compareTo(greater[%d], value)", i, i);
    }

    return true;
}

void
parcObjectTesting_AssertHashCode(const PARCObject *x, const void *y)
{
    assertFalse(x == y, "The parameters x and y cannot be the same value.");
    assertTrue(parcObject_Equals(x, y), "The parameters x and y must be equal");

    PARCHashCode xCode = parcObject_HashCode(x);
    PARCHashCode yCode = parcObject_HashCode(y);

    assertTrue(xCode == yCode, "Expected the HashCode of two equal objects to be equal.");
}

void
parcObjectTesting_AssertHashCodeImpl(PARCHashCode (*hashCode)(const void *a), void *a)
{
    PARCHashCode code1 = hashCode(a);
    PARCHashCode code2 = hashCode(a);
    assertTrue(code1 == code2, "HashCode function does not consistently return the same value.");
}

static void
_parcObjectTesting_AssertCopy(const PARCObject *instance)
{
    PARCObject *copy = parcObject_Copy(instance);
    if (copy == instance) {
        parcObject_Release(&copy);
        assertFalse(true, "Copy should not be the same object");
    }
    if (!parcObject_Equals(instance, copy)) {
        parcObject_Release(&copy);
        assertTrue(false, "Object fails Copy Test");
    }

    parcObject_Release(&copy);
}

static void
_parcObjectTesting_AssertEqualsWrapper(const PARCObject *a,
                                       const PARCObject *b,
                                       const PARCObject *c,
                                       ...)
{
    va_list ap;
    va_start(ap, c);

    _parcObjectTesting_AssertEquals((bool (*)(const void *, const void *))parcObject_Equals, a, b, c, ap);

    va_end(ap);
}

static void
_parcObjectTesting_AssertToJSON(const PARCObject *instance)
{
    PARCJSON *json = parcObject_ToJSON(instance);
    char *result = parcJSON_ToString(json);
    assertNotNull(result, "Something should be returned");
    parcMemory_Deallocate(&result);
    parcJSON_Release(&json);
}

static void
_parcObjectTesting_AssertToString(const PARCObject *instance)
{
    char *result = parcObject_ToString(instance);
    assertNotNull(result, "Something should be returned");
    parcMemory_Deallocate(&result);
}

void
parcObjectTesting_AssertObjectConformance(const PARCObject *inst1,
                                          const PARCObject *inst2,
                                          const PARCObject *inst3,
                                          const PARCObject *lesser,
                                          const PARCObject *greater)
{
    assertNotNull(inst1, "The value of x cannot be NULL.");
    assertNotNull(inst2, "The value of y cannot be NULL.");
    assertNotNull(inst3, "The value of z cannot be NULL.");
    assertNotNull(lesser, "The value of z cannot be NULL.");
    assertNotNull(greater, "The value of z cannot be NULL.");

    parcObject_AssertValid(inst1);
    parcObject_AssertValid(inst2);
    parcObject_AssertValid(inst3);
    parcObject_AssertValid(lesser);
    parcObject_AssertValid(greater);

    // Acquire/Release
    parcObjectTesting_AssertAcquireReleaseImpl(inst1);

    // Equals
    _parcObjectTesting_AssertEqualsWrapper(inst1, inst2, inst3, lesser, greater, NULL);

    // Copy
    _parcObjectTesting_AssertCopy(inst1);

    // Compare
    const void *equals[] = { inst1, inst2, NULL };
    const void *lessThan[] = { lesser, NULL };
    const void *greaterThan[] = { greater, NULL };
    parcObjectTesting_AssertCompareToImpl(parcObject_Compare, inst1, equals, lessThan, greaterThan);

    // HashCode
    parcObjectTesting_AssertHashCode(inst1, inst2);

    // ToJSON
    _parcObjectTesting_AssertToJSON(inst1);

    // ToString
    _parcObjectTesting_AssertToString(inst1);
}

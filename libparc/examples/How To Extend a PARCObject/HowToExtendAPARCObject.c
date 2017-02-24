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
 *
 * This example shows a simple extention of an existing PARC Object implementation
 * (PARCString) to replace the default implementation of the Compare() function with another.
 *
 * The demonstration shows how to reverse the sort order of a PARCSortedList
 * containing a list of PARCString instances without changing PARCSortedList nor PARCString.
 *
 */
#include <stdio.h>

#include <parc/algol/parc_SortedList.h>
#include <parc/algol/parc_String.h>

/*
 * This is the default behaviour of the PARCSortedList implementation.
 */
void
forwardOrder(void)
{
    PARCSortedList *sortedList = parcSortedList_Create();

    PARCString *aaa = parcString_Create("aaa");
    PARCString *aab = parcString_Create("aab");
    PARCString *aac = parcString_Create("aac");

    parcSortedList_Add(sortedList, aaa);
    parcSortedList_Add(sortedList, aab);
    parcSortedList_Add(sortedList, aac);

    parcSortedList_Display(sortedList, 0);
    parcString_Release(&aaa);
    parcString_Release(&aab);
    parcString_Release(&aac);
}

/*
 * This function will be substituted for the default Compare implementation in the PARCString object.
 */
int
parcString_ReverseCompare(const PARCString *string, const PARCString *other)
{
    return parcString_Compare(string, other) * -1;
}

parcObject_Extends(PARCReverseString, PARCString,
                   .compare = (PARCObjectCompare *) parcString_ReverseCompare);


PARCString *
parcMyString_Create(const char *string)
{
    PARCString *result = parcString_Create(string);

    // By setting the descriptor to our special descriptor here, we effectively
    // substitute the default compare function with our parcString_ReverseCompare
    parcObject_SetDescriptor(result, &parcObject_DescriptorName(PARCReverseString));

    return result;
}

void
reverseOrder(void)
{
    PARCSortedList *sortedList = parcSortedList_Create();

    PARCString *aaa = parcMyString_Create("aaa");
    PARCString *aab = parcMyString_Create("aab");
    PARCString *aac = parcMyString_Create("aac");

    parcSortedList_Add(sortedList, aaa);
    parcSortedList_Add(sortedList, aab);
    parcSortedList_Add(sortedList, aac);

    parcSortedList_Display(sortedList, 0);
    parcString_Release(&aaa);
    parcString_Release(&aab);
    parcString_Release(&aac);
}

int
main(int argc, const char *argv[argc])
{
    forwardOrder();
    reverseOrder();
    return 0;
}

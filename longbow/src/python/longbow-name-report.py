#! /usr/bin/env python
# Copyright (c) 2017 Cisco and/or its affiliates.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#

import os
import sys
import argparse

sys.path.append("../site-packages/longbow/")
sys.path.append("@INSTALL_PYTHON_DIR@")
import NameReport

if __name__ == '__main__':
    '''
    @(#) name-report @VERSION@ @DATE@
    @(#)   All Rights Reserved. Use is subject to license terms.
    '''
    desc = '''
Print a score representing the percentage of compliance with the naming conventions for one or more C source and object files.

$ ./longbow-name-report parc_Object.c
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-names 100.00 enum-names 100.0 typedef-names 100.0
$
$ echo parc_Object.c | ./parc-name-grade -
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-names 100.00 enum-names 100.0 typedef-names 100.0
$

Default Output (--summary):
namespace, module-name[, topic, score]

namespace: Namespace of the file, like `parc`
module-name: The name of the file, like `parc_ArrayList.c`
topic: The name of the topic: valid-name, function-name-conformance, or enum-name-conformance
score: A context-sensitive value related to the topic (valid-name: True/False, function/enum-name-conformance: 0-100)

Finegrain Output (--finegrain):
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-names 100.00 enum-names 100.0 typedef-names 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_Acquire 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_AssertValid 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_Compare 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_Copy 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_Create 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_Display 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_Equals 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_GetReferenceCount 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_HashCode 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_Release 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_TestAcquireContractRaw 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object function-name parcObject_ToJSON 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/LibpgetEnumerationsFromFilesarc/parc/algol parc_Object function-name parcObject_ToString 100.0
/Users/cwood/Projects/DistilleryBranches/Case1073/Libparc/parc/algol parc_Object typedef-name _ObjectHeader 100.0
'''

    parser = argparse.ArgumentParser(prog='longbow-name-report', formatter_class=argparse.RawDescriptionHelpFormatter, description=desc)
    parser.add_argument('-a', '--average', default=False, action="store_true", help="Print an average summary of the naming conformance results for all modules")
    parser.add_argument('-s', '--summary', default=False, action="store_true", help="Print a summary of the naming conformance results for each module")
    parser.add_argument('-f', '--finegrain', default=False, action="store_true", help="Print the individual results for each function, typedef, and enumeration in each module.")
    parser.add_argument('-o', '--output', default="text", action="store", required=False, type=str, help="the output format: \"text\" or \"csv\"")
    parser.add_argument('-d', '--distribution', default="[99, 90]", action="store", required=False, type=str, help="a list containing the score distributions for pretty-printing. Default [99, 90]")
    parser.add_argument('-t', '--trace', default=False, action="store_true", help="Turn on exception tracing to debug an issue with the tool.")
    parser.add_argument('-', '--stdin', default=False, action="store_true", required=False, help="Read the list of files from standard input.")
    parser.add_argument('-p', '--opath', default="", action="store", required=False, type=str, help="Specify the path for object files, can be a path to a static library.")
    parser.add_argument("files", help="Files to check", nargs="*")

    args = parser.parse_args()

    targets = []
    if args.stdin:
        for line in sys.stdin:
            targets.append(line.strip())
    else:
        targets = args.files

    if (len(targets) == 0):
        parser.print_usage()
        sys.exit(1)

    NameReport.commandLineMain(args, targets, args.opath)

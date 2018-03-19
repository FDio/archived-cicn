
# Introduction
The git repository structure is described in this file. It can also be derived
by scanning the repo itself. The master branch is read only and contains this
README.md file only.

The repository is organized in several orphaned branches, each one containing
a sub-project. The naming convention naming branches is described in this
document.

## Branch naming conventions
Project cicn contains several sub-projects. Sub-project *subp* has a master
branch with name  origin/subp/master. All commits associated to sub-project
subp will belong to the orphaned branch origin/subp. All branches associated
to subp must be named as subp/branch-name.

## Sub projects contained in the cicn git repository

1. cicn-plugin
2. sb-forwarder
3. libicnet
4. cframework
5. ccnxlib
6. http-server
7. viper
8. vicn
9. android-sdk

## Sub projects description

Name                | Description             |  Language and style
------------------- | ----------------------- | -------------------
1. cicn-plugin      | VPP forwarder           | C GNU style
2. sb-forwarder     | socket-based forwarder  | C GNU style
3. libicnet         | socket API              | C++11 Google style
4. cframework       | C framework             | C GNU style
5. ccnxlibs         | CCNx libraries          | C GNU style
6. http-server      | HTTP server             | C++11 Google style
7. viper            | Qt/QML video player     | C++/QML Qt style
8. vicn             | vICN framework          | python-3 and bash
9. android-sdk      | Android SDK for ICN     | cmake

## Example:

For sub-project cicn-plugin, the master branch is cicn-plugin/master
that can be cloned as follows:

$ git clone -b cicn-plugin/master https://gerrit.fd.io/r/cicn cicn-plugin
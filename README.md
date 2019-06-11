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

| Name                | Description             |  Language and style |
| ------------------- | ----------------------- | ------------------- |
| 1. cicn-plugin      | VPP forwarder           | C GNU style         |
| 2. sb-forwarder     | socket-based forwarder  | C GNU style         |
| 3. libicnet         | socket API              | C++11 Google style  |
| 4. cframework       | C framework             | C GNU style         |
| 5. ccnxlibs         | CCNx libraries          | C GNU style         |
| 6. http-server      | HTTP server             | C++11 Google style  |
| 7. viper            | Qt/QML video player     | C++/QML Qt style    |
| 8. vicn             | vICN framework          | python-3 and bash   |
| 9. android-sdk      | Android SDK for ICN     | cmake               |

## Example:

For sub-project cicn-plugin, the master branch is cicn-plugin/master
that can be cloned as follows:

```bash
$ git clone -b cicn-plugin/master https://gerrit.fd.io/r/cicn cicn-plugin
```

## How to manage different master branches

It is suggested to clone each subproject branch in a different workspace to 
avoid error prone operations. The cicn git repo stores several projects which
are independent one to another. While access control and isolation is
guaranteed at a certain level by gerrit, using one single workspace is
discouraged.

```bash
git clone -b cicn-plugin/master  https://gerrit.fd.io/r/cicn cicn-plugin;
git clone -b sb-forwarder/master https://gerrit.fd.io/r/cicn sb-forwarder;
git clone -b libicnet/master     https://gerrit.fd.io/r/cicn libicnet;
git clone -b cframework/master   https://gerrit.fd.io/r/cicn cframework;
git clone -b ccnxlibs/master     https://gerrit.fd.io/r/cicn ccnxlibs;
git clone -b http-server/master  https://gerrit.fd.io/r/cicn http-server;
git clone -b viper/master        https://gerrit.fd.io/r/cicn viper;
git clone -b vicn/master         https://gerrit.fd.io/r/cicn vicn;
git clone -b android-sdk         https://gerrit.fd.io/r/cicn android-sdk;
```

### For committers

By having multiple sub-projects in the same repo, it is highly recommended
to use the following approach while using branches and pushing patch sets.

```bash
$ subp = cicm-plugin
$ committer = user

$ git clone -b cicn-plugin/master ssh://committer@gerrit.fd.io:29418/cicn subp;
$ scp -p -P 29418 committer@gerrit.fd.io:hooks/commit-msg subp/.git/hooks/;
```

If you use an email alias like user+fdio@email.com that is registered in the
gerrit frontend it is recommended to set the following kind of configuration

```bash
$ git config --local user.email "$committer+fdio@email.com"
$ git config --local alias.push-for-review "push origin HEAD:refs/for/$subp/master"
```

this allows to avoid pushing for review to different sub-project branches
using the command

```bash
$ git push-for-review
```

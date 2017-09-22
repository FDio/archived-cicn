k#!/bin/bash
set -euxo pipefail
IFS=$'\n\t'

SCRIPT_PATH=$( cd "$(dirname "${BASH_SOURCE}")" ; pwd -P )
APT_PATH=`which apt-get` || true
apt_get=${APT_PATH:-"/usr/local/bin/apt-get"}

BUILD_TOOLS_UBUNTU="build-essential doxygen"
DEPS_UBUNTU="devscripts debhelper python3-all python3-setuptools python3-docutils python3-sphinx python3-networkx python3-pyparsing python3-pip"
DEPS_PYTHON="autobahn pyopenssl"

setup() {

    DISTRIB_ID=$1

    if [ "$DISTRIB_ID" == "Ubuntu" ]; then
        sudo ${apt_get} update || true
    fi
}

# Parameters:
# $1 = Package name
#
build_package() {

    PACKAGE_NAME=$1

    ARCHITECTURE="all"

    # Figure out what system we are running on
    if [ -f /etc/lsb-release ];then
        . /etc/lsb-release
        DEB=ON
        RPM=OFF
    else
        echo "ERROR: System configuration not recognized. Build failed"
        exit -1
    fi

    echo ARCHITECTURE: $ARCHITECTURE
    echo DISTRIB_ID: $DISTRIB_ID
    echo DISTRIB_RELEASE: $DISTRIB_RELEASE
    echo DISTRIB_CODENAME: $DISTRIB_CODENAME
    echo DISTRIB_DESCRIPTION: $DISTRIB_DESCRIPTION

    setup $DISTRIB_ID
    # Install package dependencies
    if [ $DISTRIB_ID == "Ubuntu" ]; then
        echo $BUILD_TOOLS_UBUNTU $DEPS_UBUNTU | xargs sudo ${apt_get} install -y --allow-unauthenticated
        echo $DEPS_PYTHON | xargs sudo pip3 install --upgrade
    fi

    # do nothing but print the current slave hostname
    hostname

    # Make the package
    VERSION=$(bash $SCRIPT_PATH/version)

    cat << EOF > $SCRIPT_PATH/../debian/changelog
vicn ($VERSION) UNRELEASED; urgency=medium

  * Initial release (Closes: #nnnn)

 -- Mauro Sardara <mauro.sardara@cisco.com>  Tue, 18 Oct 2016 12:10:07 +0200
EOF

    mkdir -p $SCRIPT_PATH/../vicn_build_root
    ls -1 | while read line; do if [ "$line" != "$(basename $SCRIPT_PATH)" ]; then mv $line $SCRIPT_PATH/../vicn_build_root; fi done || true
    cd $SCRIPT_PATH/../vicn_build_root

    debuild --no-lintian --no-tgz-check -i -us -uc -b

    cd $SCRIPT_PATH/..
    mkdir build
    mv *.deb build

    echo "*******************************************************************"
    echo "* $PACKAGE_NAME BUILD SUCCESSFULLY COMPLETED"
    echo "*******************************************************************"

    exit 0
}

PACKAGE_NAME="VICN"
pushd $SCRIPT_PATH/..
build_package $PACKAGE_NAME
popd

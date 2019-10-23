#!/bin/bash
# basic build script example
set -euxo pipefail
IFS=$'\n\t'
SCRIPT_PATH=$( cd "$(dirname "${BASH_SOURCE}")" ; pwd -P )
APT_PATH=`which apt-get` || true
apt_get=${APT_PATH:-"/usr/local/bin/apt-get"}

BUILD_TOOLS_UBUNTU="build-essential doxygen xmlstarlet"
LIBSSL_LIBEVENT_UBUNTU="libevent-dev libssl-dev"
DEPS_UBUNTU="$LIBSSL_LIBEVENT_UBUNTU"

BUILD_TOOLS_GROUP_CENTOS="'Development Tools'"
LIBSSL_LIBEVENT_CENTOS="libevent-devel openssl-devel"
DEPS_CENTOS="$LIBSSL_LIBEVENT_CENTOS"

PACKAGECLOUD_RELEASE_REPO_DEB="https://packagecloud.io/install/repositories/fdio/release/script.deb.sh"
PACKAGECLOUD_RELEASE_REPO_RPM="https://packagecloud.io/install/repositories/fdio/release/script.rpm.sh"

LATEST_EPEL_REPO="http://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm"

update_cmake_repo() {

    cat /etc/resolv.conf
    echo "nameserver 8.8.8.8" | sudo tee -a /etc/resolv.conf
    cat /etc/resolv.conf

    CMAKE_INSTALL_SCRIPT_URL="https://cmake.org/files/v3.8/cmake-3.8.0-Linux-x86_64.sh"
    CMAKE_INSTALL_SCRIPT="/tmp/install_cmake.sh"
    curl ${CMAKE_INSTALL_SCRIPT_URL} > ${CMAKE_INSTALL_SCRIPT}

    sudo mkdir -p /opt/cmake
    sudo bash ${CMAKE_INSTALL_SCRIPT} --skip-license --prefix=/opt/cmake
    export PATH=/opt/cmake/bin:$PATH
}

# Parameters:
# $1 = Distribution codename
#
update_qt_repo() {
    DISTRIBUTION_CODENAME=$1

    if [ "$DISTRIBUTION_CODENAME" != "bionic" ] && [ "$DISTRIBUTION_CODENAME" != "xenial" ]; then
        echo "No valid distribution specified when calling 'update_qt_repo'. Exiting.."
        exit -1
    fi

    sudo DEBIAN_FRONTEND=noninteractive ${apt_get} install -y --allow-unauthenticated software-properties-common
    sudo add-apt-repository --yes ppa:beineri/opt-qt571-$DISTRIBUTION_CODENAME

    wget -q -O - http://archive.getdeb.net/getdeb-archive.key | sudo apt-key add -
    sudo sh -c "echo 'deb http://archive.getdeb.net/ubuntu $DISTRIBUTION_CODENAME-getdeb apps' >> /etc/apt/sources.list.d/getdeb.list"

    sudo ${apt_get} update
}

# Parameters:
# $1 = Distribution id
# $2 = Distribution codename
#
update_fdio_repo() {
    DISTRIB_ID=$1
    DISTRIB_CODENAME=$2

    NEXUS_PROXY=${NEXUSPROXY:-"http://nexus.fd.io"}
    REPO_CICN_URL=""
    REPO_VPP_URL=""

    if [ "$DISTRIB_ID" == "Ubuntu" ]; then
        curl -s ${PACKAGECLOUD_RELEASE_REPO_DEB} | sudo bash
    elif [ "$DISTRIB_ID" == "CentOS" ]; then
        curl -s ${PACKAGECLOUD_RELEASE_REPO_RPM} | sudo bash
        curl ${LATEST_EPEL_REPO} > epel-release-latest-7.noarch.rpm
        rpm -ivh epel-release-latest-7.noarch.rpm || true
        rm epel-release-latest-7.noarch.rpm
    else
        echo "Distribution $DISTRIB_CODENAME is not supported"
        exit -1
    fi
}

setup() {

    DISTRIB_ID=$1

    DISTRIB_CODENAME=$2

    ARCH=`uname -m`
    if [ "$ARCH" == "x86_64" ] || [ "$ARCH" == "x86" ]; then
        update_cmake_repo
    fi
    update_fdio_repo $DISTRIB_ID $DISTRIB_CODENAME


    if [ "$DISTRIB_ID" == "Ubuntu" ]; then
        sudo ${apt_get} update || true
    fi
}

# Parameters:
# $1 = Package name
#
build_package() {

    PACKAGE_NAME=$1

    ARCHITECTURE=`uname -m`

    # Figure out what system we are running on
    if [ -f /etc/lsb-release ];then

        . /etc/lsb-release
        DEB=ON
        RPM=OFF

        if [ "$ARCHITECTURE" == "x86_64" ]; then
            ARCHITECTURE="amd64"
        elif [ "$ARCHITECTURE" == "aarch64" ]; then
            ARCHITECTURE="arm64"
        fi

    elif [ -f /etc/redhat-release ];then

        sudo yum install -y redhat-lsb
        DISTRIB_ID=`lsb_release -si`
        DISTRIB_RELEASE=`lsb_release -sr`
        DISTRIB_CODENAME=`lsb_release -sc`
        DISTRIB_DESCRIPTION=`lsb_release -sd`

        DEB=OFF
        RPM=ON
    else
        echo "ERROR: System configuration not recognized. Build failed"
        exit -1
    fi

    echo ARCHITECTURE: $ARCHITECTURE
    echo DISTRIB_ID: $DISTRIB_ID
    echo DISTRIB_RELEASE: $DISTRIB_RELEASE
    echo DISTRIB_CODENAME: $DISTRIB_CODENAME
    echo DISTRIB_DESCRIPTION: $DISTRIB_DESCRIPTION

    setup $DISTRIB_ID $DISTRIB_CODENAME $ARCHITECTURE

    # Install package dependencies
    if [ $DISTRIB_ID == "Ubuntu" ]; then
        echo $BUILD_TOOLS_UBUNTU $DEPS_UBUNTU | xargs sudo DEBIAN_FRONTEND=noninteractive ${apt_get} install -y --allow-unauthenticated
    elif [ $DISTRIB_ID == "CentOS" ]; then
        echo $BUILD_TOOLS_GROUP_CENTOS | xargs sudo yum groupinstall -y --nogpgcheck
        echo $DEPS_CENTOS | xargs sudo yum install -y --nogpgcheck || true
    fi

    # do nothing but print the current slave hostname
    hostname

    # Make the package
    mkdir -p $SCRIPT_PATH/../build && pushd $SCRIPT_PATH/../build

    rm -rf *
    cmake -DCMAKE_INSTALL_PREFIX=/usr -DRPM_PACKAGE=$RPM -DDEB_PACKAGE=$DEB -DDISTRIBUTION=$DISTRIB_CODENAME -DARCHITECTURE=$ARCHITECTURE ..
    make package

    find . -not -name '*.deb' -not -name '*.rpm' -print0 | xargs -0 rm -rf -- || true

    popd

    echo "*******************************************************************"
    echo "* $PACKAGE_NAME BUILD SUCCESSFULLY COMPLETED"
    echo "*******************************************************************"

    exit 0
}

PACKAGE_NAME="LIBPARC"
PACKAGE_DEPS="LIBPARC_DEPS"
pushd $SCRIPT_PATH/..
build_package $PACKAGE_NAME
popd

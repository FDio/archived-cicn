#!/bin/bash
set -euxo pipefail
IFS=$'\n\t'

SCRIPT_PATH=$( cd "$(dirname "${BASH_SOURCE}")" ; pwd -P )
APT_PATH=`which apt-get` || true
apt_get=${APT_PATH:-"/usr/local/bin/apt-get"}

BUILD_TOOLS_UBUNTU="build-essential cmake"
LIBSSL_LIBEVENT_UBUNTU="libevent-dev libssl-dev"
DEPS_UBUNTU="$LIBSSL_LIBEVENT_UBUNTU longbow-dev libparc-dev libccnx-common-dev libccnx-transport-rta-dev libicnet-dev libboost-system-dev libboost-regex-dev libboost-filesystem-dev"

BUILD_TOOLS_GROUP_CENTOS="'Development Tools'"
BUILD_TOOLS_SINGLE_CENTOS="cmake"
LIBSSL_LIBEVENT_CENTOS="libevent-devel openssl-devel"
DEPS_CENTOS="$LIBSSL_LIBEVENT_CENTOS longbow-devel libparc-devel libccnx-common-devel libccnx-transport-rta-devel libicnet-devel boost-devel"

# Parameters:
# $1 = Distribution [Trusty / CentOS]
#
update_cmake_repo() {

    DISTRIBUTION=$1

    if [ "$DISTRIBUTION" == "trusty" ]; then
        sudo ${apt_get} install -y --allow-unauthenticated software-properties-common
        sudo add-apt-repository --yes ppa:george-edison55/cmake-3.x
    elif [ "$DISTRIBUTION" == "CentOS" ]; then
        sudo cat << EOF > cmake.repo
[cmake-repo]
name=Repo for cmake3
baseurl=http://mirror.ghettoforge.org/distributions/gf/el/7/plus/x86_64/
enabled=1
gpgcheck=0
EOF
        sudo cat << EOF > jsoncpp.repo
[jsoncp-repo]
name=Repo for jsoncpp
baseurl=http://dl.fedoraproject.org/pub/epel/7/x86_64/
enabled=1
gpgcheck=0
EOF
        sudo mv cmake.repo /etc/yum.repos.d/cmake.repo
        sudo mv jsoncpp.repo /etc/yum.repos.d/jsoncpp.repo
    fi
}

# Parameters:
# $1 = Distribution codename
#
update_qt_repo() {
    DISTRIBUTION_CODENAME=$1

    if [ "$DISTRIBUTION_CODENAME" != "trusty" ] && [ "$DISTRIBUTION_CODENAME" != "xenial" ]; then
        echo "No valid distribution specified when calling 'update_qt_repo'. Exiting.."
        exit -1
    fi

    sudo ${apt_get} install -y --allow-unauthenticated software-properties-common
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

        if [ "$DISTRIB_CODENAME" == "xenial" ]; then
            REPO_VPP_URL="${NEXUS_PROXY}/content/repositories/fd.io.stable.1701.ubuntu.xenial.main/"
            REPO=${REPO_NAME:-"master.ubuntu.xenial.main"}
            REPO_CICN_URL="${NEXUS_PROXY}/content/repositories/fd.io.${REPO}"
        elif [ "$DISTRIB_CODENAME" == "trusty" ]; then
            REPO_VPP_URL="${NEXUS_PROXY}/content/repositories/fd.io.stable.1701.ubuntu.trusty.main/"
            REPO=${REPO_NAME:-"master.ubuntu.trusty.main"}
            REPO_CICN_URL="${NEXUS_PROXY}/content/repositories/fd.io.${REPO}"
        else
            echo "Distribution $DISTRIB_CODENAME is not supported"
            exit -1
        fi

        echo "deb ${REPO_VPP_URL} ./" | sudo tee /etc/apt/sources.list.d/99fd.io.list
        echo "deb ${REPO_CICN_URL} ./" | sudo tee /etc/apt/sources.list.d/99fd.io.master.list

    elif [ "$DISTRIB_ID" == "CentOS" ]; then
        REPO_VPP_URL="${NEXUS_PROXY}/content/repositories/fd.io.centos7/"
        REPO=${REPO_NAME:-"master.centos7"}
        REPO_CICN_URL="${NEXUS_PROXY}/content/repositories/fd.io.${REPO}"

                sudo cat << EOF > fdio.repo
[fdio-vpp-master]
name=fd.io master branch latest merge
baseurl=${REPO_VPP_URL}
enabled=1
gpgcheck=0

[fdio-cicn-master]
name=fd.io master branch latest merge
baseurl=${REPO_CICN_URL}
enabled=1
gpgcheck=0
EOF
        sudo mv fdio.repo /etc/yum.repos.d/fdio.repo
    else
        echo "Distribution $DISTRIB_CODENAME is not supported"
        exit -1
    fi
}

setup() {

    DISTRIB_ID=$1
    DISTRIB_CODENAME=$2

    if [ "$DISTRIB_ID" == "Ubuntu" ]; then
        if [ "$DISTRIB_CODENAME" == "trusty" ]; then
            update_cmake_repo $DISTRIB_CODENAME
        fi

        update_fdio_repo $DISTRIB_ID $DISTRIB_CODENAME

        sudo ${apt_get} update || true

    elif [ "$DISTRIB_ID" == "CentOS" ]; then
        update_cmake_repo $DISTRIB_ID
        update_fdio_repo $DISTRIB_ID $DISTRIB_CODENAME
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

    setup $DISTRIB_ID $DISTRIB_CODENAME

    # Install package dependencies
    if [ $DISTRIB_ID == "Ubuntu" ]; then
        echo $BUILD_TOOLS_UBUNTU $DEPS_UBUNTU | xargs sudo ${apt_get} install -y --allow-unauthenticated
    elif [ $DISTRIB_ID == "CentOS" ]; then
        echo $BUILD_TOOLS_GROUP_CENTOS | xargs sudo yum groupinstall -y --nogpgcheck
        echo $BUILD_TOOLS_SINGLE_CENTOS | xargs sudo yum install -y --nogpgcheck
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

PACKAGE_NAME="HTTP_SERVER"
pushd $SCRIPT_PATH/..
build_package $PACKAGE_NAME
popd

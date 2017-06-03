#!/bin/bash
# basic build script example
set -euxo pipefail
IFS=$'\n\t'

SCRIPT_PATH=$( cd "$(dirname "${BASH_SOURCE}")" ; pwd -P )
APT_PATH=`which apt-get` || true
apt_get=${APT_PATH:-"/usr/local/bin/apt-get"}

PACKAGE_NAME="RADIO_EMULATOR"
RADIO_EMULATOR_DEPS_UBUNTU="pkg-config libboost-all-dev libsqlite3-dev libopenmpi-dev libxml2-dev libwebsocketpp-dev"

BUILD_TOOLS="build-essential cmake"

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

# Parameters
# $1 = DISTRIB_ID
# $2 = DISTRIB_CODENAME
#
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

# Parameters
# $1 = WIFI / LTE
#
build() {
    PARAM=$1
    mkdir -p build
    cd build
    find . -not -name '*.deb' -not -name '*.rpm' -print0 | xargs -0 rm -rf -- || true
    echo $PARAM | xargs cmake -DCMAKE_INSTALL_PREFIX=/usr -DRPM_PACKAGE=$RPM -DDEB_PACKAGE=$DEB -DDISTRIBUTION=$DISTRIB_CODENAME -DARCHITECTURE=$ARCHITECTURE ..
    make
}

change_version() {
    OLD_PACKAGE=$1
    NEW_PACKAGE=$2
    B_NUMBER=$3

    mkdir tmp
    pushd tmp
    ar p ../${OLD_PACKAGE} control.tar.gz | tar -xz
    sed -i s/3.24.1-8/3.24.1-${B_NUMBER}/g control
    mv ../${OLD_PACKAGE} ../${NEW_PACKAGE}
    tar czf control.tar.gz *[!z]
    ar r ../${NEW_PACKAGE} control.tar.gz
    popd
    rm -rf tmp
}

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

# Install deps

if [ $DISTRIB_ID == "Ubuntu" ]; then
    echo $BUILD_TOOLS $RADIO_EMULATOR_DEPS_UBUNTU | xargs sudo ${apt_get} install -y --allow-unauthenticated || true
else
    echo "This package is currently supported only for ubuntu. Exiting.."
    exit -1
fi

BLD_NUMBER=${BUILD_NUMBER:-"1"}

# Install libns3
pushd ${SCRIPT_PATH}/../ns3-packages
sudo dpkg -i *.deb || true
sudo apt-get -f install -y --allow-unauthenticated || true
popd

# Build wifi-emualtor
pushd ${SCRIPT_PATH}/..
build "-DWIFI=ON -DLTE=OFF"
make package
find . -not -name '*.deb' -not -name '*.rpm' -print0 | xargs -0 rm -rf -- || true
popd

# Build lte-emualtor
pushd ${SCRIPT_PATH}/..
build "-DLTE=ON -DWIFI=OFF"
make package
find . -not -name '*.deb' -not -name '*.rpm' -print0 | xargs -0 rm -rf -- || true
popd

# Change build number to ns3 packages
pushd ${SCRIPT_PATH}/../ns3-packages

change_version libns3sx-3v5_3.24.1-8~xenial_amd64.deb libns3sx-3v5_3.24.1-$BLD_NUMBER~xenial_amd64.deb ${BLD_NUMBER} || true
change_version libns3sx-dev_3.24.1-8~xenial_amd64.deb libns3sx-dev_3.24.1-$BLD_NUMBER~xenial_amd64.deb ${BLD_NUMBER} || true
change_version ns3sx_3.24.1-8~xenial_amd64.deb ns3sx_3.24.1-$BLD_NUMBER~xenial_amd64.deb ${BLD_NUMBER} || true
popd

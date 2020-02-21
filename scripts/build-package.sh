#!/bin/bash
# basic build script example
set -euxo pipefail
IFS=$'\n\t'

SCRIPT_PATH=$( cd "$(dirname "${BASH_SOURCE}")" ; pwd -P )
APT_PATH=`which apt-get` || true
apt_get=${APT_PATH:-"/usr/local/bin/apt-get"}

PACKAGE_NAME="VIPER"
VIPER_DEPS_UBUNTU="zlib1g-dev git-core build-essential libxml2-dev libcurl4-openssl-dev \
libqtav-dev qt5-default libqt5svg5-dev qtdeclarative5-dev libqt5charts5-dev \
qtmultimedia5-dev libqt5multimediawidgets5 libqt5multimedia5-plugins libqt5multimedia5 \
libhicntransport-dev libavcodec-dev libavutil-dev libavformat-dev"

BUILD_TOOLS="build-essential doxygen"


# Parameters:
# $1 = Distribution id
# $2 = Distribution codename
#
update_fdio_repo() {
    DISTRIB_ID=$1
    DISTRIB_CODENAME=$2

    REPO_CICN_URL=""
    REPO_VPP_URL=""

    if [ "$DISTRIB_ID" == "Ubuntu" ]; then
        curl -s https://packagecloud.io/install/repositories/fdio/release/script.deb.sh | sudo bash
    elif [ "$DISTRIB_ID" == "CentOS" ]; then
        curl -s https://packagecloud.io/install/repositories/fdio/release/script.rpm.sh | sudo bash
    else
        echo "Distribution $DISTRIB_CODENAME is not supported"
        exit -1
    fi

}

setup() {

    DISTRIB_ID=$1
    DISTRIB_CODENAME=$2

    update_fdio_repo $DISTRIB_ID $DISTRIB_CODENAME

    if [ "$DISTRIB_ID" == "Ubuntu" ]; then
        sudo ${apt_get} update || true
    fi
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
    echo $BUILD_TOOLS $VIPER_DEPS_UBUNTU | xargs sudo ${apt_get} install -y --allow-unauthenticated
else
    echo "This package is currently supported only for ubuntu. Exiting.."
    exit -1
fi

# Compile libdash

build() {
    mkdir -p build
    cd build
    rm -rf *
    cmake -DCMAKE_INSTALL_PREFIX=/usr -DRPM_PACKAGE=$RPM -DDEB_PACKAGE=$DEB -DDISTRIBUTION=$DISTRIB_CODENAME -DARCHITECTURE=$ARCHITECTURE .. -DHICNET=ON -DICNET=OFF
    make
}

# Build libdash
pushd $SCRIPT_PATH/../libdash
build
make package
sudo make install
find . -not -name '*.deb' -not -name '*.rpm' -print0 | xargs -0 rm -rf -- || true
popd

# Build viper
pushd $SCRIPT_PATH/..
build
make package
find . -not -name '*.deb' -not -name '*.rpm' -print0 | xargs -0 rm -rf -- || true
popd

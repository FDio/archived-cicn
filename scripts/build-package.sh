#!/bin/bash
# basic build script example
set -euxo pipefail
IFS=$'\n\t'

SCRIPT_PATH=$( cd "$(dirname "${BASH_SOURCE}")" ; pwd -P )
APT_PATH=`which apt-get` || true
apt_get=${APT_PATH:-"/usr/local/bin/apt-get"}

PACKAGE_NAME="VIPER"
VIPER_DEPS_UBUNTU="zlib1g-dev git-core build-essential libxml2-dev libcurl4-openssl-dev \
                   qt57base qt57svg qt57charts-no-lgpl qt57multimedia libqtav-dev libicnet-dev \
                   libavcodec-dev libavformat-dev libswscale-dev  libavresample-dev libqml-module-qtav \
                   qt57quickcontrols qt57quickcontrols2 libboost-system-dev"

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
    sudo sh -c "echo 'deb http://archive.getdeb.net/ubuntu xenial-getdeb apps' >> /etc/apt/sources.list.d/getdeb.list"

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
    update_qt_repo $DISTRIB_CODENAME
    echo $BUILD_TOOLS $VIPER_DEPS_UBUNTU | xargs sudo ${apt_get} install -y --allow-unauthenticated
else
    echo "This package is currently supported only for ubuntu. Exiting.."
    exit -1
fi

# Create links

sudo ln -sf /usr/include/x86_64-linux-gnu/qt5/QtAV                                /opt/qt57/include/QtAV
sudo ln -sf /usr/include/x86_64-linux-gnu/qt5/QtAVWidgets                         /opt/qt57/include/QtAVWidgets
sudo ln -sf /usr/lib/x86_64-linux-gnu/qt5/mkspecs/features/av.prf                 /opt/qt57/mkspecs/features/av.prf
sudo ln -sf /usr/lib/x86_64-linux-gnu/qt5/mkspecs/features/avwidgets.prf          /opt/qt57/mkspecs/features/avwidgets.prf
sudo ln -sf /usr/lib/x86_64-linux-gnu/qt5/mkspecs/modules/qt_lib_avwidgets.pri    /opt/qt57/mkspecs/modules/qt_lib_avwidgets.pri
sudo ln -sf /usr/lib/x86_64-linux-gnu/qt5/mkspecs/modules/qt_lib_av.pri           /opt/qt57/mkspecs/modules/qt_lib_av.pri
sudo ln -sf /usr/lib/x86_64-linux-gnu/libQtAV.prl                                 /opt/qt57/lib/libQtAV.prl
sudo ln -sf /usr/lib/x86_64-linux-gnu/libQtAVWidgets.prl                          /opt/qt57/lib/libQtAVWidgets.prl
sudo ln -sf /usr/lib/x86_64-linux-gnu/libQtAVWidgets.so                           /opt/qt57/lib/libQt5AVWidgets.so
sudo ln -sf /usr/lib/x86_64-linux-gnu/libQt5AV.so                                 /opt/qt57/lib/libQt5AV.so
sudo ln -sf /usr/lib/x86_64-linux-gnu/libQtAV.so                                  /opt/qt57/lib/libQtAV.so
sudo ln -sf /usr/lib/x86_64-linux-gnu/libQt5AVWidgets.so                          /opt/qt57/lib/libQtAVWidgets.so
sudo ln -sf /usr/lib/x86_64-linux-gnu/qt5/qml/QtAV                                /opt/qt57/qml/QtAV

# Compile libdash

build() {
    mkdir -p build
    cd build
    rm -rf *
    cmake -DCMAKE_INSTALL_PREFIX=/usr -DRPM_PACKAGE=$RPM -DDEB_PACKAGE=$DEB -DDISTRIBUTION=$DISTRIB_CODENAME -DARCHITECTURE=$ARCHITECTURE ..
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

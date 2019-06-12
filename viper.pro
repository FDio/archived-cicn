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

TARGET = viper
VERSION = $$QTAV_VERSION
QT += av svg qml quick sql core gui opengl multimedia charts
android {
  QT += androidextras
}

CONFIG -= release
CONFIG += debug
CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++14 -g -fpermissive -DASIO_STANDALONE=1 -stdlib=libc++ -DQTC_ENABLE_CLANG_LIBTOOLING=ON
# Add more folders to ship with the application, here
folder_01.source = qml/Viper
folder_01.target = qml


RESOURCES += \
    viper.qrc

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += \
    Adaptation/AbstractAdaptationLogic.cpp \
    Adaptation/AdaptationLogicFactory.cpp \
    Adaptation/AlwaysLowestLogic.cpp \
    Adaptation/BufferBasedAdaptation.cpp \
    Adaptation/AdapTech.cpp \
    Adaptation/BufferBasedThreeThresholdAdaptation.cpp \
    Adaptation/Panda.cpp \
    Adaptation/Bola.cpp \
    Adaptation/RateBasedAdaptation.cpp \
    Input/DASHManager.cpp \
    Input/DASHReceiver.cpp \
    Input/MediaObject.cpp \
    MPD/AbstractRepresentationStream.cpp \
    MPD/AdaptationSetHelper.cpp \
    MPD/AdaptationSetStream.cpp \
    MPD/BaseUrlResolver.cpp \
    MPD/RepresentationStreamFactory.cpp \
    MPD/SegmentListStream.cpp \
    MPD/SegmentTemplateStream.cpp \
    MPD/SingleMediaSegmentStream.cpp \
    MPD/TimeResolver.cpp \
    MPD/MPDWrapper.cpp \
    Portable/MultiThreading.cpp \
    Managers/MultimediaManager.cpp \
    Managers/MultimediaStream.cpp \
    UI/DASHPlayer.cpp \
    UI/DASHPlayerNoGUI.cpp \
    UI/GraphDataSource.cpp \
    Input/ICNConnectionConsumerApi.cpp \
    Websocket/communication-protocol.cpp \
    Websocket/connection-pool.cpp \
    Websocket/query.cpp \
    Websocket/tcp-server.cpp \
    Websocket/websocket-server.cpp \
    Websocket/WebSocketService.cpp \
    UI/ViperGui.cpp \
    Common/ViperBuffer.cpp \
    Common/QtQuick2ApplicationViewer.cpp


lupdate_only{
SOURCES = qml/Viper/*.qml qml/Viper/*.js
}
# Installation path
target.path = $$[QT_INSTALL_BINS]


desktopfile.files = $$PWD/../../qtc_packaging/debian_generic/Viper.desktop
desktopfile.path = /usr/share/applications

COMMON = $$PWD/Common
INCLUDEPATH *= $$COMMON $$COMMON/..
isEmpty(PROJECTROOT): PROJECTROOT = $$PWD

mac: RC_FILE = $$COMMON/Viper.icns
QMAKE_INFO_PLIST = $$COMMON/Info.plist
videos.files = videos
videos.path = /
#QMAKE_BUNDLE_DATA += videos
defineTest(genRC) {
    RC_ICONS = $$COMMON/Cisco.icns
    QMAKE_TARGET_COMPANY = "Cisco Systems"
    QMAKE_TARGET_DESCRIPTION = "ICN Dash Player"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2016-2017 Cisco Systems"
    QMAKE_TARGET_PRODUCT = "Viper $$1"
    export(RC_ICONS)
    export(QMAKE_TARGET_COMPANY)
    export(QMAKE_TARGET_DESCRIPTION)
    export(QMAKE_TARGET_COPYRIGHT)
    export(QMAKE_TARGET_PRODUCT)
    return(true)
}
genRC($$TARGET)
#SystemParametersInfo
!winrt:*msvc*: LIBS += -lUser32
DEFINES += BUILD_QOPT_LIB
HEADERS *= \
    $$COMMON/Common.h \
    $$COMMON/Config.h \
    $$COMMON/QOptions.h \
    $$COMMON/CommonExport.h \
    Adaptation/AbstractAdaptationLogic.h \
    Adaptation/AdaptationLogicFactory.h \
    Adaptation/AlwaysLowestLogic.h \
    Adaptation/BufferBasedAdaptation.h \
    Adaptation/AdapTech.h \
    Adaptation/BufferBasedThreeThresholdAdaptation.h \
    Adaptation/IAdaptationLogic.h \
    Adaptation/Panda.h \
    Adaptation/Bola.h \
    Adaptation/RateBasedAdaptation.h \
    Buffer/Buffer.h \
    Buffer/IBufferObserver.h \
    Input/DASHManager.h \
    Input/DASHReceiver.h \
    Input/IDASHManagerObserver.h \
    Input/IDASHReceiverObserver.h \
    Input/IDataReceiver.h \
    Input/MediaObject.h \
    MPD/AbstractRepresentationStream.h \
    MPD/AdaptationSetHelper.h \
    MPD/AdaptationSetStream.h \
    MPD/BaseUrlResolver.h \
    MPD/IRepresentationStream.h \
    MPD/RepresentationStreamFactory.h \
    MPD/SegmentListStream.h \
    MPD/SegmentTemplateStream.h \
    MPD/SingleMediaSegmentStream.h \
    MPD/TimeResolver.h \
    MPD/IMPDWrapper.h \
    MPD/MPDWrapper.h \
    Portable/MultiThreading.h \
    Portable/Networking.h \
    Managers/IMultimediaManagerBase.h \
    Managers/IMultimediaManagerObserver.h \
    Managers/IStreamObserver.h \
    Managers/MultimediaManager.h \
    Managers/MultimediaStream.h \
    UI/DASHPlayer.h \
    UI/DASHPlayerNoGUI.h \
    UI/IDASHPlayerGuiObserver.h \
    UI/IDASHPlayerNoGuiObserver.h \
    debug.h \
    UI/GraphDataSource.h \
    Input/ICNConnectionConsumerApi.h \
    Input/IICNConnection.h \
    Websocket/communication-protocol.h \
    Websocket/connection-pool.h \
    Websocket/json.h \
    Websocket/query.h \
    Websocket/tcp-server.h \
    Websocket/websocket-server.h \
    Websocket/WebSocketService.h \
    websocketpp/base64/base64.hpp \
    websocketpp/common/asio.hpp \
    websocketpp/common/asio_ssl.hpp \
    websocketpp/common/chrono.hpp \
    websocketpp/common/connection_hdl.hpp \
    websocketpp/common/cpp11.hpp \
    websocketpp/common/functional.hpp \
    websocketpp/common/md5.hpp \
    websocketpp/common/memory.hpp \
    websocketpp/common/network.hpp \
    websocketpp/common/platforms.hpp \
    websocketpp/common/random.hpp \
    websocketpp/common/regex.hpp \
    websocketpp/common/stdint.hpp \
    websocketpp/common/system_error.hpp \
    websocketpp/common/thread.hpp \
    websocketpp/common/time.hpp \
    websocketpp/common/type_traits.hpp \
    websocketpp/concurrency/basic.hpp \
    websocketpp/concurrency/none.hpp \
    websocketpp/config/asio.hpp \
    websocketpp/config/asio_client.hpp \
    websocketpp/config/asio_no_tls.hpp \
    websocketpp/config/asio_no_tls_client.hpp \
    websocketpp/config/boost_config.hpp \
    websocketpp/config/core.hpp \
    websocketpp/config/core_client.hpp \
    websocketpp/config/debug.hpp \
    websocketpp/config/debug_asio.hpp \
    websocketpp/config/debug_asio_no_tls.hpp \
    websocketpp/config/minimal_client.hpp \
    websocketpp/config/minimal_server.hpp \
    websocketpp/extensions/permessage_deflate/disabled.hpp \
    websocketpp/extensions/permessage_deflate/enabled.hpp \
    websocketpp/extensions/extension.hpp \
    websocketpp/http/impl/parser.hpp \
    websocketpp/http/impl/request.hpp \
    websocketpp/http/impl/response.hpp \
    websocketpp/http/constants.hpp \
    websocketpp/http/parser.hpp \
    websocketpp/http/request.hpp \
    websocketpp/http/response.hpp \
    websocketpp/impl/connection_impl.hpp \
    websocketpp/impl/endpoint_impl.hpp \
    websocketpp/impl/utilities_impl.hpp \
    websocketpp/logger/basic.hpp \
    websocketpp/logger/levels.hpp \
    websocketpp/logger/stub.hpp \
    websocketpp/logger/syslog.hpp \
    websocketpp/message_buffer/alloc.hpp \
    websocketpp/message_buffer/message.hpp \
    websocketpp/message_buffer/pool.hpp \
    websocketpp/processors/base.hpp \
    websocketpp/processors/hybi00.hpp \
    websocketpp/processors/hybi07.hpp \
    websocketpp/processors/hybi08.hpp \
    websocketpp/processors/hybi13.hpp \
    websocketpp/processors/processor.hpp \
    websocketpp/random/none.hpp \
    websocketpp/random/random_device.hpp \
    websocketpp/roles/client_endpoint.hpp \
    websocketpp/roles/server_endpoint.hpp \
    websocketpp/sha1/sha1.hpp \
    websocketpp/transport/asio/security/base.hpp \
    websocketpp/transport/asio/security/none.hpp \
    websocketpp/transport/asio/security/tls.hpp \
    websocketpp/transport/asio/base.hpp \
    websocketpp/transport/asio/connection.hpp \
    websocketpp/transport/asio/endpoint.hpp \
    websocketpp/transport/base/connection.hpp \
    websocketpp/transport/base/endpoint.hpp \
    websocketpp/transport/debug/base.hpp \
    websocketpp/transport/debug/connection.hpp \
    websocketpp/transport/debug/endpoint.hpp \
    websocketpp/transport/iostream/base.hpp \
    websocketpp/transport/iostream/connection.hpp \
    websocketpp/transport/iostream/endpoint.hpp \
    websocketpp/transport/stub/base.hpp \
    websocketpp/transport/stub/connection.hpp \
    websocketpp/transport/stub/endpoint.hpp \
    websocketpp/client.hpp \
    websocketpp/close.hpp \
    websocketpp/connection.hpp \
    websocketpp/connection_base.hpp \
    websocketpp/endpoint.hpp \
    websocketpp/endpoint_base.hpp \
    websocketpp/error.hpp \
    websocketpp/frame.hpp \
    websocketpp/server.hpp \
    websocketpp/uri.hpp \
    websocketpp/utf8_validator.hpp \
    websocketpp/utilities.hpp \
    websocketpp/version.hpp \
    UI/IViperGui.h \
    UI/ViperGui.h \
    Common/ViperBuffer.h \
    Common/QtQuick2ApplicationViewer.h



SOURCES *= \
    $$COMMON/Common.cpp \
    $$COMMON/Config.cpp \
    $$COMMON/QOptions.cpp

unix:!macx:!android {
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/libdash
    equals(TRANSPORT_LIBRARY, "HICNET") {
	LIBS += -L/usr/local/lib -ldash  -lhicntransport -lavcodec -lavutil -lavformat
    	DEFINES += "HICNET=ON"
    } else {
        LIBS += -L/usr/local/lib -ldash -licnet -lavcodec -lavutil -lavformat
    	DEFINES += "ICNET=ON"
    }

}

macx:!ios {
    QMAKE_INFO_PLIST = $$COMMON/Info.plist
    ICON = $$COMMON/Viper.icns
    QMAKE_RPATHDIR += /usr/local/lib
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /usr/local/include/libdash
    INCLUDEPATH += $$[QT_HOST_PREFIX]/include/
    INCLUDEPATH += $$[QT_HOST_PREFIX]/lib/QtAV.framework/Headers
    equals(TRANSPORT_LIBRARY, "HICNET") {
        LIBS += -L"/usr/local/lib" -framework CoreServices -ldash -lavformat -lavutil -lavcodec -lswscale -lhicntransport -lssl -lcrypto
        LIBS += -F$$[QT_HOST_PREFIX]/lib/ -framework QtAV
        DEFINES += "HICNET=ON"
    } else {
        LIBS += -L"/usr/local/lib" -framework CoreServices -ldash -lavformat -lavutil -lavcodec -lswscale -licnet -lssl -lcrypto
         LIBS += -F$$[QT_HOST_PREFIX]/lib/ -framework QtAV
        DEFINES += "ICNET=ON"
    }
}
SOURCES *= main.cpp
android {
    DISTFILES += \
    android/src/io/fd/viper/ViperActivity.java \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/AndroidManifest.xml \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    INCLUDEPATH += $$(DISTILLARY_INSTALLATION_PATH)/include
    INCLUDEPATH += $$(DISTILLARY_INSTALLATION_PATH)/include/libdash
    equals(TRANSPORT_LIBRARY, "HICNET") {
        LIBS += -L"$$(DISTILLARY_INSTALLATION_PATH)/lib" -ldash -lhicntransport  -lhicn -lparc -lavcodec -lavutil -lavformat -lswresample -lcurl -lxml2 -lssl -lcrypto -lQtAV
        DEFINES += "HICNET=ON"
        ANDROID_EXTRA_LIBS += $$(DISTILLARY_INSTALLATION_PATH)/lib/libswresample.so
        ANDROID_EXTRA_LIBS += $$(DISTILLARY_INSTALLATION_PATH)/lib/libavresample.so
        ANDROID_EXTRA_LIBS += $$(DISTILLARY_INSTALLATION_PATH)/lib/libavdevice.so
        ANDROID_EXTRA_LIBS += $$(DISTILLARY_INSTALLATION_PATH)/lib/libavfilter.so
        ANDROID_EXTRA_LIBS += $$(DISTILLARY_INSTALLATION_PATH)/lib/libavcodec.so
        ANDROID_EXTRA_LIBS += $$(DISTILLARY_INSTALLATION_PATH)/lib/libavformat.so
        ANDROID_EXTRA_LIBS += $$(DISTILLARY_INSTALLATION_PATH)/lib/libswscale.so
        ANDROID_EXTRA_LIBS += $$(DISTILLARY_INSTALLATION_PATH)/lib/libavutil.so
    } else {
        LIBS += -L"$$(DISTILLARY_INSTALLATION_PATH)/lib" -licnet -ldash -lcurl  -lxml2 -lccnx_api_portal -lccnx_transport_rta -lccnx_api_control -lccnx_api_notify -lccnx_common -lparc -llongbow -llongbow-ansiterm -llongbow-textplain -levent -lssl -lcrypto -lavcodec -lavutil -lavformat   -lboost_system
        DEFINES += "ICNET=ON"
    }
}


DISTFILES += \
    qml/images/Triangle-.png \
    Common/Viper.icns \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/gradle.properties \
    android/src/org/qtav/qmlplayer/ViperActivity.java \
    android/src/io/fd/viper/ViperActivity.java

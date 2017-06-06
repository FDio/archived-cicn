cmake_minimum_required(VERSION 3.5)
project(libdash)


if(ANDROID_API)
	include_directories(${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/include)
	include_directories(${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi/include)
	option(ANDROID "Build CP api based on ANDROID log lib" ON)
	set(LIBXML2_INCLUDE_DIR "$ENV{DISTILLERY_ROOT_DIR}/external/libxml2_android/jni/libxml2/include")
	set(LIBXML2_LIBRARIES "$ENV{DISTILLERY_ROOT_DIR}/usr/lib/libxml2.a")
	set(CURL_INCLUDE_DIRS "$ENV{DISTILLERY_ROOT_DIR}/external/libcurl_android/jni/libcurl/include")
	set(CURL_LIBRARIES "$ENV{DISTILLERY_ROOT_DIR}/usr/lib/libcurl.a" "$ENV{DISTILLERY_ROOT_DIR}/usr/lib/libcurl-library.a")
	set(ANDROID_LIBRARIES "${ANDROID_NDK}/platforms/android-23/arch-arm/usr/lib/libz.a" "$ENV{NDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/$ENV{ABI}/libgnustl_shared.so"  "${ANDROID_NDK}/platforms/android-23/arch-arm/usr/lib/liblog.so")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${ANDROID_C_FLAGS} -std=c++0x -g -DLOG_BUILD")
else ()
	set(CMAKE_CXX_FLAGS "-std=c++0x -g -DLOG_BUILD")
	find_package(LibXml2 REQUIRED)
	find_package(ZLIB REQUIRED)
	find_package(CURL REQUIRED)
endif (ANDROID_API)
set(DEBUG_BUILD FALSE)
set(HEADER_FILES
        include/config.h
        include/IAdaptationSet.h
        include/IBaseUrl.h
        include/IChunk.h
        include/IConnection.h
        include/IContentComponent.h
        include/IDASHManager.h
        include/IDASHMetrics.h
        include/IDescriptor.h
        include/IDownloadableChunk.h
        include/IDownloadObserver.h
        include/IHTTPTransaction.h
        include/IMetrics.h
        include/IMPD.h
        include/IMPDElement.h
        include/IMultipleSegmentBase.h
        include/INode.h
        include/IPeriod.h
        include/IProgramInformation.h
        include/IRange.h
        include/IRepresentation.h
        include/IRepresentationBase.h
        include/ISegment.h
        include/ISegmentBase.h
        include/ISegmentList.h
        include/ISegmentTemplate.h
        include/ISegmentTimeline.h
        include/ISegmentURL.h
        include/ISubRepresentation.h
        include/ISubset.h
        include/ITCPConnection.h
        include/IThroughputMeasurement.h
        include/ITimeline.h
        include/IURLType.h
        include/libdash.h)

set(SOURCE_FILES
        include/config.h
        include/IAdaptationSet.h
        include/IBaseUrl.h
        include/IChunk.h
        include/IConnection.h
        include/IContentComponent.h
        include/IDASHManager.h
        include/IDASHMetrics.h
        include/IDescriptor.h
        include/IDownloadableChunk.h
        include/IDownloadObserver.h
        include/IHTTPTransaction.h
        include/IMetrics.h
        include/IMPD.h
        include/IMPDElement.h
        include/IMultipleSegmentBase.h
        include/INode.h
        include/IPeriod.h
        include/IProgramInformation.h
        include/IRange.h
        include/IRepresentation.h
        include/IRepresentationBase.h
        include/ISegment.h
        include/ISegmentBase.h
        include/ISegmentList.h
        include/ISegmentTemplate.h
        include/ISegmentTimeline.h
        include/ISegmentURL.h
        include/ISubRepresentation.h
        include/ISubset.h
        include/ITCPConnection.h
        include/IThroughputMeasurement.h
        include/ITimeline.h
        include/IURLType.h
        include/libdash.h
        source/helpers/Block.h
        source/helpers/BlockStream.cpp
        source/helpers/BlockStream.h
        source/helpers/Path.cpp
        source/helpers/Path.h
        source/helpers/String.cpp
        source/helpers/String.h
        source/helpers/SyncedBlockStream.cpp
        source/helpers/SyncedBlockStream.h
        source/helpers/Time.cpp
        source/helpers/Time.h
        source/manager/DASHManager.cpp
        source/manager/DASHManager.h
        source/metrics/HTTPTransaction.cpp
        source/metrics/HTTPTransaction.h
        source/metrics/TCPConnection.cpp
        source/metrics/TCPConnection.h
        source/metrics/ThroughputMeasurement.cpp
        source/metrics/ThroughputMeasurement.h
        source/mpd/AbstractMPDElement.cpp
        source/mpd/AbstractMPDElement.h
        source/mpd/AdaptationSet.cpp
        source/mpd/AdaptationSet.h
        source/mpd/BaseUrl.cpp
        source/mpd/BaseUrl.h
        source/mpd/ContentComponent.cpp
        source/mpd/ContentComponent.h
        source/mpd/Descriptor.cpp
        source/mpd/Descriptor.h
        source/mpd/Metrics.cpp
        source/mpd/Metrics.h
        source/mpd/MPD.cpp
        source/mpd/MPD.h
        source/mpd/MultipleSegmentBase.cpp
        source/mpd/MultipleSegmentBase.h
        source/mpd/Period.cpp
        source/mpd/Period.h
        source/mpd/ProgramInformation.cpp
        source/mpd/ProgramInformation.h
        source/mpd/Range.cpp
        source/mpd/Range.h
        source/mpd/Representation.cpp
        source/mpd/Representation.h
        source/mpd/RepresentationBase.cpp
        source/mpd/RepresentationBase.h
        source/mpd/Segment.cpp
        source/mpd/Segment.h
        source/mpd/SegmentBase.cpp
        source/mpd/SegmentBase.h
        source/mpd/SegmentList.cpp
        source/mpd/SegmentList.h
        source/mpd/SegmentTemplate.cpp
        source/mpd/SegmentTemplate.h
        source/mpd/SegmentTimeline.cpp
        source/mpd/SegmentTimeline.h
        source/mpd/SegmentURL.cpp
        source/mpd/SegmentURL.h
        source/mpd/SubRepresentation.cpp
        source/mpd/SubRepresentation.h
        source/mpd/Subset.cpp
        source/mpd/Subset.h
        source/mpd/Timeline.cpp
        source/mpd/Timeline.h
        source/mpd/URLType.cpp
        source/mpd/URLType.h
        source/network/AbstractChunk.cpp
        source/network/AbstractChunk.h
        source/network/DownloadStateManager.cpp
        source/network/DownloadStateManager.h
        source/portable/MultiThreading.cpp
        source/portable/MultiThreading.h
        source/portable/Networking.h
        source/xml/DOMHelper.cpp
        source/xml/DOMHelper.h
        source/xml/DOMParser.cpp
        source/xml/DOMParser.h
        source/xml/Node.cpp
        source/xml/Node.h
        source/dllmain.cpp
        source/libdash.cpp
        source/targetver.h)

find_package(LibXml2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(${WLIB_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(include)

add_library(dash SHARED ${SOURCE_FILES})
set_target_properties(dash PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(dash ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBXML2_LIBRARIES} ${ANDROID_LIBRARIES})
message("libxml ${LIBXML2_LIBRARIES} ${LIBXML2_INCLUDE_DIR}")
install(TARGETS dash DESTINATION ${CMAKE_INSTALL_PREFIX}/lib PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ COMPONENT library)
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libdash COMPONENT headers)

# Generate DEB / RPM packages

option(DEB_PACKAGE "Create deb package" OFF)
option(RPM_PACKAGE "Create deb package" OFF)

set(VENDOR "Cisco Systems" CACHE STRING "Vendor")
set(CONTACT "msardara@cisco.com" CACHE STRING "Contact")
set(DISTRIBUTION "xenial" CACHE STRING "Distribution")
set(ARCHITECTURE "amd64" CACHE STRING "Architecture")
set(PACKAGE_MAINTAINER "Mauro Sardara (msardara@cisco.com)" CACHE STRING "Maintainer")
string(TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_PACKAGE_VENDOR ${VENDOR})
set(CPACK_PACKAGE_CONTACT ${CONTACT})
set(CPACK_COMPONENTS_ALL library headers documentation)

# Get the version
execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/../scripts/version
		OUTPUT_VARIABLE PACKAGE_VERSION)

if (PACKAGE_VERSION)
	string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)
else()
	set(PACKAGE_VERSION 1.0)
endif()

if(DEB_PACKAGE)
	set(TYPE "DEBIAN")
	set(GENERATOR "DEB")

	set(CPACK_${TYPE}_LIBRARY_PACKAGE_NAME "${PACKAGE_NAME}")
	set(CPACK_${TYPE}_HEADERS_PACKAGE_NAME "${PACKAGE_NAME}-dev")
	set(CPACK_${TYPE}_DOCUMENTATION_PACKAGE_NAME "${PACKAGE_NAME}-doc")

	set(CPACK_${TYPE}_LIBRARY_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}.deb")
	set(CPACK_${TYPE}_HEADERS_FILE_NAME "${PACKAGE_NAME}-dev_${PACKAGE_VERSION}_${ARCHITECTURE}.deb")
	set(CPACK_${TYPE}_DOCUMENTATION_FILE_NAME "${PACKAGE_NAME}-doc_${PACKAGE_VERSION}_${ARCHITECTURE}.deb")

	set(CPACK_${TYPE}_PACKAGE_SHLIBDEPS ON)
	set(CPACK_${TYPE}_LIBRARY_PACKAGE_DEPENDS "")
	set(CPACK_${TYPE}_HEADERS_PACKAGE_DEPENDS "libdash (>= 1.0), libxml2-dev, libcurl4-openssl-dev")
	set(CPACK_${TYPE}_DOCUMENTATION_PACKAGE_DEPENDS "")
elseif(RPM_PACKAGE)
	set(TYPE "RPM")
	set(GENERATOR "RPM")

	set(CPACK_${TYPE}_LIBRARY_PACKAGE_NAME "${PACKAGE_NAME}")
	set(CPACK_${TYPE}_HEADERS_PACKAGE_NAME "${PACKAGE_NAME}-devel")
	set(CPACK_${TYPE}_DOCUMENTATION_PACKAGE_NAME "${PACKAGE_NAME}-doc")

	set(CPACK_${TYPE}_LIBRARY_FILE_NAME "${PACKAGE_NAME}-${PACKAGE_VERSION}.${ARCHITECTURE}.rpm")
	set(CPACK_${TYPE}_HEADERS_FILE_NAME "${PACKAGE_NAME}-devel-${PACKAGE_VERSION}.${ARCHITECTURE}.rpm")
	set(CPACK_${TYPE}_DOCUMENTATION_FILE_NAME "${PACKAGE_NAME}-doc-${PACKAGE_VERSION}.${ARCHITECTURE}.rpm")

	set(CPACK_${TYPE}_PACKAGE_AUTOREQ ON)
	set(CPACK_${TYPE}_LIBRARY_PACKAGE_REQUIRES "")
	set(CPACK_${TYPE}_HEADERS_PACKAGE_REQUIRES "libdash >= 1.0, libxml2-devel, libcurl-openssl-devel")
	set(CPACK_${TYPE}_DOCUMENTATION_PACKAGE_REQUIRES "")

	set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
else()
	return()
endif()

set(CPACK_GENERATOR ${GENERATOR})
set(CPACK_${GENERATOR}_COMPONENT_INSTALL ON)
set(CPACK_${TYPE}_PACKAGE_MAINTAINER ${PACKAGE_MAINTAINER})
set(CPACK_${TYPE}_PACKAGE_NAME ${PACKAGE_NAME})
set(CPACK_${TYPE}_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_${TYPE}_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
set(CPACK_${TYPE}_PACKAGE_RELEASE 1)
set(CPACK_${TYPE}_PACKAGE_VENDOR ${VENDOR})
set(CPACK_${TYPE}_PACKAGE_DESCRIPTION "Open-source library that provides an object orient (OO) interface to the MPEG-DASH standard")

include(CPack)
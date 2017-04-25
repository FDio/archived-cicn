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

include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(${WLIB_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(include)

add_library(dash SHARED ${SOURCE_FILES})
set_target_properties(dash PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(dash ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBXML2_LIBRARIES} ${ANDROID_LIBRARIES})
message("libxml ${LIBXML2_LIBRARIES} ${LIBXML2_INCLUDE_DIR}")
install(TARGETS dash DESTINATION ${CMAKE_INSTALL_PREFIX}/lib PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libdash)


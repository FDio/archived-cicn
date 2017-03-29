LOCAL_PATH := $(call my-dir)

# common_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

# Ignore test files
common_SRC_FILES := \
	buf.c \
	c14n.c \
	catalog.c \
	chvalid.c \
	debugXML.c \
	dict.c \
	DOCBparser.c \
	encoding.c \
	entities.c \
	error.c \
	globals.c \
	hash.c \
	HTMLparser.c \
	HTMLtree.c \
	legacy.c \
	list.c \
	nanoftp.c \
	nanohttp.c \
	parser.c \
	parserInternals.c \
	pattern.c \
	relaxng.c \
	SAX.c \
	SAX2.c \
	schematron.c \
	threads.c \
	tree.c \
	uri.c \
	valid.c \
	xinclude.c \
	xlink.c \
	xmlIO.c \
	xmlmemory.c \
	xmlmodule.c \
	xmlreader.c \
	xmlregexp.c \
	xmlsave.c \
	xmlschemas.c \
	xmlschemastypes.c \
	xmlstring.c \
	xmlunicode.c \
	xmlwriter.c \
	xpath.c \
	xpointer.c


common_C_INCLUDES += $(LOCAL_PATH)/include


include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_C_INCLUDES += $(common_C_INCLUDES)
LOCAL_SHARED_LIBRARIES += $(common_SHARED_LIBRARIES)
LOCAL_CFLAGS += -fvisibility=hidden -I$(LOCAL_C_INCLUDES)

LOCAL_MODULE:= xml2

include $(BUILD_STATIC_LIBRARY)

ROOT_DIR    = .
HEADERS_DIR = Headers
SOURCE_DIR  = Source
TOOLS_DIR   = Tools
PARSER_DIR  = $(TOOLS_DIR)/Parser
TESTER_DIR  = $(TOOLS_DIR)/Tester

LIB_SHEENBIDI   = sheenbidi
LIB_SHEENFIGURE = sheenfigure
LIB_PARSER      = sheenfigureparser
EXEC_TESTER     = sheenfiguretester

ifndef SHEENBIDI_DIR
	SHEENBIDI_DIR = ../SheenBidi/Headers
endif

ifndef CC
	CC = gcc
endif
ifndef CXX
	CXX = g++
endif

AR = ar
ARFLAGS = -r
CFLAGS = -ansi -pedantic -Wall -I$(HEADERS_DIR) -I$(SOURCE_DIR) -I$(SHEENBIDI_DIR)
CXXFLAGS = -std=c++11 -g -Wall
DEBUG_FLAGS = -DDEBUG -g -O0
RELEASE_FLAGS = -DNDEBUG -DSF_CONFIG_UNITY -Os

DEBUG = Debug
RELEASE = Release

DEBUG_SOURCES = $(SOURCE_DIR)/ArabicEngine.c \
                $(SOURCE_DIR)/GlyphDiscovery.c \
                $(SOURCE_DIR)/GlyphManipulation.c \
                $(SOURCE_DIR)/GlyphPositioning.c \
                $(SOURCE_DIR)/GlyphSubstitution.c \
                $(SOURCE_DIR)/List.c \
                $(SOURCE_DIR)/Locator.c \
                $(SOURCE_DIR)/SFAlbum.c \
                $(SOURCE_DIR)/SFArtist.c \
                $(SOURCE_DIR)/SFBase.c \
                $(SOURCE_DIR)/SFCodepoints.c \
                $(SOURCE_DIR)/SFFont.c \
                $(SOURCE_DIR)/SFJoiningTypeLookup.c \
                $(SOURCE_DIR)/SFPattern.c \
                $(SOURCE_DIR)/SFPatternBuilder.c \
                $(SOURCE_DIR)/SFScheme.c \
                $(SOURCE_DIR)/ShapingEngine.c \
                $(SOURCE_DIR)/ShapingKnowledge.c \
                $(SOURCE_DIR)/StandardEngine.c \
                $(SOURCE_DIR)/Tables/OpenType.c \
                $(SOURCE_DIR)/Tables/Variations.c \
                $(SOURCE_DIR)/TextProcessor.c \
                $(SOURCE_DIR)/UnifiedEngine.c
RELEASE_SOURCES = $(SOURCE_DIR)/SheenFigure.c

DEBUG_OBJECTS   = $(DEBUG_SOURCES:$(SOURCE_DIR)/%.c=$(DEBUG)/%.o)
RELEASE_OBJECTS = $(RELEASE_SOURCES:$(SOURCE_DIR)/%.c=$(RELEASE)/%.o)

DEBUG_TARGET   = $(DEBUG)/lib$(LIB_SHEENFIGURE).a
PARSER_TARGET  = $(DEBUG)/lib$(LIB_PARSER).a
TESTER_TARGET  = $(DEBUG)/$(EXEC_TESTER)
RELEASE_TARGET = $(RELEASE)/lib$(LIB_SHEENFIGURE).a

all:     release
release: $(RELEASE) $(RELEASE_TARGET)
debug:   $(DEBUG) $(DEBUG_TARGET)

check: tester
	./Debug/sheenfiguretester Tools/Unicode

clean: parser_clean tester_clean
	$(RM) $(DEBUG)/*.o
	$(RM) $(DEBUG)/**/*.o
	$(RM) $(DEBUG_TARGET)
	$(RM) $(RELEASE)/*.o
	$(RM) $(RELEASE_TARGET)

$(DEBUG):
	mkdir $(DEBUG)
	mkdir $(DEBUG)/Tables

$(RELEASE):
	mkdir $(RELEASE)

$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	$(AR) $(ARFLAGS) $(DEBUG_TARGET) $(DEBUG_OBJECTS)

$(RELEASE_TARGET): $(RELEASE_OBJECTS)
	$(AR) $(ARFLAGS) $(RELEASE_TARGET) $(RELEASE_OBJECTS)

$(DEBUG)/%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(RELEASE)/%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(RELEASE_FLAGS) -c $< -o $@

.PHONY: all check clean debug parser release tester

include $(PARSER_DIR)/Makefile
include $(TESTER_DIR)/Makefile

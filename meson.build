project('sheenfigure', ['c', 'cpp'],
  version : '0.1',
  meson_version : '>= 1.2.0',
  default_options : ['cpp_std=c++20'])

sheenfigure_includes = include_directories('Headers')

sheenfigure_headers = files([
  'Headers/SFAlbum.h',
  'Headers/SFArtist.h',
  'Headers/SFBase.h',
  'Headers/SFConfig.h',
  'Headers/SFFont.h',
  'Headers/SFPattern.h',
  'Headers/SFScheme.h',
  'Headers/SheenFigure.h'
])

install_headers(sheenfigure_headers, subdir : 'SheenFigure')

sheenfigure_sources = [
  'Source/ArabicEngine.c',
  'Source/GlyphDiscovery.c',
  'Source/GlyphManipulation.c',
  'Source/GlyphPositioning.c',
  'Source/GlyphSubstitution.c',
  'Source/List.c',
  'Source/Locator.c',
  'Source/OpenType.c',
  'Source/SFAlbum.c',
  'Source/SFArtist.c',
  'Source/SFBase.c',
  'Source/SFCodepoints.c',
  'Source/SFFont.c',
  'Source/SFJoiningTypeLookup.c',
  'Source/SFPattern.c',
  'Source/SFPatternBuilder.c',
  'Source/SFScheme.c',
  'Source/ShapingEngine.c',
  'Source/ShapingKnowledge.c',
  'Source/SheenFigure.c',
  'Source/StandardEngine.c',
  'Source/TextProcessor.c',
  'Source/UnifiedEngine.c',
]

sheenbidi_dep = dependency('sheenbidi',
  fallback : ['sheenbidi', 'sheenbidi_dep'])

sheenfigure_library = library('sheenfigure',
  sources : sheenfigure_sources,
  include_directories : sheenfigure_includes,
  dependencies : [sheenbidi_dep],
  version : meson.project_version(),
  install : true)

sheenfigure_dep = declare_dependency(
  include_directories : sheenfigure_includes,
  link_with : sheenfigure_library)

parser_includes = include_directories('Tools')

parser_library = library('parser',
  sources : files(['Tools/Parser/ArabicShaping.cpp']),
  include_directories : parser_includes,
  install : false)

parser_dep = declare_dependency(
  include_directories : parser_includes,
  link_with : parser_library)

tester_sources = files([
  'Tools/Tester/AlbumTester.cpp',
  'Tools/Tester/FontTester.cpp',
  'Tools/Tester/GlyphManipulationTester.cpp',
  'Tools/Tester/GlyphPositioningTester.cpp',
  'Tools/Tester/GlyphSubstitutionTester.cpp',
  'Tools/Tester/JoiningTypeLookupTester.cpp',
  'Tools/Tester/ListTester.cpp',
  'Tools/Tester/LocatorTester.cpp',
  'Tools/Tester/MiscTester.cpp',
  'Tools/Tester/PatternTester.cpp',
  'Tools/Tester/SchemeTester.cpp',
  'Tools/Tester/TextProcessorTester.cpp',
  'Tools/Tester/main.cpp',
  'Tools/Tester/OpenType/Builder.cpp',
  'Tools/Tester/OpenType/Writer.cpp',
  'Tools/Tester/Utilities/Convert.cpp',
  'Tools/Tester/Utilities/SFPattern+Testing.cpp',
  'Tools/Tester/Utilities/Unicode.cpp',
])

sheenfigure_tester = executable('sheenfigure_tester',
  sources : tester_sources,
  dependencies : [sheenfigure_dep, sheenbidi_dep, parser_dep],
  install : false)

test('sheenfigure_tester', sheenfigure_tester)

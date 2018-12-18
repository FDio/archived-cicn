add_subdirectory(site-packages)

install( FILES parc_uncrustify.cfg DESTINATION ${CMAKE_INSTALL_PREFIX}/etc )

macro(AddLongBowPythonScript scriptFile)
  configure_file(${ARGV0}.py ${ARGV0} @ONLY)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${ARGV0} COMPONENT library DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endmacro(AddLongBowPythonScript)

set(ScriptList
  longbow-doxygen-report
  longbow-generate-about
  longbow-preprocess
  longbow-code
  longbow-complexity-report
  longbow-coverage-report
  longbow-bytearray
  longbow-ansigcov
  longbow-name-report
  longbow-size-report
  longbow-style-report
  longbow-test-run
  longbow-test-suite
  longbow-vocabulary-report
  )

foreach(script ${ScriptList})
  AddLongBowPythonScript(${script})
endforeach()

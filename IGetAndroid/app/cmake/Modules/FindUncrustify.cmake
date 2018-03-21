# Find uncrustify program
#
find_program( UNCRUSTIFY_BIN uncrustify
                PATHS
				$ENV{UNCRUSTIFY_HOME}
				)

message( "-- UNCRUSTIFY found in ${UNCRUSTIFY_BIN}" )

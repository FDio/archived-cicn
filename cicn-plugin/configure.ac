
AC_INIT(cicn_plugin, 1.0)
AM_INIT_AUTOMAKE

AC_PROG_LIBTOOL
AM_PROG_AS
AC_PROG_CC
AM_PROG_CC_C_O

# always give this switch, should be hard-wired(?)
#    ../configure ... --with-plugin-toolkit
AC_ARG_WITH(plugin-toolkit,
            AC_HELP_STRING([--with-plugin-toolkit],
                [build using the vpp toolkit]),
            [with_plugin_toolkit=${prefix}/include],
            [with_plugin_toolkit=.])

AC_SUBST(TOOLKIT_INCLUDE,[${with_plugin_toolkit}])
AM_CONDITIONAL(WITH_PLUGIN_TOOLKIT, test "$with_plugin_toolkit" != ".")

# control build inclusion of cicn UT modules
#   include:  <default>
#   omit:     ../configure ... --without-cicn-test
AC_ARG_WITH(cicn-test,
            AC_HELP_STRING([--without-cicn-test],
                [disable support for cicn-test]),
            [],
            [with_cicn_test=yes])

AC_SUBST(TOOLKIT_INCLUDE,[${with_cicn_test}])
AM_CONDITIONAL(WITH_CICN_TEST, test "x$with_cicn_test" != x"no")

AC_OUTPUT([Makefile])

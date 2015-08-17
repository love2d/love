AC_DEFUN([ACLOVE_CXX_FLAG_TEST], [ dnl WARNING: NOT REENTRANT
	aclove_cxx_flag_test_save_cflags="$CXXFLAGS"
	CXXFLAGS="$1"
	AC_MSG_CHECKING([whether $CXX supports flag $1])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
		[AC_MSG_RESULT([yes])]; $2,
		[AC_MSG_RESULT([no]); $3])
	CXXFLAGS="$aclove_cxx_flag_test_save_cflags"
])

AC_DEFUN([ACLOVE_GET_GCC_VERSION], [
	aclove_gcc_version_found="yes"
	AC_COMPUTE_INT(aclove_gcc_version_major, __GNUC__,, aclove_gcc_version_found="no")
	AC_COMPUTE_INT(aclove_gcc_version_minor, __GNUC_MINOR__,, aclove_gcc_version_found="no")
	AC_COMPUTE_INT(aclove_gcc_version_patch, __GNUC_PATCHLEVEL__,, aclove_gcc_version_found="no")
])

AC_DEFUN([ACLOVE_GET_CLANG_VERSION], [
	aclove_clang_version_found="yes"
	AC_COMPUTE_INT(aclove_clang_version_major, __clang_major__,, aclove_clang_version_found="no")
	AC_COMPUTE_INT(aclove_clang_version_minor, __clang_minor__,, aclove_clang_version_found="no")
	AC_COMPUTE_INT(aclove_clang_version_patch, __clang_patchlevel__,, aclove_clang_version_found="no")
])

AC_DEFUN([ACLOVE_CPP11_TEST_FLAG], [
	aclove_cpp11_test_cxx11name="no"
	ACLOVE_CXX_FLAG_TEST([-std=c++0x], aclove_cpp11_test_cxx11name="c++0x", [])
	ACLOVE_CXX_FLAG_TEST([-std=c++11], aclove_cpp11_test_cxx11name="c++11", [])
	AS_VAR_IF([aclove_cpp11_test_cxx11name], [no],
		[AC_MSG_ERROR([LÖVE needs a C++ compiler with C++11 support])],
		[CXXFLAGS="$CXXFLAGS -std=$aclove_cpp11_test_cxx11name"])
])

AC_DEFUN([ACLOVE_CPP11_CHECK_VERSION], [ dnl compiler, targetmajor, targetminor, on-failure
	aclove_cpp11_check_version_status="no"
	AC_MSG_CHECKING([whether $1 version is at least $2.$3])
	AS_IF([test "$aclove_[]$1[]_version_major" -gt $2], aclove_cpp11_check_version_status="yes")
	AS_IF([test "$aclove_[]$1[]_version_major" -eq $2 && test "$aclove_[]$1[]_version_minor" -ge $3], aclove_cpp11_check_version_status="yes")
	AC_MSG_RESULT([$aclove_cpp11_check_version_status])
	AS_VAR_IF([aclove_cpp11_check_version_status], [no],
		[$4])
])

AC_DEFUN([ACLOVE_CPP11_TEST_VERSION_GCC], [
	ACLOVE_CPP11_CHECK_VERSION([gcc], 4, 7,
		[AC_MSG_ERROR([LÖVE needs a GCC version of at least 4.7])])
])

AC_DEFUN([ACLOVE_CPP11_TEST_VERSION_CLANG], [
	ACLOVE_CPP11_CHECK_VERSION([clang], 3, 1,
		[AC_MSG_ERROR([LÖVE needs a clang version of at least 3.1])])
])

AC_DEFUN([ACLOVE_CPP11_TEST], [
	ACLOVE_CPP11_TEST_FLAG

	ACLOVE_GET_GCC_VERSION
	ACLOVE_GET_CLANG_VERSION

	# Since clang also sets gcc headers, check clang after
	aclove_cpp11_test_compiler="unknown"
	AS_VAR_IF([aclove_gcc_version_found], [yes], aclove_cpp11_test_compiler="gcc")
	AS_VAR_IF([aclove_clang_version_found], [yes], aclove_cpp11_test_compiler="clang")

	AS_CASE([$aclove_cpp11_test_compiler],
		[gcc], [ACLOVE_CPP11_TEST_VERSION_GCC],
		[clang], [ACLOVE_CPP11_TEST_VERSION_CLANG],
		[AC_MSG_WARN([Could not determine compiler version])])
])

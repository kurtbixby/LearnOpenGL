#ifndef COMPILER_WARNING_H
#define COMPILER_WARNING_H

#define STRINGISE_IMPL(x)
#define STRINGISE(x) STRINGISE_IMPL(x)

#if _MSC_VER
#	define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : "
#	define WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#else
#	define WARN(exp) ("WARNING: " exp)
#endif

#endif // !COMPILER_WARNING_H
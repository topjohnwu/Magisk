// Workaround pcre2_chartables.c symlink to pcre2_chartables.c.dist failing on Windows NDK if Cygwin git used,
// and NDK not directly accepting a .c.dist file in LOCAL_SRC_FILES list.

#include "pcre/src/pcre2_chartables.c.dist"

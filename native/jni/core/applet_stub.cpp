#include <sys/stat.h>

#include <magisk.h>

int main(int argc, char *argv[]) {
	umask(0);
	cmdline_logging();
	return APPLET_STUB_MAIN(argc, argv);
}

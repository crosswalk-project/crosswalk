CC         = gcc

SOURCES    = main.c dialog_launcher.c date_time_dialog.c
PROGRAM    = dialog-launcher

TEST_SOURCES = dialog_launcher_test.c
TEST_PROGRAM = dialog-launcher-test

CFLAGS     += `pkg-config --cflags edbus ecore dbus-1 elementary`
LDFLAGS    += `pkg-config --libs edbus ecore dbus-1 elementary`

all:
	$(CC) -O0 -g3 -ggdb3 $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $(PROGRAM)
	$(CC) $(TEST_SOURCES) $(CFLAGS) $(LDFLAGS) -o $(TEST_PROGRAM)
clean:
	@rm -rf $(PROGRAM) $(TEST_PROGRAM)
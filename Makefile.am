ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = ttysend

WARNINGS = -Wall -Wextra \
    -Wno-unused-result -Wno-missing-field-initializers

ttysend_CFLAGS = $(WARNINGS) -std=gnu99

EXTRA_DIST = LICENSE

ttysend_SOURCES = ttysend.c

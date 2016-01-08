/*
 * The MIT License (MIT)
 *
 * Copyright © 2016 Franklin "Snaipe" Mathieu <http://snai.pe/>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define cleanup(Fn) __attribute__((cleanup(Fn)))
#define autoclose cleanup(close_stack)

struct options {
    char *device;
    char *data;
    char *input;
    bool newline;
};

static inline void close_stack(int *fd) {
    if (*fd != -1)
        close(*fd);
}

static inline void free_options(struct options *opt) {
    free(opt->data);
    free(opt->input);
}

# define USAGE \
    "usage: %s [-n|--newline] [-d|--data <data>] [-i|--input [file]] <device>\n\n" \
    "    -n|--newline: print a newline in the device before exiting.\n" \
    "    --data: print the specified string to the device and exit.\n" \
    "    --input: print the contents of the file (or stdin if none specified) " \
            "to the device and exit.\n" \
    "    <device>: The path to the desired tty/pty device to send data to.\n"

int get_options(int argc, char *argv[], struct options *opts) {
    static struct option long_opts[] = {
        {"newline",         optional_argument,  0, 'n'},
        {"data",            required_argument,  0, 'd'},
        {"input",           optional_argument,  0, 'i'},
        {0,                 0,                  0,  0 }
    };

    char *progname = argv[0];

    bool do_print_usage = argc == 1;
    for (int c; (c = getopt_long(argc, argv, "nd:i::", long_opts, NULL)) != -1;) {
        switch (c) {
            case 'n': opts->newline = true; break;
            case 'd': opts->data = strdup(optarg); break;
            case 'i': opts->input = strdup(optarg ? optarg : "-"); break;
            case '?':
            default : do_print_usage = true; break;
        }
    }

    if (!do_print_usage && optind >= argc) {
        fprintf(stderr, "Expected device path, but none given.\n");
        do_print_usage = true;
    }

    if (do_print_usage) {
        fprintf(stderr, USAGE, progname);
        return 0;
    }

    opts->device = argv[optind];
    return 1;
}

int ttysend(int fd, char *buf, size_t size) {
    for (size_t i = 0; i < size; ++i)
        if (ioctl(fd, TIOCSTI, buf + i) == -1)
            return -1;
    return 0;
}

int main(int argc, char *argv[]) {
    cleanup(free_options) struct options opts = { .device = "" };
    if (!get_options(argc, argv, &opts))
        return EXIT_FAILURE;

    autoclose int fd = open(opts.device, O_RDWR);
    if (fd == -1) {
        perror("Could not open tty device");
        return EXIT_FAILURE;
    }

    if (opts.data) {
        if (ttysend(fd, opts.data, strlen(opts.data)) == -1) {
            perror("Could not write data to device");
            return EXIT_FAILURE;
        }
    }
    if (opts.input) {
        autoclose int in = !strcmp(opts.input, "-")
            ? STDIN_FILENO
            : open(opts.input, O_RDONLY);

        char buf[4096];
        for (ssize_t rd = 0; (rd = read(in, buf, sizeof(buf))) > 0;) {
            if (ttysend(fd, buf, rd) == -1) {
                perror("Could not write file contents to device");
                return EXIT_FAILURE;
            }
        }
    }
    if (opts.newline) {
        if (ttysend(fd, "\n", 1) == -1) {
            perror("Could not write newline to device");
            return EXIT_FAILURE;
        }
    }

    return 0;
}

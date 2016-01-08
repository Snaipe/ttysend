# ttysend

A simplistic C utility to send data to other tty/pts.

## Compilation

```bash
$ ./autogen.sh
$ mkdir build && cd $_
$ ../configure && make
```

## Usage

    ttysend [-n|--newline] [-d|--data <data>] [-i|--input [file]] <device>

        -n|--newline: print a newline in the device before exiting.
        --data: print the specified string to the device and exit.
        --input: print the contents of the file (or stdin if none specified) to the device and exit.
        <device>: The path to the desired tty/pty device to send data to.

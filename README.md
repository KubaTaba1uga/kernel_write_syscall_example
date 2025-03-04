# write_syscall_example

A simple C program that prints "Hello world!" using the Linux `write` syscall.

## Building

1. Ensure you have gcc installed.
2. Build the program:
   ```bash
   make
   ```

## Usage

- Run the program:
  ```bash
  ./use_write_directly
  ```
- Trace syscalls with:
  ```bash
  strace ./use_write_directly
  ```

## License

MIT

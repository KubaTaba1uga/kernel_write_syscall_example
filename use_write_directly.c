/*
 * use_write_directly.c
 ****************************************************************
 * Brief Description:
 * A very simple module which prints messages to stdoutput directly via
 *  write linux kernel syscall.
 *
 * To display write syscall documentation do:
        man 2 write
 *
 * To display strace documentation do:
        man strace
 *
 * Once code is compiled we can run it with strace to see syscalls
 *  invoked by the process:
	$ strace ./use_write_directly
	1.      execve("./use_write_directly", ["./use_write_directly"], 0x7ffe1c24b940 / 28 vars /) = 0
	2.      brk(NULL)                               = 0x55a68cf91000
	3.      mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f9e70c40000
	4.      access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
	5.      openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
	6.      newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=41202, ...}, AT_EMPTY_PATH) = 0
 	7.      mmap(NULL, 41202, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f9e70c35000
	8.      close(3)                                = 0
	9.      openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
	10.     read(3, "ELF>t"..., 832) = 832
	11.     pread64(3, "@@@"..., 784, 64) = 784
	12.     newfstatat(3, "", {st_mode=S_IFREG|0755, st_size=1922136, ...}, AT_EMPTY_PATH) = 0
	13.     pread64(3, "@@@"..., 784, 64) = 784
	14.     mmap(NULL, 1970000, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f9e70a54000
	15.     mmap(0x7f9e70a7a000, 1396736, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x26000) = 0x7f9e70a7a000
	16.     mmap(0x7f9e70bcf000, 339968, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x17b000) = 0x7f9e70bcf000
	17.     mmap(0x7f9e70c22000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1ce000) = 0x7f9e70c22000
	18.     mmap(0x7f9e70c28000, 53072, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f9e70c28000
	19.     close(3)                                = 0
	20.     mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f9e70a51000
	21.     arch_prctl(ARCH_SET_FS, 0x7f9e70a51740) = 0
	22.     set_tid_address(0x7f9e70a51a10)         = 114992
	23.     set_robust_list(0x7f9e70a51a20, 24)     = 0
	24.     rseq(0x7f9e70a52060, 0x20, 0, 0x53053053) = 0
	25.     mprotect(0x7f9e70c22000, 16384, PROT_READ) = 0
	26.     mprotect(0x55a66f4fe000, 4096, PROT_READ) = 0
	27.     mprotect(0x7f9e70c72000, 8192, PROT_READ) = 0
	28.     prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
	29.     munmap(0x7f9e70c35000, 41202)           = 0
	30.     write(1, "Hello world!", 14Hello world!)= 14
	31.     exit_group(0)                           = ?
	32.     +++ exited with 0 +++

 * Now let's analyze the trace.
         1. From `man execve` we can read: execve() executes the program referred to by pathname.
         2. From `man brk` we can read: ... (brk) sets  the end of the data segment to the value
	    specified by addr, ... 0 can be used to find the current location of the program break.
	    As i understand it `data segment` is heap, so brk(0) fetches last address of heap
	    assigned to the process. That's why it return memory address instead of 0 or -1.
	    In other words brk normally allocates and deallocates memory but when used with 0
	    results in last heap memory address.
	 3. From `man mmap`: creates a new memory mapping in the virtual address space of the calling
	    process. In other words it takes memory from other process or file and maps it directly
	    to our process memory. On this line it just allocates 8192 bytes of memory and do not map
	    it to anything.
	 4. From `man access`: access() checks whether the calling process can access the file
	    pathname. But it results in ENOENT which measns there is no such file as `ld.so.preload`,
	 5. From `man openat`: The open() system call opens the file specified by pathname. In this
	    case it opens "/etc/ld.so.cache" and assigns it to file descriptor number 3.
	 6. From `man newfstatat`: fstatat() retrieve information about the file pointed to by
	    pathname.
	 7. mmap maps 41202 bytes from file associated with file descriptor number 3 into process
	    memory. The returned address (0x7f9e70c35000) is where this file segment is now accessible.
	 8. From `man close`: close() closes a file descriptor. So this line closes file descriptor 3
	    which were allocated for "/etc/ld.so.cache". File descriptor is independent from memory
	    mapping that's why we can close file descriptor but not the lasser.
	 9. From `man openat`: The open() system call opens the file specified by pathname. In this
	    case it opens "/lib/x86_64-linux-gnu/libc.so.6" and assigns it to file descriptor number 3.
         10. From `man read`: read() attempts to read up to count bytes from file descriptor fd
             into the buffer starting at buf. So in this case it reads 832 bytes from file descriptor
	     number 3.
	 11. From `man pread`: pread() reads up to count bytes from file descriptor fd at offset
	     into the buffer starting at buf ... The file offset is not changed. So it does what the
	     read does but does not change file offset. In this case it just reads more of file
	     descriptor number 3.
	 12-19. These lines utilises functions already known to us to load glibc file descriptor into
	        process memory.
	 20. Allocates 12288 bytes and do not map it to anything.
	 21. From `man arch_prctl`: arch_prctl() sets architecture-specific process or thread state.
	     On amd64 the FS register is used to point to a thread-specific data area. Syscall sets
	     the FS register to the address 0x7f9e70a51740. This is essential for accessing
	     thread-local variables and other per-thread data that libraries (like glibc) rely on. 
	 22. From `man set_tid_address`: The system call set_tid_address() sets the clear_child_tid
	     value for the calling thread to tidptr.  I don't quite get what is this function doing
             srry.
	 23. From `man set_robust_list`: A thread can inform  the  kernel of  the  location of its
             robust futex list using set_robust_list(). From what i understand futexes are some
             fancy locking mechanism and this list is needed to use them.
         24. I found some documentation online here https://manpages.opensuse.org/Tumbleweed/librseq-devel/rseq.2.en.html
	     so from the website: The rseq() ABI accelerates specific user-space operations by
	     registering a per-thread data structure shared between kernel and user-space.  It's
	     pretty self explonatory.
	 25-27. From `man mprotect`: mprotect() changes the access protections for the calling
	     process's memory pages ... If the calling process tries to access memory in a manner
	     that violates the protections, then the kernel generates a SIGSEGV signal for the
	     process. So this syscall sets some memory regions to read only.
         28. From `man prlimit`: Given a process ID and one or more resources, prlimit tries to
             retrieve and/or modify the limits. So this syscall is querying current thread for
	     current stack memory limit.
	 29. munmap dereferences memory allocated in line 7.
	 30. Writes to file descriptor number 1, 14 bytes of the string "Hello world!".
	 31. From `man exit_group`: This system call terminates all threads in the calling process.
 *	
 */

#include <unistd.h>

int main(void) {
  char msg[] = "Hello world!\n";

  // STDOUT_FILENO is equal `1` according to `man stdout`.
  write(STDOUT_FILENO, msg, sizeof(msg) / sizeof(char));

  return 0;
}

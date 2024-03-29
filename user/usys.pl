#!/usr/bin/perl -w

# Generate usys.S, the stubs for syscalls.
# 通过调用entry子例程，并传递系统调用的名称作为参数，
# 可以生成每个系统调用的存根代码。这些存根代码的目的
# 是在用户程序和操作系统内核之间提供一个接口，使得用
# 户程序可以通过调用这些存根来执行相应的系统调用。
print "# generated by usys.pl - do not edit\n";

print "#include \"kernel/syscall.h\"\n";

sub entry {
    my $name = shift;
    print ".global $name\n";
    print "${name}:\n";
    # 将sys_trace的代号加载到a7的寄存器中
    print " li a7, SYS_${name}\n";
    # ecall指令从用户态切入到了内核态中
    print " ecall\n";
    print " ret\n";
}
	
entry("fork");
entry("exit");
entry("wait");
entry("pipe");
entry("read");
entry("write");
entry("close");
entry("kill");
entry("exec");
entry("open");
entry("mknod");
entry("unlink");
entry("fstat");
entry("link");
entry("mkdir");
entry("chdir");
entry("dup");
entry("getpid");
entry("sbrk");
entry("sleep");
entry("uptime");
entry("trace");
entry("sysinfo")

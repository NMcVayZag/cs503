# Assignment #10: Isolation/Namespaces

1. A brief introduction to Linux user namespaces (in your own words)
- linux namespaces are a feature that allow for the isolation and virtualization of systems reasources for processes so that you can have multiple isolated environments over a single host. Through using namespaces your process can have its own set of global reasources such as network interfaces, mount points, and process ids and more so that it has its own environment. Each of these things are isolated via namespaces -  a network namespace, a process id namespace,
a mount namespace, a UTS namespace(hostname and domain name isolation), 
a IPC namespace(inter-process communication isolation like messaging queues, semaphores - meaning processes in different IPC namespaces cannot talk to one another), 
user namespace(isolates user and group Ids in containers),
cgroup namespaces(isolates control groups for managing/limiting resources like CPU, Memory, and disk I/O) so that difference processes have their own limits 

These different systems of namespaces allow for the containerizing of processes so that they operating in their own virualized environment.

2. Terminal outputs from each part of the lab
## Part 1: Exploring User Namespaces Basics

1. Examine your current user ID and groups:

```
id
```
output: uid=501(nickmcvay) gid=501(nickmcvay) groups=501(nickmcvay),4(adm),27(sudo),44(video),50(staff)
Record this information for comparison later.

2. List your current namespaces:

```
ls -la /proc/self/ns/
```
Take note of the user namespace identifier. (4026532218)

output: total 0
dr-x--x--x 2 nickmcvay nickmcvay 0 Mar 11 17:56 .
dr-xr-xr-x 9 nickmcvay nickmcvay 0 Mar 11 17:56 ..
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 cgroup -> 'cgroup:[4026532218]'
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 ipc -> 'ipc:[4026532216]'
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 mnt -> 'mnt:[4026532214]'
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 net -> 'net:[4026532219]'
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 pid -> 'pid:[4026532217]'
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 pid_for_children -> 'pid:[4026532217]'
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 time -> 'time:[4026531834]'
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 time_for_children -> 'time:[4026531834]'
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 user -> 'user:[4026531837]'
lrwxrwxrwx 1 nickmcvay nickmcvay 0 Mar 11 17:56 uts -> 'uts:[4026532215]'

3. Create a new user namespace:

```
unshare --user bash
```
          nickmcvay@ubuntu:/Users/nickmcvay/cs503/Assignments/10-Isolation/starter$
 output:  nobody@ubuntu:/Users/nickmcvay/cs503/Assignments/10-Isolation/starter$ 
4. Inside this new bash shell (in the new user namespace), check your user and group IDs:

```
id
```
output: uid=65534(nobody) gid=65534(nogroup) groups=65534(nogroup)

Notice how your UID and GID appear as 65534 (nobody).

5. Examine the namespace identifier to confirm you're in a different user namespace:

```
ls -la /proc/self/ns/user
```
lrwxrwxrwx 1 nobody nogroup 0 Mar 11 18:00 /proc/self/ns/user -> 'user:[4026532484]'

Compare it with the value you noted earlier.
It looks as if that I only have one group and my nameppace identifier is different.

You might also try `whoami` and observe that it prints "nobody".

6. Check your new namespace's UID mapping:

```
cat /proc/self/uid_map
cat /proc/self/gid_map
```
output: NO OUTPUT
Note that these files are likely empty, indicating no mappings are established.

7. Exit the namespace:

```
exit
```
now back to previous namespace:

nickmcvay@ubuntu:/Users/nickmcvay/cs503/Assignments/10-Isolation/starter$

## Part 2: Creating User Namespaces with UID/GID Mappings

For this part, we'll use a simple C program to create a user namespace with custom mappings. The C program is `userns_child.c` in the starter folder

Compile and run this program:

```
make
./userns_child
```

Observe the output and answer these questions:

- What is your UID in the new user namespace?
in this new user namespace my UID is 0(root)
- What capabilities do you have in the user namespace?
In this namespace I have significant capabilities over the system including the following commands:

ep Bounding set =cap_chown,cap_dac_override,cap_dac_read_search,cap_fowner,cap_fsetid,cap_kill,cap_setgid,cap_setuid,cap_setpcap,cap_linux_immutable,cap_net_bind_service,cap_net_broadcast,cap_net_admin,cap_net_raw,cap_ipc_lock,cap_ipc_owner,cap_sys_module,cap_sys_rawio,cap_sys_chroot,cap_sys_ptrace,cap_sys_pacct,cap_sys_admin,cap_sys_boot,cap_sys_nice,cap_sys_resource,cap_sys_time,cap_sys_tty_config,cap_mknod,cap_lease,cap_audit_write,cap_audit_control,cap_setfcap,cap_mac_override,cap_mac_admin,cap_syslog,cap_wake_alarm,cap_block_suspend,cap_audit_read,cap_perfmon,cap_bpf,cap_checkpoint_restore

for instance: cap_chown - allows me to change file ownership by user or groupid

- How do the UID mappings work?
UID mappings allow different user ids between namespaces in containerized environments like docker. This mapping allows processes in containers to run as non-root users inside the container while still being able to map back to another user on the host system such as the root user. Which is important for security and Isolation so that a process inside a container has limited permissions on the host system.
- there are:
    - host UID/GID which are usually the real users or groups on the system
    - container UID/GIDs which are the users/groups inside the container where these ids have permissions in container but much more limited on host.
    - the UID mapping between the containers internal UID/GIDs and hosts UID/GIDs which allow a user to be a root user in container(UID 0) and be a non-privaledged user on the host.

## Part 3: Exploring Isolation with User Namespaces

Run `check_isolation.sh` in the starter folder:

```bash
chmod +x check_isolation.sh
./check_isolation.sh
```
nickmcvay@ubuntu:/Users/nickmcvay/cs503/Assignments/10-Isolation/starter$ chmod +x check_isolation.sh
./check_isolation.sh
*** BEFORE ENTERING USER NAMESPACE ***
=== Process Information ===
PID: 80607
UID/GID: 501/501
Username: nickmcvay
Groups: 501 4 27 44 50

=== Namespace Information ===
user namespace: user:[4026531837]
mnt namespace: mnt:[4026532214]
pid namespace: pid:[4026532217]
net namespace: net:[4026532219]
ipc namespace: ipc:[4026532216]
uts namespace: uts:[4026532215]

=== Capability Information ===
Bounding set =cap_chown,cap_dac_override,cap_dac_read_search,cap_fowner,cap_fsetid,cap_kill,cap_setgid,cap_setuid,cap_setpcap,cap_linux_immutable,cap_net_bind_service,cap_net_broadcast,cap_net_admin,cap_net_raw,cap_ipc_lock,cap_ipc_owner,cap_sys_module,cap_sys_rawio,cap_sys_chroot,cap_sys_ptrace,cap_sys_pacct,cap_sys_admin,cap_sys_boot,cap_sys_nice,cap_sys_resource,cap_sys_time,cap_sys_tty_config,cap_mknod,cap_lease,cap_audit_write,cap_audit_control,cap_setfcap,cap_mac_override,cap_mac_admin,cap_syslog,cap_wake_alarm,cap_block_suspend,cap_audit_read,cap_perfmon,cap_bpf,cap_checkpoint_restore

=== Process List (as seen by this process) ===
UID          PID    PPID  C STIME TTY          TIME CMD
root           1       0  0 Mar01 ?        00:01:33 /sbin/init
root           9       1  0 Mar01 ?        01:44:36 orbstack-agent: ubuntu
root         123       1  0 Mar01 ?        00:02:11 /usr/lib/systemd/systemd-journald
root         176       1  0 Mar01 ?        00:00:00 /usr/lib/systemd/systemd-udevd

*** AFTER ENTERING USER NAMESPACE ***
=== Process Information ===
PID: 80622
UID/GID: 65534/65534
Username: nobody
Groups: 65534

=== Namespace Information ===
user namespace: user:[4026532484]
mnt namespace: mnt:[4026532214]
pid namespace: pid:[4026532217]
net namespace: net:[4026532219]
ipc namespace: ipc:[4026532216]
uts namespace: uts:[4026532215]

=== Capability Information ===
Bounding set =cap_chown,cap_dac_override,cap_dac_read_search,cap_fowner,cap_fsetid,cap_kill,cap_setgid,cap_setuid,cap_setpcap,cap_linux_immutable,cap_net_bind_service,cap_net_broadcast,cap_net_admin,cap_net_raw,cap_ipc_lock,cap_ipc_owner,cap_sys_module,cap_sys_rawio,cap_sys_chroot,cap_sys_ptrace,cap_sys_pacct,cap_sys_admin,cap_sys_boot,cap_sys_nice,cap_sys_resource,cap_sys_time,cap_sys_tty_config,cap_mknod,cap_lease,cap_audit_write,cap_audit_control,cap_setfcap,cap_mac_override,cap_mac_admin,cap_syslog,cap_wake_alarm,cap_block_suspend,cap_audit_read,cap_perfmon,cap_bpf,cap_checkpoint_restore

=== Process List (as seen by this process) ===
UID          PID    PPID  C STIME TTY          TIME CMD
nobody         1       0  0 Mar01 ?        00:01:33 /sbin/init
nobody         9       1  0 Mar01 ?        01:44:36 orbstack-agent: ubuntu
nobody       123       1  0 Mar01 ?        00:02:11 /usr/lib/systemd/systemd-journald
nobody       176       1  0 Mar01 ?        00:00:00 /usr/lib/systemd/systemd-udevd

Note the differences in the output before and after entering the user namespace.
username goes to "nobody"
I only have one group now
all my PIDs in process list are the same though

## Part 4: Practical Applications - Running a Program with "Root" Privileges

1. Compile and use it to run commands as "root" (in the namespace):

```
make
./fake_root id
./fake_root bash -c "id && whoami && ls -la /proc/self/ns/"
```
output:
make
./fake_root id
./fake_root bash -c "id && whoami && ls -la /proc/self/ns/"
Parent process: UID=501, GID=501, PID=80719
Created child process with PID: 80720
Running as UID: 0, GID: 0 (should be 0 for both)
Successfully set hostname in new UTS namespace
uid=0(root) gid=0(root) groups=0(root),65534(nogroup)
Child exited with status: 0
Parent process: UID=501, GID=501, PID=80721
Created child process with PID: 80722
Running as UID: 0, GID: 0 (should be 0 for both)
Successfully set hostname in new UTS namespace
uid=0(root) gid=0(root) groups=0(root),65534(nogroup)
root
total 0
dr-x--x--x 2 root root 0 Mar 11 18:30 .
dr-xr-xr-x 9 root root 0 Mar 11 18:30 ..
lrwxrwxrwx 1 root root 0 Mar 11 18:30 cgroup -> 'cgroup:[4026532218]'
lrwxrwxrwx 1 root root 0 Mar 11 18:30 ipc -> 'ipc:[4026532216]'
lrwxrwxrwx 1 root root 0 Mar 11 18:30 mnt -> 'mnt:[4026532214]'
lrwxrwxrwx 1 root root 0 Mar 11 18:30 net -> 'net:[4026532219]'
lrwxrwxrwx 1 root root 0 Mar 11 18:30 pid -> 'pid:[4026532217]'
lrwxrwxrwx 1 root root 0 Mar 11 18:30 pid_for_children -> 'pid:[4026532217]'
lrwxrwxrwx 1 root root 0 Mar 11 18:30 time -> 'time:[4026531834]'
lrwxrwxrwx 1 root root 0 Mar 11 18:30 time_for_children -> 'time:[4026531834]'
lrwxrwxrwx 1 root root 0 Mar 11 18:30 user -> 'user:[4026532485]'
lrwxrwxrwx 1 root root 0 Mar 11 18:30 uts -> 'uts:[4026532486]'
Child exited with status: 0

Notice that inside the user namespace, your UID is 0 (root), but this is only within the namespace!
Only within the child process.

2. Try to create a file as "root" in your home directory, using your program:

```
./fake_root touch ~/root_test_file
ls -la ~/root_test_file
```
output:
Parent process: UID=501, GID=501, PID=80818
Created child process with PID: 80819
Running as UID: 0, GID: 0 (should be 0 for both)
Successfully set hostname in new UTS namespace
Child exited with status: 0
-rw-r--r-- 1 nickmcvay nickmcvay 0 Mar 11 18:32 /home/nickmcvay/root_test_file

The file is created, but notice who owns it on the real system!
still owned my host user nickmcvay

## Part 5: Exploring the Limitations

1. Try to perform privileged operations in your user namespace:

```
mkdir -p /tmp/test_mount
./fake_root bash -c "mount -t tmpfs none /tmp/test_mount"
```
output:

Parent process: UID=501, GID=501, PID=80889
Created child process with PID: 80890
Running as UID: 0, GID: 0 (should be 0 for both)
Successfully set hostname in new UTS namespace
mount: /tmp/test_mount: permission denied.
       dmesg(1) may have more information after failed mount system call.
Child exited with status: 32

This will likely fail with "Operation not permitted" because of missing capabilities or namespace configurations.

2. Try to access network namespaces (which usually require real root):

```
./fake_root ip link add dummy0 type dummy
```
output: 
Parent process: UID=501, GID=501, PID=80927
Created child process with PID: 80928
Running as UID: 0, GID: 0 (should be 0 for both)
Successfully set hostname in new UTS namespace
RTNETLINK answers: Operation not permitted
Child exited with status: 2

Note and document what errors you encounter.
We were able to set the hostname in the UTS namespace however the RTNETLINK failed as operation is not permitted.


3. Answers to the following questions:
   - How do user namespaces provide the illusion of having root privileges?
        - some user namespaces will give that illusion as they have root priviledges inside a container but not when outside on the host system.
   - What is the purpose of UID/GID mapping in user namespaces?
        - the purpose of mapping UID/GID mapping in user namespaces is to control what permissions a UID/GID will have in the containers environment versus on the host system environemnt so that a root UID(0) in the container is mapped to a non-priviledged-userID in the host system environment. This allows for process isolation and security between processes in the container and on the system.
   - What limitations did you encounter when working with user namespaces?
        - Some limitations I encountered included not having sufficient permissions when operating in a non-root namespace. That and certain commands would work much differently when operating incontainer or child process than on host system. Also keeping track of the UID mappings and tracing those mappings was surely a challenge.
   - How might user namespaces be used in container technology?
        - in container technology user namespaces allow for isolation and securtiy between processes that are running inside and outside containers. They are very key in that they allow safe allocation of root user abilities in container while being mapped to non-priviledged-UIDs on the host. So that if a malicious UID broke out of container onto the host it would have no permissions. Namespaces are also powerful in that they allow containerized solutions to have processes running as root users, as neccessary for applications, within the container but not outside of it.

   - What security implications do user namespaces have?
        - They are very key in that they allow safe allocation of root user abilities in container while being mapped to non-priviledged-UIDs on the host.

   - Why are other namespace types typically not available to unprivileged users?
   So other namespace types outside user namespaces such as: PID, Network, IPC, Mount, UTS - are not available to unpriviledged users given that that would cause major security concerns and could significantly impact overall health of the host system. Access to these namespace types can impact much more on the system than access through the user namespaces could (since root user is still protected). If a malicous user got access to these namespaces then they could damage the host or other containers running on the host.

4. A conclusion section with your insights and any challenges you faced.
Namespaces are a crutial mechanism of creating & managing containerized environments on a host system gracefully. It was very cool to learn about these different namespace types and the role they play in containerization. Specifically, it was cool to learn about how permissions and resource allocation could be controlled across multiple containers and the host system. However, getting a grasp was not a breeze it was hard at first to visualize the mapping of UIDs between containers and the host system and why that was an important aspect to control for secturity and virtualization.
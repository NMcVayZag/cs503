1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: I think for provides the spawning of a child process from a parent - without fork we would have to child to pick up the execvp

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**: If the fork sys call fails then the child process is not created - I will report the error to console and then exit the program with code 1.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**: The execvp() method finds the command to execute by looking at the first command and then looking for it in the directories available to the PATH environment variable.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The Wait method allows us to hault our main process until the child process completes. If we didn't call wait our main process might finish first leading to a output randomly spawning in our terminal.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS() method provides the status code of the child process returned to the wait() statement - it is important because the completion status of that code may directly impact the main process so we need to know if it was successfull or not.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**: My implementation tracked the appearance of double quotes and allowed for the spaces inbetween to be written over while the quotes themselves were not. this was critical in the case that you're passing in a string to a command that requires certain whitespace.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**: Parsing logic was much harder since it was more than just splitting strings by spaces and pipes it required a token by token digestion to add selective chars to the right argument. I overhauled my parsing and I think it was for the better.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  signals are fundamental in linux systems allowing for control of processes(stop or pause certain processes), asynchronous communication(allowing processes to be alerted to external events) & sending of notifications on events(examples include segmentaion faults, timers, and the starting and execution of child processes).
    - these differ from other forms of interprocess communication in that they're typically more asynchronous, directional, and simple versus other forms of IPC. In addition, they are used more for sending simple notifications rather than raw data. Lastly, other IPC is more flexible and complex - allowing for handling synchronization, and data transfers.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  
    - 1: SIGKILL - forces the termination of a process immediately without any cleanup - this cannot be blocked or denied by the process. Some common use cases are a frozen program or stopping a hostile process
    - 2: SIGTERM - the default signal used to terminate a process with care allowing the process to catch and handle the signal allowing for the process to complete critical cleanup(closing files, cleaning up reasorces) before terminating.
    - 3: SIGINT - the signal sent to interupt a process through cntrl+C - also allows to be caught and handled by the program. typically used in programs that await user interaction - I use it when debugging some broken code.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  when a process recieves a sigstop signal it is paused by the operating system. All the resources of the process are maintained while frozen and are available when the process becomes unfrozen. It cannot be caught or ignored by the process because that's how it was intended to be - it is utilized to freeze a process unconditionally and is managed by the kernel. A very common use case for this signal is debugging where you create breakpoints in your code and then roll through them.

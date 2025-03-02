1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

the program could continue on with other processes still in progress that would be randomly outputing to the terminal and these late processes could cause the program to produce inconsistent or unsatisfactory results. It could also create zombie processes that never have their exit statuses cleaned up and could simply take up memory. The child process could be using file descriptors or memory and won't have them relinquished until they have been "waited on".

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

If we leave the pipes open, the following child processes might be able to use them which would disrupt or maybe even break the pipeline - after they are copied to STIN and STDOUT they should be closed for that reason.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

cd is implemented as a built-in rather than a external command because it directly modifier the state of the current shell process. If this was done in a child process it would not alter the shell state it would just occur in the child process and the state of parent process would go unchanged.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

I would modify my implementation to dyamically allocate reasorces based on how many piped commands are inputed. This will increase the robustness of my program but also the complexity like using malloc() for the memory allocation of pipes and freeing them when complete. The main cons are that I will need to allocate and deallocate memory for these pipes, I will have a harder time debugging with large piping processes and erro handling will become a lot more tedious.

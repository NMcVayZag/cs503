1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**: fgets is a good choice for this application because it will read the user input up to the length of the input -1 which will stop it from hitting the end of the input and creating buffer overflow and it also stops at \n and will put a null character at the end of the string when done reading. This is ideal for our small one line commands that need to be processed as strings.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  We needed to use malloc to allocate memory for cmd_buff given that we needed to dynamically change the size of the buffer for the user's input so our code is flexible and efficient. It also allows us to prevent buffer overflow.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**: if we didn't trim leading and trailing spaces then our shell wouldn't recognize the inputed commands as valid given that whitespace which would cause the process to fail.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  after researching- redirection provides the user with the ability to choose where the input and output of a command go versus the default terminal's input and output
    here are 3 redirection examples for our code:
    1- we could output our cleaned command summaries into a file that could be read or maybe a log of those commands. We would be challenged with file handeling which requires us to handle the file differently based on its existance
    2- we could have an input file that is redirected as a command list so that we could put test cases in a file and then run them against our shell for testing. This would give us reading challenges tracking where is the EOF and also iterating through rows of the file. Also have to deal with existance of file challenges
    3- we could redirect our printing of the drexel dragon to writing it to a output file. This would also give us similar file handling challenges

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  
    1- while redirection is used to change the destination or source of input or output to a file for a single command. It allows the command to read from or write to files rather than just dealing 
    with standard stdin and stout. This is different from piping where the output of one command is connected to the input of another command - allowing the output of a process to be and input of another
    without having to use any intermediate files.
    2- another difference is redirection is often a one-way process whereas with piping it is typically a two-way flow of data betweeen processes
    3- redirection is used to handle input and output flow of one caommand at a time whereas piping is often used to link multiple commands in chain to execute a sequence of processes


- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**: 
    - stdout represents regular command output while stderr is for error messages - its very crucial to keep these two streams of data separate so we can accurately interpret the results of a command
    - this is also helpful in redirection - so that one can log successful/regular command outputs in one file and the errors in another. This also makes error handling and debugging a more clear process.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  our shell should handle errors by sending them to their own output file and logging the occurance and making sure our code returns a 1 for failure. I can see how in some cases we would 
    want our program to output both sdout and sterror so we can get the whole picture on the attempted execution of a command - in this case we would want to merge them into one log file and we can do this
    with the following redirection strategy:   $ write some_arg 2>&1. Another way we should be handling our errors is deciphering which one should cause the program to immediately exit and which ones won't
    impact the remainder of the commands so we can continue- either way we should be logging stdout and sterror along the process.

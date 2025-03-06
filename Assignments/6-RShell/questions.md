1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

We determine if the commands output is fully recieved from the server when we recieve the EOF character through the socket. For partial messaging we simply reuse the same loop in the recieving command so that partial message peices can be appended to the buffer space. To get the complete message we only stop reading from the socket when we recieve the EOF char.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

A networked shell protocol should define and detect the beginning and end commmands by specifiying, communicating, and confirming on delimiters to denote the beginning of a message and the end of a message - for instance in our situation the start of a message was denoted by any btyes and the end was denoted by an EOF character. If this is not properly implemented and consistent across the client and server then one side may stop reading before


3. Describe the general differences between stateful and stateless protocols.

_answer here_

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

_answer here_

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

_answer here_
1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

We determine if the commands output is fully recieved from the server when we recieve the EOF character through the socket. For partial messaging we simply reuse the same loop in the recieving command so that partial message peices can be appended to the buffer space. To get the complete message we only stop reading from the socket when we recieve the EOF char.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

A networked shell protocol should define and detect the beginning and end commmands by specifiying, communicating, and confirming on delimiters to denote the beginning of a message and the end of a message - for instance in our situation the start of a message was denoted by any btyes and the end was denoted by an EOF character. If this is not properly implemented and consistent across the client and server then one side may stop reading before the message has finsihed coming across the socket and in other cases the server/client will keep waiting after reading the supposed "EOF" character not recognizing it as the end of message delimiter


3. Describe the general differences between stateful and stateless protocols.

A stateful protocol will manage a state or interaction context between the server and the client so that messages occur in the context of previous interactions. Stateful protocols allow for a more interactive and capable experience but are more resource intensive. A stateless protocol will hold no state between interactions between sever and client so that every interaction is treated independently. Statless protocol are less reasource intensive but less interactive and capable.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

The reason why you would use UDP in some cases is that it has very low overhead so for cases where you do not need reliability to send every bit of a message it is the most effective option in terms of latency. So in cases where latency is prioritized over 100% reliable data transmission UDP is a great solution protcol. 

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

Socket Interface. The OS allows applications to use network communications via the abstraction of sockets that provide us with a variety of syscalls(socket,bind,listen,accept,connect,send,recv,close) that allow us to manage network connections between other systems via network ip addresses(remote or local). The OS abstracts the network stack, error handling/timeouts, and socket data management.
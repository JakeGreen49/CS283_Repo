1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

_The remote client determines the command output is fully received when it detects the EOF character in the stream it is receiving from the server. To handle partial reads, we can loop over the recv() system call until the client receives that EOF character._

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

_The network shell protocol can do one of two things to define the beginning/end of a command sent over TCP. One option is to simply define special characters that delineate the end of a message, which is what was done in this assignment (the null terminator defined the end of a message from the client to the server, and the EOF character defined the end of a message from the server to the client). Another option is to include a header that contains information on the message_ including _the message's length, then that information could be parsed and utilized to determine the exact byte where the message ends._

3. Describe the general differences between stateful and stateless protocols.

_Stateful protocols are ones in which the server keeps track of information about the session and the statuses of previous requests; stateful protocols are generally slower with more overhead, but are 100% reliable. Stateless protocols are ones in which each message is independent (ie. does not rely on any information about previous interactions); stateless protocols are a lot faster with less overhead, but are unreliable, meaning some information could occasionally be lost in data transfer._

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

_UDP generally transfers data a lot faster than TCP does. So, if speed in data transfer is desired while ocassional packet loss is deemed acceptable, UDP is a far better choice than TCP. An example of where this is the case is video streaming; generally, it is preferred to stream the videos faster, even if that means there could be occasional reductions to picture quality due to packet loss._

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

_The operating system provides a networking API that allows applications to use network communications. Included in this API are the following relevant system calls: the socket() system call provides a socket, which are basically just file descriptors that allow for network communication; the send() system call will send data across the socket, and it is analagous to the write() system call; the recv() system call will receive data across a socket, and it is analagous to the read() system call._

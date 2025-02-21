1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  _execvp (or any flavor of exec system call) wipes all of the memory of the process that called it to load in the ELF file that was inputted into it. So, if we only called execvp, our shell program would be wiped from memory and replaced with the command, meaning our shell could only run a single command. The fork system call creates a child process copied from the shell; if we call execvp in this child process, its memory will be wiped and replaced with the file to run, and once that finishes we return to the parent process, our shell, so we are able to run more commands._

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  _When the fork system call fails, it returns a value of -1 in the parent process and no child process was created. My implementation checks for this negative return value right away in its surrounding function and returns early, as the parent process cannot wait on a child process that does not exist. Moreover, when fork() fails it sets the errno based on the failure, and my code will print a specific error message based on this errno._

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  _The execvp() system call searches for the command through the PATH environment variable, which is a list of the directories that contain executables that can be run in the shell without specifying the entire file path. Essentially, by searching through the PATH variable, it makes it so that our shell can run commands without having to specify the full file path, thus mirroring the behavior of most popular shells._

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  _We call wait() so that the parent process does not continue running along side the newly forked child process. If we did not do this, the parent process could terminate before the child process does, in which case the child's new parent would be the init process and might continue running in the background indefinitely, leading to undefined behavior._

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  _This provides the return value of the child process from a fork. It is important to have this information because it tells us if the process executed properly or threw and error, which we otherwise would not have access to if we did not use WEXITSTATUS._

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  _I used a bool called 'in_quotes' that is initially set to false and switched whenever a quotation mark is found iterating through a string. Then, if the bool is true, all spaces are passed into the cmd_buff struct, otherwise the spaces are ignored. This is necessary to do to allow the user of the shell an option for passing in strings that are not to be trimmed of their extra spaces; this might be useful if one wanted to echo something like 'hello     world', which would not be possible in our shell without implementing quoted arguments._

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  _The biggest changes in the parsing logic were allowing for quoted arguments and not parsing piped commands. The latter was fairly easy to refactor, as the code that parsed piped command strings had to parse the individual commands, so I just took this code near verbatim for its purpose. The former was much more tricky to implement, as I had to keep track of whether we are in quotes at any point as we iterate through the string. I ended up having to define a new variable and add more conditionals in order to properly implement this feature. Another thing I had to refactor was using a different data structure to hold the command information, and this was only tricky insofar as I wrote my code on Vim and had to get used to using a different names for and in the data structure when writing my code._

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  _The purpose of signals is to trigger certain behaviors while a process is running, such as pausing or ending the process. Unlike other forms of IPC, signals can (and, in fact, only) directly influence the actions of a process while that process is in the midst of running._

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  _The default behaviors of all three of those signals has the same effect: terminate the process to which they are sent. SIGINT is a signal to interrupt the process and can be sent directly by the user with the ctrl C key; though its default behavior is to terminate the process, the process has the option to handle this signal and do something else entirely, such as ignoring it. SIGTERM is a signal to terminate a process and is typically sent by other processes; similarly to SIGINT, the process is free to handle SIGTERM as it chooses. SIGKILL is a signal to terminate a process immediately; processes have no choice in how to handle SIGKILL, and do not even get the opportunity to exit gracefully upon receiving it. SIGINT is generally used when a user wants to stop a process, SIGTERM is generally used by processes to make other processes exit gracefully, and SIGKILL is generally used as only a last resort to terminate a process (given that it allows no option for graceful exit)._

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  _When a process receives SIGSTOP, its execution is paused and can be later resumed if it receives the SIGCONT signal. Unlike SIGINT, SIGSTOP cannot be caught or ignored. This is the case because SIGSTOP (as well as SIGCONT) are used for job control, so it is imperative to the proper and orderly operation of the operating system that these signals go through when they are sent._

Assignment Description: SimpleShell Implementation
Overview:
This assignment requires you to implement a basic shell program called SimpleShell in C. The implementation involves reading and executing user commands with specific features and constraints, as outlined below.

Implementation Details:
Shell Behavior:

The SimpleShell program should continuously perform the following tasks in an infinite loop:
Display a command prompt of your choice.
Read the user command from standard input.
Parse the command into the command name and its arguments (if any).
Execute the parsed command along with the arguments.
Command Input Constraints:

User commands will follow these rules:
Commands and arguments are separated by whitespace.
Commands will not include backslashes or quotes.


echo you should be aware of the plagiarism policy

you should be aware of the plagiarism policy


Execution Method:

The shell must execute commands by creating a child process using system calls (as discussed in Lectures 06 and 07).
Use any of the exec family functions to execute the commands.

Supported Commands:

ls
ls /home
echo you should be aware of the plagiarism policy
wc -l fib.c
wc -c fib.c
grep printf helloworld.c
ls -R
ls -l
./fib 40
./helloworld
sort fib.c
uniq file.txt
cat fib.c | wc -l
cat helloworld.c | grep print | wc -l

fib: A Fibonacci number calculator.
helloworld: A "Hello, World!" program.

The file file.txt is created with repetitive lines to test the uniq command.


Piping:

The shell must support pipes (|) for chaining commands.
History Command:

Implement a history command to display the list of commands entered in the current session of SimpleShell, including their arguments.
Termination Behavior:

Upon termination of the shell, display the following details for each command executed in the current session:
Process PID.
Time at which the command was executed.
Total duration taken for execution.

Add support for background processes using & at the end of a command.

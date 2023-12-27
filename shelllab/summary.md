# helper routines that manipulate the job list

- clearjob - Clear the entries in a job struct
- initjobs - Initialize the job list
- maxjid - Returns largest allocated job ID
- addjob - Add a job to the job list
- deletejob - Delete a job whose PID=pid from the job list
- fgpid - Return PID of current foreground job, 0 if no such job
- getjobpid  - Find a job (by PID) on the job list
- getjobjid  - Find a job (by JID) on the job list
- pid2jid - Map process ID to job ID
- listjobs - Print the job list

# Other helper routines
- usage - print a help message
- unix_error - unix-style error routine
- app_error - application-style error routine
- sio_reverse - Reverse a string (from K&R)
- sio_ltoa - Convert long to base b string (from K&R)
- sio_strlen - Return length of string (from K&R)
- sio_copy - Copy len chars from fmt to s (by Ding Rui)
- sio_puts(char s[]) /* Put string */
- sio_putl(long v) /* Put long */
- sio_put(const char *fmt, ...) // Put to the console. only understands %d
- sio_error(char s[]) /* Put error message and exit */
- Signal - wrapper for the sigaction function
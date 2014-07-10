#ifndef __PHAGE2014_SHELL_H__
#define __PHAGE2014_SHELL_H__

/* Returns TRUE if the shell has terminated (or hasn't been launched yet) */
int shellTerminated(void);

/* Runs a new shell thread */
void shellRestart(void);

#endif /* __PHAGE2014_SHELL_H__ */

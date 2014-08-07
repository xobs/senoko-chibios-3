#ifndef __SENOKO_SHELL_H__
#define __SENOKO_SHELL_H__

<<<<<<< HEAD
void senokoShellInit(void);

/* Runs a new shell thread */
void senokoShellRestart(void);
=======
/* Returns TRUE if the shell has terminated (or hasn't been launched yet) */
int shellTerminated(void);

/* Runs a new shell thread */
void shellRestart(void);
>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c

#endif /* __SENOKO_SHELL_H__ */

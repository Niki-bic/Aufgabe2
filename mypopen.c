/**
 * @file mypopen.c
 * BES - mypopen.c function and associated functions
 * Projekt 2
 *
 * Gruppe 13
 *
 * @author Binder Patrik         <ic19b030@technikum-wien.at>
 * @author Ferchenbauer Nikolaus <ic19b013@technikum-wien.at>
 * @author Pittner Stefan        <ic19b003@technikum-wien.at>
 * @date 2020/05/4
 *
 * @version 1.x
 *
 */

/**
 * -------------------------------------------------------------- includes --
 */

#include "mypopen.h"

/**
 * --------------------------------------------------------------- globals --
 */

static pid_t g_childpid = -1;
static FILE *g_stream = NULL;

/**
 * ------------------------------------------------------------- functions --
 */

/**
 * \brief reset - function to reset globals
 * @details this function resets the global variables
 * in case the fuction gets called more often
 *
 * \return no return value
*/

void reset(void) {
	g_childpid = -1;
	g_stream = NULL;
} 
// end reset


/**
 * \brief mypopen - a simple implementation of the popen function to fork a process
 * @details this function generates a pipe and creates a child process with 
 * the systemcall fork, inside the child process the system function dup2 
 * redirects either stdout or stdin from the child process to the used end,
 * either to the read or write end of the pipe, then the execl systemcall
 * opens the bash shell with the -c flag (man bash) to execute the given 
 * command from the string, the file descriptor (fd) is returned to 
 * the caller as a file stream
 *
 * \param command - the bash command which the pipe is opened for
 * \param modus - "r" read or "w" write - direction of pipe 
 *
 * \return NULL if an error occured and errno is set, global filepointer on success
*/

FILE *mypopen(const char *const command, const char *const type) {
	// Filedeskriptor
	int fd[2] = {0};                                    
	// prüft ob my popen zwei Mal geöffnet ist
	if (g_childpid != -1) {                             
		errno = EAGAIN;
		return NULL;
	}
	// prüft ob eine falsche Type eingegeben wurde
	if (type == NULL || (strcmp(type, "r") != 0 && strcmp(type, "w") != 0)) {  
		errno = EINVAL;
		return NULL;
	}
	// pipe wird angelegt
	if (pipe(fd) == -1) {                              
		return NULL;
	}
	// child-process mittels fork() erzeugen
	switch (g_childpid = fork()) {  
		// fehler bei fork()
		case -1:                                       
			(void) close(fd[0]);
			(void) close(fd[1]);
			return NULL;
		// child-process erzeugen
		case 0:                                         
			child_process(fd, type, command);
			break;
		// parent-process erzeugen
		default:                                        
			parent_process(fd, type);
	}

	return g_stream;
} 
// end mypopen


/**
 * \brief child_process - function called inside the child process for redirection
 * @details this function calls the system function dup2 to redirect either stdout 
 * or stdin from the child process to the used end, either to the read or write end 
 * of the pipe, then the execl systemcall opens the bash shell with the -c 
 * flag (man bash) to execute the given command from the string
 *
 * \param *fd - the file descriptor with which the pipe was created
 * \param type - "r" read or "w" write - direction of pipe 
 * \param command - command which gets executed by the bash shell
 *
 * \return no return value
*/


void child_process(int *fd, const char *const type, const char *const command) {
	// command-Eingabe wird in pipe geschrieben
	if (type[0] == 'r') { 
		 // lese-Ende der pipe schließen
		(void) close(fd[0]);                           
		// nur duplizieren wenn noch nicht existent
        if (fd[1] != STDOUT_FILENO) {           
			// dupliziert Filedeskriptor
			if (dup2(fd[1], STDOUT_FILENO) == -1) {     
				(void) close(fd[1]);
				_exit(EXIT_FAILURE);
			}
			// schreib-Ende wird nicht mehr benötigt
			if (close(fd[1]) == -1) {                   
				_exit(EXIT_FAILURE);
			}
		}
	// wenn type == 'w'	
	} else {   
		// schreib-Ende der pipe schließen
		(void) close(fd[1]);                            
		// nur duplizieren wenn noch nicht existent
        if (fd[0] != STDIN_FILENO) {         
			// dupliziert Filedeskriptor
			if (dup2(fd[0], STDIN_FILENO) == -1) {      
				(void) close(fd[0]);
				_exit(EXIT_FAILURE);
			}
		// lese-Ende wird nicht mehr benötigt
		(void) close(fd[0]);                            
		}
	}
	// ausführen des Befehls
	(void) execl("/bin/sh", "sh", "-c", command, (char *) NULL);  
	// wird nur im Fehlerfall ausgeführt
	_exit(EXIT_FAILURE);                                
} 
// end child_process


/**
 * \brief parent_process - function called inside the parent process to 
 * convert the file descriptor
 * @details this function executes the systemcall fdopen to convert 
 * the handed file descriptor to a file stream for returning it with the 
 * mypopen function
 *
 * \param *fd - the file descriptor with which the pipe was created
 * \param type - "r" read or "w" write - direction of pipe 
 *
 * \return no return value
*/

void parent_process(int *fd, const char *const type) {
	// im parent-process wird gelesen
	if (type[0] == 'r') {      
		// schreib-Ende der pipe schließen
		(void) close(fd[1]);                            
		// filedeskriptor in Stream umwandeln
		if ((g_stream = fdopen(fd[0], "r")) == NULL) {  
			reset();
			(void) close(fd[0]);
		}
	// type == 'w' im parent wird geschrieben
	} else {   
		// lese-Ende der pipe schließen
		(void) close(fd[0]);                            
		// filedeskriptor in Stream umwandeln
		if ((g_stream = fdopen(fd[1], "w")) == NULL) { 
			// globals zurücksetzen
			reset();
			(void) close(fd[1]);
		}
	}
} 
// end parent_process


/**
 * \brief mypclose - function waiting for the child process to terminate
 * @details this function waits for the process called by mypopen to terminate,
 * it returns the exit status of the child-process and closes the passed file stream
 * if it is equal to the associated one from mypopen
 *
 * \param *stream - filepointer handed over by the caller
 *
 * \return -1 on error, exit status of the child process on success
*/

int mypclose(FILE *stream) {
	pid_t wait_pid = 0;
	int status = -1;
	// prüfen ob mypopen schon aufgerufen wurde
	if (g_childpid == -1 && g_stream == NULL) {         
		errno = ECHILD;
		return -1;
	//prüfen ob popen schon aufgerufen wurde aber mypclose ein NULL Pointer übergeben wurde
	} else if (g_childpid != -1 && g_stream == NULL) {
		errno = EINVAL;
		return -1;
	}
	// prüfen ob der richtige stream übergeben wurde
	if (stream != g_stream) {                           
		errno = EINVAL;
		return -1;
	}
	// stream schließen
	if (fclose(stream) != 0) {                          
		(void) kill(g_childpid, SIGKILL);
		return -1;
	}
	// auf child-process warten
	while ((wait_pid = waitpid(g_childpid, &status, 0)) != g_childpid) {
		if (wait_pid == -1 && errno == EINTR) {
			continue;
		}
		// fehler beim Warten auf child-process
		errno = ECHILD;
		g_stream = NULL;
		return -1;   
	}
	// globals zurücksetzen
	reset();
	// exit-status prüfen
	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	} else {
		errno = ECHILD;
		return -1;
	}
} 
// end mypclose


/**
 * =================================================================== eof ==
 */

 /**
 * mode: c
 * c-mode: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * end
 *
 */

#include "mypopen.h"


static pid_t g_childpid = -1;
static FILE *g_stream = NULL;


void reset(void) {
	g_childpid = -1;
	g_stream = NULL;
} 
// end reset


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


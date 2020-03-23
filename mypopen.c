#include "mypopen.h"


FILE *mypopen(const char *const command, const char *const type){
	int fd[2];                                               // Filedeskriptor
	FILE *stream;                                            // Filepointer

	if(strcmp(type, "r") != 0 && strcmp(type, "w") != 0){    // type ist falsch
		errno = EINVAL;
		return NULL;
	}

	if(pipe(fd) == -1){                                      // pipe einrichten
		// errno = 
		return NULL;
	}

	pid_t child_pid;

	switch(child_pid = fork()){                              // Kindprozess mittels fork() erzeugen
		case -1:                                             // Fehler bei fork()
			// errno = 
			return NULL;
			break;
		case 0:                                              // child-process
			child_process(fd, command, type);
			break;
		default:                                             // parent-process
			return parent_process(fd, type);
	}
} // end mypopen


void child_process(int *fd, const char *const command, const char *const type){
	if(strcmp(type, "r") == 0){                         // command-Eingabe wird in pipe geschrieben
		close(fd[0]);                                   // Lese-Ende der pipe schließen

        if(fd[1] != STDOUT_FILENO){                     // nur duplizieren wenn noch nicht existent
			if(dup2(fd[1], STDOUT_FILENO) == -1){       // dupliziert Filedeskriptor
				// errno = 
				exit(EXIT_FAILURE);
			}
		}
		if (close(fd[1]) == -1){                        // nach dem Duplizieren wird das
			// errno =                                  // Schreib-Ende nicht mehr benötigt
			exit(EXIT_FAILURE);
		}
	}
	else{                                               // type == 'w'  
		close(fd[1]);                                   // Schreib-Ende der pipe schließen

        if(fd[0] != STDIN_FILENO){                      // nur duplizieren wenn noch nicht existent
			if(dup2(fd[0], STDIN_FILENO) == -1){        // dupliziert Filedeskriptor
				exit(EXIT_FAILURE);
			}
		}

		close(fd[0]);                                   // Lese-Ende wird nicht mehr benötigt
	}

	execl("/bin/sh", "sh", "-c", command, (char *)NULL);  // Ausführen des Befehls

	// errno =
	exit(EXIT_FAILURE);
} // end child_process


FILE *parent_process(int *fd, const char *const type){
	FILE *stream;

	if(strcmp(type, "r") == 0){                           // im parent-process wird gelesen
		close(fd[1]);                                     // Schreib-Ende der pipe schließen

		stream = fdopen(fd[0], "r");
	}
	else{ // type == 'w'                                     im parent-process wird geschrieben
		close(fd[0]);                                     // Lese-Ende der pipe schließen

		stream = fdopen(fd[1], "w");
	}

	return stream;
} // end parent_process


int mypclose(FILE *stream){
	pid_t wait_pid = -1;
	int status = -1;

	while((wait_pid = wait(&status))){                    // auf child-process warten
		if(wait_pid != -1){
			continue;
		}
		if(errno == EINTR){
			continue;
		}

		return -1;                                        // Fehler beim Warten auf child-process
	}

	return (fclose(stream));
} // end mypclose



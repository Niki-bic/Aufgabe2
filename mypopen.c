// noch zu tun: errno richtig setzen, pipe-Enden im Fehlerfall richtig schließen
// sicherstellen dass nur ein child-process generiert wird

#include "mypopen.h"


static pid_t child_pid = -1;


FILE *mypopen(const char *const command, const char *const type){
	if(child_pid != -1){
		// errno = 
		return NULL;
	}
	

	int fd[2];                                               // Filedeskriptor
	FILE *stream = NULL;                                     // Filepointer

	if(strcmp(type, "r") != 0 && strcmp(type, "w") != 0){    // type ist falsch
		errno = EINVAL;
		return NULL;
	}

	if(pipe(fd) == -1){                                      // pipe einrichten
		return NULL;
	}

	switch(child_pid = fork()){                              // Kindprozess mittels fork() erzeugen
		case -1:                                             // Fehler bei fork()
			close(fd[0]);
			close(fd[1]);
			return NULL;
		case 0:                                              // child-process
			child_process(fd, type, command);
			break;
		default:                                             // parent-process
			stream = parent_process(fd, type);
	}
	return stream;
} // end mypopen


void child_process(int *fd, const char *const type, const char *const command){
	if(strcmp(type, "r") == 0){                         // command-Eingabe wird in pipe geschrieben
		close(fd[0]);                                   // Lese-Ende der pipe schließen

        if(fd[1] != STDOUT_FILENO){                     // nur duplizieren wenn noch nicht existent
			if(dup2(fd[1], STDOUT_FILENO) == -1){       // dupliziert Filedeskriptor
				close(fd[1]);
				exit(EXIT_FAILURE);
			}
		}
		if (close(fd[1]) == -1){                        // nach dem Duplizieren wird das
			exit(EXIT_FAILURE);
		}
	}
	else{                                               // type == 'w'  
		close(fd[1]);                                   // Schreib-Ende der pipe schließen

        if(fd[0] != STDIN_FILENO){                      // nur duplizieren wenn noch nicht existent
			if(dup2(fd[0], STDIN_FILENO) == -1){        // dupliziert Filedeskriptor
				close(fd[0]);
				exit(EXIT_FAILURE);
			}
		}

		close(fd[0]);                                   // Lese-Ende wird nicht mehr benötigt
	}

	execl("/bin/sh", "sh", "-c", command, (char *)NULL);  // Ausführen des Befehls

	exit(EXIT_FAILURE);
} // end child_process


FILE *parent_process(int *fd, const char *const type){
	FILE *stream = NULL;

	if(strcmp(type, "r") == 0){                           // im parent-process wird gelesen
		close(fd[1]);                                     // Schreib-Ende der pipe schließen

		if((stream = fdopen(fd[0], "r")) == NULL){
			close(fd[0]);
		}
	}
	else{                                                 // type == 'w' im parent wird geschrieben
		close(fd[0]);                                     // Lese-Ende der pipe schließen

		if((stream = fdopen(fd[1], "w")) == NULL){
			close(fd[1]);
		}
	}

	return stream;
} // end parent_process


int mypclose(FILE *stream){
	int status = -1;
	pid_t wait_pid = 0;

	fclose(stream);

	while((wait_pid = waitpid(child_pid, &status, 0)) != child_pid){   // auf child-process warten
		if(wait_pid != -1){
			continue;
		}
		if(errno == EINTR){
			continue;
		}

		// errno = 
		return -1;                                        // Fehler beim Warten auf child-process
	}

	if(WIFEXITED(status) == 1){
		return WEXITSTATUS(status);
	}
	else{
		// errno = 
		return -1;
	}
} // end mypclose



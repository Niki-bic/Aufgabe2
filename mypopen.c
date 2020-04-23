// noch zu tun: errno richtig setzen, pipe-Enden im Fehlerfall richtig schließen


#include "mypopen.h"


static pid_t g_childpid = -1;
static FILE *g_stream = NULL;


void reset(void) {
	g_childpid = -1;
	g_stream = NULL;
} // end rest


FILE *mypopen(const char *const command, const char *const type) {
	int fd[2] = {0};                                         // Filedeskriptor

	if (g_childpid != -1) {                                  // child existiert schon
		errno = EAGAIN;
		return NULL;
	}

	if (strcmp(type, "r") != 0 && strcmp(type, "w") != 0) {  // type ist falsch
		errno = EINVAL;
		return NULL;
	}

	if (pipe(fd) == -1) {                                    // pipe einrichten
		return NULL;
	}

	switch (g_childpid = fork()) {                           // Kindprozess mittels fork() erzeugen
		case -1:                                             // Fehler bei fork()
			(void) close(fd[0]);
			(void) close(fd[1]);
			return NULL;
		case 0:                                              // child-process
			child_process(fd, type, command);
			break;
		default:                                             // parent-process
			parent_process(fd, type);
	}

	return g_stream;
} // end mypopen


void child_process(int *fd, const char *const type, const char *const command) {
	if (type[0] == 'r') {                               // command-Eingabe wird in pipe geschrieben
		(void) close(fd[0]);                            // Lese-Ende der pipe schließen

        if (fd[1] != STDOUT_FILENO) {                   // nur duplizieren wenn noch nicht existent
			if (dup2(fd[1], STDOUT_FILENO) == -1) {     // dupliziert Filedeskriptor
				(void) close(fd[1]);
				exit(EXIT_FAILURE);
			}
		}

		if (close(fd[1]) == -1) {                       // Schreib-Ende wird nicht mehr benötigt
			exit(EXIT_FAILURE);
		}
	} else {                                            // type == 'w'  
		(void) close(fd[1]);                            // Schreib-Ende der pipe schließen

        if (fd[0] != STDIN_FILENO) {                    // nur duplizieren wenn noch nicht existent
			if (dup2(fd[0], STDIN_FILENO) == -1) {      // dupliziert Filedeskriptor
				(void) close(fd[0]);
				exit(EXIT_FAILURE);
			}
		}

		(void) close(fd[0]);                            // Lese-Ende wird nicht mehr benötigt
	}

	(void) execl("/bin/sh", "sh", "-c", command, (char *) NULL);  // Ausführen des Befehls

	exit(EXIT_FAILURE);                                 // wird nur im Fehlerfall ausgeführt
} // end child_process


void parent_process(int *fd, const char *const type) {
	if (type[0] == 'r') {                                 // im parent-process wird gelesen
		(void) close(fd[1]);                              // Schreib-Ende der pipe schließen

		if ((g_stream = fdopen(fd[0], "r")) == NULL) {    // Filedeskriptor in Stream umwandeln
			reset();
			(void) close(fd[0]);
		}   
	} else {                                              // type == 'w' im parent wird geschrieben
		(void) close(fd[0]);                              // Lese-Ende der pipe schließen

		if ((g_stream = fdopen(fd[1], "w")) == NULL) {    // Filedeskriptor in Stream umwandeln
			reset();
			(void) close(fd[1]);
		}
	}
} // end parent_process


int mypclose(FILE *stream) {
	pid_t wait_pid = 0;
	int status = -1;

	if (g_childpid == -1 && g_stream == NULL) {                             // mypopen wurde nicht aufgerufen
		errno = ECHILD;
		return -1;
	}
	else if (g_childpid != -1 && g_stream == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (stream != g_stream) {                           // falscher stream wurde übergeben
		errno = EINVAL;
		return -1;
	}

	if (fclose(stream) != 0) {                          // stream schließen
		(void) kill(g_childpid, SIGKILL);
		return -1;
	}       

	while ((wait_pid = waitpid(g_childpid, &status, 0)) != g_childpid) {// auf child-process warten
		if (wait_pid != -1 && errno == EINTR) {
			continue;
		}
		errno = ECHILD;
		g_stream = NULL;
		return -1;                                        // Fehler beim Warten auf child-process
	}

	reset();

	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	} else {
		errno = ECHILD;
		return -1;
	}
} // end mypclose


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 256
#define READ 0
#define WRITE 1

void intHandler();

int main() {

	pid_t pid, pid2;
	int status;
	signal(SIGINT, intHandler);

	while (1) {
		char command[SIZE];
		char command1[SIZE] = "default";
		char msg1[SIZE] = "default";
		char check[SIZE] = "default";
		char command2[SIZE] = "default";
		char msg2[SIZE] = "default";

		int control = 0;  // 0 = control error, 1 = 1 command
						  //2 = +msg, 3 = Redirection, 4,5 = pipe
		int amper = 0;	// 0 = front,  1 = back
		int redirect1 = 3;
		int redirect2 = 3;
		int filedes[2];
		int fd;
		int i = 0;
		int n = 0;
		char line[SIZE];

		printf("$ ");
		fgets(command, 256, stdin);

		sscanf(command, "%s %s %s %s %s", command1, msg1, check, command2, msg2);




		if ((strncmp(msg1, "default", 7) == 0)) {
			control = 1;
		}
		else if ((strncmp(command2, "default", 7) == 0) && (strncmp(msg2, "default", 7) == 0)) {
			control = 2;
		}
		if (msg1[0] == '<' || msg1[0] == '>' || command2[0] == '<' || command2[0] == '>') {
			control = 3;
		}

		if (check[0] == '&') {
			amper = 1;
		}
		else if (msg1[0] == '|') {
			control = 4;
		}
		else if (check[0] == '|') {
			control = 5;
		}

		//printf("%s %s %s %s %s  %d  %d\n",command1, msg1, check ,command2, msg2, control, amper);

		/*  fork  */
		if ((pid = fork()) < 0) {
			printf("fail to call fork\n");
			return -1;
		}
		else if (pid == 0) { /* child */

			if ((strncmp(command1, "exit", 4) == 0) || (strncmp(command1, "logout", 6) == 0)) {
				exit(1);
			}

			if (control == 1) { //1command
				execlp(command1, command1, NULL);
				printf("exec fail\n");
				_exit(127);
			}
			else if (control == 2) { //+msg
				execlp(command1, command1, msg1, NULL);
				printf("exec fail\n");
				_exit(127);
			}
			else if (control == 3) { //redirection
				if (msg1[0] == '<') {
					redirect1 = READ;
				}
				else if (msg1[0] == '>') {
					redirect1 = WRITE;
				}
				if (command2[0] == '<') {
					redirect2 = READ;
				}
				else if (command2[0] == '>') {
					redirect2 = WRITE;
				}

				if (redirect1 != 3 && redirect2 == 3) {
					if (redirect1 == READ) {
						if ((fd = open(check, O_RDONLY | O_CREAT, 0644)) < 0) {
							printf("file open input error\n");
							_exit(127);
						}
						//printf("input\n");
						//printf("redirect  %s  %s  %s\n",command1, msg1, check);
						dup2(fd, 0);
						close(fd);
					}
					else {

						if ((fd = open(check, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
							printf("file open output error\n");
							_exit(127);
						}
						//printf("output\n");
						//printf("redirect  %s  %s  %s\n",command1, msg1, check);

						dup2(fd, 1);
						close(fd);
					}
					execlp(command1, command1, NULL);
					printf("exec fail\n");
					_exit(127);
				}
				else if (redirect1 == 3 && redirect2 == 3) {
					printf("redirect error\n");
					_exit(127);
				}
				else {
					if (redirect1 == READ) {
						if ((fd = open(check, O_RDONLY | O_CREAT, 0644)) < 0) {
							printf("file open input error\n");
							_exit(127);
						}
						//printf("input\n");
						//printf("redirect  %s  %s  %s\n",command1, msg1, check);
						dup2(fd, 0);
						close(fd);

					}
					else {
						if ((fd = open(check, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
							printf("file open output error\n");
							_exit(127);
						}
						//printf("output\n");
						//printf("redirect  %s  %s  %s\n",command1, msg1, check);

						dup2(fd, 1);
						close(fd);
					}
					if (redirect2 == READ) {
						if ((fd = open(msg2, O_RDONLY | O_CREAT, 0644)) < 0) {
							printf("file open input error\n");
							_exit(127);
						}
						//printf("input\n");
						//printf("redirect  %s\n",msg2);
						dup2(fd, 0);
						close(fd);
					}
					else {
						if ((fd = open(msg2, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
							printf("file open output error\n");
							_exit(127);
						}
						//printf("output\n");
						//printf("redirect  %s\n",msg2);

						dup2(fd, 1);
						close(fd);
					}
					execlp(command1, command1, NULL);
					printf("exec fail\n");
					_exit(127);
				}
			}
			else if (control == 4 || control == 5) { //pipe

				if ((pipe(filedes)) < 0) {
					printf("fail to call pipe\n");
					return -1;
				}
				if ((pid2 = fork()) < 0) {
					printf("fail to call fork\n");
					return -1;
				}
				else if (pid2 == 0) { // child (write pipe)
									  //printf("pipe child\n");
					close(filedes[READ]);
					dup2(filedes[WRITE], 1);
					close(filedes[WRITE]);
					if (control == 5) {
						execlp(command1, command1, msg1, NULL);
						printf("pipe child error\n");
						_exit(127);
					}
					else {
						execlp(command1, command1, NULL);
						printf("pipe child error\n");
						_exit(127);
					}
				}
				else { // parent  (read pipe)
					   //printf("pipe parent\n");
					close(filedes[WRITE]);
					dup2(filedes[READ], 0);
					close(filedes[READ]);
					if (control == 5) {
						if (strncmp(msg2, "default", 7) == 0) {
							execlp(command2, command2, NULL);
							printf("pipe child error\n");
							_exit(127);
						}
						else {
							execlp(command2, command2, msg2, NULL);
							printf("pipe child error\n");
							_exit(127);
						}
					}
					else {
						execlp(check, check, NULL);
						printf("pipe parent error\n");
						_exit(127);
					}
				}


			}
			else {	//control = 0 error
				printf("control error\n");
				_exit(127);
			}
		}
		else {  /* parent */

			if ((strncmp(command1, "exit", 4) == 0) || (strncmp(command1, "logout", 6) == 0)) {
				wait(&status);
				return 0;
			}
			if (amper == 0) {
				if (waitpid(pid, &status, 0) == -1) {
					if (errno != EINTR)
						return -1;
				}
			}
			else {

			}
		}

	}
}

void intHandler() {
	printf("\nyou can't exit this way\n");
}
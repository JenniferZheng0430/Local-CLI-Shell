#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/mman.h>

#define LIGHT_BLUE "\033[1;34m" // imitate shell
#define LIGHT_GREEN "\033[1;32m"
#define NONE "\033[m"
#define clear() printf("\033[H\033[J")

#define normal 0 	// no pipe
#define pipe1 1		// a | b
#define pipe2 2    // a | b | c

void get_param(char* param);							// get paramaners
int find(char* param);									// find if the command exists
void cd(char *path);									// cd
void anal(char* param, int* num, char arr[100][256]);	// analyze command line
void exec(int param_num, char para[100][256]);			// execute command
void print_dir();                                       // imitate shell


int main(int argc, char* argv[])
{
    clear();
    while(1)
    {
        int param_num = 0;                 // e.g. ls -l = 2
		char *param = (char*) malloc (256);// store the command string user input
        char param_arr[100][256];          // formatted command string
        print_dir();                       // imitate shell
        printf(NONE"$ ");
        get_param(param);                  // get the input from the user, store in param
        if((strcmp("exit", param) == 0 )){ // type exit to quit the program
            break;
        }
        anal(param, &param_num, param_arr); // split the commend into a 2D array
        if(strcmp(param_arr[0],"cd") == 0)  // cd to the place // TODO
        {
            cd(param_arr[1]);
            continue;
        }
        exec(param_num, param_arr);         // execute the program
        free(param);
    }
    return 0;
}


void get_param(char* param)
{
    char ch;
    int i = 0;
    while(1){
        ch = getchar();
        if(ch == '\n') break;
        param[i++] = ch;
    }
    param[i] = '\0';
}


void anal(char* param, int* num, char arr[100][256])
{
    char* p = param;
    char* q = param;
    int number = 0;
    while(1)
    {
        if(p[0]=='\0')
        break;
        if(p[0]==' ')
        p++;
        else
        {
            q=p;
            number=0;                          // "l" "s" "\0"
            while(q[0]!='\0' && q[0]!=' '){    // "-" "l" "\0"
                q++;                           // "|" "\0"
                number++;                      // "d" "a" "t" "e" "\0"
            }
            strncpy(arr[*num],p,number+1);
            arr[*num][number]='\0';
            *num = *num+1;
            p=q;
        }
    }
}


void exec(int param_num, char para[100][256])
{
    pid_t pid1;                 //  child 1               child 1
    pid_t pid2;                 //     |                   /   |
    pid_t pid3;                 //  child 2          child 2   child 3
    int fd1[2];
    int fd2[2];
    char * arg[param_num+1];

    int stat_val1;
	int stat_val2;
    int i, j, x = 0, y = 0;             // util for split the arg123
    int flag = 0;                // how many "|" in total
    int how = 0;                 // 0:normal, 1:a|b, 2:a|b|c
    int bg = 0;
    int position1 = 0;
    int position2 = 0;

    for(i = 0; i < param_num; i++)
    {
        arg[i] = (char*)para[i];        // store whatever user input into arg1
    }
    arg[param_num]=NULL;

    // if(!find(arg1[0])) return;           // no such command

    for(i = 0; i < param_num; i++)		 // chechk how many | are there
    {
        if(strcmp(para[i], "|") == 0)
        {
            flag++;
        }
    }
    
    if(flag ==0){
        printf("--- oh, no pipe ---\n");
        how = normal;
    }

    if(flag == 1){
        printf("--- this is a pipe b ---\n");
        how = pipe1;
    }

    if(flag ==2){
        printf("--- this is a pipe b pipe c ---\n");
        how = pipe2;
    }

    if(how == pipe1) // process command, split it into arg1 and arg2
    {
        for(i = 0; i < param_num; i++)
        {
            if(strcmp(para[i], "|") == 0)
            {
                position1 = i;
                break;
            }
        }
        // char * arg1[j+1];
        // for(j=0; j<i; j++){
        //     arg1[j] = para[j];
        // }
        // arg1[j+1] = NULL;

        // char * arg2[param_num-i];
        // for(j = i+1; j< param_num; j++, x++){
        //     arg2[x] = para[j];
        // }
        // arg2[x+1] = NULL;
    }

    // if(how == pipe2) // process command, split it into arg1, arg2 and arg3
    // {
    //     for(i = 0; i < param_num; i++)
    //     {
    //         if(strcmp(para[i], "|") == 0)
    //         {
    //             arg1[i] = NULL;
    //             for(j = i+1; j < param_num; j++, x++)
    //             {
    //                 arg2[x] = para[j];
    //                 arg1[j] = 0;
    //             }
    //             break;
    //         }
    //     }
    //     for(i=0; i < x; i++)
    //     {
    //         if(strcmp(arg2[i], "|") == 0)
    //         {
    //             arg2[i] = NULL;
    //             for(j = i+1; j < x; j++, y++)
    //             {
    //                 arg3[y] = arg2[j];
    //             }
    //             arg3[y+1] = NULL;
    //             break;
    //         }
    //     }
    // printf("%s\n", arg1[0]);
    // printf("%s\n", arg2[0]);
    // printf("%s\n", arg3[0]);
    // }

    pid1 = fork();

    if(pid1<0){
        printf("child 1 creation failed\n");
        exit(EXIT_FAILURE);
    }
    else{
        switch (how){

            case 0:
                if(pid1 == 0)
                {
                    execvp(arg[0], arg);
                    exit(0);
                }
                break;

            case 1:
                if(pid1 == 0)
                {
                    char * arg1[position1+1];
                    for(j=0; j<position1; j++){
                        arg1[j] = arg[j];
                    }
                    arg1[j+1] = NULL;

                    char * arg2[param_num-position1];
                    for(j = position1+1; j< param_num; j++, x++){
                        arg2[x] = arg[j];
                    }
                    arg2[x+1] = NULL;

                    // char * arg1[3] = {"ls", "-l", NULL};
                    // char * arg2[3] = {"grep", ".c", NULL};

                    printf("child 1 created!\n");
                    pipe(fd1);
                    if (pipe(fd1) < 0){
                        printf("pipe 1 creation failed\n");
                        exit(EXIT_FAILURE);
                    }
                    pid2 = fork();
                    if(pid2<0){
                        printf("child 2 creation failed\n");
                        exit(EXIT_FAILURE);
                    }
                    else{
                        if(pid2 == 0) // child 2
                        {
                            // arg1 = {"pwd", NULL};
                            printf("child 2 created!");
                            close(1);
                            dup(fd1[1]);// write by redirecting standard output to pipe 1
                            close(fd1[1]);
                            close(fd1[0]);
                            printf("child 2 going to execute!\n");
                            execvp(arg1[0], arg1);
                            // execvp(exp[0], exp);
                            perror ("Execvp failed while child 2\n");
                            exit(EXIT_FAILURE);
                        }
                        else // child 1
                        {
                            wait(NULL);
                            // arg2 = {"wc", "-c", NULL};
                            close(0);
                            dup(fd1[0]); // reading redirected ouput of ls through pipe 1
                            close(fd1[1]);
                            close(fd1[0]);
                            printf("child 1 going to execute!\n");
                            execvp(arg2[0], arg2);
                            perror ("Execvp failed while child 2\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                break;

            // case 2:
            //     if(pid == 0)
            //     {
            //         pid2 = vfork();
            //         if(pid2 == 0)
            //         {
            //             fd = open("./trans", O_RDWR | O_CREAT | O_TRUNC, 0777);
            //             dup2(fd, 1);
            //             execvp(arg[0], arg); // ls [ls, -l, NULL]
            //             exit(0);
            //         }
            //         else
            //         {
            //             waitpid(pid2, &stat_val, 0);
            //             pid3 = vfork();
            //             if(pid3 == 0)
            //             {
            //                 fd = open("./trans", O_RDONLY);
            //                 dup2(fd, 0);
            //                 fd2 = open("./trans", O_RDWR | O_CREAT | O_TRUNC, 0777);
            //                 dup2(fd2, 1);
            //                 execvp(arg1[0], arg1); // ls [ls, -l, NULL]
            //                 exit(0);
            //             }
            //             else{
            //                 waitpid(pid3, &stat_val, 0);
            //                 fd2 = open("./trans", O_RDONLY);
            //                 dup2(fd, 0);
            //                 execvp(arg2[0], arg2);
            //                 exit(0);
            //             }
            //         }
            //     }
            //     break;

            default:
                break;
        }
    }
	if(bg) return;
    waitpid(pid1, &stat_val2, 0);
}


int find(char* param)
{
    DIR* dir;
    struct dirent* ptr;
    int ice = 0;
    dir = opendir("/bin");
    readdir(dir);
    while(ptr = readdir(dir))
    {
        if(strcmp(param, ptr->d_name) == 0)
            return 1;
    }
    closedir(dir);
    printf("Command '%s' not found\n", param);
    return 0;
}


void cd(char *path)
{
    if(chdir(path) < 0)
        perror("chdir");
}


void print_dir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf(LIGHT_GREEN"Dir:");
    printf(LIGHT_BLUE"%s", cwd);
}    fd = open("./trans", O_RDONLY);
            //                 dup2(fd, 0);
            //                 fd2 = open("./trans", O_RDWR | O_CREAT | O_TRUNC, 0777);
            //                 dup2(fd2, 1);
            //                 execvp(arg1[0], arg1); // ls [ls, -l, NULL]
            //                 exit(0);
            //             }
            //             else{
            //                 waitpid(pid3, &stat_val, 0);
            //                 fd2 = open("./trans", O_RDONLY);
            //                 dup2(fd, 0);
            //                 execvp(arg2[0], arg2);
            //                 exit(0);
            //             }
            //         }
            //     }
            //     break;

            default:
                break;
        }
    }
	if(bg) return;
    waitpid(pid1, &stat_val2, 0);
}


int find(char* param)
{
    DIR* dir;
    struct dirent* ptr;
    int ice = 0;
    dir = opendir("/bin");
    readdir(dir);
    while(ptr = readdir(dir))
    {
        if(strcmp(param, ptr->d_name) == 0)
            return 1;
    }
    closedir(dir);
    printf("Command '%s' not found\n", param);
    return 0;
}


void cd(char *path)
{
    if(chdir(path) < 0)
        perror("chdir");
}


void print_dir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf(LIGHT_GREEN"Dir:");
    printf(LIGHT_BLUE"%s", cwd);
}
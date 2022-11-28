#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<ctype.h>
#include <unistd.h>     //ls,pwd,cd
#include <fcntl.h>      //ls
#include <dirent.h>     //ls
#include <sys/stat.h>   //ls,mkdir,rmdir
#include <sys/types.h>  //rmdir

#define BUFSIZE 64
#define DELIM " \t\r\n\a"

// Color codes
#define RESET_COLOR "\e[m"
#define COLOR_GREEN "\e[0;32m"
#define COLOR_YELLOW "\e[0;33m"
#define COLOR_CYAN "\e[0;36m"

static int filter(const struct dirent *unused);

// Procedure prototypes
char **separate_line(char *line);
int run(char **arg);
int no_cmds();
void show_man(char* cmd);

int cmd_cat(char **arg);
int cmd_cd(char **arg);
int cmd_clear(char **arg);
int cmd_cp(char **arg);
int cmd_echo(char **arg);
int cmd_exit(char **arg);
int cmd_help(char **arg);
int cmd_ls(char **arg);
int cmd_man(char **arg);
int cmd_mkdir(char **arg);
int cmd_pwd(char **arg);
int cmd_rmdir(char **arg);
int cmd_rm(char **arg);
int cmd_wc(char **arg);

int (*realizar_cmd[]) (char **) = {
    &cmd_cat,
    &cmd_cd,
    &cmd_clear,
    &cmd_cp,
    &cmd_echo,
    &cmd_exit,
    &cmd_help,
    &cmd_ls,
    &cmd_man,
    &cmd_mkdir,
    &cmd_pwd,
    &cmd_rmdir,
    &cmd_rm,
    &cmd_wc
};

char *cmds[] = {
    "cat",
    "cd",
    "clear",
    "cp",
    "echo",
    "exit",
    "help",
    "ls",
    "man",
    "mkdir",
    "pwd",
    "rmdir",
    "rm",
    "wc"
};


int main (int argc, char ** argv)
{
    int exitStatus = 0;
    char *prompt = COLOR_GREEN "Shell> " RESET_COLOR;
    char cmd[1024];

    while (!exitStatus)
    {
        int ret;
        ret = write(1, prompt, strlen(prompt));
        if (!ret)
        {
            exitStatus = 1;
            break;
        }

        char *cursor = cmd;
        int cont = 0;
        char last_char = 1;
        for(ret = 1; ret && (++cont < (1024-1)) && (last_char != '\n');cursor++)
        {
            ret = read(0, cursor, 1);
            last_char = *cursor;
        }
        *cursor = '\0';

        if (!ret)
        {
            exitStatus = 1;
            break;
        }

        if(cmd[0]!='\n')
        {
            char **args = separate_line(cmd);
            exitStatus = run(args);
            free(args);
        }

    }

    return 0;
}


char **separate_line(char *line)
{
    int bufsize = BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "Error in memory allocation\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "Error in memory allocation\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int run(char **arg){
    int i;
    if(arg[0]==NULL)
        return 1;

    for(i=0; i<no_cmds();i++)
    {
        if(strcmp(arg[0],cmds[i])==0)
            return (*realizar_cmd[i])(arg);

    }

    printf("Error: cmd not found.\n");
    return 0;

}

int no_cmds()
{
    return sizeof(cmds) / sizeof(char *);
}

int cmd_cat(char **arg)
{
    int fd,i;
    char buf[1];
    if (arg[1] == NULL)
        printf("Error: Insufficient number of arguments\n");
    else
    {
        fd=open(arg[1],O_RDONLY,0777);
        if(fd<0)
            printf("File open error");
        else
        {
            while((i=read(fd,buf,1))>0)
                printf("%c",buf[0]);

            close(fd);
        }
    }
    return 0;
}

int cmd_cd(char **arg)
{
    if (arg[1] == NULL)
        printf("Error: Insufficient number of arguments\n");
    else
    {
        if (chdir(arg[1]) != 0)
            printf("Error: 'cd' cmd failed\n");
    }
    return 0;
}

int cmd_clear(char **arg)
{
    if(arg[1] != NULL)
    {
        printf("Error: The 'clear' cmd does not accept any arguments.\n");
        return 0;
    }

     printf("\033[2J\033[1;1H");
     fflush(stdout);

     return 0;
}

int cmd_cp(char **arg)
{
    FILE *src,*dest;
    char c;

    if(arg[2]== NULL)
    {
        printf("Error: Insufficient number of arguments\n");
        return 0;
    }
    src=fopen(arg[1],"r");
    dest=fopen(arg[2],"w");
    if(src==NULL || dest==NULL)
    {
        printf("Error: Error in the source or destination file\n");
        return 0;
    }
    while((c=fgetc(src))!=EOF)
    {
        fputc(c,dest);
    }
    fclose(src);
    fclose(dest);

    return 0;
}

int cmd_echo(char **arg)
{
    int i;
    int k=0;
    while(arg[++k])
    {
        for(i=0;i<strlen(arg[k]);i++)
            printf("%c",arg[k][i]);

        printf(" ");
    }
    printf("\n");

    return 0;
}

int cmd_exit(char **arg)
{
    if (arg[1] != NULL)
    {
        printf("Error: Usage: exit The 'exit' cmd does not accept any arguments\n");
        return 0;
    }
    return 1;
}

int cmd_help(char **arg)
{
    int i;

    if (arg[1] != NULL)
    {
        printf("Error: The 'help' cmd does not accept any arguments\n");
        return 0;
    }

    printf("\n");
    printf("Shell\n");
    printf("List of available cmds:\n");

    for (i = 0; i < no_cmds(); i++)
        printf("  %s\n", cmds[i]);

    return 0;
}

int cmd_ls(char** arg)
{
    struct dirent **contents;
    int no_contents;

    if(arg[1]==NULL)
    {
        if((no_contents = scandir("./", &contents, filter, alphasort))<0)
            printf("Error: 'ls' cmd failed \n");
    }
    else if(arg[2]==NULL)
    {
        if((no_contents = scandir(arg[1], &contents, filter, alphasort))<0)
            printf("Error: the cmd is ls + one argument\n");
    }

    int i;
    for(i=0; i<no_contents; i++)
    {
        char* name = contents[i]->d_name;
        if (strcmp(name,".")!=0 && strcmp(name,"..")!=0)
        {
            if(!access(name,X_OK))
            {
                int fd = -1;
                struct stat st;

                fd = open(name, O_RDONLY, 0);
                if(-1 == fd)
                {
                    printf("\n Error: Failed to open file/directory\n");
                    return -1;
                }

                fstat(fd, &st);
                if(S_ISDIR(st.st_mode))
                    printf(COLOR_CYAN"%s     "RESET_COLOR,name);
                else
                    //printf(COLOR_YELLOW"%s     "RESET_COLOR,name);
                    printf("%s     ",name);
                close(fd);
            }
            else
                printf("%s     ",name);
        }
    }
    printf("\n");
    return 0;

}

static int filter(const struct dirent *unused)
{
    return 1;
}

int cmd_man(char **arg)
{

    if (arg[1]==NULL)
    {
      printf("You can search for information about the cmds by typing: \n");
      char* str = "  man [cmd_name]\n";
      printf(COLOR_YELLOW"%s  "RESET_COLOR,str);
      str= "Example: man ls\n";
      printf(COLOR_CYAN"%s  "RESET_COLOR,str);
      printf("\n");

      return 0;
    }
    else if (arg[2]!=NULL)
    {
        printf("Error: The 'man' cmd accepts at most one argument.\n");
        return 0;
    }
    else
    {
        char* name_cmd = arg[1];
        show_man(name_cmd);
    }

    return 0;
}

void show_man(char* cmd)
{
    if (strcmp(cmd,"cd")==0)
        printf("'cd' cmd: \n Description: Change directory. \n Usage: cd [/path/to/change]\n Limitations: Works only if given an argument\n");
    else if (strcmp(cmd,"clear")==0)
        printf("'clear' cmd: \n Description: Clears the screen and places the prompt at the beginning of it. \n Usage: clear\n Limitations: Only partially clears the screen. (It is a partial version of clear)\n");
    else if (strcmp(cmd,"cp")==0)
        printf("cmd 'cp': \n Description: Copies files to the indicated directory. \n Usage: cp [file_name] [/target/copy_file_name] )\n");
    else if (strcmp(cmd,"echo")==0)
        printf("'echo' cmd: \n Description: Display a line of text/string in standard output. \n Usage: echo [string] \n Limitations: -This version, if it receives an argument between quotes, still prints the quotes -The $\n functionality is not considered");
    else if (strcmp(cmd,"exit")==0)
        printf("cmd 'exit': \n Description: Exit the MiniShell. \n Usage: exit\n");
    else if (strcmp(cmd,"help")==0)
        printf("'help' cmd: \n Description: Show help. \n Usage: help \n");
    else if (strcmp(cmd,"ls")==0)
        printf("cmd 'ls': \n Description: List the contents of a directory. \n Usage: ls \n Limitations: Output format (grouping, alphabetical order, etc) \n");
    else if (strcmp(cmd,"man")==0)
        printf("cmd 'man': \n Description: Show a brief manual of each cmd. \n Usage: man [cmd_name]\n");
    else if (strcmp(cmd,"mkdir")==0)
        printf("cmd 'mkdir': \n Description: Create directory if it doesn't exist. \n Usage: mkdir [directory_name] \n");
    else if (strcmp(cmd,"pwd")==0)
        printf("'pwd' cmd: \n Description: Print the current working directory. \n Usage: pwd \n");
    else if (strcmp(cmd,"rmdir")==0)
        printf("cmd 'rmdir': \n Description: Delete a directory and the files it contains. \n Usage: rmdir [directory_name] \n");
    else if (strcmp(cmd,"rm")==0)
        printf("cmd 'rm': \n Description: Delete a given file. \n Usage: rm [filename] \n");
    else
      printf("Error- The cmd entered does not exist\n");

}

int cmd_mkdir(char **arg)
{
    int check = mkdir(arg[1],0777);
        if (!check)
                printf("Directory created\n");
        else{
                printf("Unable to create directory\n");
                //exit(1);
        }

        return 0;

}

int cmd_pwd(char **arg)
{

    if(arg[1] != NULL)
    {
        printf("Error:The 'pwd' cmd does not accept any arguments.\n");
        return 0;
    }
    char cwd[1024];
    chdir("/path/to/change/directory/to");
    getcwd(cwd, sizeof(cwd));
    printf("Current working directory: %s\n", cwd);
    return 0;
}

int cmd_rmdir(char **arg)
{

    char dirName[16];
    int ret = 0;

    ret = rmdir(arg[1]);

    if (ret == 0)
        printf("Given empty directory removed successfully\n");
    else
        printf("Unable to remove directory %s\n", dirName);

    return 0;

}

int cmd_rm(char **arg)
{

    if (arg[1]==NULL)
        printf("Error: 'rm' cmd must have one argument\n");
    else if (arg[2]!=NULL)
        printf("Error: The 'rm' cmd cannot have more than one argument.\n");

    if(remove(arg[1]) == -1)
        perror("Error: Can't delete file");
    return 0;
}

int cmd_wc(char **arg)
{
    if (arg[1]==NULL)
        printf("Error: 'wc' cmd must have one argument\n");
    else
    {
        FILE *fp;
        char buffer[4096];
        int total_words = 0;
        int total_lines = 0;
        int total_bytes = 0;

            int res = 0;
            int word_count = 0;
            int line_count = 0;
            fp = fopen(arg[1], "r");

            if (fp != NULL) {
                while (!feof(fp)) {
                    res = fread(buffer, 1, (sizeof buffer)-1, fp);
                    for (int i=0; i < res; i++) {
                        if ((isspace(buffer[i]) && !isspace(buffer[i+1])) ||
                                (!isspace(buffer[i]) && (i+1) == res)) {
                            ++word_count;
                        }
                        if (buffer[i]== '\n') {
                            ++line_count;
                        }
                    }
                }
                fclose(fp);

                printf("%7d %7d %7d %s \n", line_count, word_count, res, arg[1]);

                total_words += word_count;
                total_lines += line_count;
                total_bytes += res;
        }
        else
            printf("wc: %s: open: No such file or directory \n", arg[1]);

    return 0;

    }
}

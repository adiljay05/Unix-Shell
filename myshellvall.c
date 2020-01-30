#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "BCSF16M516:- "

int execute(char* arglist[]);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);

int argnum = 0; //slots used
int val=0;
pid_t pid1;

struct jobsAtBackground
{
  	int count;
 	pid_t pid;
  	char command[100];
} arrayOfBackgroundJobs[10];
int indexofarray=0;

int inputRedirection(char *arglist[]){
    int i=0;
    while(1){
        if(arglist[i]==NULL)
            break;

        if(strcmp(arglist[i],"<")==0){
            int fd0= open(arglist[i+1], O_RDONLY);
            if(fd0 < 0){
                perror("can't open the file");
                exit(0);
            }
            dup2(fd0,0);
            close(fd0);

            return i;
        }
        i++;
    }
    return -1;
}

void ouputRedirection(char* arglist[], int tI){
    int i=0;
    while(1){
        if(arglist[i]==NULL)
            break;

        if(strcmp(arglist[i],">")==0){
            int fd1= open(arglist[i+1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if(fd1 < 0){
                perror("can't open the file");
                exit(0);
            }
            dup2(fd1,1);
            close(fd1);

            arglist[i]=NULL;
            arglist[i+1]=NULL;
                    
            break;
        }
        i++;

    }
    if(tI!=-1){
        arglist[tI]=NULL;
        arglist[tI+1]=NULL;
    }

}

void printArray()
{
	if (indexofarray==0)
	{
		printf("array is empty\n");
	}
	for(int i=0;i<10;i++)
	{ 
		if(arrayOfBackgroundJobs[i].count!=-1)
		{
			printf("[%d]    Running    ",arrayOfBackgroundJobs[i].count );
    		printf("%s\n",arrayOfBackgroundJobs[i].command );
		}
	}
}


void saveToFile(char *cmdline)
{
    rename("history.txt","history1.txt");
    FILE *fp=fopen("history.txt","w");
    char ch[1000]={'\0'};
    int historyCheck=0;
    if(fp == NULL)
    {
        printf("Error!\n");
        exit(1);
    }
    fprintf(fp,"%s\n", cmdline);
    FILE *fp1=fopen("history1.txt","r");
    if (fp1 == NULL)
    {
        perror("Error!\n");
        exit(1);
    }
    while(( fgets(ch,sizeof(ch),fp1)) != NULL && historyCheck<10)
    {
        fprintf(fp,"%s", ch);  
        historyCheck++;
    }
    fclose(fp);
    fclose(fp1);
    remove("history1.txt");

}

int scanSemiColon(char *cmdline)
{
	int i=0;
	while(cmdline[i]!='\0')
	{
		if(cmdline[i]==';')
			return i;
		i++;
	}
	return -1;
}

char ch[10][1000]={'\0'};
void loadHistory()
{
  int z=0;
  FILE *fp1=fopen("history.txt","r");
    if (fp1 == NULL)
    {
        perror("Error!\n");
        exit(1);
    }
    while(( fgets(ch[z],sizeof(ch[z]),fp1)) != NULL)
    {   
      //printf("%s",ch[z] );
        z++;
    }
    //strcpy(cmdline,ch);
    fclose(fp1);

}
int scanPipe(char *cmdline)
{
	int i=0;
	while(cmdline[i]!='\0')
	{
		if(cmdline[i]=='|')
			return i;
		i++;
	}
	return -1;
}
void extract(char * cmdline,int pos)
{
  strcpy(cmdline,ch[pos]);
}
void initiallize()
{
	for (int i=0;i<10; i++)
	{
		arrayOfBackgroundJobs[i].count=-1;
	}
}
int main(int argc,char **argv){
    if(argc==3&&strcmp(argv[1],"-f")==0)
    {
        int file=open(argv[2],O_RDONLY);
        if(file<0)
        {
            printf("could not open the file\n");
            return 0;
        }
        dup2(file,0);
        close(file);
    }
	initiallize();
    char *cmdline;
    char** arglist;
    system("clear");
    int index3=-1,index2=-1;
    char* prompt = PROMPT;
    int status;
    int backup_input=dup(0);
    int backup_output=dup(1);
    while((cmdline = read_cmd(prompt,stdin)) != NULL){
    	index2=scanPipe(cmdline);
    	if(index2!=-1)
    	{
    		system(cmdline);
    		continue;
    	}
    	index3=scanSemiColon(cmdline);
    	if(index3!=-1)
    	{
    		system(cmdline);
    		continue;
    	}
        argnum=0;
        if(strcmp(cmdline,"!-1")==0)
        {
            loadHistory();
            extract(cmdline,0);
            //printf("cmdline : %s",cmdline);
            int o=0;
            while(cmdline[o]!='\n')
            {
              o++;
            }
            cmdline[o]='\0';
            printf("%s\n",cmdline);
            //continue;
        }
        else if(cmdline[0]=='!')
        {
            loadHistory();
            int position1=(int)cmdline[1]-48;
            //printf("position %d\n",position1 );
            extract(cmdline,position1-1);
            //printf("cmdline : %s",cmdline);
            int o=0;
            while(cmdline[o]!='\n')
            {
              o++;
            }
            cmdline[o]='\0';
            printf("%s\n",cmdline);

        }
      //printf("cmdline : %s",cmdline);
        if((arglist = tokenize(cmdline)) != NULL){
            if(strcmp(arglist[0],"exit")==0){
            	//system("clear");
            	printf("Exiting \n");
                exit(0);
            }
            if(strcmp(arglist[0],"jobs")==0)
            {
              	printArray();
              	argnum=0;
              	continue;
            }
            if(strcmp(arglist[0],"kill")==0)
            {
            	pid_t p1;
            	for(int i=0;i<10;i++)
            	{
            		//printf("%d\n",arrayOfBackgroundJobs[i].count);
            		if(arrayOfBackgroundJobs[i].count==(int)(arglist[1][0]-48))
            		{
            			p1=arrayOfBackgroundJobs[i].pid;
            			arrayOfBackgroundJobs[i].count=-1;
            			//printf("In%d\n",(int)p1 );
            			break;
            		}
            	}
            	printf("%d\n",p1 );
            	kill((pid_t)p1,9);
            	argnum=0;
            	continue;
            }
            // if(strcmp(arglist[0],"cd")==0)
            // {
            // 	printf("%s\n",arglist[1] );
            // 	chdir(arglist[1]);
            // 	continue;
            // }
            if(strcmp(arglist[argnum-1],"&")==0)
            {
              	//printf("& found in command\n");
              	arglist[argnum-1]='\0';
              	val=1;
            }
            int i=inputRedirection(arglist);
            ouputRedirection(arglist,i);

            execute(arglist);
                
            dup2(backup_input,0);
            dup2(backup_output,1);
            
            saveToFile(cmdline);
        if(val==1)
          {
            //printf("value is 1 now\n");
            for(int i=0;i<10;i++)
            {
            	if(arrayOfBackgroundJobs[i].count==-1)
            	{
            		arrayOfBackgroundJobs[i].pid=pid1;
		            //printf("%d\n", pid1);
		            printf("[%d]  %d\n",i+1,pid1 );
		            arrayOfBackgroundJobs[i].count=i+1;
		            strcpy(arrayOfBackgroundJobs[i].command,*arglist);
		            indexofarray++;
		            //printf("indexofarray incremented\n");
		            val=0;
		            break;
            	}
            }
        //printf("%s\n",*arglist );
            //printf("value is zero now\n");
          }
            for(int j=0; j < MAXARGS+1; j++)
                free(arglist[j]);
            free(arglist);
            free(cmdline);
     }
     argnum=0;
  }//end of while loop
   printf("\n");
   return 0;
}

void func(int signum) 
{ 
    wait(NULL); 
} 

int execute(char* arglist[]){
   int status;
   
   int cpid = fork();
   //printf("%d\n",cpid );
   pid1=cpid;
   switch(cpid){
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            execvp(arglist[0], arglist);
            
            perror("Command not found...");
            exit(1);
        default:
			if(val!=1)
				waitpid(cpid, &status, 0);
			else
			{
				signal(SIGCHLD, func);
			}

         //printf("child exited with status %d \n", status >> 8);
        return 0;
   }
}
char** tokenize(char* cmdline){
//allocate memory
   char** arglist = (char**)malloc(sizeof(char*)* (MAXARGS+1));
   for(int j=0; j < MAXARGS+1; j++){
     arglist[j] = (char*)malloc(sizeof(char)* ARGLEN);
      bzero(arglist[j],ARGLEN);
    }
   if(cmdline[0] == '\0')//if user has entered nothing and pressed enter key
      return NULL;
   
   char*cp = cmdline; // pos in string
   char*start;
   int len;
   while(*cp != '\0'){
      while(*cp == ' ' || *cp == '\t') //skip leading spaces
          cp++;
      start = cp; //start of the word
      len = 1;
      //find the end of the word
      while(*++cp != '\0' && !(*cp ==' ' || *cp == '\t'))
         len++;
      strncpy(arglist[argnum], start, len);
      arglist[argnum][len] = '\0';
      argnum++;

   }
   arglist[argnum] = NULL;
   return arglist;
}      

char* read_cmd(char* prompt, FILE* fp){
   printf("\033[1;31m%s\033[0m", prompt);
  int c; //input character
   int pos = 0; //position of character in cmdline
   char* cmdline = (char*) malloc(sizeof(char)*MAX_LEN);
   while((c = getc(fp)) != EOF){
       if(c == '\n')
    break;
       cmdline[pos++] = c;
   }
//these two lines are added, in case user press ctrl+d to exit the shell
   if(c == EOF && pos == 0) 
      return NULL;
   cmdline[pos] = '\0';
   return cmdline;
}

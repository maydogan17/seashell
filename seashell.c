#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>            //termios, TCSANOW, ECHO, ICANON
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <mcheck.h>
const char * sysname = "seashell";

enum return_codes {
	SUCCESS = 0,
	EXIT = 1,
	UNKNOWN = 2,
};
struct command_t {
	char *name;
	bool background;
	bool auto_complete;
	int arg_count;
	char **args;
	char *redirects[3]; // in/out redirection
	struct command_t *next; // for piping
};

typedef unsigned char BYTE;

//-----------------------------PART-1-------------------------------------------
char* execHelper(char* cmd){
  char locations[9][20] = {"usr/local/sbin/", "/usr/local/bin/", "/usr/sbin/", "/usr/bin/", "/sbin/", "/bin/", "/usr/games/", "/usr/local/games/", "/snap/bin/"};

  for(int i = 0; i < 9; i++){
    char* dum = (char*)malloc(strlen(locations[i]) + strlen(cmd) + 1);
    strcpy(dum, locations[i]);
    strcat(dum, cmd);

    if( access( dum, F_OK ) == 0 ) {
      return dum;
    }
    free(dum);
  }
  return "";
}
//-----------------------------PART-1-END---------------------------------------


//-----------------------------PART2--------------------------------------------
struct key_value
{
   char* key;
   char* value;
};

struct key_value keymap[100];

int hasVal(char* key){
  for(int i = 0; i < 100;i++){
    if(keymap[i].key != NULL){
      if(strcmp(keymap[i].key , key) == 0){
        return i;
      }
    }
  }
    return -1;
}

char* giveVal(char* key){
  if(hasVal(key) > -1){
    char* result = keymap[hasVal(key)].value;
    return result;
  }
  return "EMPTY";
}

void putMap(char* key, char* value){
  if(hasVal(key) > -1){
    int pos = hasVal(key);
    struct key_value kv;
    kv.key = key;
    kv.value = value;
    keymap[pos] = kv;
  }
  else{
    for(int i = 0; i < 100;i++){

      if(keymap[i].key == NULL){
        struct key_value kv;
        kv.key = key;
        kv.value = value;

        keymap[i] = kv;
        break;
      }
    }
  }
}

void deleteAll(){
  struct key_value kv;
  kv.key = NULL;
  kv.value = NULL;
  for(int i = 0; i < 100; i++){
    keymap[i] = kv;
  }
}


void mapPrinter(){
  for(int i = 0; i < 100; i++){
    if(keymap[i].key != NULL){
      printf("Name: %s    Directory: %s \n", keymap[i].key, keymap[i].value);
    }
  }
}

void delete(char* key){
  if(hasVal(key)>-1){
    struct key_value kv;
    kv.key = NULL;
    kv.value = NULL;
    keymap[hasVal(key)] = kv;
  }
}

void mapSaver(){
  FILE *fptr;
  fptr = fopen("list.txt", "w");
  if (fptr == NULL) {
      printf("Error! opening file");
      exit(1);
  }
  for(int i = 0; i < 100; i++){
    if(keymap[i].key != NULL){
      //printf("Name: %s    Directory: %s \n", keymap[i].key, keymap[i].value);
      fprintf (fptr, "%s ",keymap[i].key);
      fprintf (fptr, "%s\n",keymap[i].value);
    }
  }

  fclose(fptr);

}

void mapLoder(){
  char d[100];

  FILE *fptr;
  fptr = fopen("list.txt", "r");
  if (fptr == NULL) {
      printf("Error! opening file");
      exit(1);
  }

  while (fgets(d,100,fptr)!= NULL){

    char* seper = " ";
    char* line= (char*)malloc(strlen(d)+1);
    strcpy(line,d);
    char *sep2 = strtok(line, seper); // it seperates each word
    char* keyL= (char*)malloc(strlen(sep2)+1);
    strcpy(keyL,sep2);
    sep2 = strtok(NULL, seper);
    char* valueL= (char*)malloc(strlen(sep2)+1);
    strncpy(valueL,sep2,(strlen(sep2)-1));
    char sp[]="\0";
    strcat(valueL,sp);
    putMap(keyL, valueL);


  }
  fclose(fptr);
}


int shortdir(struct command_t *command){
		if(strcmp(command->args[1], "set") ==0){
			char* key1 =(char*)malloc(strlen(command->args[2]) + 1);
			strcpy(key1, command->args[2]);

			char* val1 = getcwd((char * ) NULL, 0);

			putMap(key1, val1);


			printf("Key: %s  Directory: %s  Position: %d\n",keymap[hasVal(key1)].key,  keymap[hasVal(key1)].value, hasVal(key1));
			//printf("Key: %s  Val: %s\n",keymap[0].key,  keymap[0].value);

			return SUCCESS;
		}
		else if(strcmp(command->args[1], "jump") ==0){
			char* key1 =(char*)malloc(strlen(command->args[2]) + 1);
			strcpy(key1, command->args[2]);

			char* value = (char*)malloc(strlen(giveVal(key1)) + 1);
			strcpy(value, giveVal(key1));


			if(strcmp(value, "EMPTY") != 0){
				chdir(value);

				printf("Current Directory: %s\n", getcwd((char * ) NULL, 0));

				free(key1);
				free(value);
				return SUCCESS;
			}

			return EXIT;

		}
		else if(strcmp(command->args[1], "del") ==0){
			char* key1 = command->args[2];
			delete(key1);

			return SUCCESS;
		}
		else if(strcmp(command->args[1], "clear") ==0){
			deleteAll();
			printf("Associations cleared.\n");
			return SUCCESS;
		}
		else if(strcmp(command->args[1], "list") ==0){
			mapPrinter();

			return SUCCESS;
		}
		else if(strcmp(command->args[1], "save") ==0){
			mapSaver();
			printf("%s\n", "Associations are saved locally.");

			return SUCCESS;
		}
		else if(strcmp(command->args[1], "load") ==0){
			mapLoder();
			printf("%s\n", "Locally stored associations are loaded.");

			return SUCCESS;
		}
		else{
			return EXIT;
		}
	}

//-----------------------------PART2-END---------------------------------------

//-----------------------------PART3--------------------------------------------
char* toLower(char* s) {
  for(char *p=s; *p; p++){
    *p=tolower(*p);
  }
  return s;
}


int checker(char* line, char* wr){
  char* seper = " ";
  char liner[strlen(line)+1];
  strcpy(liner,line);
  char *sep2 = strtok(liner, seper); // it seperates each word

  while(sep2 != NULL){

    if(strcmp(toLower(sep2),toLower(wr))==0){
      return 1;
    }
    sep2 = strtok(NULL, seper);
  }
  return 0;
}

void highlight(char* word, char* clr, char* path){

/*
char* full_path = (char*)malloc(strlen(backup_path) + strlen(s2) + 1);
*/
  char* currentPath = getcwd((char * ) NULL, 0);
  char* filepath = (char*)malloc(strlen(currentPath) + strlen(path) + 2);
  filepath = strcat(currentPath, "/");
  filepath = strcat(filepath, path);



  char c[1000];
  char seperator[] = " ";
  FILE *fptr;
  fptr = fopen(filepath, "r");
  if (fptr == NULL) {
      printf("Error! opening file");
      exit(1);
  }
  free(filepath);
  char* color = clr;


  char* dummy = (char*)malloc(strlen(word) + 1);
  strcpy(dummy, word);
  char* compare = (char*)malloc(strlen(word) + 1);
  strcpy(compare, dummy);
  char* compare2 = strcat(dummy, "\n");

  while (fgets(c,1000,fptr)!= NULL){
    char try[strlen(c)+1];
    strcpy(try,c);

    int valid = checker(try,compare);

    char *sep = strtok(c, seperator);
    if(valid==1){
      while(sep != NULL){

        if(strcmp(toLower(sep),toLower(compare))==0 || strcmp(toLower(sep),toLower(compare2))==0){


          if(strcmp(color, "r") == 0){
            printf("\033[0;31m");
            printf("%s ", sep);
            printf("\033[0m");
          }
          if(strcmp(color, "g") == 0){
            printf("\033[0;32m");
            printf("%s ", sep);
            printf("\033[0m");
          }
          if(strcmp(color, "b") == 0){
            printf("\033[0;34m");
            printf("%s ", sep);
            printf("\033[0m");
          }

        }
        else{
          printf("%s ", sep);
        }

        sep = strtok(NULL, seperator);
      }
    }

  }
  free(dummy);
  free(compare);
  fclose(fptr);


}
//-----------------------------PART3-END---------------------------------------

//-----------------------------PART4-START-------------------------------------

void goodmorning(char* time, char* path1){

    char dummy[strlen(time)+1];
    strcpy(dummy,time);

    char seperator[] = ".";
    char* sep = strtok(dummy, seperator);

    char* hour = (char*)malloc(strlen(sep) + 1);
    strcpy(hour,sep);

    char path[strlen(path1)+1];
    strcpy(path,path1);

    sep = strtok(NULL, seperator);
    char* min = (char*)malloc(strlen(sep) + 1);
    strcpy(min,sep);

    char c[]= "XDG_RUNTIME_DIR=/run/user/$(id -u) /usr/bin/rhythmbox-client --play-uri";
    char *full = (char*)malloc(strlen(hour)+strlen(min)+8+strlen(c)+strlen(path)+2);

  	strcat(full,min);
    char space[]= " ";
    char star[]= " * * * ";
    strcat(full,space);
		strcat(full,hour);
    strcat(full,star);
    strcat(full,c);
    strcat(full,space);
    strcat(full,path);
		char newline[]= "\n";
		strcat(full,newline);


    FILE *fptr;
    fptr = fopen("crontab.txt", "w");
    if (fptr == NULL) {
        printf("Error! opening file");
        exit(1);
    }

    fprintf (fptr, "%s",full);
    fclose(fptr);

    free(hour);
    free(min);


}


//-----------------------------PART4-END---------------------------------------


//-----------------------------PART-5-------------------------------------------

void kdiffByte(char* txt1, char* txt2){
  char c1[1000];
  char c2[1000];

  FILE *fptr1;
  FILE *fptr2;
  fptr1 = fopen(txt1, "r");
  fptr2 = fopen(txt2, "r");
  if (fptr1 == NULL) {
      printf("Error! opening file");
      exit(1);
  }
  if (fptr2 == NULL) {
      printf("Error! opening file");
      exit(1);
  }

  int sum1 = 0;
  int sum2 = 0;

  while(fgets(c1,sizeof(c1),fptr1) != NULL){

    int loop;
    int i;
    int sum = 0;
    loop = 0;
    i = 0;

    int len = strlen(c1);
    BYTE arr[len];

    while(c1[loop] != '\0')
    {
        arr[i] = c1[loop];
        sum = sum + arr[i];
        i++;
        loop++;
    }

    sum1 = sum1 + sum;
  }

  while(fgets(c2,sizeof(c2),fptr2) != NULL){

    int loop;
    int i;
    int sum = 0;
    loop = 0;
    i = 0;

    int len = strlen(c2);
    BYTE arr[len];

    while(c2[loop] != '\0')
    {
        arr[i] = c2[loop];
        sum = sum + arr[i];
        i++;
        loop++;
    }

    sum2 = sum2 + sum;
  }

  int res = abs(sum1 - sum2);

  fclose(fptr1);
  fclose(fptr2);

  if(sum1 == sum2){
    printf("%s\n", "The two files are identical");
  }
  else{
    printf("The two files are different in %d bytes\n", res);
  }



}


void kdiff(char* txt1, char* txt2){
  char c;
  char c3;

  char c1[1000];
  char c2[1000];
  char seperator[] = " ";
  FILE *fptr1;
  FILE *fptr2;
  fptr1 = fopen(txt1, "r");
  fptr2 = fopen(txt2, "r");
  if (fptr1 == NULL) {
      printf("Error! opening file");
      exit(1);
  }
  if (fptr2 == NULL) {
      printf("Error! opening file");
      exit(1);
  }

  int lineCount1 = 1;
  int lineCount2 = 1;

  int line1=1;
  int line2=1;
  int difCount=0;

  for (c = getc(fptr1); c != EOF; c = getc(fptr1)){
    if (c == '\n'){
      lineCount1 = lineCount1 + 1;
    }
  }
  for (c3 = getc(fptr2); c3 != EOF; c3 = getc(fptr2)){
    if (c3 == '\n'){
      lineCount2 = lineCount2 + 1;
    }
  }

  fclose(fptr1);
  fclose(fptr2);

  fptr1 = fopen(txt1, "r");
  fptr2 = fopen(txt2, "r");
  if (fptr1 == NULL) {
      printf("Error! opening file");
      exit(1);
  }
  if (fptr2 == NULL) {
      printf("Error! opening file");
      exit(1);
  }

  printf("Line count1 : %d Line count 2 : %d\n",lineCount1, lineCount2 );
/*
  char* dummy = (char)malloc(strlen(word) + 1);
  strcpy(dummy, word);
  char compare = (char)malloc(strlen(word) + 1);
  strcpy(compare, dummy);
  char compare2 = strcat(dummy, "\n");
*/
  while (line1 <= lineCount1 || line2 <= lineCount2){

    fgets(c1,1000,fptr1);
    fgets(c2,1000,fptr2);

    char try1[strlen(c1)+1];
    char try2[strlen(c2)+1];

    if(line1 > lineCount1){
      strcpy(try1," ");
      strcpy(try2,c2);
    }
    else if(line2 > lineCount2){
      strcpy(try1,c1);
      strcpy(try2," ");
    }
    else{
      strcpy(try1,c1);
      strcpy(try2,c2);
    }


      if(strcmp(try1,try2) != 0){
        printf("%s : Line %d: %s",txt1,line1,try1);
        printf("%s : Line %d: %s\n",txt2,line2,try2);

        difCount++;
      }

      // sep1 = strtok(NULL, seperator);
      // sep2 = strtok(NULL, seperator);

    line1++;
    line2++;
  }

  if(difCount==0){
    printf("The two text files are identical\n");
  }else{
    printf("%d different lines found\n", difCount);
  }

  fclose(fptr1);
  fclose(fptr2);

}

//-----------------------------PART5-END----------------------------------------


//-----------------------------PART-6-------------------------------------------
 	void print_image(FILE *fptr, char* clr)
 	{
 	    char read_string[128];

 	    while(fgets(read_string,sizeof(read_string),fptr) != NULL){
 				if(strcmp(clr, "r") == 0){
 					printf("\033[0;31m");
 				}
 				if(strcmp(clr, "g") == 0){
 					printf("\033[0;32m");
 				}
 				if(strcmp(clr, "b") == 0){
 					printf("\033[0;34m");
 				}
 				printf("%s",read_string);
 			}
 			printf("\033[0m");

 	}

 	void printer(char* name, char* color){
 	  char* filename;
 	  if(strcmp(name, "jordan")==0){
 	    filename = "jordan.txt";
 	  }
 	  else if(strcmp(name, "shrek")==0){
 	        filename = "shrek.txt";
 	  }
 	  else if(strcmp(name, "earth")==0){
 	        filename = "earth.txt";
 	  }
 	  else if(strcmp(name, "paris")==0){
 	        filename = "paris.txt";
 	  }
 	  else{
 	        filename = "nothing.txt";
 	  }
 	  FILE *fptr = NULL;

 	  if((fptr = fopen(filename,"r")) == NULL)
 	  {
 	      fprintf(stderr,"error opening %s\n",filename);
 	  }

 	  print_image(fptr, color);

 	  fclose(fptr);
 	}
 //-----------------------------PART6-END----------------------------------------


void print_command(struct command_t * command)
{
	int i=0;
	printf("Command: <%s>\n", command->name);
	printf("\tIs Background: %s\n", command->background?"yes":"no");
	printf("\tNeeds Auto-complete: %s\n", command->auto_complete?"yes":"no");
	printf("\tRedirects:\n");
	for (i=0;i<3;i++)
		printf("\t\t%d: %s\n", i, command->redirects[i]?command->redirects[i]:"N/A");
	printf("\tArguments (%d):\n", command->arg_count);
	for (i=0;i<command->arg_count;++i)
		printf("\t\tArg %d: %s\n", i, command->args[i]);
	if (command->next)
	{
		printf("\tPiped to:\n");
		print_command(command->next);
	}


}
/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t *command)
{
	if (command->arg_count)
	{
		for (int i=0; i<command->arg_count; ++i)
			free(command->args[i]);
		free(command->args);
	}
	for (int i=0;i<3;++i)
		if (command->redirects[i])
			free(command->redirects[i]);
	if (command->next)
	{
		free_command(command->next);
		command->next=NULL;
	}
	free(command->name);
	free(command);
	return 0;
}
/**
 * Show the command prompt
 * @return [description]
 */
int show_prompt()
{
	char cwd[1024], hostname[1024];
    gethostname(hostname, sizeof(hostname));
	getcwd(cwd, sizeof(cwd));
	printf("%s@%s:%s %s$ ", getenv("USER"), hostname, cwd, sysname);
	return 0;
}
/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */
int parse_command(char *buf, struct command_t *command)
{
	const char *splitters=" \t"; // split at whitespace
	int index, len;
	len=strlen(buf);
	while (len>0 && strchr(splitters, buf[0])!=NULL) // trim left whitespace
	{
		buf++;
		len--;
	}
	while (len>0 && strchr(splitters, buf[len-1])!=NULL)
		buf[--len]=0; // trim right whitespace

	if (len>0 && buf[len-1]=='?') // auto-complete
		command->auto_complete=true;
	if (len>0 && buf[len-1]=='&') // background
		command->background=true;

	char *pch = strtok(buf, splitters);
	command->name=(char *)malloc(strlen(pch)+1);
	if (pch==NULL)
		command->name[0]=0;
	else
		strcpy(command->name, pch);

	command->args=(char **)malloc(sizeof(char *));

	int redirect_index;
	int arg_index=0;
	char temp_buf[1024], *arg;
	while (1)
	{
		// tokenize input on splitters
		pch = strtok(NULL, splitters);
		if (!pch) break;
		arg=temp_buf;
		strcpy(arg, pch);
		len=strlen(arg);

		if (len==0) continue; // empty arg, go for next
		while (len>0 && strchr(splitters, arg[0])!=NULL) // trim left whitespace
		{
			arg++;
			len--;
		}
		while (len>0 && strchr(splitters, arg[len-1])!=NULL) arg[--len]=0; // trim right whitespace
		if (len==0) continue; // empty arg, go for next

		// piping to another command
		if (strcmp(arg, "|")==0)
		{
			struct command_t *c=malloc(sizeof(struct command_t));
			int l=strlen(pch);
			pch[l]=splitters[0]; // restore strtok termination
			index=1;
			while (pch[index]==' ' || pch[index]=='\t') index++; // skip whitespaces

			parse_command(pch+index, c);
			pch[l]=0; // put back strtok termination
			command->next=c;
			continue;
		}

		// background process
		if (strcmp(arg, "&")==0)
			continue; // handled before

		// handle input redirection
		redirect_index=-1;
		if (arg[0]=='<')
			redirect_index=0;
		if (arg[0]=='>')
		{
			if (len>1 && arg[1]=='>')
			{
				redirect_index=2;
				arg++;
				len--;
			}
			else redirect_index=1;
		}
		if (redirect_index != -1)
		{
			command->redirects[redirect_index]=malloc(len);
			strcpy(command->redirects[redirect_index], arg+1);
			continue;
		}

		// normal arguments
		if (len>2 && ((arg[0]=='"' && arg[len-1]=='"')
			|| (arg[0]=='\'' && arg[len-1]=='\''))) // quote wrapped arg
		{
			arg[--len]=0;
			arg++;
		}
		command->args=(char **)realloc(command->args, sizeof(char *)*(arg_index+1));
		command->args[arg_index]=(char *)malloc(len+1);
		strcpy(command->args[arg_index++], arg);
	}
	command->arg_count=arg_index;
	return 0;
}
void prompt_backspace()
{
	putchar(8); // go back 1
	putchar(' '); // write empty over
	putchar(8); // go back 1 again
}
/**
 * Prompt a command from the user
 * @param  buf      [description]
 * @param  buf_size [description]
 * @return          [description]
 */
int prompt(struct command_t *command)
{
	int index=0;
	char c;
	char buf[4096];
	static char oldbuf[4096];

    // tcgetattr gets the parameters of the current terminal
    // STDIN_FILENO will tell tcgetattr that it should write the settings
    // of stdin to oldt
    static struct termios backup_termios, new_termios;
    tcgetattr(STDIN_FILENO, &backup_termios);
    new_termios = backup_termios;
    // ICANON normally takes care that one line at a time will be processed
    // that means it will return if it sees a "\n" or an EOF or an EOL
    new_termios.c_lflag &= ~(ICANON | ECHO); // Also disable automatic echo. We manually echo each char.
    // Those new settings will be set to STDIN
    // TCSANOW tells tcsetattr to change attributes immediately.
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);


    //FIXME: backspace is applied before printing chars
	show_prompt();
	int multicode_state=0;
	buf[0]=0;
  	while (1)
  	{
		c=getchar();
		// printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

		if (c==9) // handle tab
		{
			buf[index++]='?'; // autocomplete
			break;
		}

		if (c==127) // handle backspace
		{
			if (index>0)
			{
				prompt_backspace();
				index--;
			}
			continue;
		}
		if (c==27 && multicode_state==0) // handle multi-code keys
		{
			multicode_state=1;
			continue;
		}
		if (c==91 && multicode_state==1)
		{
			multicode_state=2;
			continue;
		}
		if (c==65 && multicode_state==2) // up arrow
		{
			int i;
			while (index>0)
			{
				prompt_backspace();
				index--;
			}
			for (i=0;oldbuf[i];++i)
			{
				putchar(oldbuf[i]);
				buf[i]=oldbuf[i];
			}
			index=i;
			continue;
		}
		else
			multicode_state=0;

		putchar(c); // echo the character
		buf[index++]=c;
		if (index>=sizeof(buf)-1) break;
		if (c=='\n') // enter key
			break;
		if (c==4) // Ctrl+D
			return EXIT;
  	}
  	if (index>0 && buf[index-1]=='\n') // trim newline from the end
  		index--;
  	buf[index++]=0; // null terminate string

  	strcpy(oldbuf, buf);

  	parse_command(buf, command);

  	// print_command(command); // DEBUG: uncomment for debugging

    // restore the old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &backup_termios);
  	return SUCCESS;
}
int process_command(struct command_t *command);
int main()
{
	while (1)
	{
		struct command_t *command=malloc(sizeof(struct command_t));
		memset(command, 0, sizeof(struct command_t)); // set all bytes to 0

		int code;
		code = prompt(command);
		if (code==EXIT) break;

		code = process_command(command);
		if (code==EXIT) break;

		free_command(command);
	}

	printf("\n");
	return 0;
}








int process_command(struct command_t *command)
{
	int r;
	if (strcmp(command->name, "")==0) return SUCCESS;

	if (strcmp(command->name, "exit")==0)
		return EXIT;

	if (strcmp(command->name, "cd")==0)
	{
		if (command->arg_count > 0)
		{
			r=chdir(command->args[0]);
			if (r==-1)
				printf("-%s: %s: %s\n", sysname, command->name, strerror(errno));
			return SUCCESS;
		}
	}

	pid_t pid=fork();
	if (pid==0) // child
	{
		/// This shows how to do exec with environ (but is not available on MacOs)
	    // extern char** environ; // environment variables
		// execvpe(command->name, command->args, environ); // exec+args+path+environ

		/// This shows how to do exec with auto-path resolve
		// add a NULL argument to the end of args, and the name to the beginning
		// as required by exec

		// increase args size by 2
		command->args=(char **)realloc(
			command->args, sizeof(char *)*(command->arg_count+=2));

		// shift everything forward by 1
		for (int i=command->arg_count-2;i>0;--i)
			command->args[i]=command->args[i-1];

		// set args[0] as a copy of name
		command->args[0]=strdup(command->name);
		// set args[arg_count-1] (last) to NULL
		command->args[command->arg_count-1]=NULL;

		/*
		printf("command name: %s\n", command->name);
		printf("command arg[0]: %s\n", command->args[0]);
		printf("command arg[1]: %s\n", command->args[1]);
		printf("command arg[2]: %s\n", command->args[2]);
		*/

		if(strcmp(command->name, "highlight") == 0){

			highlight(command->args[1], command->args[2], command->args[3]);
		}
		else if(strcmp(command->name, "painter") == 0){
			printer(command->args[1], command->args[2]);
		}
		else if(strcmp(command->name, "goodMorning") == 0){
			goodmorning(command->args[1], command->args[2]);
			execlp("crontab", "crontab", "crontab.txt", NULL);

		}
		else if(strcmp(command->name, "kdiff") == 0){
			if(strcmp(command->args[1], "-a") == 0){
				kdiff(command->args[2], command->args[3]);
			}
			else if(strcmp(command->args[1], "-b") == 0){
				kdiffByte(command->args[2], command->args[3]);
			}
			else{
				kdiff(command->args[1], command->args[2]);
			}
		}

		else{
			char* path = execHelper(command->name);

			execv(path, command->args);
		}

		exit(0);
		/// TODO: do your own exec with path resolving using execv()
	}
	else
	{

		command->args=(char **)realloc(
		command->args, sizeof(char *)*(command->arg_count+=2));

		// shift everything forward by 1
		for (int i=command->arg_count-2;i>0;--i)
		command->args[i]=command->args[i-1];

		// set args[0] as a copy of name
		command->args[0]=strdup(command->name);
		// set args[arg_count-1] (last) to NULL
		command->args[command->arg_count-1]=NULL;

		if(strcmp(command->name, "shortdir") == 0){
			shortdir(command);
		}

		if (!command->background)
			wait(0); // wait for child process to finish

		return SUCCESS;
	}

	// TODO: your implementation here

	printf("-%s: %s: command not found\n", sysname, command->name);
	return UNKNOWN;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define TOKEN_DELIMITERS   "\a\t\r\n"
#define TOKEN_BUFFER_SIZE   64
#define RL_BUFFER_SIZE     1024
#define EXIT_SUCCESS     0
#define EXIT_FAILURE     1


/* 
 * Initial function declarations for our native shell commands.
 */
int std_cd(char** args);
int std_help(char** args);
int std_exit(char** args);


/*
 * String array of native commands.
 */
char* native_str[] = {
  "cd",
  "help",
  "exit"
};


int (*native_func[]) (char**) = {
  &std_cd,
  &std_help,
  &std_exit
};


int num_native_cmds() {
  return sizeof(native_str) / sizeof(char*);
}


//===== Native function implementations. =====/

/*
 * Because C handles changing directories differently with
 * child processes, we have to interpret it ourselves,
 * then pass the changed directory along to the successive
 * child process[es].
 */
int 
std_cd(char** args) {
  if (args[1] == NULL) {
    fprintf(stderr, "Standards: expected argument for \"cd\", but received NULL.\n");
  } else {
    if(chdir(args[1] != 0) {
      perror("Standards: could not change directory.\n");
    }
  }
  
  return 1;
}


/*
 * Prints out the native functions from the shell and other
 * miscellaneous usage information.
 */
int 
std_help(char** args) {
  printf("Joshua Crotts' Standards Shell.\n");
  printf("Type the program you want to execute, followed by its arguments, then hit enter.\n");
  printf("The following arguments are native to this shell application:\n");
  
  for (int i = 0; i < num_native_cmds(); i++) {
    printf("%s\n", native_str[i]);
  }
  
  printf("Use the man command for information relevant to other programs.\n");
  return 1;
}


/*
 * Main read, parse, and execution loop.
 *
 * Input from stdin, parsed through the appropriate function,
 * and executed.
 *
 * Code continues until the status code returns a negative or
 * non-positive (non-zero) value.
 */
void 
std_loop(void) {
  char*  line;
  char** args;
  int    status;
  
  do {
    printf("> ");
    line = std_read_line();
    args = std_split_line(line);
    status = std_execute(args);
    
    free(line);
    free(args)
  } while (status);
}


/*
 * Reads in a line into a char buffer.
 *
 * If the buffer is exceeded, it is dynamically re-allocated.
 */
char* 
std_read_line(void) {
  int   buffer_size = RL_BUFFER_SIZE;
  int   position    = 0;
  int   c;
  
  char* buffer      = malloc(sizeof(char) * buffer_size);
  
  if (!buffer) {
    fprintf(stderr, "Standards: Allocation error: %s.\n", buffer);
    exit(EXIT_FAILURE); 
  }
  
  while (true) {
    // Read in a character from the standard input stream.
    c = getchar();
    
    // If we run into the end of the line or the file, we put a null
    // character in the buffer and quit.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
      position++;
    }
    
    // If we exceed the buffer, then we need to re-allocate it.
    if (position >= buffer_size) {
      //  Increase the buffer size by the constant factor.
      buffer_size += RL_BUFFER_SIZE;
      buffer = realloc(buffer, buffer_size);
      if (!buffer) {
        fprintf(stderr, "Standards: Allocation error: %s.\n", buffer);
        exit(EXIT_FAILURE); 
      }
    }
  }
}


/*
 * Tokenizes the arguments line using the delimiters 
 * as specified above. 
 */
char**
std_split_line(char* line) {
  int    buffer_size = TOKEN_BUFFER_SIZE;
  int    position    = 0;
  
  char** tokens      = malloc(sizeof(char*) * buffer_size);
  char*  curr_token;
  
  if (!tokens) {
    fprintf(stderr, "Standards: Allocation error: %s.\n", buffer);
    exit(EXIT_FAILURE);
  }
  
  token = strtok(line, TOKEN_DELIMITERS);
  
  while (token != NULL) {
    tokens[position] = token;
    positions++;
    
    if (position >= buffer_size) {
      buffer_size += TOKEN_BUFFER_SIZE;
      tokens = realloc(tokens, sizeof(char*) * buffer_size);
      
      if (!tokens) {
        fprintf(stderr, "Standards: Allocation error: %s.\n", buffer);
        exit(EXIT_FAILURE);
      }
    }
    
    token = strtok(NULL, TOKEN_DELIMITERS);
  }
  
  tokens[position] = NULL;
  return tokens;
}


/*
 * Launches a command using a separate child process. We
 * temporarily halt the parent processing id while executing
 * the child one.
 */
int 
std_launch(char** args) {
  pid_t pid;
  pid_t wpid;
  
  int   status;
  
  pid = fork();
  
  if (pid == 0) {
    // If we are on the child process:
    if(execvp(args[0], args) == -1) {
      perror("Standards error: could not execute command.\n");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // If the PID is less than 0, there is an error.
    perror("Error forking: process ID code: %d.\n");
  } else {
    // Otherwise, we successfully created the child process.
    //
    // Continue looping until the process is either exited
    // or killed.
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  
  return 1;
}


/*
 * Executes a command with specified arguments. 
 * 
 * If the supplied argument is a native command, we
 * execute it instead.
 */
int
std_execute(char** args) {
  if (args[0] == NULL) {
    // No command was entered. Just reprint the shell
    // input prompt.
    return 1;
  }
  
  for(int i = 0; i < num_native_cmds(); i++) {
    if(strcmp(args[0], native_str[i]) == 0) {
      return (*native_func[i])(args);
    }
  }
  
  return std_launch(args);
}


/*
 * Main function.
 */
int 
main(int argc, char* argv[]) 
  // Main interpretation loop.
  std_loop();
  
  return EXIT_SUCCESS;
}
/*Wesley varrasso 
 * cssc0096 
 * proffessor John Carroll
 * oct 1 2020
 */
// header file for p2 
  
// set header files 
#include "getword.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include "CHK.h"   
#include <libgen.h>
// set constants 
#define MAXARGS 100
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
// declare globals 
char *outptr, *inptr , *pipedptr, *negPtr, *eofPtr, *badEnv;
int pipeCount, inputCount;
int pipes[10];
char* w;
char bigbuff[MAXARGS*255];
char *newargv[MAXARGS];
int flag; 
// declare flags 
extern bool isSlash;
extern bool isNeg;
extern bool isTilde;
extern bool dLessThan;
bool dontFork;
bool lessThan;
bool doubleLessThan;
bool greaterThan;
bool chDir;
bool isPipe;
bool environ;
bool forkBackground;
//declare functions
void myhandler(int signum);
int parse();
void fork_and_exec();
void pipe_and_exec();

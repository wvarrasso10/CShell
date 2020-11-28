#include "p2.h"
/*Wesley Varrasso
 * Professor John Carroll
 * cs570
 * cssc0096
 **/

/*
 * Synopsis program 2:
 * program 2 is a command line interpreter for unix systems 
 * specifications: every line of input is 0 or more words
 * to agree with program 1 p2 reissues prompt if no input is entered
 * p2 terminates if first word is EOF 
 * metacharacters from program 1 have special significance 
 * namely | < > &  
 * must handle redirection like normal shells using < > |
 * also handle cd like real shells  
 * handles 100 args of 255 characters 
 * main calls parse function to handle syntacal analysis 
 * then calls fork_and_exec function to handle child processes 
 * file redirection and exececute commands using execvp.
 **/
bool isSlash;
bool isNeg;
bool isTilde;
bool dLessThan;
main(int argc, char *argv[])
{
	int c, i;
	char *path;
	size_t pathmax = 50;
	char *prompt = ":570: ";
	char *baseName, *pbuf;
	char buf[50];
	int cd;
	setpgid(0,0);
	//pid_t pid;
    //int status;
	// setup sighandler
	(void) signal(SIGTERM,myhandler);
	
	for(;;){
		//pid = waitpid(-1,&status,WNOHANG);
		//print prompt of shell
		printf("%s", prompt);
		// call parse for syntax analysis
		c = parse();
		// check for EOF
		if( c == -255) break;
		//check for environ flag
	  	if (environ){
		    //check if only 1 arg
		    if (newargv[2] == NULL) {
			if(getenv(newargv[1]) == NULL){
			    //if no args entered show empty
			    printf("\n");
			}
		        //print environ variable
			else{printf("%s\n",getenv(newargv[1]));}
		    }
		    //if two args entered setenv
		    else if (newargv[3] == NULL) {
			setenv(newargv[1],newargv[2],1);
		    } 
		    //to many args entered report error
		    else { fprintf(stderr,"getenv: Too many arguments\n"); }
		    environ = false;
		}
		// check for cd if true dont fork
		else if (chDir){
			// if no path entered cd $HOME
			if(newargv[1] == NULL) path = getenv("HOME");
			//check if to many args report err 
			if(newargv[2] != NULL) {fprintf(stderr,"chdir: too many arguments\n"); }
			// else set path  
			if (newargv[2] == NULL){ 
				if(newargv[1]!= NULL)path = newargv[1];
				// cd to path check if fail 
				if( (cd = chdir(path)) < 0) {
					perror("cd failed");
					exit(1);
				}
				//append tail dir to front of prompt
				pbuf = getcwd(buf,pathmax);
				baseName = basename(pbuf);
				prompt = baseName;
				strcat(prompt,":570: ");
			}	
			// reset flag
			chDir = false;
		}
		//error cases 
		else if (dontFork == true) {
		    if (inputCount > 1) { fprintf(stderr, "ambiguous input\n");}
		    else if(newargv[0] == NULL || isPipe == true || dLessThan == true ) { 
			isPipe = false; 
			dLessThan = false;
			fprintf(stderr,"NULL command\n"); 
	            }	
		    else{fprintf(stderr,"Undefined Variable,%s\n",badEnv);}
		    dontFork = false;
		}
		else  {
		    //call function to fork process and execute command
		    fork_and_exec();	
		}
		// clean newargv from prev command
		for (i = 0; i< c; i++) newargv[i] = NULL;   	
    }	   
    //kill child processes
    killpg(getpgrp(), SIGTERM);
    printf("p2 terminated.\n");
    exit(0);
		
}//endparse

//my handler function for sighandler setup
void myhandler(int signum) {
}
// parse function returns arguement count  if reached end of input
// parse sets global flags so commands can be executed
// appends to big buff and newargv by incrementing w and ptr
int parse(){
   char **ptr;
   bool skip = false, skipLessThan = false, skipGreaterThan = false;
   bool subDir = true, skipBackground = false;
   int count = 0,argcount =0,tokcount =0;
   char *token, *searchTok;
   char* path, *str, *tempstr, *containsSubDir;
   char* line = NULL;
   size_t lineBuffSize = 0;
   ssize_t lineSize;
   FILE *file; 
   FILE *tempFile;
   char *fileName = "tempFile.txt\0";
   const char sep[2] = ":";
   eofPtr = NULL;
   inputCount = 0;
   pipeCount = 0;
   ptr = newargv;
   w= bigbuff; 
   greaterThan = false;
   lessThan = false;
   
   for(;;){
	// get the return from getword
	int wordSize;
 	wordSize = getword(w);
	// check for EOF
	if (wordSize == -255 && count == 0 ) {
		return -255; 
		break;
	}
	// handle the $ case strip negative 
	if( wordSize < 0 ){ 
		wordSize = abs(wordSize);
		wordSize ++;
	}
        // get name of file after either < or > was entered skip makes sure files dont go in buff and set ptr
        if(skip == true && greaterThan == true && skipGreaterThan != true && doubleLessThan != true) outptr = w;
        // check for <<
        if(doubleLessThan == true){ 
		inputCount++;
		doubleLessThan = false; 
		eofPtr = w;
        }
	//check for < flag
	if(skip == true && lessThan == true && skipLessThan != true){ inptr = w; inputCount++;}
	// check for multiple < or <<
        if (inputCount > 1) { dontFork = true;}
	//check for & and | and make sure / doesnt preceed them set flags
	if (w[0] == '&' && isSlash == true ) skipBackground = true;
        // check ambiguous case 
	if(w[0] == '&' && (bigbuff[count-2] == '>' || bigbuff[count-2] == '<')) {
		skipBackground = true;
	}

	if (w[0] == '|' && isSlash != true){ newargv[argcount] = NULL; argcount++; ptr++;}		
        //check for $variable
        if(isNeg == true && isSlash == false){
		//check if $variable is valid
		if(getenv(w) != NULL){ dontFork = false; }
		//invalid variable dont fork
		else { dontFork = true; badEnv = w; }
		//check for > 
		if (greaterThan == true) outptr = strdup(getenv(w)) ;
		else if(greaterThan == false){
		    *(ptr) = getenv(w);
			//if (greaterThan == true) outptr = *(ptr);
		    argcount++;
		    ptr++;
		}
		//if > set outptr to path of $variable
		//if(greaterThan == true) outptr = getenv(w);
		isNeg = false;
		skip = true;
        }
        //check for ~ at beginning of word
        if(isTilde == true && isSlash == false) {
		skip = true; 
		isTilde = false;
		// check if lone ~  
		if(wordSize == 1) *(ptr) = getenv("HOME");
		//check for / after ~ 
		else if (w[1] == '/'){
		    //append homedir to front of w
		    w++;
		    *(ptr) = getenv("HOME");
		    strcat(*(ptr),w);	
		    argcount++;
		    ptr++;
		} 
		//~ is follow by a string search /etc/passwd
		else{
		    file = fopen("/etc/passwd","r");
		    if (!file){
			fprintf(stderr, "Error opening file" );
			exit(6);
		    }
		    // check if ~word has a /subdir appended
		    tempstr = strdup(w); 
		    if( (containsSubDir = strpbrk(tempstr, "/")) == NULL){
			searchTok = w;
			subDir = false;
		    }
		    else{ searchTok = strsep(&tempstr,"/");}
		    lineSize = getline(&line,&lineBuffSize,file);
		    // get lines from /etc/passwd
		    while (lineSize >= 0 ){
			tokcount =0;
			lineSize = getline(&line,&lineBuffSize, file);
			token = strtok(line, ":");
			//check if user found
			if(strcmp(token,searchTok) == 0){
			    //if found break loop
			    lineSize = -2;
			    //get 6th token in line
			    while(token != NULL){
				tokcount++;	
				if(tokcount == 6){
				    //append path to newargv
				    path = token;
				    *(ptr) = path; 
				    // subdir case append subdir
				    if(subDir){
					strcat(*(ptr),"/");
					strcat(*(ptr),tempstr);
				    }
				    //append to buffer and increment counts
				    strcpy(bigbuff+count,*(ptr));
				    wordSize = (int)strlen(*(ptr));
				    argcount++;
				    ptr++;
				}
				token = strtok(NULL,sep);
			    } 
			}
		    }
		    fclose(file);
		    //eof reached user not found
		if (lineSize == -1) { fprintf(stderr, "Unknown User: %s\n", w); exit(9);}
	     }
	}
             
		// check if slash flag was enabled or not a meta character then add words to newargv
        if(wordSize!= 0 && ( w[0] != '<' && w[0] != '>' &&  w[0] != '|'&& skip == false) || isSlash == true ){	
		// set w = to newargv ptr and advance 
		*(ptr) = w;	
		argcount++;
		ptr++;
	}
	    // reset skip and increment count and w  
	skip = false;
	w += wordSize;
	count += wordSize;
	//check for << after line has been parsed 
	if(wordSize == 0 && dLessThan == true){
	     //check leftside != NULL and eof word != null and not ambiguous
	     if(newargv[0] != NULL && strlen(eofPtr) > 0 && inputCount <2) { 
		//open temp file 
		tempFile = fopen(fileName,"w+");
		//loop through stdin input
		for (;;) {
		    lineSize = getline(&line,&lineBuffSize, stdin) ;
		    str = strdup(line);
		    str[lineSize-1] = '\0';
		    //check if line matches eof word
		    if( strcmp(str,eofPtr) == 0) break;
		    //store in file
		    fputs(line,tempFile);
		}
		    //close file and set inptr for dup2
		    fclose (tempFile);
		    inptr = fileName;
	     }
		 //fail case dont fork report error
		else{dontFork =true;}
	}    

        if(dLessThan == true && bigbuff[count-1] == '<' && isSlash != true){ 
		skip = true; 
		doubleLessThan = true;
	}
	    //setflags when meta characters are encountered set skip to skip next word (file )
	if(bigbuff[count-1] == '<' && wordSize == 1 && isSlash != true) {
		// check if > flag was already set
		if(greaterThan == true)
			skipGreaterThan = true;	
		skip = true; 
		lessThan = true; 
	}
	if(bigbuff[count-1] == '>' && isSlash != true) { 
		// check if < flag was already set
		if(lessThan == true)
			skipLessThan = true;
		skip = true; 
		greaterThan = true;
	}
        //check for "environ" string
	if(wordSize == 7 && bigbuff[count-1] == 'n' && bigbuff[count-2] == 'o'
        && bigbuff[count-3] == 'r' && bigbuff[count-4] == 'i'&& bigbuff[count-5] == 'v' 
        && bigbuff[count-6] == 'n' && bigbuff[count-7] == 'e'&& argcount == 1)
		environ = true;
	// set flag for cd check of 2 letter word and cd 
	if(wordSize == 2 && bigbuff[count-1] == 'd' && bigbuff[count-2] == 'c' && argcount == 1)
		chDir = true;
	// set flags for pipe and set location of pipe end 
	if(bigbuff[count-1] == '|' && isSlash != true) {
		isPipe = true; 
		pipes[pipeCount] = argcount;
		pipeCount++;
	}  
	// check if & is at the end of the word 
	if( bigbuff[count-2] == '&' && wordSize == 0 && skipBackground == false) {
		newargv[argcount-1] = NULL; 
		forkBackground = true;
	}
	//check for null argument at end of pipe report error dont fork
	if(wordSize == 0 && newargv[pipes[pipeCount -1]] == NULL && isPipe ==true){
		dontFork = true; 
	}
	isSlash = false;
 	// insert null between words	
   	bigbuff[count] = '\0';
   	w++;
   	count++;
	if( wordSize == 0 ) { 
		return argcount;
		break;
	}
   }
}
/*fork_and_exec() forks child process from parent then perorms file io
 * redirection based off flags from parse then perform execvp to execute commands
 */
void fork_and_exec(){
	int in_fd;
	int readOnly = O_RDONLY;
	pid_t pid , status;
	// flush before fork
	fflush(stderr);
	fflush(stdout);
	// skip if pipe was found
	if ( isPipe!= true) {
	if ((pid = fork()) < 0) { // fork parent check if failed
		perror("err: fork failed\n");
		exit(4);
	}
	else if(pid == 0) { //child has been forked successfully do child things
		if(lessThan == true || dLessThan == true){ //check flag if < was set
			//get file descriptor of inptr check if open failed
			if((in_fd = open(inptr, readOnly, S_IRUSR | S_IWUSR)) < 0){
				fprintf(stderr, "%s couldn't be opened\n", inptr);	
				exit(5);
			}
			// put stdin into in_fd using dup2
			dup2(in_fd,STDIN_FILENO);
			close(in_fd);
			if(dLessThan == true)  
				remove("tempFile.txt");
			inptr = NULL;
		}
		
		if(greaterThan == true) { // check for > flag 
			int out_fd;
			// get file descripter of out file
			if((out_fd = open(outptr, O_WRONLY | O_CREAT | O_TRUNC | O_CREAT | O_EXCL, 
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) { 
				fprintf(stderr, "%s :couldn't be opened\n", outptr); 
				exit(5);
			}
			// set std out to outfile descripter (io redirection)
			dup2(out_fd ,STDOUT_FILENO);
			close(out_fd);
			outptr = NULL;
		}
		   // execute command 	
		if (execvp( newargv[0], newargv) < 0 ) {
			perror("exec failed");
			fprintf(stderr, "%s: command failed\n", newargv[0]);
			exit(6);	
		}
		greaterThan = false;
		lessThan = false;
		dLessThan = false;
	}
	// wait for child to terminate 
	else if (forkBackground == false) {  
 			while( wait (&status) != pid) ;
	}
	// if background process dont wait
	else if (forkBackground == true) { 
		printf("%s [%d]\n" , newargv[0], pid);  
		forkBackground = false;  
	}
	
	} 
	// if pipe flag go to pipe function	
	else if (isPipe == true) pipe_and_exec(); 
	dLessThan = false;
}
// pipe function to create vertical piping and execute commands
void pipe_and_exec() {
    int fd[20];
    pid_t grandkid;	
    int i =0, j= 0, x = 0, k =0;
    int in_fd, out_fd;
    pid_t kidpid; 
    isPipe = false;
    // flush before fork
    fflush(stdout);
    fflush(stderr);
	//pipe(fd+ (pipeCount*2));
    //printf("before fork=%d,%d\n" , getpid(),getppid());
    // fork a child
    if((kidpid = fork()) < 0) {		 
        perror("fork failed\n");
        exit(4);
    }
	// do child things 
    else if (kidpid == 0) {
        // create the pipes
        for (i = 0; i< pipeCount ; i++){
           pipe(fd+ (i*2));
        }
	//loop and fork grandchild
        for( i = 0; i< pipeCount ; i++){
            fflush(stdout);
            fflush(stderr);
            if((grandkid = fork()) < 0) { // fork grandchild
                perror("grandkid fork failed\n");
                exit(4);
            }
	
            else if(grandkid != 0 ) { // do grandkid stuff
                // check if last child 
                if( i == 0) {
                    //check lesstan flag or doubleLessThan
                    if(greaterThan == true) {
			if((out_fd = open(outptr,O_WRONLY|O_CREAT|O_TRUNC|O_EXCL,
                		S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0) {
                            perror("err:output file open failed\n");
                            fprintf(stdin,"%s couldn't be opened \n", outptr);
                            exit(6);
                        }
                        //perform io redirection 
			dup2(out_fd, STDOUT_FILENO);
                        close(out_fd);
                        outptr = NULL;
                    }
			dup2(fd[0],STDIN_FILENO);
                    for(j = 0; j< pipeCount*2; j++)
                        close(fd[j]);
			if( execvp(newargv[pipes[pipeCount-1]], newargv + pipes[pipeCount-1]) < 0) {
                   // if( execvp(newargv[0], newargv) < 0) {
                        perror("leftside of pipe exec failed\n");
                        exit(6);
                    }
                }
                //middle children if 
                else if(pipeCount > 1 && i> 0) {
                    // redirect stdout and stdin
                    dup2(fd[i*2],STDIN_FILENO);
                    dup2(fd[(i*2)-1],STDOUT_FILENO);
                    // close file descriptors
                    for(x = 0; x < pipeCount*2; x++)
                        close(fd[x]);
                    //exec children right to left
                    if (execvp(newargv[pipes[pipeCount-1-i]], newargv + pipes[pipeCount-1-i]) < 0) {
                        perror("pipe exec failed\n");
                        exit(6);
                    }
                }
            }// end gc 
        }//end for
            //check for greater flag in child
         if(lessThan == true || dLessThan == true) {
		if((in_fd = open(inptr, O_RDONLY, S_IRUSR | S_IWUSR)) < 0) {
            //open outfile
                perror("err: output file open failed \n");
                fprintf(stdin,"%s couldn'tbe opened \n",inptr );
                exit(6);
            }
            //redirect io
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
	    if(dLessThan == true) remove(inptr);
            inptr = NULL;
        }
	dup2(fd[(2*pipeCount)-1], STDOUT_FILENO);
        // get read end of pipe
        for (k = 0; k < pipeCount*2; k++)
            close(fd[k]);
	if( execvp(newargv[0], newargv) < 0) {
        // execute the right command
            perror("right side pipe exec failed\n");
            fprintf(stderr, "%s: command failed\n", newargv[pipeCount]);
            exit(5);
        }
	dLessThan = false;
	lessThan = false; 
    }//end child
    // reap children until the kidpid is found because it is the right side of the pipe
    else {
	for (;;) {
		pid_t pid;
		pid = wait(NULL);
		if( pid == kidpid){ 
			break;
		}
	}
    }
}



#include <stdio.h>
#include "getword.h"
#include "p2.h"
#include <string.h>
#include <stdlib.h>
/*wesleyvarrasso
 * Proffessor John Carroll
 * cs570
 * cssc0096
 * due 9/1/20
 *Synopsis of getword function:
 * Lexical analizer that gets one word from input stream 
 * Input: A pointer to the begining of the character string 
 * Output: returns -255 if EOF encounterd otherwise it
 * returns count of characters in word with some exceptions
 * A word is a string containing one matacharacter ot a string consisting 
 * of non metacharacters delimited by blanks tabs newlines metacharacters or EOF metacharacters are <, >, |, &, <<. 
 * Special characters: $, ~, \ are special characters and follow special rules.
 * if \ precedes a metacharacter that character is treated normally and not like a delimiter
 * if line has zero or more blank/tabs and reaches EOF and string is empty return -255
 * If $ preceeds a word without a \ infront the word count becomes negative 
 * and $ is left out else its a regular char
 * If ~ begins the word and not \ then the $HOME path is appended to the begining
 * and the rest of the characters are appended to the end else it is treated normally
 * Max word size is 255 if larger than 255-1 the next getword call begins the 
rest of the word
 */
int getword(char *w)
{
    int iochar;
    int i = 0;
    int count = 0;
    int isNegative = 0;
    int nonSpecial = 0;
    while ((iochar = getchar()) != EOF )
    {
         w[i] = iochar;
        /*if at start of word and newline return 0*/
        if( count == 0 && ( w[i] == '\n')  )
        {
            w[i] = '\0';
            return 0;
        }
        /* if double \\ is encountered \\ becomes \ and becomes nonspecial */
        if(w[i]== '\\' && w[i-1] == '\\'){
            *w--;
            count--;
            nonSpecial = 1;
        }
        /*if not at start of word and tab space or newline is encounted */
        if ( count != 0 && (w[i] == '\t' || w[i] == '\0' || w[i]== '\n'|| w[i] == ' '))
        {
            /*if only newline is encounted put newline back on input stream so it can be counted*/
            if(w[i] == '\n' && w[i-1] != '\\')
            {
                ungetc('\n', stdin);
            }
            /*if a single slash and newline is encountered treat as space and overwrite slash */
            if(w[i] == '\n' && w[i-1] == '\\' && nonSpecial == 0)
            {
                *w--;
                count--;
            }
            /*if a double slash and newline is encountered treat slash is not special and newline gets put back
 *             on stream to be counted */
            if(w[i] == '\n' && w[i-1] == '\\' && nonSpecial == 1)
            {
                ungetc('\n', stdin);
                nonSpecial = 0;
            }
            /*if a single slash and space is encountered treat as space and overwrite slash
 *             and embed a space into the word */
            if(w[i] == ' ' && w[i-1] == '\\' )
            {
                w[i-1] = ' ';
                continue;
            }

            w[i] = '\0';
            /*check if word is negative */
            if(isNegative == 0)
            {
                return count;
            }
            if(isNegative == 1)
            {
                if(count != 0)
                {
                    count -=1;
                }
                return -(count);
            }
        }
        /*if a single slash and ~ is encountered if so treat overwrite \ and ~ is not special */
        if(w[i-1] == '\\'&& w[i] == '~'){
            w[i-1] = '~';
            continue;
            }
        /*if only ~ is encountered at start of word append Home path */
        if(count == 0 && w[i] == '~'&& w[i-1] != '\\'){
	/*
            size_t j;
            char x[255];
            *strcpy(x, getenv("HOME"));
            for (j = 0; j < strlen(x); j++){
                w[i] = x[j];
                *w++;
                count++;
            }
            *w--;
            count--;
	*/
            *w--;
            count--;
            isTilde = true;	
        }  
        /*if STORAGE-1 is reached end word return count */ 

        if(count == 253){
	    count++;
	    *w++;
            return count;
        }
        /*check if slash is encountered before $ if so overwrite slash and $ is not special*/
        if(w[i-1] == '\\'&& w[i] == '$')
        {
            w[i-1] = '$';
            continue;
        }
        /*check if slash and metacharacter are reached if so overwrite slash and treat metachar
 *         as nonspecial*/
        if( w[i-1] == '\\' && (w[i] == '<'|| w[i]== '>' || w[i] == '|' || w[i]== '&') )
        {
	    isSlash = true; 
            w[i-1] = w[i];
            continue;
        }
	if( w[i-1] == '\\' && (w[i] != '<'|| w[i] != '>' || w[i] != '|' || w[i] != '&' || w[i] != '$' || w[i] != '~') ){
            w[i-1] = w[i];
            continue;
        }

        /*check if metacharacters are not preceeded by slash and if not at begining of word*/
        if( w[i-1] != '\\' && (w[i] == '<'|| w[i] == '>' || w[i] == '|' || w[i] == '&' ) )
        {
            /*if not a begin put on back on stream so they can be counted*/
            if(count != 0)
            {
                if(w[i] == '<')
                {
                    ungetc('<',stdin);
                }
                if( w[i] == '>' )
                {
                    ungetc('>',stdin);
                }
                if( w[i] == '|')
                {
                    ungetc('|',stdin);
                }
                if( w[i] == '&')
                {
                    ungetc('&',stdin);
                }
                w[i] = '\0';
                return count;
            }
            else if(count == 0 && w[i]== '<' )
            {
                char temp = getchar();
                if( w[i] == temp )
                {
                    count++;
                    *w++;
                    w[i]= temp;
	                dLessThan = true;	
                }
                else
                {
                    ungetc(temp, stdin);
                }
                *w++;
                count++;
                w[i] = '\0';
                return count;
            }
            else
            {
                count++;
                *w++;
                w[i] = '\0';
                return count;
            }
        }
        if (count == 0 && w[i] == '$')
        {
            isNegative = 1;
            isNeg = true;	
            *w--;
        }

        if (count == 0 && w[i] == ' '|| w[i] == '\t')
        {
            count -= 1;
            *w--;
        }
        *w++;
        count += 1;
    }
    w[i] = '\0';
    if(count != 0)
    {
        if(isNegative == 0)
        {
            return count;
        }
        if(isNegative == 1)
        {
            if(count != 0)
            {
                count -=1;
            }
            return -(count);
        }
    }
    return -255;
}

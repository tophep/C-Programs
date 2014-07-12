#include <stdio.h>
#include <unistd.h>

/**

    Moving Text On The Command Line

    This is a basic example of how to create moving or animated text on
    the command line.

    Make your command line programs fancier by adding dynamic loading bars or waiting messages

    The special character '\r' returns the cursor to the beginning of the current line on the screen.
    The special character '\b' moves the cursor back a single character on the current line.
    Printing on top of old characters overwrites them.

    Works on OSX and Linux 


**/

int main() {
	printf("\tWaiting");
	fflush(stdout);
	sleep(1);
    while(1){
    	for (int i = 0; i < 11; i++) {
            //wait a second to produce the dynamic moving effect
        sleep(1);
        printf(".");
        fflush(stdout);
    	}
    	sleep(3);
    	printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    	fflush(stdout);	
    }
    
    printf("\n");
    return 0;
}
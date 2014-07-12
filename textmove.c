#include <stdio.h>
#include <unistd.h>
int main() {
	printf("\tWaiting");
	fflush(stdout);
	sleep(1);
    while(1){
    	for (int i = 0; i < 11; i++) {
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
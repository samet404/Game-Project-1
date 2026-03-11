#include <stdbool.h>

/*
 * Executes given shell command, prints the output and assigns pclose status code as to status pointer.
 * If the status code is not zero, that means program failed with error. The exit status code can be retrieved by using WIF... macros. 
 * 
 * name - Used to uniquely function call.
*/
void cmd(char* name, int *status, char* cmd);
/*
 * Prints debugging information about status code if shell command fails.
*/
void cmd_infoOnError(char* name, int status);
/*
* Exits from build step on error
*/
void cmd_exitOnError(char* name, int status);
/*
 * returns bool indicates program execution was failed or not
*/
bool cmd_isFailed(int status);

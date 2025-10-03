#include "revert_string.h"
#include <stdio.h>
void RevertString(char *str)
{
	int len = 0;
    char *end = str;
    
    while (*end != '\0') {
        len++;
        end++;
    }
    
    for (int i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = tmp;
    }
}


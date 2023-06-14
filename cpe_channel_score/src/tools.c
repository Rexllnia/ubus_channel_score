#include "tools.h"

int execute_cmd(char *cmd,char **rbuf) 
{
    FILE *fp;

    if (cmd == NULL) {
        return FAIL;
    }

    fp = popen(cmd, "r");
    if (fp == NULL) {
        return FAIL;
    }
    if (rbuf == NULL) {
        pclose(fp);
        return SUCCESS;
    }
    *rbuf =(char *) malloc(MAX_POPEN_BUFFER_SIZE);
    if (rbuf == NULL) {
        pclose(fp);
        return FAIL;
    }
    
    
    fread(*rbuf,sizeof(char),MAX_POPEN_BUFFER_SIZE,fp);
    printf("%s\r\n",*rbuf);
    pclose(fp);
    return SUCCESS;
}
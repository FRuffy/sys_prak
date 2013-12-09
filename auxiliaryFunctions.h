#ifndef auxiliaryFunctions
#define auxiliaryFunctions


int openConfig(char* name);
int performConnection(int sock);
char*  recvServerInfo(char* buffer);
int checkServerReply(int sock, char* buffer);
int initConnection(int argc, char** argv);
void sendReplyFormatted(int sock, char* reply);
char* antistrcat(char* dest, char* src);
int readGameField(char *buffer);
int printGameField();



#endif

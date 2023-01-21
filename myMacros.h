#pragma once
#define CHECK_RETURN_0(pointer) (*pointer==NULL? 0)

#define CHECK_RETURN_NULL(pointer) (pointer==NULL?NULL)

#define CHECK_MSG_RETURN_0(pointer,msg) if(pointer==NULL)\
printf("%s",msg);\
return 0;

#define CHECK_0_MSG_CLOSE_FILE(f) int num=fgetc(f);\
if(num==0)\
printf("The value is 0");\
fclose(f);\
return 0;

#define CHECK_NULL_MSG_CLOSE_FILE(f,pointer) ;\
if(pointer == NULL)\
printf("The value empty"); \
fclose(f); \
return NULL;

#define MSG_CLOSE_RETURN_0(f,msg) ;\
printf("%s", msg); \
fclose(f); \
return 0;
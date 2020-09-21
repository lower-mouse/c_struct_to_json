#include "test.h"
#include "json_base.h"
#include "cJSON.h"
#include "string.h"

int main()
{
    int succ = 0;
    char buf[1024];
    testCfg_t test;
    memset(&test, 0, sizeof(test));
    test.array[0] = 1;
    test.array[1] = 1;
    test.array[2] = 1;
    test.number = 2;
    test.c = 'a';
    test.timeFormat = TIME_DD_MM_YY;
    test.positionArray[0].x = 100;
    test.positionArray[0].y = 200;
    test.positionArray[1].x = 50;
    test.positionArray[1].y = 150;
    test.structure.x = 100;
    test.structure.y = 200;

    snprintf(test.string, sizeof(test.string), "hello world!");
    wrap_s2jstring(testCfg, &test, buf, 1024, succ);
    if(succ != 0)
    {
        printf("osdCfg to string failed, errno:%d\n", succ);
        return -1;
    }

    printf("string lenght:%lu\n%s",strlen(buf), buf);

    memset(&test, 0, sizeof(test));
    wrap_jstring2s(testCfg, &test, buf, succ);
    if(succ != 0)
    {
        printf("osdCfg to struct failed, errno:%d\n", succ);
        return -1;
    }
    
    memset(buf, 0, sizeof(buf));
    wrap_s2jstring(testCfg, &test, buf, 1024, succ);
    if(succ != 0)
    {
        printf("osdCfg to string failed, errno:%d\n", succ);
        return -1;
    }

    printf("string lenght:%lu\n%s",strlen(buf), buf);
    return 0;
}


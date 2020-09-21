#include "cJSON.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>    
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_base.h"

#define LD_JSON_FILE_SIZE_MAX 102400

int json_file_save(const cJSON *item, int fd, const char* path)
{
    if(item == NULL)
    {
        return -1;
    }

    if(fd < 0)
    {
        if(path == NULL)
        {
            return -1;
        }
        
        fd = open(path, O_WRONLY|O_CREAT|O_TRUNC);
        if(fd < 0)
        {
            printf("file %s:fopen failed:0x%x\n", path, errno);
            return -1;
        }
    }

    char* str = cJSON_Print(item);
    int len = strlen(str);
    int writelen = write(fd, str, len);
    if(writelen != len)
    {
        printf("writelen:%d len:%d errno:%d\n", writelen, len, errno);
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

cJSON *json_file_parse(const char *Path)
{
    if(Path == NULL)
    {
        return NULL;
    }
    
    int fd = open(Path, O_RDONLY);
    if(fd < 0)
    {
        printf("file %s:fopen failed:0x%x\n", Path, errno);
        return NULL;
    }

    struct stat st;
    if(fstat(fd, &st) < 0)
    {
        printf("file stat failed:0x%x\n", errno);
        return NULL;
    }

    printf("file st_size:%ld\n", st.st_size);
    if(st.st_size <= 0 || st.st_size > LD_JSON_FILE_SIZE_MAX)
    {
        printf("file st_size:%ld\n", st.st_size);
        return NULL;
    }

    char* filebuf = (char*)malloc(st.st_size);
    if(filebuf == NULL)
    {
        printf("malloc failed\n");
        return NULL;
    }

    int readlen = read(fd, filebuf, st.st_size);
    if(readlen < 0)
    {
        printf("read json failed:0x%x\n", errno);
        return NULL;
    }else if(readlen < st.st_size)
    {
        printf("read json len:%d less %ld\n", readlen, st.st_size);
    }

//    printf("%s\n", filebuf);
    cJSON *root = NULL;
    root = cJSON_Parse(filebuf);
    if(root == NULL)
    {
        printf("cJson parse failed\n");
        return NULL;
    }

    return root;
}

int json_int_j2s(const cJSON * const item, const char * const string, int* value)
{
    if(value == NULL || item == NULL)
    {
        return -1;
    }

    const cJSON * node = NULL;
    if(string == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, string);
    }
    
    if(cJSON_IsInvalid(node))
    {
        CFG_WARN("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    if(cJSON_IsNumber(node) == 0)
    {
        printf("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    double v = cJSON_GetNumberValue(node);
    *value = (int)v;
    return 0;
}

int json_int_s2j(cJSON * item, const char * const name, int* in)
{
    if(item == NULL)
    {
        return -1;
    }

    cJSON * node = NULL;
    if(name == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, name);
    }

    if(cJSON_IsInvalid(node))
    {
        double v = *in; 
        node = cJSON_AddNumberToObject(item, name, v);
        if(cJSON_IsInvalid(node))
        {
            printf("create name failed\n");
            return -1;
        }
    }else
    {
        cJSON_SetNumberValue(node, *in);
        node->type = cJSON_Number;
    }

    return 0;
}

int json_long_j2s(const cJSON * const item, const char * const string, long* value)
{
    if(value == NULL || item == NULL)
    {
        return -1;
    }

    const cJSON * node = NULL;
    if(string == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, string);
    }
    
    if(cJSON_IsInvalid(node))
    {
        CFG_WARN("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    if(cJSON_IsNumber(node) == 0)
    {
        printf("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    double v = cJSON_GetNumberValue(node);
    *value = (long)v;
    return 0;
}

int json_long_s2j(cJSON * item, const char * const name, long* in)
{
    if(item == NULL)
    {
        return -1;
    }

    cJSON * node = NULL;
    if(name == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, name);
    }

    if(cJSON_IsInvalid(node))
    {
        double v = *in; 
        node = cJSON_AddNumberToObject(item, name, v);
        if(cJSON_IsInvalid(node))
        {
            printf("create name failed\n");
            return -1;
        }
    }else
    {
        cJSON_SetNumberValue(node, *in);
        node->type = cJSON_Number;
    }

    return 0;
}

int json_float_j2s(const cJSON * const item, const char * const string, float* value)
{
    if(value == NULL || item == NULL)
    {
        return -1;
    }

    const cJSON * node = NULL;
    if(string == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, string);
    }
    
    if(cJSON_IsInvalid(node))
    {
        CFG_WARN("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    if(cJSON_IsNumber(node) == 0)
    {
        printf("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    double v = cJSON_GetNumberValue(node);
    *value = (float)v;
    return 0;
}


int json_float_s2j(cJSON * item, const char * const name, float* in)
{
    if(item == NULL)
    {
        return -1;
    }

    cJSON * node = NULL;
    if(name == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, name);
    }

    if(cJSON_IsInvalid(node))
    {
        double v = *in; 
        node = cJSON_AddNumberToObject(item, name, v);
        if(cJSON_IsInvalid(node))
        {
            printf("create name failed\n");
            return -1;
        }
    }else
    {
        cJSON_SetNumberValue(node, *in);
        node->type = cJSON_Number;
    }

    return 0;
}

int json_double_j2s(const cJSON * const item, const char * const string, double* value)
{
    if(value == NULL || item == NULL)
    {
        return -1;
    }

    const cJSON * node = NULL;
    if(string == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, string);
    }
    
    if(cJSON_IsInvalid(node))
    {
        CFG_WARN("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    if(cJSON_IsNumber(node) == 0)
    {
        printf("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    double v = cJSON_GetNumberValue(node);
    *value = v;
    return 0;
}

int json_double_s2j(cJSON * item, const char * const name, double* in)
{
    if(item == NULL)
    {
        return -1;
    }

    cJSON * node = NULL;
    if(name == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, name);
    }

    if(cJSON_IsInvalid(node))
    {
        double v = *in; 
        node = cJSON_AddNumberToObject(item, name, v);
        if(cJSON_IsInvalid(node))
        {
            printf("create name failed\n");
            return -1;
        }
    }else
    {
        cJSON_SetNumberValue(node, *in);
        node->type = cJSON_Number;
    }

    return 0;
}

int json_string_j2s(const cJSON * const item, const char * const string, char* value, int max)
{
    if(value == NULL || item == NULL || max <= 0)
    {
        return -1;
    }

    const cJSON * node = NULL;
    if(string == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, string);
    }

    if(cJSON_IsInvalid(node))
    {
        CFG_WARN("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    char* v = cJSON_GetStringValue(node);
    if(v == NULL)
    {
        printf("read:%s string failed\n", string);
        return -1;
    }
    
    strncpy(value, v, max - 1);
    value[max-1] = '\0';
    return 0;
}

int json_string_s2j(cJSON *item, const char * const name, char* in)
{
    if(in == NULL || item == NULL)
    {
        return -1;
    }

    cJSON * node = NULL;
    if(name == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, name);
    }

    if(cJSON_IsInvalid(node) || node == NULL)
    {
        node = cJSON_AddStringToObject(item, name, in);
        if(cJSON_IsInvalid(node))
        {
            printf("create name failed\n");
            return -1;
        }
    }else
    {
        if(node)
        cJSON_SetValuestring(node, in);
    }

    return 0;
}

int json_char_j2s(const cJSON * const item, const char * const string, char* value)
{
    if(value == NULL || item == NULL)
    {
        return -1;
    }

    const cJSON * node = NULL;
    if(string == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, string);
    }

    if(cJSON_IsInvalid(node))
    {
        CFG_WARN("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    if(cJSON_IsNumber(node) == 0)
    {
        printf("node isn't number\n");
        return -1;
    }
    
    double v = cJSON_GetNumberValue(node);
    *value = (char)v;
    
    return 0;
}

int json_char_s2j(cJSON *item, const char * const name, char* in)
{
    if(item == NULL)
    {
        return -1;
    }

    cJSON * node = NULL;
    if(name == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, name);
    }

    if(cJSON_IsInvalid(node))
    {
        double v = *in; 
        node = cJSON_AddNumberToObject(item, name, v);
        if(cJSON_IsInvalid(node))
        {
            printf("create name failed\n");
            return -1;
        }
    }else
    {
        cJSON_SetIntValue(node, *in);
        node->type = cJSON_Number;
    }

    return 0;
}

int json_bool_j2s(const cJSON * const item, const char * const string, bool* value)
{
    if(value == NULL || item == NULL)
    {
        return -1;
    }

    const cJSON * node = NULL;
    if(string == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, string);
    }

    if(cJSON_IsInvalid(node))
    {
        CFG_WARN("%s find node:%s failed\n", item->string, string);
        return -1;
    }

    if(cJSON_IsBool(node) == 0)
    {
        printf("%s: is not bool\n", string);
        return -1;
    }

    if(cJSON_IsTrue(node))
    {
        *value = true;
    }else
    {
        *value = false;
    }
    return 0;
}

int json_bool_s2j(cJSON *item, const char * const name, bool* in)
{
    if(item == NULL)
    {
        return -1;
    }

    cJSON * node = NULL;
    if(name == NULL)
    {
        node = item;
    }else
    {
        node = cJSON_GetObjectItem(item, name);
    }

    if(cJSON_IsInvalid(node))
    {
        node = cJSON_AddBoolToObject(item, name, *in);
        if(cJSON_IsInvalid(node))
        {
            printf("create name failed\n");
            return -1;
        }
    }else
    {
        //cJSON_ReplaceItemInObject(item, name, cJSON_CreateBool(*in));

        node->type = (*in == true)?cJSON_True:cJSON_False;
    }

    return 0;
}

int cfgPathGet(const char* cfg, char* path, int len)
{
    if(cfg == NULL || path == NULL)
    {
        return -1;
    }

    snprintf(path, len, "%s/%s.t", "config", cfg);
    return 0;
}


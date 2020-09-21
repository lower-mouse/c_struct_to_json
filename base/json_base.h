#ifndef __JSON_PARSE_BASE_H_
#define __JSON_PARSE_BASE_H_

#include "cJSON.h"
#include "stdio.h"

cJSON *json_file_parse(const char *Path);
int json_file_save(const cJSON *item, int fd, const char* path);

int json_int_j2s(const cJSON * const item, const char * const string, int* value);
int json_long_j2s(const cJSON * const item, const char * const string, long* value);
int json_float_j2s(const cJSON * const item, const char * const string, float* value);
int json_double_j2s(const cJSON * const item, const char * const string, double* value);
int json_string_j2s(const cJSON * const item, const char * const string, char* value, int max);
int json_char_j2s(const cJSON * const item, const char * const string, char* value);
int json_bool_j2s(const cJSON * const item, const char * const string, bool* value);


int json_bool_s2j(cJSON *item, const char * const name, bool* in);
int json_char_s2j(cJSON *item, const char * const name, char* in);
int json_string_s2j(cJSON *item, const char * const name, char* in);
int json_int_s2j(cJSON *item, const char * const name, int* in);
int json_long_s2j(cJSON * item, const char * const name, long* in);
int json_float_s2j(cJSON * item, const char * const name, float* in);
int json_double_s2j(cJSON * item, const char * const name, double* in);
int cfgPathGet(const char* cfg, char* path, int len);

#define CFG_OK            0
            
#define CFG_ERRNO_ERR     -1
#define CFG_ERRNO_TRANS   -2
#define CFG_ERRNO_PRINT   -3
#define CFG_ERRNO_PARSE   -4
#define CFG_ERRNO_NOT_FIND   -5
#define CFG_ERRNO_SAVE   -6


#define ASSERT_POINT(p) \
            do\
            {\
            if(NULL == p)\
                {\
                CFG_ERR("point error\n");\
                return -1;\
                }\
            }while(0)
#ifndef CFG_INFO       
#define CFG_INFO(fmt, arg...) \
        do\
        {\
                printf("\033[0;34m[%s:%d]:\033[0m",__func__,__LINE__);\
			    printf(fmt, ##arg);\
        }while(0)
#endif        

#define CFG_WARN(fmt, arg...) 
#ifndef CFG_WARN       
#define CFG_WARN(fmt, arg...) \
        do\
        {\
                printf("\033[0;33m[%s:%d]:\033[0m",__func__,__LINE__);\
                printf(fmt, ##arg);\
        }while(0)
#endif        

#ifndef CFG_ERR
#define CFG_ERR(fmt, arg...) \
            do\
            {\
                    printf("\033[0;31m[%s:%d]:\033[0m",__func__,__LINE__);\
                    printf(fmt, ##arg);\
            }while(0)
#endif            

#define array_j2s(item, type, name, out, max) \
do \
{\
    if(item == NULL)\
    {\
        break;\
    }\
    \
    cJSON *_node = NULL;\
    _node = cJSON_GetObjectItem(item, name);\
    if(cJSON_IsInvalid(_node))\
    {\
        CFG_ERR("find node failed\n");\
        break;\
    }\
\
    int array_size = cJSON_GetArraySize(_node);\
    for(int _i = 0; _i < array_size && _i < max; _i++)\
    {\
        cJSON *_array = cJSON_GetArrayItem(_node, _i);\
        type##_j2s(_array, NULL, &out[_i]);\
    }\
}while(0)

#define array_s2j(item, type, name, out, max) \
do\
{\
    ASSERT_POINT(item);\
    ASSERT_POINT(name);\
    ASSERT_POINT(out);\
\
    cJSON * _node = NULL;\
    _node = cJSON_GetObjectItem(item, name);\
    if(cJSON_IsInvalid(_node))\
    {\
        _node = cJSON_AddArrayToObject(item, name);\
    }\
\
    for(int _i = 0; _i < max; _i++)\
    {\
        cJSON * _array = NULL;\
        _array = cJSON_GetArrayItem(_node, _i);\
        if(_array == NULL)\
        {\
            _array = cJSON_CreateObject();\
            type##_s2j(_array, NULL, &out[_i]);\
            cJSON_AddItemToArray(_node, _array);\
        }else\
        {\
            type##_s2j(_array, NULL, &out[_i]);\
        }\
    }\
}while(0)

#define wrap_s2jstring(name, cfg, out, len, succ)  \
do\
{ \
    succ = CFG_ERRNO_ERR;\
    cJSON * _item = cJSON_CreateObject();\
    int _ret = json_##name##_s2j(_item, NULL, cfg);\
    if(_ret != 0){succ = CFG_ERRNO_TRANS; cJSON_Delete(_item); break;}\
\
    _ret = cJSON_PrintPreallocated(_item, out, len, 1);\
    if(_ret == 0){succ = CFG_ERRNO_PRINT; cJSON_Delete(_item); break;}\
\
    cJSON_Delete(_item);\
    succ = CFG_OK;\
}while(0)	

#define wrap_jstring2s(name, cfg, in, succ)  \
do\
{ \
    succ = CFG_ERRNO_ERR;\
    cJSON * _item = cJSON_Parse(in);\
    if(_item == NULL){succ = CFG_ERRNO_PARSE; cJSON_Delete(_item); break;}\
\
    int _ret = json_##name##_j2s(_item, NULL, cfg);\
    if(_ret != 0){succ = CFG_ERRNO_TRANS; cJSON_Delete(_item); break;}\
\
    cJSON_Delete(_item);\
    succ = CFG_OK;\
}while(0)	

#define wrap_s2jfile(name, cfg, file, succ)  \
do\
{ \
    succ = CFG_ERRNO_ERR;\
    cJSON * _item = json_file_parse(file);\
    if(_item == NULL)\
    {\
        _item = cJSON_CreateObject();\
        if(_item == NULL){succ = CFG_ERRNO_PARSE; break;}\
    }\
\
    int _ret = json_##name##_s2j(_item, #name, cfg);\
    if(_ret != 0){succ = CFG_ERRNO_TRANS; cJSON_Delete(_item); break;}\
\
    _ret = json_file_save(_item, -1, file);\
    if(_ret != 0){succ = CFG_ERRNO_SAVE; cJSON_Delete(_item); break;}\
\
    cJSON_Delete(_item);\
    succ = CFG_OK;\
}while(0)	

#define wrap_jfile2s(name, cfg, file, succ)  \
do\
{ \
    succ = CFG_ERRNO_ERR;\
    cJSON * _item = json_file_parse(file);\
    if(_item == NULL){succ = CFG_ERRNO_PARSE; break;}\
\
    int _ret = json_##name##_j2s(_item, #name, cfg);\
    if(_ret != 0){succ = CFG_ERRNO_TRANS; cJSON_Delete(_item); break;}\
\
    cJSON_Delete(_item);\
    succ = CFG_OK;\
}while(0)	

#endif

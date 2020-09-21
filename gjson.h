#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__
#include <unordered_map>
#include <list>
#include <vector>
#include <string>
#include "cJSON.h"
#include "string.h"

#define CFG_STRING_LEN 128

typedef struct child_node
{
    char name[CFG_STRING_LEN];
    char alias[CFG_STRING_LEN];
    char type[CFG_STRING_LEN];
    char description[CFG_STRING_LEN];
    int  max;
}child_node_t;

#define    NODE_ATTR_STRUCT   0x01
#define    NODE_ATTR_ENUM     0x02
#define    NODE_ATTR_DEFINE   0x04
#define    NODE_ATTR_SIMPLE   0x08
#define    NODE_ATTR_UNKOWN   0x10
//#define    NODE_ATTR_ROOT     0x10


typedef struct struct_node
{
    char type[CFG_STRING_LEN];
    char document[CFG_STRING_LEN];
    int  attr;//struct; enum; define
    std::vector<child_node_t> child_map;
}struct_node_t;

class translate
{
public:    
    translate(const char *path, const char *prefix, const char *name);
    ~translate();

    int serialization();
    int serialization_root(cJSON *node);
    int iscomplex(child_node_t* node);
    int ischar(child_node_t* node);
    int dump();
    int dump_root();
    int genum(struct_node_t node, std::ofstream* fout);
    int gstruct(struct_node_t node, std::ofstream* fout);
    int gjson2struct(struct_node_t* node, std::ofstream* fout);
    int gstruct2json(struct_node_t* node, std::ofstream* fout);
    int genum2json(struct_node_t* node, std::ofstream* fout);
    int gjson2enum(struct_node_t* node, std::ofstream* fout);
    int gheader_file(std::ofstream* fout);
    int gsource_file(std::ofstream* fout);
    int child_type_get(child_node_t* node);
    
    int check_paramter();
    void copy_base();
    int run();

    std::unordered_map<std::string, struct_node_t*> m_root;
    std::list<struct_node_t*> m_list;
    char m_path[CFG_STRING_LEN];
    char m_prefix[CFG_STRING_LEN];
    char m_out_name[CFG_STRING_LEN];
public:    
};

#endif

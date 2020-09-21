#include "gjson.h"
#include "json_base.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>    
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <unordered_map>
#include <string.h>
#include <ctype.h>
   
char *strToupper(char *string)
{
    char *p;
    p=string;
    while (*p!='\0')
    {
        if(islower(*p))    
         *p=toupper(*p);
        p++;
 }
 return string;
}

int json_attr_get(const cJSON * const item, int *attr)
{
    if(item == NULL || attr == NULL)
    {
        return -1;
    }
    *attr = NODE_ATTR_STRUCT; // default

    char *point = NULL;
    char* start = NULL;
    char tmp[CFG_STRING_LEN] = {0};
    char word[CFG_STRING_LEN] = {0};
    if(json_string_j2s(item, "attr", tmp, CFG_STRING_LEN) != 0)
    {
        CFG_ERR("%s get attr failed\n", item->string);
        return -1;
    }
    
    start = tmp;
    for(int i = 0; *start == ' ' && i < CFG_STRING_LEN; i++, start++); // jump leading space

    do
    {
        int len = 0;
        point = strchr(start, ' ');
        if(point)
        {
            len = point - start;
        }else
        {
            len = strlen(start);
        }

//        CFG_INFO("attr word:%s len:%d\n", word, len);
        if(len == 0)
        {
            break;
        }
        
        strncpy(word, start, len);
        word[len] = '\0';

        if(strcmp(word, "struct") == 0)
        {
            *attr = NODE_ATTR_STRUCT;
        }else if(strcmp(word, "enum") == 0)
        {
            *attr = NODE_ATTR_ENUM;

        }else if(strcmp(word, "define") == 0)
        {
            *attr = NODE_ATTR_DEFINE;

        }else
        {
            *attr = NODE_ATTR_UNKOWN;
            CFG_ERR("unrecognized string:%s\n", word);
        }
        
        start = point+1;
    }while(point);
    
    return 0;
}


int json_child_get(const cJSON * const item, std::vector<child_node_t> &child_map)
{
    if(item == NULL)
    {
        return -1;
    }
    
    cJSON *node = NULL;
    node = cJSON_GetObjectItem(item, "child");
    if(cJSON_IsInvalid(node))
    {
        CFG_ERR("find node failed\n");
        return -1;
    }

    int array_size = cJSON_GetArraySize(node);
    for(int i = 0; i < array_size; i++)
    {
        cJSON *array = cJSON_GetArrayItem(node, i);
        child_node_t child;
        memset(&child, 0, sizeof(child));

        if(json_string_j2s(array, "name", child.name, CFG_STRING_LEN) != 0)
        {
            CFG_ERR("type:%s don't find child name, continue this item\n", item->string);
            continue;
        }

        if(json_string_j2s(array, "alias", child.alias, CFG_STRING_LEN) != 0)
        {
            strncpy(child.alias, child.name, CFG_STRING_LEN -1);
        }
        
        json_string_j2s(array, "type", child.type, CFG_STRING_LEN);
        json_string_j2s(array, "description", child.description, CFG_STRING_LEN);
        json_int_j2s(array, "max", &child.max);

        child_map.push_back(child);
    }
    
    return 0;
}

int translate::child_type_get(child_node_t* node)
{
    if(node == NULL)
    {
        return -1;
    }

    if(iscomplex(node) == 0)
    {
        return NODE_ATTR_SIMPLE;
    }

    std::string s(node->type);

    std::unordered_map<std::string, struct_node_t*>::iterator it = m_root.find(s);
    if(m_root.end() != it)
    {
        //struct_node_t node = it->second;
        return it->second->attr;
    }
    

    return NODE_ATTR_UNKOWN;
}

int translate::iscomplex(child_node_t* node)
{
    if(node == NULL)
    {
        return 0;
    }
    
    if(strncmp(node->type, "char", sizeof(node->type)) == 0)
    {
        return 0;
    }
    else if(strncmp(node->type, "int", sizeof(node->type)) == 0)
    {
        return 0;
    }
    else if(strncmp(node->type, "float", sizeof(node->type)) == 0)
    {
        return 0;
    }
    else if(strncmp(node->type, "double", sizeof(node->type)) == 0)
    {
        return 0;
    }    
    else if(strncmp(node->type, "bool", sizeof(node->type)) == 0)
    {
        return 0;
    }
    else if(strncmp(node->type, "long", sizeof(node->type)) == 0)
    {
        return 0;
    }
    
    return 1;
}

int translate::ischar(child_node_t* node)
{
    if(node == NULL )
    {
        return 0;
    }

    if(strncmp(node->type, "char", sizeof(node->type)))
    {
        return 0;
    }

    return 1;
}

translate::translate(const char *configPath, const char *prefix, const char *name)
{
    if(configPath != NULL)
    {
        memset(m_path, 0, sizeof(m_path));
        strncpy(m_path, configPath, sizeof(m_path)-1);
    }
    
    if(prefix != NULL)
    {
        memset(m_prefix, 0, sizeof(m_prefix));
        strncpy(m_prefix, prefix, sizeof(m_prefix)-1);
    }
    
    if(name != NULL)
    {
        memset(m_out_name, 0, sizeof(m_out_name));
        strncpy(m_out_name, name, sizeof(m_out_name)-1);
    }

    m_root.clear();
    m_list.clear();
}

translate::~translate()
{
    for(std::list<struct_node_t*>::iterator it = m_list.begin(); it != m_list.end(); it++)
    {
        struct_node_t* node = *it;
        if(node)
        {
            free(node);
        }
    }

    m_list.clear();
    m_root.clear();
}

int translate::serialization()
{
    cJSON *root = json_file_parse(m_path);
    ASSERT_POINT(root);

    cJSON *node = root->child;
    ASSERT_POINT(node);

    for(cJSON *it = node; it != NULL; it = it->next)
    {
        serialization_root(it);
    }

    cJSON_Delete(root);
    return 0;
}

int translate::serialization_root(cJSON *node)
{
    ASSERT_POINT(node);
    struct_node_t* st = new struct_node_t;
    if(st == NULL)
    {
        CFG_ERR("malloc failed\n");
        return -1;
    }

    memset(st, 0, sizeof(struct_node_t));
    CFG_INFO("parse string type:(%s)\n", node->string);
    
    snprintf(st->type, CFG_STRING_LEN, "%s", node->string);
    json_string_j2s(node, "document", st->document, CFG_STRING_LEN);        
        
    if(json_child_get(node, st->child_map) != 0)
    {
        CFG_ERR("json_child_get failed\n");
        delete st;
        return -1;
    }
    
    if(json_attr_get(node, &st->attr) != 0)
    {
        CFG_ERR("json_attr_get failed\n");
        delete st;
        return -1;
    }

    std::string s(st->type);
    if(m_root.end() != m_root.find(s)) //is exist
    {
        CFG_ERR("%s:dumplate, please check json file!!!\n", st->type);
        delete st;
        return -1;
    }
    
    m_root.insert(std::pair<std::string, struct_node_t*>(s, st));
    m_list.push_front(st);
    return 0;
}

int translate::dump()
{
    for(std::list<struct_node_t*>::iterator it = m_list.begin(); it != m_list.end(); it++)
    {
        struct_node_t* node = *it;
        printf("node type:%s==============================================\n", node->type);
        printf("node attr:%d\n", node->attr);
        printf("node document:%s\n", node->document);
        for(std::vector<child_node_t>::iterator child = node->child_map.begin(); child != node->child_map.end(); child++)
        {
            printf("child type:%-18s", (*child).type);
            printf("name:%-18s", (*child).name);
            printf("alias:%-18s", (*child).alias);
            printf("max:%-8d\n", (*child).max);
        }
    }

    return 0;
}

int translate::run()
{
    if(check_paramter() != 0)
    {
        CFG_ERR("check paramter failed\n");
        return -1;
    }

    serialization();
    copy_base();

    char tmp[256];
    snprintf(tmp, 256, "%s/include/%s.h", m_prefix, m_out_name);
    std::ofstream fout;
    fout.open(tmp, std::ios_base::out | std::ios_base::trunc);
    gheader_file(&fout);
    fout.close();

    snprintf(tmp, 256, "%s/%s.cpp", m_prefix, m_out_name);
    std::ofstream fsrc_out;
    fsrc_out.open(tmp, std::ios_base::out | std::ios_base::trunc);
    gsource_file(&fsrc_out);
    fsrc_out.close();

    return 0;
}

int translate::check_paramter()
{
    char tmp[256];
    if(access(m_path, F_OK) != 0)
    {
        CFG_ERR("%s is not exist\n", m_path);
        return -1;
    }
        
    if(access(m_prefix, F_OK) != 0)
    {
        CFG_ERR("%s is not exist\n", m_prefix);
        return -1;
    }

    snprintf(tmp, 256, "%s/include", m_prefix);
    if(access(tmp, F_OK) != 0)
    {
        CFG_ERR("%s is not exist, create this directory\n", tmp);
        if(mkdir(tmp, S_IRWXU|S_IRWXG|S_IRWXO) != 0)
        {
            CFG_ERR("create file:%s failed, errno:0x%x\n", m_prefix, errno);
            perror("mkdir failed");
            return -1;
        }
    }

    return 0;
}

void translate::copy_base()
{
    char tmp[256];
    snprintf(tmp, 256, "cp base/*.c base/*.cpp %s/", m_prefix);
    system(tmp);
    snprintf(tmp, 256, "cp base/*.h %s/include/", m_prefix);
    system(tmp);

    return ;
}

int translate::gstruct(struct_node_t node, std::ofstream* fout)
{
    if(fout->is_open() == false)
    {
        CFG_ERR("file not open\n");
        return -1;
    }
    
    std::ostringstream  ostr;
    ostr << "\n\n//" << node.document << std::endl;
    ostr << "typedef struct _"<< node.type << "_t{\n";
    for(std::vector<child_node_t>::iterator childit = node.child_map.begin(); childit != node.child_map.end(); childit++)
    {
        child_node_t child = *childit;
        int type = child_type_get(&child);
        if(type & NODE_ATTR_STRUCT)
        {
            ostr << "\tstruct _" <<(*childit).type << "_t";           // out  variable
        }else if(type & NODE_ATTR_SIMPLE)
        {
            ostr << "\t" <<(*childit).type;           // out  variable
        }else if(type & NODE_ATTR_ENUM)
        {
            ostr << "\t" <<(*childit).type << "_e";           // out  variable
        }else if(type & NODE_ATTR_UNKOWN)
        {
            CFG_ERR("struct %s not find child_type:%s name:%s\n", node.type, child.type, child.name);
            continue;
        }else
        {
            CFG_ERR("type illegal struct %s not find child_type:%s name:%s\n", node.type, child.type, child.name);
            continue;
        }
        
        ostr << "  " <<(*childit).name;           // out  name
        if((*childit).max > 1)
        {
            ostr <<"[" << (*childit).max << "]";  // array out []
        }
        
        ostr << ";";                           // out ';'
        if(strlen((*childit).description) > 0)   // out variable description
        {
            ostr << "  //" << (*childit).description;
        }
    
        ostr << std::endl;            
    }
    ostr << "}" << node.type << "_t;\n";          // out }struct_name;
    
    fout->write(ostr.str().c_str(), ostr.str().length());

}

int translate::genum(struct_node_t node, std::ofstream* fout)
{
    if(fout->is_open() == false)
    {
        CFG_ERR("file not open\n");
        return -1;
    }
    
    std::ostringstream  ostr;
    ostr << "\n\n//" << node.document << std::endl;
    ostr << "typedef enum _"<< node.type << "_e{\n";
    for(std::vector<child_node_t>::iterator child = node.child_map.begin(); child != node.child_map.end(); child++)
    {
        ostr << "\t" <<(*child).name << ",";           // out  variable

        if(strlen((*child).description) > 0)   // out variable description
        {
            ostr << "  //" << (*child).description;
        }
    
        ostr << std::endl;            
    }
    ostr << "}" << node.type << "_e;\n";          // out }struct_name;
    
    fout->write(ostr.str().c_str(), ostr.str().length());
}

int translate::gheader_file(std::ofstream* fout)
{
    if(fout->is_open() == false)
    {
        CFG_ERR("file not open\n");
        return -1;
    }
    
    char define_str[128];
    snprintf(define_str, 128, "%s", m_out_name);
    strToupper(define_str);
    
    std::ostringstream  ostr;
    ostr << "#ifndef __" << define_str << "_H__\n";
    ostr << "#define __" << define_str << "_H__\n";
    ostr << "#include \"cJSON.h\"\n";
    fout->write(ostr.str().c_str(), ostr.str().length());
    ostr.seekp(0, std::ios_base::beg);
    
    for(std::list<struct_node_t*>::iterator it = m_list.begin(); it != m_list.end(); it++)
    {
        struct_node_t node = *(*it);
        if(node.attr & NODE_ATTR_STRUCT)
        {
            gstruct(node, fout);
        }else if(node.attr & NODE_ATTR_ENUM)
        {
            genum(node, fout);
        }else if(node.attr & NODE_ATTR_DEFINE)
        {
            
        }else
        {

        }
    }
    
    for(std::list<struct_node_t*>::iterator it = m_list.begin(); it != m_list.end(); it++)
    {
        struct_node_t node = *(*it);
        if(node.attr & NODE_ATTR_STRUCT)
        {
            ostr << "int json_" << node.type << "_j2s(const cJSON * item, const char * const string, " << node.type << "_t *cfg);\n";
            ostr << "int json_" << node.type << "_s2j(cJSON * item, const char * const string, " << node.type << "_t *cfg);\n";
        }else if(node.attr & NODE_ATTR_ENUM)
        {
            ostr << "int json_" << node.type << "_j2s(const cJSON * item, const char * const string, " << node.type << "_e *cfg);\n";
            ostr << "int json_" << node.type << "_s2j(cJSON * item, const char * const string, " << node.type << "_e *cfg);\n";
        }
    }

    ostr << "\n#endif //\n";
    fout->write(ostr.str().c_str(), ostr.str().length());
    return 0;
}

int translate::gsource_file(std::ofstream* fout)
{
    if(fout->is_open() == false)
    {
        CFG_ERR("file not open\n");
        return -1;
    }
    
    std::ostringstream  ostr;
    ostr<<"#include \"" << m_out_name  << ".h\" \n";
    ostr<<"#include \"json_base.h\" \n";
    ostr<<"#include \"cJSON.h\" \n";
    ostr<<"\n";
    
    fout->write(ostr.str().c_str(), ostr.str().length());

    for(std::list<struct_node_t*>::iterator it = m_list.begin(); it != m_list.end(); it++)
    {
        struct_node_t* node = (*it);
        if(node->attr & NODE_ATTR_STRUCT)
        {
            gjson2struct(node, fout);
            gstruct2json(node, fout);
        }else if(node->attr & NODE_ATTR_ENUM)
        {
            genum2json(node, fout);
            gjson2enum(node, fout);
        }else if(node->attr & NODE_ATTR_UNKOWN)
        {
            CFG_ERR("node type:%s attr:%d type unrecognized\n", node->type, node->attr);
        }
    }
    return 0;
}

int translate::gjson2struct(struct_node_t* node, std::ofstream* fout)
{
    if(node == NULL)
    {
        return -1;
    }
    
    if(fout->is_open() == false)
    {
        CFG_ERR("file not open\n");
        return -1;
    }

    std::ostringstream  ostr;
    ostr << "int json_" << node->type << "_j2s(const cJSON * node, const char * const string, " << node->type << "_t *cfg)\n";
    ostr << "{\n";
    ostr << "\tASSERT_POINT(node);\n";
    ostr << "\tASSERT_POINT(cfg);\n";
    ostr << "\tconst cJSON * item = node;\n\n";
    ostr << "\tif(string)\n\
    {\n\
        item = cJSON_GetObjectItem(node, string);\n\
        if(cJSON_IsInvalid(item))\n\
        {\n\
            CFG_ERR(\"%s find node:%s failed\\n\", node->string, string);\n\
            return -1;\n\
        }\n\
    }\n\n";

    for(std::vector<child_node_t>::iterator it = node->child_map.begin(); it != node->child_map.end(); it++)
    {
        child_node_t child = *it;
        int type = child_type_get(&child);
        if(type == NODE_ATTR_UNKOWN)
        {
            CFG_ERR("struct %s not find child_type:%s name:%s\n", node->type, child.type, child.name);
            continue;
        }
        
        if(child.max > 1 && 0 == ischar(&child))
        {
            ostr << "\tarray_j2s(item, " << "json_" << child.type << "," << "\"" << child.alias << "\"" << "," << "cfg->" << child.name << ", " << child.max <<");\n";
        }else if(child.max > 1)
        {
            ostr << "\tjson_string_j2s(item, \"" << child.alias << "\", cfg->" << child.name << ", " << child.max << ");\n";
            //string
        }else
        {
            //non array
            ostr << "\tjson_" <<child.type << "_j2s(item, \"" << child.alias << "\", &cfg->" << child.name << ");\n";
        }        
    }
    ostr << "\treturn 0;\n";
    ostr << "}\n\n";

    fout->write(ostr.str().c_str(), ostr.str().length());
    return 0;
}

int translate::gstruct2json(struct_node_t* node, std::ofstream* fout)
{
    if(node == NULL)
    {
        return -1;
    }
    
    if(fout->is_open() == false)
    {
        CFG_ERR("file not open\n");
        return -1;
    }

    char tmpbuf[1024];
    snprintf(tmpbuf, 1024, "int json_%s_s2j(cJSON * item, const char * const name, %s_t *cfg)\n \
{\n\
    ASSERT_POINT(item);\n\
    ASSERT_POINT(cfg);\n\
    \n\
    cJSON * node = NULL;\n\
    if(name == NULL)\n\
    {\n\
        node = item;\n\
    }else\n\
    {\n\
        node = cJSON_GetObjectItem(item, name);\n\
    }\n\
    \n\
    if(cJSON_IsInvalid(node))\n\
    {\n\
        node = cJSON_AddObjectToObject(item, name);\n\
        if(cJSON_IsInvalid(node))\n\
        {\n\
            CFG_ERR(\"create name failed\\n\");\n\
            return -1;\n\
        }\n\
    }\n\n", node->type, node->type);
    fout->write(tmpbuf, strlen(tmpbuf));
      
    for(std::vector<child_node_t>::iterator it = node->child_map.begin(); it != node->child_map.end(); it++)
    {
        child_node_t child = *it;
        int type = child_type_get(&child);
        if(type == NODE_ATTR_UNKOWN)
        {
            CFG_ERR("struct %s not find child_type:%s name:%s\n", node->type, child.type, child.name);
            continue;
        }
        
        if(child.max > 1 && 0 == ischar(&child))
        {
            //array
            snprintf(tmpbuf, 1024, "\tarray_s2j(node, json_%s, \"%s\", cfg->%s, %d);\n", child.type, child.alias, child.name, child.max);
        }else if(child.max > 1)
        {            
            //string
            snprintf(tmpbuf, 1024, "\tjson_string_s2j(node, \"%s\", cfg->%s);\n",child.alias, child.name);
        }else
        {
            snprintf(tmpbuf, 1024, "\tjson_%s_s2j(node, \"%s\", &cfg->%s);\n", child.type, child.alias, child.name);
        }  
        
        fout->write(tmpbuf, strlen(tmpbuf));
    }

    snprintf(tmpbuf, 1024, "\treturn 0;\n}\n\n");
    fout->write(tmpbuf, strlen(tmpbuf));
    return 0;
}

int translate::genum2json(struct_node_t* node, std::ofstream* fout)
{
    if(node == NULL)
    {
        return -1;
    }
    
    if(fout->is_open() == false)
    {
        CFG_ERR("file not open\n");
        return -1;
    }

    char tmpbuf[1024];
    snprintf(tmpbuf, 1024, "int json_%s_s2j(cJSON * item, const char * const name, %s_e *cfg)\n\
{\n\
    ASSERT_POINT(item);\n\
    ASSERT_POINT(cfg);\n\
    \n\
    return json_int_s2j(item, name, (int*)cfg);\n\
}\n\n", node->type, node->type);

    fout->write(tmpbuf, strlen(tmpbuf));
    return 0;
}

int translate::gjson2enum(struct_node_t* node, std::ofstream* fout)
{
    if(node == NULL)
    {
        return -1;
    }
    
    if(fout->is_open() == false)
    {
        CFG_ERR("file not open\n");
        return -1;
    }

    char tmpbuf[1024];
    snprintf(tmpbuf, 1024, "int json_%s_j2s(const cJSON * item, const char * const name, %s_e *cfg)\n\
{\n\
    ASSERT_POINT(item);\n\
    ASSERT_POINT(cfg);\n\
    \n\
    return json_int_j2s(item, name, (int*)cfg);\n\
}\n\n", node->type, node->type);

    fout->write(tmpbuf, strlen(tmpbuf));
    return 0;
}

int help()
{
    printf("\ngjson:usage: gjson [-p prefix] [-c config] [-o out_file] [-h] [-v]\n");
    printf(" -p 指定输出目录, .h文件存放在prefix/include目录下\n");
    printf(" -c 指定c struct 描述文件路径\n");
    printf(" -o 输出文件名前缀\n");
    printf(" -h 输出帮助信息\n");
    printf(" -v 输出详细信息\n");
    
    return 0;
}

int main(int argc,char *argv[])       
{      
	int ch;       
	opterr = 0; 

    int dump_info = 0;
    int gtranslate = 0;
    char prefix[128] = {0};
    char gtranslate_path[128] = {0};
    char config_path[128] = {0};
    strncpy(prefix, ".", 127);
    
	while((ch = getopt(argc,argv,"p:n:b:o:c:hv"))!=-1)       
	{       
//		printf("optind:%d\n",optind);       
//		printf("optarg:%s\n",optarg);       
//		printf("ch:%c\n",ch);       
		switch(ch)     
		{       
			case 'p': // specify prefix, default ./       
                strncpy(prefix, optarg, 127);
				printf("prefix:%s\n",prefix);       
				break;       
			case 'o': ////指定json 和 struct 相互转换的代码存放的文件名    
			    gtranslate = 1;
                strncpy(gtranslate_path, optarg, 127);
				printf("gtranslate_path:%s\n",gtranslate_path);       
				break; 
            case 'c': //配置文件存放位置 
                strncpy(config_path, optarg, 127);
                printf("config_path:%s\n",config_path); 
                break;
            case 'h':
                help();
                break;
            case 'v':
                dump_info = 1;
                break;
			default:       
				printf("Unknown option:%d\n",ch);
                return -1;
		}
	}

    if(gtranslate)
    {
        translate tt(config_path, prefix, gtranslate_path);
        tt.run();
        (dump_info)?tt.dump():0;        
    }
	return 0;
}



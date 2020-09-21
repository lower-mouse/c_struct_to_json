## gjson

### 简介

gjson 是一个 c  结构体与 json 格式文件相互转换的**代码生成器**，通过一个 c  结构体的描述文件，gjson可以生成对应的头文件和对应的cpp代码，这个想法是来自 gsoap, 通过wsdl 文件生成相应的struct 与 XML 相互转换的代码。

### 编译
```
cd c_struct_to_json/
make
```
### 运行
执行完下面的命令后， 在out目录下就有生成的文件了
```
./gjson -h  //显示帮助信息
./gjson -p out/ -c config/test.txt -o test //test.txt 是一个 demo 配置文件
```

### 结构体描述文件
下面的json就是 config/test.txt 中的内容， 通过结构体描述文件我们可以设定struct 与 json之间的转换规则。
```json
"testCfg":{
	"attr":"struct",
	"document":"this is a osd config",
	"child":[
		{"type":"int", "name":"number", "alias":"test number", "max":1, "description":"this is a number"},
		{"type":"int", "name":"array", "alias":"test array", "max":5},
		{"type":"char", "name":"c", "max":1},
		{"type":"char", "name":"string", "max":128},
		{"type":"timeFormat", "name":"timeFormat"},
		{"type":"position", "name":"positionArray", "max":4},
		{"type":"position", "name":"structure"}
	]	
},
"position":{
	"attr":"struct",
	"document":"description osd position",
	"child":[
		{"type":"int","name":"x","max":1,"description":"this is x position of osd"},
		{"type":"int","name":"y","max":1}
	]
},
"timeFormat":
{
	"attr":"enum",
	"document":"description enum format",
	"child":[
		{"name":"TIME_YY_MM_DD", "description":"time format year--month--day"},
		{"name":"TIME_DD_MM_YY"}
		]
}
```
上面的是一个 json 格式的描述文件, 每一个 json 对象代表一个结构体或者枚举, 它描述了 2 个结构体和 1 个枚举类型， 下面两个表是介绍 json 对象和和 child 中字段的含义。
字段      |描述
----------|---------
attr      |用来描述是结构体还是枚举，有 struct, enum 两种选择
document  |这个字段会作为注释放在结构体上方
child     |结构体中的成员

字段      |描述
----------|---------
type      |结构体成员的类型， 支持char int float double long bool 这几种基本类型, 如果是结构体类型需要有这种类型的描述(枚举类型不需要填这个字段)
name  	  |成员名 (不能包含空格)
alias     |结构体成员变量转换成 json 对象的名字， (不填则采用成员名)
max       |max 大于1, 则认为是数组(可不填)
description    |结构体成员的注释(可不填)

**note:** 结构体描述文件中的json对象的顺序可要注意， 在有依赖的情况下，被依赖的结构体描述应该放在下面， 例如在 testCfg 的成员中有 position 对象， 所以 position 的定义需要在 testCfg 下方。

### 测试生成的代码
在执行上面的命令后，就生成好了代码，在out目录下 testMain.cpp 的文件中, 有一个怎么使用自动化代码的 demo , 如果没有改 config/test.txt 文件， 可以直接 make 编译通过， 然后运行

下面的结构体是根据配置文件生成的， 可以看到结构体成员类型和成员名都与我们设定的一致
```
//this is a osd config
typedef struct _testCfg_t{
	int  number;  //this is a number
	int  array[5];
	char  c;
	char  string[128];
	timeFormat_e  timeFormat;
	struct _position_t  positionArray[4];
	struct _position_t  structure;
}testCfg_t;
```

下面的json就是通过我们自动生成的代码生成的， 里面的数据是demo中预填的
```json
{
	"test number":	2,
	"test array":	[1, 1, 1, 0, 0],
	"c":	97,
	"string":	"hello world!",
	"timeFormat":	1,
	"positionArray":	[{
			"x":	100,
			"y":	200
		}, {
			"x":	50,
			"y":	150
		}, {
			"x":	0,
			"y":	0
		}, {
			"x":	0,
			"y":	0
		}],
	"structure":	{
		"x":	100,
		"y":	200
	}
}
```
### 怎么使用代码
我们使用这个工具的目的就是把**我们定义的结构体**很方便的转成**我们期待的json字符串**或者json文件， 在定义好结构体描述文件，生成好对应的代码，只需要**一行代码**就可以完成我们的需求。
```
int succ = 0;
int bufLenght = 1024; 
char buf[bufLenght];
testCfg_t test;
memset(&test, 0, sizeof(test));

wrap_s2jstring(testCfg, &test, buf, bufLenght, ret);
```
**wrap_s2jstring** 是一个宏， 将结构体转换成 json 字符串
第一个参数：是需要转换结构体的类型(去掉结尾的 _t 就是, 注意**不需要加" "**)
第二个参数：是需要转换结构体的指针
第三个参数：输出 json 字符串buffer
第四个参数：buffer 长度
第五个参数：返回值， 成功为 0, 失败可以参考错误码
```
wrap_jstring2s(name, cfg, in, succ)
```
**wrap_jstring2s** 是一个宏， 将 json 字符串解析到结构体

```
wrap_s2jfile(name, cfg, file, succ)
```
**wrap_s2jfile** 是一个宏， 将结构体转换成 json 文件
```
wrap_jfile2s(name, cfg, file, succ)
```
**wrap_jfile2s** 是一个宏， 将 json 文件解析到结构体

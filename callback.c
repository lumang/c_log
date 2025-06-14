#include<stdio.h>
// 函数指针
typedef int (*callback)(int, int);// 函数指针类型

typedef struct 
{
    char  *name;
    callback func;
}Fun;

int add(int a, int b)
{
	return a+b;
}

int sub(int a, int b)
{
	return a-b;
}

int main()
{
	Fun funs[] = {
		{"add", add},
		{"sub", sub},
	};
    Fun funs2[]={
        {"add",add},
        {"sub",sub}
    };
    Fun *fun = funs;
	int a = 10;
	int b = 20;
	int i = 0;
	for(i = 0; i < sizeof(funs)/sizeof(funs[0]); i++)
	{
	    printf("%s: %d\n", funs[i].name, funs[i].func(a, b));
	}

    for(i = 0; i < sizeof(funs)/sizeof(funs[0]); i++)
    {
        printf("%s: %d\n",fun[i].name,fun[i].func(a,b));
    }
}
#include<stdio.h>
#include"../include/pm_ehash.h"
#include<string.h>

PmEHash* ehash;
char s[110000],a[110000];
uint64_t key,value;
int len;

/*@读入一个64位无符号整数 成功返回1 失败返回0*/
bool getnum(uint64_t &num){
    if (scanf("%s",a)!=1) return 0;
    len=strlen(a);
    num=0;
    for(int i=0;i<len;i++){
        if (a[i]>='0'&&a[i]<='9'){
            num=num*10+(a[i]-'0');
        }else return 0;
    }
    return 1;
}
void hint(){
    printf("\nPlease enter a character and some operands.\nI %%llu %%llu: Insert key value\nS %%llu: Search key\nU %%llu %%llu: Update key value\nR %%llu: Remove key\nC: Clean all data files.\nE: End.\n\n");
}
int main(){

	ehash=new PmEHash;

    hint();
    while (1){
        if (scanf("%s",s)==1){

            if (s[0]=='E'||s[0]=='e') break;

            if (s[0]=='C'||s[0]=='c'){
                ehash->clean();
                putchar('\n');
                continue;
            }

            if (s[0]=='I'||s[0]=='i'){
                if (!getnum(key)){
                    hint();
                    continue;
                }
                if (!getnum(value)){
                    hint();
                    continue;
                }
               
                if (ehash->insert((kv){key,value})==0)
                    printf("Insert succeeded.\n");
                else
                    printf("Insert failed: The same key exists.\n");
                
                continue;
            }

            if (s[0]=='S'||s[0]=='s'){
                if (!getnum(key)){
                    hint();
                    continue;
                }

                if (ehash->search(key,value)==0)
                    printf("Search succeeded: The kv_pair(%lu,%lu) exists.\n",key,value);
                else
                    printf("Search failed: the key does not exist.\n");

                continue;
            }

            if (s[0]=='U'||s[0]=='u'){
                if (!getnum(key)){
                    hint();
                    continue;
                }
                if (!getnum(value)){
                    hint();
                    continue;
                }

                if (ehash->update((kv){key,value})==0)
                    printf("Update succeeded.\n");
                else
                    printf("Update failed: the key does not exist.\n");

                continue;
            }

            if (s[0]=='R'||s[0]=='r'){
                if (!getnum(key)){
                    hint();
                    continue;
                }

                if (ehash->remove(key)==0)
                    printf("Remove succeeded.\n");
                else
                    printf("Remove failed: the key does not exist.\n");

                continue;
            }
            hint();
        }else break;
    }

    delete ehash;
    
	return 0;
}
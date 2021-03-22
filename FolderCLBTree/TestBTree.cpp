//测试程序2  
#include <iostream>  
#include <cstdlib>  
#include <ctime>  
#include "CLBTree.h"  
using namespace std;  
  
int main()  
{     
    srand((int)time(0));  
    const int iSize = 100000;  //插入次数   
    const int dSize = 100000;  //删除次数  
    const int num = 10;       //测试组数  
    int *iKey = new int[iSize];  
    int *dKey = new int[dSize];   
    int i, j;  
    for(j = 0; j < num; j++)  //测试组数，每次测试都是插入iSize次，删除dSize次  
    {  
        for(i = 0; i < iSize; i++)  //插入数据生成  
            iKey[i] = rand()%iSize;  
        for(i = 0; i < dSize; i++)  
            dKey[i] = rand()%iSize; //删除数据生成  
  
        int m = rand()%400 + 3;  //随机生成3阶到402阶  
        BTree<int> btree(m, NULL);      
        cout<<"----------第"<<j<<"组插入测试----------"<<endl;  
        for(i = 0; i < iSize; i++) //插入测试  
            btree.Insert(iKey[i]);  
        cout<<"第"<<j<<"组插入测试成功，为"<<m<<"阶B树"<<endl;  
        cout<<"----------第"<<j<<"组删除测试----------"<<endl;  
        for(i = 0; i < dSize; i++) //删除测试  
            btree.Delete(dKey[i]);  
        cout<<"第"<<j<<"组删除测试成功，为"<<m<<"阶B树"<<endl<<endl;  
    }  
    delete [] iKey;  
    delete [] dKey;  
    return 0;  
}
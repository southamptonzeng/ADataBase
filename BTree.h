#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>         
#include<stdlib.h>
#include<time.h>  
#define BTREELENGTH 50
#define BTLEN (sizeof(BTNode))
#define MAXINT 100  
typedef enum status
{
    TRUE,
    FALSE,
    OK,
    ERROR,
    OVERFLOW,
    EMPTY
}Status;
typedef unsigned long KeyType;

//**********************************B树****************************************
#define  m  3   // B树的阶，此设为4
typedef struct 
{
    KeyType  key;
    unsigned long data;
} Record;
typedef struct BTNode 
{
    int             keynum;        // 结点中关键字个数，即结点的大小
    struct BTNode  *parent;        // 指向双亲结点
    KeyType         key[m + 1];      // 关键字向量，0号单元未用
    struct BTNode  *ptr[m + 1];      // 子树指针向量
//  Record         *recptr[m + 1];   // 记录指针向量，0号单元未用
                                     //在此添加其他自定义数据
} BTNode, *BTree;                // B树结点和B树的类型
typedef struct 
{
    BTNode  *pt;      // 指向找到的结点
    int      i;       // 1..m，在结点中的关键字序号
    int      tag;     // 1:查找成功，0:查找失败
} Result;           // 在B树的查找结果类型   
//**********************************B树****************************************

//**********************************队列***************************************
typedef struct LNode {
    BTree data;     // 数据域
    struct LNode *next;     // 指针域
} LNode, *LinkList;
//**********************************队列***************************************

/***
*  @name           Status InitQueue_L(LinkList &L)
*  @description    初始化队列
*  @return         成功返回OK，开辟空间失败返回OVERFLOW
*  @notice
***/
Status InitQueue_L(LinkList &L);
/***
*  @name           LNode* MakeNode_L(BTree e)
*  @description    构造队列结点
*  @return         返回结点地址
*  @notice
***/
LNode* MakeNode_L(BTree e);
/***
*  @name           Status Enqueue_L(LNode *p, BTree e)
*  @description    队列的入队
*  @return         成功返回OK，否则返回ERROR
*  @notice
***/
Status Enqueue_L(LNode *p, BTree e);
/***
*  @name           Status Dequeue_L(LNode *p, BTree &e)
*  @description    队列的出队
*  @return         成功返回OK，否则返回ERROR
*  @notice
***/
Status Dequeue_L(LNode *p, BTree &e);
/***
*  @name           void DestroyQueue(LinkList L)
*  @description    队列的销毁
*  @return         无返回
*  @notice
***/
void DestroyQueue(LinkList L);
/***
*  @name           Status  IfEmpty(LinkList L)
*  @description    判断队列是否为空
*  @return         空返回TRUE，不空返回FALSE，否则返回ERROR
*  @notice         
***/
Status  IfEmpty(LinkList L);
/***
*  @name           Status ergodic(BTree T, LinkList L, int newline, int sum)
*  @description    print需要用到的递归遍历程序
*  @return         成功返回OK
*  @notice         此处用到队列
***/
Status ergodic(BTree T, LinkList L, int newline, int sum);
/***
*  @name           Status print(BTree T)
*  @description    层次遍历并分层输出B树
*  @return         成功返回OK
*  @notice
***/
Status print(BTree T);
/***
*  @name           Status findMax(BTree T, BTree &p,int ans)
*  @description    寻找最大关键字的结点,T为要寻找的树，p为返回的节点,ans为第几个
*  @return         成功返回OK,否则返回ERROR
*  @notice
***/
Status findMax(BTree T, BTree &p, int &ans);
/***
*  @name           Status findMin(BTree T, BTree &p,int ans)
*  @description    寻找最小关键字的结点,T为要寻找的树，p为返回的节点,ans为第几个
*  @return         成功返回OK,否则返回ERROR
*  @notice
***/
/***
*  @name           Status findBTree(BTree T, BTree &p, int &ans, KeyType k)
*  @description    寻找 ,T为要寻找的树，p为返回的节点，ans为第几个元素，k为要找的值
*  @return         成功返回OK,否则返回ERROR
*  @notice
***/
Status findBTree(BTree T, BTree &p, int &ans, KeyType k);
/***
*  @name           Status renewParent(BTree p)
*  @description    告诉孩子们亲身爸爸是谁
*  @return         成功返回OK,否则返回ERROR
*  @notice
***/
Status renewParent(BTree p);
/***
*  @name           int whichSon(BTree T)
*  @description    找出是父亲的第几个孩子
*  @return         成功返回第几个孩子,否则返回-1
*  @notice
***/
int whichSon(BTree T);
/***
*  @name           status splitBTree(BTree T)
*  @description    递归实现分裂节点操作
*  @return         成功返回OK,否则返回ERROR
*  @notice
***/
Status splitBTree(BTree T); //此时分裂的节点一定会是超出最大值的。
/***
*  @name           Status insertBTree(BTree &T, Record e)
*  @description    插入实现元素的插入
*  @return         成功返回OK,如果存在则返回FALSE，否则返回ERROR
*  @notice
***/
Status insertBTree(BTree &T, Record e);
/***
*  @name           Status borrowBNode(BTree &T)
*  @description    递归实现，向兄弟借元素，否则和兄弟合并
*  @return         成功返回OK,否则返回ERROR
*  @notice         这种情况应该是T为单元素结点
***/
Status borrowBNode(BTree T);
/***
*  @name           Status deleteBTreeRecord(BTree &T, Record e)
*  @description    实现B树元素的删除
*  @return         成功返回OK,否则返回ERROR
*  @notice
***/
Status deleteBTreeRecord(BTree &T, Record e);
/***
*  @name           Status initBTree(BTree &t)
*  @description    初始化一个空B树
*  @return         成功返回OK
*  @notice
***/
Status initBTree(BTree &t);
/***
*  @name           Status test()
*  @description    针对数据结构实验做的测试函数
*  @return         成功返回OK
*  @notice
***/
Status test();
/***
主函数
***/
/*
int  main()
{
    test();
    return 0;
}
*/
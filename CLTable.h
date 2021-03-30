#ifndef CLTable_H
#define CLTable_H

#include <pthread.h>
#include "CLStatus.h"
#include "BTree.h"
/*
用于向文件TABLE_FILE_NAME中，查询和添加数据
*/
struct Table
{
	ulong tableAttribute[100];
};

class CLTable
{
public:
	static CLTable* GetInstance();//获取对象指针
	static CLStatus WriteTableMsg(const struct Table *ptableMsg);//获取指针,再写数据
	CLStatus WriteTable(const struct Table *ptableMsg);//写数据
	
	static CLStatus SearchTableMsg(unsigned int attriNum,unsigned long minimum,unsigned long maximum);//获取指针,再读取数据
	CLStatus SearchTable(unsigned int attriNum,unsigned long minimum,unsigned long maximum);//读取数据
	
	static CLStatus ReadOneTableMsg(unsigned long tableRow);
	CLStatus ReadOneTable(unsigned long tableRow);//读取一行数据
	
	static CLStatus ShowBTree(void);
	CLStatus ShowBtree(void);
	
	static void ShowTableMsg(const char *strMsg,const struct Table *ptableMsg);
	static struct Table CreateRandTableMsg(bool IsShow,unsigned long minimum,unsigned long maximum);
	CLStatus Flush();//刷缓存

	static CLStatus SaveBTreeMsg();
	static CLStatus ReadBTreeMsg();
private:
	
	CLStatus SaveBTree();
	BTNode * save_Node(BTree &p,unsigned long father_node_id);
	
	CLStatus ReadBTree();

	static void OnProcessExit();//终止函数
	//CLStatus WriteMsgAndErrcodeToFile(const char *ptableMsg, const char *pstrErrcode);
	static pthread_mutex_t *InitializeMutex();
	
private:
	CLTable(const CLTable&);
	CLTable& operator=(const CLTable&);
	CLTable();
	~CLTable();

private:
	int m_Fd;
	BTree m_Btree;
	pthread_mutex_t *m_pMutexForUsingTable;
	static CLTable *m_pTable;//对象所在的地址
	static pthread_mutex_t *m_pMutexForCreatingTable;//互斥量
	
	unsigned long last_node_id;//本节点索引
	BTNode BNodeSaveCache[100];
	BTNode BNodeLoadCache[100];

private:
	//char *m_pTableWriteBuffer;
	//char *m_pTableReadBuffer;
	struct Table *m_pTableWriteBuffer;//写缓存区地址
	struct Table *m_pTableReadBuffer;//读缓存区地址
	
	//unsigned int m_nUsedBytesForBuffer;//缓存区使用了多少字节
	unsigned int m_nUsedLinesForBuffer;//缓存区使用了多少行数据
	const unsigned int ROW_LEN = sizeof(struct Table);//每行多少字节

private:
	bool m_bFlagForProcessExit;//主进程是否已经退出
};

#endif
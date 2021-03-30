#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "CLTable.h"
//#include "CLLogger.h"

#define TABLE_FILE_NAME "CLTable.txt"
#define INDEX_FILE_NAME "BTreeIndex.txt"

#define WRITE_BUFFER_LINE	100/////////4096,ROW_LEN=800byte
#define READ_BUFFER_LINE 	1000/////////4096,ROW_LEN=800byte,800*10000

#define WRITE_BUFFER_SIZE_TABLE_FILE WRITE_BUFFER_LINE*ROW_LEN/////////4096,ROW_LEN=800byte
#define READ_BUFFER_SIZE_TABLE_FILE	 READ_BUFFER_LINE*ROW_LEN/////////4096,ROW_LEN=800byte,800*10000


CLTable* CLTable::m_pTable = 0;
pthread_mutex_t *CLTable::m_pMutexForCreatingTable = CLTable::InitializeMutex();

pthread_mutex_t *CLTable::InitializeMutex()
{
	pthread_mutex_t *p = new pthread_mutex_t;

	if(pthread_mutex_init(p, 0) != 0)
	{
		delete p;
		return 0;
	}

	return p;
}

CLTable::CLTable()
{
	m_Fd = open(TABLE_FILE_NAME, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR); 
	if(m_Fd == -1)
		throw "In CLTable::CLTable(), open error";

	m_pTableWriteBuffer = new struct Table[WRITE_BUFFER_LINE];
	m_pTableReadBuffer	= new struct Table[READ_BUFFER_LINE];
	initBTree(m_Btree);
	
	m_nUsedLinesForBuffer = 0;

	m_bFlagForProcessExit = false;

	m_pMutexForUsingTable = new pthread_mutex_t;
	if(pthread_mutex_init(m_pMutexForUsingTable, 0) != 0)
	{
		m_Btree=NULL;
		delete m_pMutexForUsingTable;
		delete [] m_pTableWriteBuffer;
		delete [] m_pTableReadBuffer;
		close(m_Fd);

		throw "In CLTable::CLTable(), pthread_mutex_init error";
	}
	//srand((unsigned)time(NULL));//////////////////////////////////////////
}

CLStatus CLTable::WriteTableMsg(const struct Table *ptableMsg)
{
	CLTable *pTable = CLTable::GetInstance();
	if(pTable == 0)
		return CLStatus(-1, 0);
	
	CLStatus s = pTable->WriteTable(ptableMsg);
	if(s.IsSuccess())
		return CLStatus(0, 0);
	else
		return CLStatus(-1, 0);
}

CLStatus CLTable::Flush()
{
	if(pthread_mutex_lock(m_pMutexForUsingTable) != 0)
		return CLStatus(-1, 0);

	try
	{
	    if(m_nUsedLinesForBuffer == 0)
		    throw CLStatus(0, 0);

		if(write(m_Fd, m_pTableWriteBuffer, m_nUsedLinesForBuffer*ROW_LEN) == -1)
			throw CLStatus(-1, errno);

		m_nUsedLinesForBuffer = 0;

		throw CLStatus(0, 0);
	}
	catch(CLStatus &s)
	{
		if(pthread_mutex_unlock(m_pMutexForUsingTable) != 0)
			return CLStatus(-1, 0);
		
	    return s;
	}
}

CLStatus CLTable::WriteTable(const struct Table *ptableMsg)
{
	if(ptableMsg == 0)
		return CLStatus(-1, 0);
	if(sizeof(*ptableMsg) == 0)
		return CLStatus(-1, 0);
	if(pthread_mutex_lock(m_pMutexForUsingTable) != 0)
		return CLStatus(-1, 0);
	try
	{
		if(m_bFlagForProcessExit)
		{
			if(write(m_Fd, ptableMsg, ROW_LEN) == -1)
				throw CLStatus(-1, errno);
			else
				throw CLStatus(0, 0);
		}
		unsigned int nleftroom = WRITE_BUFFER_SIZE_TABLE_FILE - m_nUsedLinesForBuffer*ROW_LEN;
		if(ROW_LEN > nleftroom)
		{
			if(write(m_Fd, m_pTableWriteBuffer, m_nUsedLinesForBuffer*ROW_LEN) == -1)
				throw CLStatus(-1, errno);

			m_nUsedLinesForBuffer = 0;
		}
		memcpy(m_pTableWriteBuffer + m_nUsedLinesForBuffer, ptableMsg, ROW_LEN);
		m_nUsedLinesForBuffer ++;
		throw CLStatus(0, 0);
	}
	catch (CLStatus& s)
	{
		unsigned long nowRow=lseek(m_Fd,0,SEEK_END)/ROW_LEN+m_nUsedLinesForBuffer-1;
		Record  d;
		d.key=(*ptableMsg).tableAttribute[0];
		d.data=nowRow;
		insertBTree(m_Btree,d);
		printf("write line %ld in file\n",nowRow);
		if(pthread_mutex_unlock(m_pMutexForUsingTable) != 0)
			return CLStatus(-1, 0);
		return s;
	}
}

CLStatus CLTable::ShowBTree()
{
	CLTable *pTable = CLTable::GetInstance();
	if(pTable == 0)
		return CLStatus(-1, 0);
	
	CLStatus s = pTable->ShowBtree();
	if(s.IsSuccess())
		return CLStatus(0, 0);
	else
		return CLStatus(-1, 0);
}
CLStatus CLTable::ShowBtree()
{
	if(pthread_mutex_lock(m_pMutexForUsingTable) != 0)
		return CLStatus(-1, 0);
	print(m_Btree);
	if(pthread_mutex_unlock(m_pMutexForUsingTable) != 0)
		return CLStatus(-1, 0);
	return CLStatus(0,0);
}

struct BTNode * CLTable::save_Node(BTree &p,unsigned long father_node_id)
{
	unsigned long my_node_id;//本节点索引
    if(p==NULL)
		return NULL;
	my_node_id=last_node_id;
	printf("now is %ld\n",my_node_id);
	try
	{
		//存储数据
		memcpy(BNodeSaveCache + my_node_id*sizeof(BTNode), p, sizeof(BTNode));
		//储存父亲节点指针
		if(my_node_id!=0)
			BNodeSaveCache[my_node_id].parent=(struct BTNode *)father_node_id;
		else
			BNodeSaveCache[my_node_id].parent=NULL;
		//储存子节点节点指针
		for(int i=0;i<=m;i++)
		{
			BNodeSaveCache[my_node_id].ptr[i]=NULL;
		}
		for(int i=0;i<=p->keynum;i++)
		{
			if(p->ptr[i]!=NULL)
			{
				last_node_id++;
				BNodeSaveCache[my_node_id].ptr[i]=save_Node(p->ptr[i],my_node_id);
			}
			else
				BNodeSaveCache[my_node_id].ptr[i]=NULL;
		}
	}
	catch(int a){}

	return (struct BTNode *)my_node_id;
}

CLStatus CLTable::SaveBTreeMsg()
{
	CLTable *pTable = CLTable::GetInstance();
	if(pTable == 0)
		return CLStatus(-1, 0);
	
	CLStatus s = pTable->SaveBTree();
	if(s.IsSuccess())
		return CLStatus(0, 0);
	else
		return CLStatus(-1, 0);
}

CLStatus CLTable::SaveBTree()
{
	/*
    int keynum;             //节点当前关键字个数
    KeyType key[m+1];       //关键字数组，key[0]未用
    struct BTNode *parent;  //双亲结点指针
    struct BTNode *ptr[m+1]; //孩子结点指针数组
    Record *recptr[m+1];
	*/

	last_node_id=0;
	if(m_Btree!=NULL)//(save_Node(p,0)!=NULL)
	{
		save_Node(m_Btree,0);
		
		int s_Fd = open(INDEX_FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);//O_APPEND 
		lseek(m_Fd,0,SEEK_SET);
		write(s_Fd, BNodeSaveCache, (last_node_id+1)*sizeof(BTNode));
		close(s_Fd);
		
		printf("save %ld node in file\n",last_node_id+1);
		printf("0 node has keynum=%d\n",BNodeSaveCache[0].keynum);
		for(int i=1;i<=m_Btree->keynum;i++)
		{
			if(i==1) ;
			else printf(" ");
			printf("%ld",BNodeSaveCache[0].key[i]);
		}
		printf("\n");
		for(int i=0;i<=BNodeSaveCache[0].keynum;i++)
		{
			if(BNodeSaveCache[0].ptr[i]!=NULL)
			{
				printf("child %ld\n",(unsigned long)BNodeSaveCache[0].ptr[i]);
			}
		}
	}
	else
	{
		printf("save 0 node in file\n");
	}
}

CLStatus CLTable::ReadBTreeMsg()
{
	CLTable *pTable = CLTable::GetInstance();
	if(pTable == 0)
		return CLStatus(-1, 0);
	
	CLStatus s = pTable->ReadBTree();
	if(s.IsSuccess())
		return CLStatus(0, 0);
	else
		return CLStatus(-1, 0);
}

CLStatus CLTable::ReadBTree()
{
	int r_Fd = open(INDEX_FILE_NAME, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR); 
	unsigned long BNodeNum=(lseek(r_Fd,0,SEEK_END)-lseek(r_Fd,0,SEEK_SET))/sizeof(BTNode);
	printf("file have %ld node\n",BNodeNum);
	read(r_Fd,BNodeLoadCache,BNodeNum*sizeof(BTNode));
	close(r_Fd);
	/*
    int keynum;             //节点当前关键字个数
    KeyType key[m+1];       //关键字数组，key[0]未用
    struct BTNode *parent;  //双亲结点指针
    struct BTNode *ptr[m+1]; //孩子结点指针数组
    Record *recptr[m+1];
	*/
	m_Btree=BNodeLoadCache;
	for(int i=0;i<BNodeNum;i++)
	{
		BNodeLoadCache[i].parent=(struct BTNode *)((unsigned long)BNodeLoadCache+(unsigned long)BNodeLoadCache[i].parent);
		for(int j=0;j<=BNodeLoadCache[i].keynum;j++)
		{
			if(BNodeLoadCache[i].ptr[j]!=NULL)
				BNodeLoadCache[i].ptr[j]=(struct BTNode *)((unsigned long)BNodeLoadCache+(unsigned long)BNodeLoadCache[i].ptr[j]);
			else
				BNodeLoadCache[i].ptr[j]=NULL;
		}
	}
}

CLStatus CLTable::SearchTableMsg(unsigned int attriNum,unsigned long minimum,unsigned long maximum)
{
	CLTable *pTable = CLTable::GetInstance();
	if(pTable == 0)
		return CLStatus(-1, 0);
	
	CLStatus s = pTable->SearchTable(attriNum,minimum,maximum);
	if(s.IsSuccess())
		return CLStatus(0, 0);
	else
		return CLStatus(-1, 0);
}

CLStatus CLTable::SearchTable(unsigned int attriNum,unsigned long minimum,unsigned long maximum)
{
	struct Table *p_table;
	int findNum=0,readByteInBuffer=0,readLineInBuffer=0;
	off_t CurrentPosition,EndPosition;
	char buf[20];

	if(pthread_mutex_lock(m_pMutexForUsingTable) != 0)
		return CLStatus(-1, 0);
	try
	{
		if(m_nUsedLinesForBuffer>0)//刷新数据
		{
			if(write(m_Fd, m_pTableWriteBuffer, m_nUsedLinesForBuffer*ROW_LEN) == -1)
				throw CLStatus(-1, errno);
			m_nUsedLinesForBuffer = 0;
		}
		EndPosition=lseek(m_Fd,0,SEEK_END);
		CurrentPosition=lseek(m_Fd,0,SEEK_SET);
		printf("File have %ld line,",(EndPosition-CurrentPosition)/ROW_LEN);
		printf("searching line whose attribute[%2d] is between %ld and %ld\n",attriNum,minimum,maximum);
		do
		{
			CurrentPosition=lseek(m_Fd,0,SEEK_CUR);
			readByteInBuffer=read(m_Fd,m_pTableReadBuffer,READ_BUFFER_SIZE_TABLE_FILE);
			if(readByteInBuffer==-1)
				throw CLStatus(-1, 0);
			readLineInBuffer=readByteInBuffer/ROW_LEN;
			for(int line_i=0;line_i<readLineInBuffer;line_i++)
			{
				p_table=m_pTableReadBuffer+line_i;
				if((*p_table).tableAttribute[attriNum]>=minimum && (*p_table).tableAttribute[attriNum]<=maximum)
				{
					findNum++;
					if(findNum>10)
					{
						printf("More line is not being displayed\n");
						break;
					}
					printf("find line %ld,attri[%2d]=%ld,	^%d^\n",line_i+(unsigned long)(CurrentPosition/ROW_LEN)
					,attriNum,(*p_table).tableAttribute[attriNum],findNum);
					//snprintf(buf, 20, "find line %d",line_i+(unsigned long)(CurrentPosition/ROW_LEN));
					//ShowTableMsg(buf,p_table);
				}
			}
		}
		while(readLineInBuffer==READ_BUFFER_LINE);
		throw CLStatus(0, 0);
	}
	catch (CLStatus& s)
	{
		lseek(m_Fd,0,SEEK_END);
		if(pthread_mutex_unlock(m_pMutexForUsingTable) != 0)
			return CLStatus(-1, 0);
		return s;
	}
}

CLStatus CLTable::ReadOneTableMsg(unsigned long tableRow)
{
	CLTable *pTable = CLTable::GetInstance();
	if(pTable == 0)
		return CLStatus(-1, 0);
	
	CLStatus s = pTable->ReadOneTable(tableRow);
	if(s.IsSuccess())
		return CLStatus(0, 0);
	else
		return CLStatus(-1, 0);
}

CLStatus CLTable::ReadOneTable(unsigned long tableRow)
{
	struct Table table;
	
	if(pthread_mutex_lock(m_pMutexForUsingTable) != 0)
		return CLStatus(-1, 0);
	try
	{
		if(m_nUsedLinesForBuffer>0)//刷新数据
		{
			if(write(m_Fd, m_pTableWriteBuffer, m_nUsedLinesForBuffer*ROW_LEN) == -1)
				throw CLStatus(-1, errno);
			m_nUsedLinesForBuffer = 0;
		}

		throw CLStatus(0, 0);
	}
	catch (CLStatus& s)
	{
		lseek(m_Fd,tableRow*ROW_LEN,SEEK_SET);
		if(read(m_Fd,&table,ROW_LEN)!=ROW_LEN)
			return CLStatus(-1, 0);
		lseek(m_Fd,0,SEEK_END);
		ShowTableMsg("read a line",&table);
		if(pthread_mutex_unlock(m_pMutexForUsingTable) != 0)
			return CLStatus(-1, 0);
		return s;
	}
}

void CLTable::ShowTableMsg(const char *strMsg,const struct Table *ptableMsg)
{
	struct Table table=*ptableMsg;
	for(int line_i=0;line_i<100;line_i++)
	{
		if(line_i==0)
		{
			printf("******************************************************************************************************************\n");
			printf("%s\n",strMsg);
			//printf("*read a line                                                                                                     *\n");
			printf("******************************************************************************************************************\n");
		}
		else if(line_i%5==0)printf("\n");
		printf("Attri[%2d]=%8ld\t",line_i,table.tableAttribute[line_i]);
		if(line_i==99)printf("\n");
		//if(line_i==99)printf("******************************************************************************************************************\n");
	}
}

struct Table CLTable::CreateRandTableMsg(bool IsShow,unsigned long minimum,unsigned long maximum)
{
	struct Table table;
	unsigned long randa;
	
	for(int i=0;i<100;i++)
	{
		randa=(rand()%(maximum-minimum+1))+minimum;
		table.tableAttribute[i]=randa;
	}
	if(IsShow)
		ShowTableMsg("create a new line",&table);
	return table;
}

CLTable* CLTable::GetInstance()
{
	if(m_pTable != 0)
		return m_pTable;
	
	if(m_pMutexForCreatingTable == 0)
		return 0;

	if(pthread_mutex_lock(m_pMutexForCreatingTable) != 0)
		return 0;

	if(m_pTable == 0)
	{
		try
		{
			m_pTable = new CLTable;
		}
		catch(const char*)
		{
			pthread_mutex_unlock(m_pMutexForCreatingTable);
			return 0;
		}
		
		if(atexit(CLTable::OnProcessExit) != 0)
		{
			m_pTable->m_bFlagForProcessExit = true;

			if(pthread_mutex_unlock(m_pMutexForCreatingTable) != 0)
				return 0;
		}
		else
			if(pthread_mutex_unlock(m_pMutexForCreatingTable) != 0)
				return 0;

		return m_pTable;
	}
	if(pthread_mutex_unlock(m_pMutexForCreatingTable) != 0)
		return 0;
	return m_pTable;
}

void CLTable::OnProcessExit()
{
	CLTable *pTable = CLTable::GetInstance();
	if(pTable != 0)
	{
		pthread_mutex_lock(pTable->m_pMutexForUsingTable);
		pTable->m_bFlagForProcessExit = true;
		pthread_mutex_unlock(pTable->m_pMutexForUsingTable);

		pTable->Flush();
	}
}
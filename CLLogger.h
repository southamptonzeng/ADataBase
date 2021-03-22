#ifndef CLLogger_H
#define CLLogger_H

#include <pthread.h>
#include "CLStatus.h"

struct Table {
	ulong attributes[100];
};
/*
�������ļ�LOG_FILE_NAME�У���¼��־��Ϣ
*/
class CLLogger
{
public:
	static CLLogger* GetInstance();
	static CLStatus WriteLogMsg(const struct Table* sTable);
	CLStatus WriteLog(const struct Table* sTable);
	CLStatus Flush();

private:
	static void OnProcessExit();

	CLStatus WriteMsgAndErrcodeToFile(const struct Table* sTable);

	static pthread_mutex_t *InitializeMutex();

private:
	CLLogger(const CLLogger&);
	CLLogger& operator=(const CLLogger&);

	CLLogger();
	~CLLogger();

private:
	int m_Fd;
	pthread_mutex_t *m_pMutexForWritingLog;
	
	static CLLogger *m_pLog;
	static pthread_mutex_t *m_pMutexForCreatingLogger;

private:
	char *m_pLogBuffer;
	unsigned int m_nUsedBytesForBuffer;

private:
	bool m_bFlagForProcessExit;
};

#endif
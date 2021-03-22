#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CLLogger.h"

#define LOG_FILE_NAME "CLLogger.txt"
#define BUFFER_SIZE_LOG_FILE 4096

CLLogger* CLLogger::m_pLog = 0;
pthread_mutex_t *CLLogger::m_pMutexForCreatingLogger = CLLogger::InitializeMutex();

pthread_mutex_t *CLLogger::InitializeMutex()
{
	pthread_mutex_t *p = new pthread_mutex_t;

	if(pthread_mutex_init(p, 0) != 0)
	{
		delete p;
		return 0;
	}

	return p;
}

CLLogger::CLLogger()
{
	m_Fd = open(LOG_FILE_NAME, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR); 
	if(m_Fd == -1)
		throw "In CLLogger::CLLogger(), open error";

	m_pLogBuffer = new char[BUFFER_SIZE_LOG_FILE];
	m_nUsedBytesForBuffer = 0;

	m_bFlagForProcessExit = false;

	m_pMutexForWritingLog = new pthread_mutex_t;
	if(pthread_mutex_init(m_pMutexForWritingLog, 0) != 0)
	{
		delete m_pMutexForWritingLog;
		delete [] m_pLogBuffer;
		close(m_Fd);

		throw "In CLLogger::CLLogger(), pthread_mutex_init error";
	}
}

CLStatus CLLogger::WriteLogMsg(const struct Table* sTable)
{
	CLLogger *pLog = CLLogger::GetInstance();
	if(pLog == 0)
		return CLStatus(-1, 0);
	
	CLStatus s = pLog->WriteLog(sTable);
	if(s.IsSuccess())
		return CLStatus(0, 0);
	else
		return CLStatus(-1, 0);
}

CLStatus CLLogger::Flush()
{
	if(pthread_mutex_lock(m_pMutexForWritingLog) != 0)
		return CLStatus(-1, 0);

	try
	{
	    if(m_nUsedBytesForBuffer == 0)
		    throw CLStatus(0, 0);

		if(write(m_Fd, m_pLogBuffer, m_nUsedBytesForBuffer) == -1)
			throw CLStatus(-1, errno);

		m_nUsedBytesForBuffer = 0;

		throw CLStatus(0, 0);
	}
	catch(CLStatus &s)
	{
		if(pthread_mutex_unlock(m_pMutexForWritingLog) != 0)
			return CLStatus(-1, 0);
		
	    return s;
	}
}

CLStatus CLLogger::WriteMsgAndErrcodeToFile(const struct Table* sTable)
{
    if(write(m_Fd, sTable, sizeof(struct Table)) == -1)
		return CLStatus(-1, errno);
	return CLStatus(0, 0);
}

CLStatus CLLogger::WriteLog(const struct Table* sTable)
{
	if(sTable == 0)
		return CLStatus(-1, 0);

	if(sizeof(*sTable) == 0)
		return CLStatus(-1, 0);



	unsigned int len_strmsg = sizeof(struct Table);

	if(pthread_mutex_lock(m_pMutexForWritingLog) != 0)
		return CLStatus(-1, 0);
	
	try
	{
		if((len_strmsg > BUFFER_SIZE_LOG_FILE) || (m_bFlagForProcessExit))
			throw WriteMsgAndErrcodeToFile(sTable);

		unsigned int nleftroom = BUFFER_SIZE_LOG_FILE - m_nUsedBytesForBuffer;
		if(len_strmsg > nleftroom)
		{
			if(write(m_Fd, m_pLogBuffer, m_nUsedBytesForBuffer) == -1)
				throw CLStatus(-1, errno);

			m_nUsedBytesForBuffer = 0;
		}

		memcpy(m_pLogBuffer + m_nUsedBytesForBuffer, sTable, len_strmsg);
		m_nUsedBytesForBuffer += len_strmsg;


		throw CLStatus(0, 0);
	}
	catch (CLStatus& s)
	{
		if(pthread_mutex_unlock(m_pMutexForWritingLog) != 0)
			return CLStatus(-1, 0);

		return s;
	}
}

CLLogger* CLLogger::GetInstance()
{
	if(m_pMutexForCreatingLogger == 0)
		return 0;

	if(pthread_mutex_lock(m_pMutexForCreatingLogger) != 0)
		return 0;

	if(m_pLog == 0)
	{
		try
		{
			m_pLog = new CLLogger;
		}
		catch(const char*)
		{
			pthread_mutex_unlock(m_pMutexForCreatingLogger);
			return 0;
		}
		
		if(atexit(CLLogger::OnProcessExit) != 0)
		{
			m_pLog->m_bFlagForProcessExit = true;

			if(pthread_mutex_unlock(m_pMutexForCreatingLogger) != 0)
				return 0;

			printf("注册atexit函数失败\n");
		}
		else
			if(pthread_mutex_unlock(m_pMutexForCreatingLogger) != 0)
				return 0;

		return m_pLog;
	}

	if(pthread_mutex_unlock(m_pMutexForCreatingLogger) != 0)
		return 0;

	return m_pLog;
}

void CLLogger::OnProcessExit()
{
	CLLogger *pLogger = CLLogger::GetInstance();
	if(pLogger != 0)
	{
		pthread_mutex_lock(pLogger->m_pMutexForWritingLog);
		pLogger->m_bFlagForProcessExit = true;
		pthread_mutex_unlock(pLogger->m_pMutexForWritingLog);

		pLogger->Flush();
	}
}
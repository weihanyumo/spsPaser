#include "stdafx.h"
#include "CircleBuffer.h"
#include "assert.h"

#define CIC_WAITTIMEOUT  3000

CPs_CircleBuffer::CPs_CircleBuffer(const unsigned int iBufferSize)
{
	m_iBufferSize = iBufferSize;
	m_pBuffer = (unsigned char*)malloc(iBufferSize);
	m_iReadCursor = 0;
	m_iWriteCursor = 0;
	m_bComplete = false;
	m_evtDataAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CPs_CircleBuffer::~CPs_CircleBuffer()
{
	Uninitialise();
}

// Public functions
void CPs_CircleBuffer::Uninitialise()//û�б�Ҫpublic����ӿں���,long120817
{
	CloseHandle(m_evtDataAvailable);
	free(m_pBuffer);
}

//Writeǰһ��Ҫ����m_pCircleBuffer->GetFreeSize()�����FreeSize������Ҫ�ȴ�,long120817

void  CPs_CircleBuffer::Write(const void* _pSourceBuffer, const unsigned int _iNumBytes)
{
	unsigned int iBytesToWrite = _iNumBytes;
	unsigned char* pSourceReadCursor = (unsigned char*)_pSourceBuffer;

	//CP_ASSERT(iBytesToWrite <= GetFreeSize());//�޸�Ϊû���㹻�ռ�ͷ��أ�writeǰһ��Ҫ��GetFreeSize�жϣ�������뵽�����൱�ڶ������ݣ�         // long120817
	if (iBytesToWrite > GetFreeSize())
	{
		return;
	}
	//_ASSERT(m_bComplete == false);

	m_csCircleBuffer.Lock();

	if (m_iWriteCursor >= m_iReadCursor)
	{
		//              0                                            m_iBufferSize
		//              |-----------------|===========|--------------|
		//                                pR->        pW-> 
		// ����β����д�ռ�iChunkSize,long120817
		unsigned int iChunkSize = m_iBufferSize - m_iWriteCursor;

		if (iChunkSize > iBytesToWrite)
		{
			iChunkSize = iBytesToWrite;
		}

		// Copy the data
		memcpy(m_pBuffer + m_iWriteCursor,pSourceReadCursor, iChunkSize);

		pSourceReadCursor += iChunkSize;

		iBytesToWrite -= iChunkSize;

		// ����m_iWriteCursor
		m_iWriteCursor += iChunkSize;

		if (m_iWriteCursor >= m_iBufferSize)//���m_iWriteCursor�Ѿ�����ĩβ
			m_iWriteCursor -= m_iBufferSize;//���ص����0λ��,long120817

	}

	//ʣ�����ݴ�Buffer��ʼλ�ÿ�ʼд
	if (iBytesToWrite)
	{
		memcpy(m_pBuffer + m_iWriteCursor,pSourceReadCursor, iBytesToWrite);
		m_iWriteCursor += iBytesToWrite;
		//_ASSERT(m_iWriteCursor < m_iBufferSize);//�������ûʲô��˼��Ӧ��_ASSERT(m_iWriteCursor <= m_iReadCursor);long20120817
	}

	SetEvent(m_evtDataAvailable);//��������д���ź���

	m_csCircleBuffer.UnLock();
}

bool  CPs_CircleBuffer::Read(void* pDestBuffer, const size_t _iBytesToRead, size_t* pbBytesRead)
{
	size_t iBytesToRead = _iBytesToRead;
	size_t iBytesRead = 0;
	DWORD dwWaitResult;
	bool bComplete = false;

	while (iBytesToRead > 0 && bComplete == false)
	{
		dwWaitResult = WaitForSingleObject(m_evtDataAvailable, CIC_WAITTIMEOUT);//�ȴ�����д��,long120817

		if (dwWaitResult == WAIT_TIMEOUT)
		{
			//TRACE_INFO2("Circle buffer - did not fill in time!");
			*pbBytesRead = iBytesRead;
			return FALSE;//�ȴ���ʱ�򷵻�
		}

		m_csCircleBuffer.Lock();

		if (m_iReadCursor > m_iWriteCursor)
		{
			//              0                                                    m_iBufferSize
			//              |=================|-----|===========================|
			//                                pW->  pR-> 
			unsigned int iChunkSize = m_iBufferSize - m_iReadCursor;

			if (iChunkSize > iBytesToRead)
				iChunkSize = (unsigned int)iBytesToRead;

			//��ȡ����
			memcpy((unsigned char*)pDestBuffer + iBytesRead,m_pBuffer + m_iReadCursor,iChunkSize);

			iBytesRead += iChunkSize;
			iBytesToRead -= iChunkSize;

			m_iReadCursor += iChunkSize;

			if (m_iReadCursor >= m_iBufferSize)//���m_iReadCursor�Ѿ�����ĩβ
				m_iReadCursor -= m_iBufferSize;//���ص����0λ��,long120817
		}

		if (iBytesToRead && m_iReadCursor < m_iWriteCursor)
		{
			unsigned int iChunkSize = m_iWriteCursor - m_iReadCursor;

			if (iChunkSize > iBytesToRead)
				iChunkSize = (unsigned int)iBytesToRead;

			//��ȡ����
			memcpy((unsigned char*)pDestBuffer + iBytesRead,m_pBuffer + m_iReadCursor,iChunkSize);

			iBytesRead += iChunkSize;
			iBytesToRead -= iChunkSize;
			m_iReadCursor += iChunkSize;
		}

		//����и��������Ҫд
		if (m_iReadCursor == m_iWriteCursor)
		{
			if (m_bComplete)//������һ��whileѭ������ֵͨ��SetComplete()���ã����߼�ʲô��˼��long120817
				bComplete = true;
		}
		else//�������ݿ��Զ���SetEvent������һ��whileѭ����ʼ���Բ����ٵȴ�,long120817
			SetEvent(m_evtDataAvailable);

		m_csCircleBuffer.UnLock();
	}

	*pbBytesRead = iBytesRead;

	return bComplete ? false : true;

}
//  0                                                m_iBufferSize
//  |------------------------------------------------|
//  pR
//  pW
//��дָ�����
void  CPs_CircleBuffer::Flush()
{
	m_csCircleBuffer.Lock();
	m_iReadCursor = 0;
	m_iWriteCursor = 0;
	m_csCircleBuffer.UnLock();

}
//��ȡ�Ѿ�д���ڴ�
unsigned int CPs_CircleBuffer::GetUsedSize()
{
	return m_iBufferSize - GetFreeSize();

}


unsigned int CPs_CircleBuffer::GetFreeSize()
{
	unsigned int iNumBytesFree;

	m_csCircleBuffer.Lock();

	if (m_iWriteCursor < m_iReadCursor)
	{
		//              0                                                    m_iBufferSize
		//              |=================|-----|===========================|
		//                                pW->  pR-> 
		iNumBytesFree = (m_iReadCursor - 1) - m_iWriteCursor;
	}
	else if (m_iWriteCursor == m_iReadCursor)
	{
		iNumBytesFree = m_iBufferSize;
	}
	else
	{
		//              0                                                    m_iBufferSize
		//              |-----------------|=====|---------------------------|
		//                                pR->   pW-> 
		iNumBytesFree = (m_iReadCursor - 1) + (m_iBufferSize - m_iWriteCursor);
	}

	m_csCircleBuffer.UnLock();

	return iNumBytesFree;

}
//�ú���ʲôʱ����ã�long120817
void  CPs_CircleBuffer::SetComplete()
{
	m_csCircleBuffer.Lock();
	m_bComplete = true;
	SetEvent(m_evtDataAvailable);
	m_csCircleBuffer.UnLock();
}
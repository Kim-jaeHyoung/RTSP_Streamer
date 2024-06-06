#include "rtpprocessthread.h"

CRtpProcessThread::CRtpProcessThread()
{
}

CRtpProcessThread::~CRtpProcessThread()
{
}

void CRtpProcessThread::run()
{
	CRtpProcess* pRtpProcess = new CRtpProcess();
	bool initialized = pRtpProcess->initialize();
	if (initialized == false)
	{
		qDebug() << __FUNCTION__ << "RtpProcess initialze fail";
	}
	else
	{
		qDebug() << __FUNCTION__ << "RtpProcess initialze suuccess";
	}

	QObject::connect(this, &CRtpProcessThread::receiveRtpData, pRtpProcess, &CRtpProcess::receiveRtpData);
	QObject::connect(this, &CRtpProcessThread::deleteLater, pRtpProcess, &CRtpProcess::deleteLater);

	exec();
}

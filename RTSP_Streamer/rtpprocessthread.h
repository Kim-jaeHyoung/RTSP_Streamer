#pragma once
#include <qthread.h>

#include "rtpprocess.h"

class CRtpProcessThread : public QThread
{
	Q_OBJECT
public:
	CRtpProcessThread();
	~CRtpProcessThread();

protected:
	void run() override;

signals:
	void receiveRtpData(const QByteArray& rtpData);
};


#pragma once
#include <qobject.h>
#include <qtcpserver.h>

class CTcpRtspServer : public QObject
{
public:
	CTcpRtspServer();
	~CTcpRtspServer();

	bool initialize();

private:
	QTcpServer* m_pTcpServer = nullptr;
};


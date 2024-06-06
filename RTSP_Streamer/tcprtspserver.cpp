#include "tcprtspserver.h"

CTcpRtspServer::CTcpRtspServer()
{
}

CTcpRtspServer::~CTcpRtspServer()
{
}

bool CTcpRtspServer::initialize()
{
	m_pTcpServer = new QTcpServer();
	if (m_pTcpServer == nullptr)
		return false;

	if (m_pTcpServer->listen(QHostAddress::Any, 554) == false)
		return false;

	return true;
}

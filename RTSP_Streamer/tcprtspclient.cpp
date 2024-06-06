#include "tcprtspclient.h"

CTcpRtspClient::CTcpRtspClient()
{
}

CTcpRtspClient::~CTcpRtspClient()
{
}

bool CTcpRtspClient::initialize()
{
	m_pTcpSocket = new QTcpSocket();
	if (m_pTcpSocket == nullptr)
		return false;

	QObject::connect(m_pTcpSocket, &QTcpSocket::connected, this, &CTcpRtspClient::onConnected);
	QObject::connect(m_pTcpSocket, &QTcpSocket::disconnected, this, &CTcpRtspClient::onDisconnected);
	//QObject::connect(m_pTcpSocket, &QTcpSocket::error, this, &CTcpRtspClient::onError);
	QObject::connect(m_pTcpSocket, &QTcpSocket::hostFound, this, &CTcpRtspClient::onHostFound);
	QObject::connect(m_pTcpSocket, &QTcpSocket::stateChanged, this, &CTcpRtspClient::onStateChanged);
	QObject::connect(m_pTcpSocket, &QTcpSocket::readyRead, this, &CTcpRtspClient::onReadyRead);

	m_pTcpSocket->connectToHost("210.99.70.120", 1935);

	return true;
}

void CTcpRtspClient::sendRTSPOperation(ERtspOperation rtspOption, int sessionId)
{
	switch (rtspOption)
	{
	case ERtspOperation_OPTIONS:
		sendOptionsRequest(++m_seqNo);
		break;
	case ERtspOperation_DESCRIBE:
		sendDescribeRequest(++m_seqNo);
		break;
	case ERtspOperation_SETUP:
		sendSetupRequest(++m_seqNo, 8000, 8001);
		break;
	case ERtspOperation_PLAY:
		sendPlayRequest(++m_seqNo, sessionId);
		break;
	default:
		break;
	}
}

void CTcpRtspClient::sendOptionsRequest(int seqNo)
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << seqNo;
	QByteArray requestMessage = "OPTIONS " + QByteArray("rtsp://210.99.70.120:1935/live/cctv001.stream") + " RTSP/1.0" + CRLF;
	requestMessage += "CSeq: " + QByteArray::number(seqNo) + CRLF;
	requestMessage += CRLF;

	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << requestMessage;

	m_rtspOperationMap.insert({ seqNo, ERtspOperation_OPTIONS });

	m_pTcpSocket->write(requestMessage);
}

void CTcpRtspClient::sendDescribeRequest(int seqNo)
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << seqNo;
	QByteArray requestMessage = "DESCRIBE " + QByteArray("rtsp://210.99.70.120:1935/live/cctv001.stream") + " RTSP/1.0" + CRLF;
	requestMessage += "CSeq: " + QByteArray::number(seqNo) + CRLF;
	requestMessage += CRLF;

	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << requestMessage;

	m_rtspOperationMap.insert({ seqNo, ERtspOperation_DESCRIBE });

	m_pTcpSocket->write(requestMessage);
}

void CTcpRtspClient::sendSetupRequest(int seqNo, int portStart, int portEnd)
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << seqNo << portStart << portEnd;
	QByteArray requestMessage = "SETUP " + QByteArray("rtsp://210.99.70.120:1935/live/cctv001.stream") + " RTSP/1.0" + CRLF;
	requestMessage += "CSeq: " + QByteArray::number(seqNo) + CRLF;
	//requestMessage += "Transport: RTP/AVP;unicast;client_port=" + QByteArray::number(portStart) + "-" + QByteArray::number(portEnd) + CRLF;
	requestMessage += "Transport: RTP/AVP;unicast;interleaved=0-1" + CRLF;
	requestMessage += CRLF;

	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << requestMessage;
		
	m_rtspOperationMap.insert({ seqNo, ERtspOperation_SETUP });

	m_pUdpSocket = new QUdpSocket();
	if (m_pUdpSocket == nullptr)
		return;

	if (m_pUdpSocket->bind(QHostAddress::AnyIPv4, portStart) == false)
	{
		qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << "socket bind fail";
		return;
	}
	else
	{
		qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << "socket bind success";
		QObject::connect(m_pUdpSocket, &QUdpSocket::readyRead, this, &CTcpRtspClient::onRtpReadyRead);
	}

	m_pTcpSocket->write(requestMessage);
}

void CTcpRtspClient::sendPlayRequest(int seqNo, int sessionId)
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << seqNo;
	QByteArray requestMessage = "PLAY " + QByteArray("rtsp://210.99.70.120:1935/live/cctv001.stream") + " RTSP/1.0" + CRLF;
	requestMessage += "CSeq: " + QByteArray::number(seqNo) + CRLF;
	requestMessage += "Range: npt=5-20" + CRLF;
	requestMessage += "Session: " + QByteArray::number(sessionId) + CRLF;
	requestMessage += CRLF;

	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << requestMessage;

	m_rtspOperationMap.insert({ seqNo, ERtspOperation_PLAY });

	m_pTcpSocket->write(requestMessage);
}

void CTcpRtspClient::sendPauseRequest(int seqNo)
{
}

void CTcpRtspClient::sendTeardownRequest(int seqNo)
{
}

int CTcpRtspClient::parseResponse(const QByteArray& response, int& seqNo)
{
	QStringList parsedResponseList = QString(response).split(CRLF);
	if (parsedResponseList.size() < 2)
		return 0;

	int rtspStatusNo = 0;
	seqNo = 0;
	for (auto parsedResponse : parsedResponseList)
	{
		QStringList parsedLineList = parsedResponse.split(' ');
		if (parsedLineList.size() < 2)
			continue;

		if (parsedLineList[0].contains("RTSP") == true)
			rtspStatusNo = parsedLineList[1].toInt();
		else if (parsedLineList[0].contains("CSeq:") == true)
			seqNo = parsedLineList[1].toInt();

		if (rtspStatusNo != 0 && seqNo != 0)
			break;
	}

	return rtspStatusNo;
}

int CTcpRtspClient::parseSessionId(const QByteArray& response)
{
	QStringList parsedResponseList = QString(response).split(CRLF);
	if (parsedResponseList.size() < 2)
		return 0;

	int sessionId = 0;
	for (auto parsedResponse : parsedResponseList)
	{
		QStringList parsedLineList = parsedResponse.split(' ');
		if (parsedLineList.size() < 2)
			continue;

		if (parsedLineList[0].contains("Session") == true)
		{
			if(parsedLineList[1].contains(';') == false)
				sessionId = parsedLineList[1].toInt();
			else
			{
				QStringList temp = parsedLineList[1].split(";");
				sessionId = temp[0].toInt();
			}
			break;
		}
	}

	return sessionId;
}

void CTcpRtspClient::onConnected()
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__;
	m_seqNo = 0;
	sendRTSPOperation(ERtspOperation_OPTIONS);
}

void CTcpRtspClient::onDisconnected()
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__;
	m_seqNo = 0;
}

void CTcpRtspClient::onError(QAbstractSocket::SocketError socketError)
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << socketError;
}

void CTcpRtspClient::onHostFound()
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__;
}

void CTcpRtspClient::onStateChanged(QAbstractSocket::SocketState socketState)
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << socketState;
}

void CTcpRtspClient::onReadyRead()
{
	QMutexLocker mutexLocker(&m_mutex);
	QByteArray response = m_pTcpSocket->readAll();
	int index = 0;
	//qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << response.size() << index;
	while (index < response.size() || index > 0)
	{
		//rtp(0), rtcp(1) packet
		if (response[index] == '$')
		{
			char channel = response[++index];
			unsigned int length = (response[index] << 8) | response[++index];
			QByteArray packet = response.mid(++index, length);
			index += length;

			if (channel == 0)
			{
				emit sendRtpData(packet);
				//qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << "rtp packet processing";
			}
			else if(channel == 1)
				qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << "rtcp packet processing";
		}
		else
		{
			int endOfMessage = response.indexOf(CRLF + CRLF, index);
			if (endOfMessage == -1)
			{
				//qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << "abnormal rtsp packet";
				break;
			}

			QByteArray packet = response.mid(index, endOfMessage + 4);
			index += endOfMessage + 4;

			qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << packet;

			int seqNo = 0;
			parseResponse(packet, seqNo);

			auto it = m_rtspOperationMap.find(seqNo);
			if (it == m_rtspOperationMap.end())
			{
				qDebug() << QDateTime::currentDateTime() << __FUNCTION__ << "no exist seqNo" << seqNo;
				break;
			}

			switch (it->second)
			{
			case ERtspOperation_OPTIONS:
				sendRTSPOperation(ERtspOperation_DESCRIBE);
				break;
			case ERtspOperation_DESCRIBE:
				sendRTSPOperation(ERtspOperation_SETUP);
				break;
			case ERtspOperation_SETUP:
			{
				int sessionId = parseSessionId(packet);
				sendRTSPOperation(ERtspOperation_PLAY, sessionId);
				break;
			}
			}
		}
	}
}

void CTcpRtspClient::onRtpReadyRead()
{
	qDebug() << QDateTime::currentDateTime() << __FUNCTION__;
}

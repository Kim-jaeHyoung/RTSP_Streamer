#pragma once
#include <qobject.h>
#include <qtcpsocket.h>
#include <qudpsocket.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qmutex.h>

#include <map>

enum ERtspOperation
{
	ERtspOperation_OPTIONS,
	ERtspOperation_DESCRIBE,
	ERtspOperation_SETUP,
	ERtspOperation_PLAY,
	ERtspOperation_PAUSE,
	ERtspOperation_TEARDOWN
};

const QByteArray CRLF = "\r\n";

class CTcpRtspClient : public QObject
{
	Q_OBJECT
public:
	CTcpRtspClient();
	~CTcpRtspClient();

	bool initialize();

private:
	void sendRTSPOperation(ERtspOperation rtspOption, int sessionId = 0);
	void sendOptionsRequest(int seqNo);
	void sendDescribeRequest(int seqNo);
	void sendSetupRequest(int seqNo, int portStart, int portEnd);
	void sendPlayRequest(int seqNo, int sessionId);
	void sendPauseRequest(int seqNo);
	void sendTeardownRequest(int seqNo);

	int parseResponse(const QByteArray& response, int& seqNo);
	int parseSessionId(const QByteArray& response);

private slots:
	void onConnected();
	void onDisconnected();
	void onError(QAbstractSocket::SocketError socketError);
	void onHostFound();
	void onStateChanged(QAbstractSocket::SocketState socketState);
	void onReadyRead();

	void onRtpReadyRead();

private:
	QTcpSocket* m_pTcpSocket = nullptr;	//for rtsp
	QUdpSocket* m_pUdpSocket = nullptr;	//for rtp
	int m_seqNo = 0;
	std::map<int, ERtspOperation> m_rtspOperationMap;

	QMutex m_mutex;

signals:
	void sendRtpData(const QByteArray& rtpData);
};


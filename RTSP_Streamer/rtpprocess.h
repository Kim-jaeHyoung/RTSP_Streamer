#pragma once
#include <qobject.h>
#include <qmutex.h>
#include <qdebug.h>
#include <qimage.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avutil.lib")

const unsigned int RTP_HEADER_SIZE = 12;

const unsigned int MASK_VERSION = 0b11000000;
const unsigned int MASK_PADDING = 0b00100000;
const unsigned int MASK_EXTENTION = 0b00010000;
const unsigned int MASK_CC = 0b00001111;

const unsigned int MASK_MARKER = 0b10000000;
const unsigned int MASK_PAYLOADTYPE = 0b01111111;

struct SRtpHeader
{
	//~1byte
	unsigned int version;
	unsigned int padding;
	unsigned int extention;
	unsigned int cc;
	//~2byte
	unsigned int marker;
	unsigned int payloadtype;
	//~4byte
	unsigned int sequenceNumber;
	//~8byte
	unsigned int timestamp;
	//~12byte
	unsigned int ssrc;
};

class CRtpProcess : public QObject
{
	Q_OBJECT
public:
	CRtpProcess();
	~CRtpProcess();

	bool initialize();

private:

	SRtpHeader processRtpHeader(const QByteArray& rtpData);
	void processRtpPayload(const QByteArray& rtpPayload);

public slots:
	void receiveRtpData(const QByteArray& rtpData);

private:
	QMutex mutex;

	AVCodec* m_pAVCodec = nullptr;
	AVCodecContext* m_pAVCodecContext = nullptr;

signals:
	void initialized(bool initialized);
};


#include "rtpprocess.h"

CRtpProcess::CRtpProcess()
{
}

CRtpProcess::~CRtpProcess()
{
}

bool CRtpProcess::initialize()
{
	m_pAVCodec = const_cast<AVCodec*>(avcodec_find_decoder(AV_CODEC_ID_H264));
	if (m_pAVCodec == nullptr)
	{
		qDebug() << __FUNCTION__ << "AVCodec allocation fail";
		return false;
	}

	m_pAVCodecContext = avcodec_alloc_context3(m_pAVCodec);
	if (m_pAVCodecContext == nullptr)
	{
		qDebug() << __FUNCTION__ << "AVCodecContext allocation fail";
		return false;
	}

	int result = avcodec_open2(m_pAVCodecContext, m_pAVCodec, nullptr);
	if (result != 0)
	{
		qDebug() << __FUNCTION__ << "AVCodec open fail";
		return false;
	}

	return true;
}

SRtpHeader CRtpProcess::processRtpHeader(const QByteArray& rtpData)
{
	// 1byte
	const unsigned int version = (rtpData[1] & MASK_VERSION) >> 6;
	const unsigned int padding = (rtpData[1] & MASK_PADDING) >> 5;
	const unsigned int extention = (rtpData[1] & MASK_EXTENTION) >> 4;
	const unsigned int cc = rtpData[1] & MASK_CC;

	// 1byte
	const unsigned int marker = (rtpData[2] & MASK_MARKER) >> 1;
	const unsigned int payload = rtpData[2] & MASK_PAYLOADTYPE;

	// 2byte
	unsigned int sequenceNumber = rtpData[3] << 8;
	sequenceNumber |= rtpData[4];

	// 4byte
	unsigned int timestamp = rtpData[5] << 24;
	timestamp |= rtpData[6] << 16;
	timestamp |= rtpData[7] << 8;
	timestamp |= rtpData[8];

	// 4byte
	unsigned int SSRC = rtpData[9] << 24;
	SSRC |= rtpData[10] << 16;
	SSRC |= rtpData[11] << 8;
	SSRC |= rtpData[12];


	//qDebug() << __FUNCTION__ << rtpData << rtpData.length();
	//qDebug() << __FUNCTION__ << rtpData[1];
	qDebug() << __FUNCTION__ << "version" << version;
	qDebug() << __FUNCTION__ << "padding" << padding;
	qDebug() << __FUNCTION__ << "extention" << extention;
	qDebug() << __FUNCTION__ << "cc" << cc;
	qDebug() << __FUNCTION__ << "marker" << marker;
	qDebug() << __FUNCTION__ << "payload" << payload;
	qDebug() << __FUNCTION__ << "sequence Number" << sequenceNumber;
	qDebug() << __FUNCTION__ << "timestamp" << timestamp;
	qDebug() << __FUNCTION__ << "SSRC" << SSRC;

	SRtpHeader rtpHeader = { version, padding, extention, cc, marker, payload, sequenceNumber, timestamp, SSRC };
	return rtpHeader;
}

void CRtpProcess::processRtpPayload(const QByteArray& rtpPayload)
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = reinterpret_cast<uint8_t*>(const_cast<char*>(rtpPayload.data()));
	packet.size = rtpPayload.size();

	int response = avcodec_send_packet(m_pAVCodecContext, &packet);
	if (response < 0) {
		qDebug() << __FUNCTION__ << "Error sending a packet for decoding";
		return;
	}

	AVFrame* frame = av_frame_alloc();
	response = avcodec_receive_frame(m_pAVCodecContext, frame);
	if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
		return;
	}
	else if (response < 0) {
		qDebug() << __FUNCTION__ << "Error during decoding";
		return;
	}

	// Convert the frame to RGB format
	SwsContext* swsContext = sws_getContext(
		frame->width, frame->height, (AVPixelFormat)frame->format,
		frame->width, frame->height, AV_PIX_FMT_RGB24,
		SWS_BILINEAR, nullptr, nullptr, nullptr);

	AVFrame* rgbFrame = av_frame_alloc();
	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, frame->width, frame->height, 1);
	uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
	av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGB24, frame->width, frame->height, 1);

	sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize);

	// Create a QImage from the RGB frame
	QImage image(rgbFrame->data[0], frame->width, frame->height, QImage::Format_RGB888);
	static int index = 1;
	image.save(QString("frame%1.jpg").arg(index++));

	av_free(buffer);
	av_frame_free(&rgbFrame);
	sws_freeContext(swsContext);
	av_frame_free(&frame);
}

void CRtpProcess::receiveRtpData(const QByteArray& rtpData)
{
	if (rtpData.size() < RTP_HEADER_SIZE)
	{
		qDebug() << __FUNCTION__ << "RTP Header Size : " << rtpData.size();
		return;
	}

	SRtpHeader rtpHeader = processRtpHeader(rtpData);
	if (rtpHeader.extention == 1)
	{
		qDebug() << __FUNCTION__ << "RTP Header includes extention header";
		return;
	}

	processRtpPayload(rtpData.mid(RTP_HEADER_SIZE + 1));
}
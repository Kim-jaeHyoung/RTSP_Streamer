#include "mainwindow.h"

CMainWindow::CMainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

CMainWindow::~CMainWindow()
{
}

bool CMainWindow::initialize()
{
	if (initializeTcpRtspClient() == false)
		return false;

	if (initializeRtpProcessThread() == false)
		return false;

	return true;
}

bool CMainWindow::initializeTcpRtspClient()
{
	m_pTcpRtspClient = new CTcpRtspClient();
	if (m_pTcpRtspClient == false)
		return false;
	if (m_pTcpRtspClient->initialize() == false)
		return false;

	return true;
}

bool CMainWindow::initializeRtpProcessThread()
{
	m_pRtpProcessThread = new CRtpProcessThread();
	if (m_pRtpProcessThread == false)
		return false;

	QObject::connect(m_pTcpRtspClient, &CTcpRtspClient::sendRtpData, m_pRtpProcessThread, &CRtpProcessThread::receiveRtpData);
	QObject::connect(m_pRtpProcessThread, &CRtpProcessThread::finished, m_pRtpProcessThread, &CRtpProcessThread::deleteLater);

	m_pRtpProcessThread->start();

	return true;
}

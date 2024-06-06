#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_mainwindow.h"
#include "tcprtspclient.h"
#include "rtpprocessthread.h"

class CMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    CMainWindow(QWidget *parent = nullptr);
    ~CMainWindow();

    bool initialize();

private:
    bool initializeTcpRtspClient();
    bool initializeRtpProcessThread();

private:
    Ui::CmainWindowClass ui;
    CTcpRtspClient* m_pTcpRtspClient = nullptr;
    CRtpProcessThread* m_pRtpProcessThread = nullptr;
};

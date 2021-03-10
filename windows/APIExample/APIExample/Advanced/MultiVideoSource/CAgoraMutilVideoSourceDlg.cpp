﻿#include "stdafx.h"
#include "APIExample.h"
#include "CAgoraMutilVideoSourceDlg.h"



IMPLEMENT_DYNAMIC(CAgoraMutilVideoSourceDlg, CDialogEx)

CAgoraMutilVideoSourceDlg::CAgoraMutilVideoSourceDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MUTI_SOURCE, pParent)
{

}

CAgoraMutilVideoSourceDlg::~CAgoraMutilVideoSourceDlg()
{
}

void CAgoraMutilVideoSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIDEO, m_staVideoArea);
	DDX_Control(pDX, IDC_LIST_INFO_BROADCASTING, m_lstInfo);
	DDX_Control(pDX, IDC_STATIC_CHANNELNAME, m_staChannel);
	DDX_Control(pDX, IDC_EDIT_CHANNELNAME, m_edtChannel);
	DDX_Control(pDX, IDC_BUTTON_JOINCHANNEL, m_btnJoinChannel);
	DDX_Control(pDX, IDC_BUTTON_PUBLISH, m_btnPublish);
	DDX_Control(pDX, IDC_STATIC_DETAIL, m_staDetail);
	DDX_Control(pDX, IDC_CHECK_RAW_VIDEO, m_chkRawVideo);
}


BEGIN_MESSAGE_MAP(CAgoraMutilVideoSourceDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_MSGID(EID_JOINCHANNEL_SUCCESS), &CAgoraMutilVideoSourceDlg::OnEIDJoinChannelSuccess)
	ON_MESSAGE(WM_MSGID(EID_LEAVE_CHANNEL), &CAgoraMutilVideoSourceDlg::OnEIDLeaveChannel)
	ON_MESSAGE(WM_MSGID(EID_USER_JOINED), &CAgoraMutilVideoSourceDlg::OnEIDUserJoined)
	ON_MESSAGE(WM_MSGID(EID_USER_OFFLINE), &CAgoraMutilVideoSourceDlg::OnEIDUserOffline)
	ON_MESSAGE(WM_MSGID(EID_REMOTE_VIDEO_STATE_CHANED), &CAgoraMutilVideoSourceDlg::OnEIDRemoteVideoStateChanged)
	ON_MESSAGE(WM_MSGID(EID_CONNECTION_STATE_CHANGED), &CAgoraMutilVideoSourceDlg::OnEIDConnectionStateChanged)

	ON_BN_CLICKED(IDC_BUTTON_JOINCHANNEL, &CAgoraMutilVideoSourceDlg::OnBnClickedButtonJoinchannel)
	ON_BN_CLICKED(IDC_BUTTON_PUBLISH, &CAgoraMutilVideoSourceDlg::OnBnClickedButtonPublish)
	
	ON_BN_CLICKED(IDC_CHECK_RAW_VIDEO, &CAgoraMutilVideoSourceDlg::OnBnClickedCheckRawVideo)
END_MESSAGE_MAP()


//Initialize the Ctrl Text.
void CAgoraMutilVideoSourceDlg::InitCtrlText()
{

	m_btnPublish.SetWindowText(MultiVideoSourceCtrlPublish);//MultiVideoSourceCtrlUnPublish
	m_staChannel.SetWindowText(commonCtrlChannel);
	m_btnJoinChannel.SetWindowText(commonCtrlJoinChannel);
}


//Initialize the Agora SDK
bool CAgoraMutilVideoSourceDlg::InitAgora()
{
	//create Agora RTC engine
	m_rtcEngine = createAgoraRtcEngine();
	if (!m_rtcEngine) {
		m_lstInfo.InsertString(m_lstInfo.GetCount() - 1, _T("createAgoraRtcEngine failed"));
		return false;
	}
	CAgoraMultiVideoSourceEventHandler * p = new CAgoraMultiVideoSourceEventHandler;
	//set message notify receiver window
	p->SetMsgReceiver(m_hWnd);
	p->SetChannelId(0);
	m_vecVidoeSourceEventHandler.push_back(p);
	agora::rtc::RtcEngineContext context;
	std::string strAppID = GET_APP_ID;
	context.appId = strAppID.c_str();
	context.eventHandler = p;
	//initialize the Agora RTC engine context.
	int ret = m_rtcEngine->initialize(context);
	if (ret != 0) {
		m_initialize = false;
		CString strInfo;
		if (ret == -101) m_lstInfo.InsertString(m_lstInfo.GetCount(), InvalidAppidError); strInfo.Format(_T("initialize failed: %d"), ret);
		m_lstInfo.InsertString(m_lstInfo.GetCount(), strInfo);
		return false;
	}
	else
		m_initialize = true;
	m_lstInfo.InsertString(m_lstInfo.GetCount(), _T("initialize success"));
	//enable video in the engine.
	m_rtcEngine->enableVideo();
	m_lstInfo.InsertString(m_lstInfo.GetCount(), _T("enable video"));
	//set channel profile in the engine to the CHANNEL_PROFILE_LIVE_BROADCASTING.
	m_rtcEngine->setChannelProfile(CHANNEL_PROFILE_LIVE_BROADCASTING);
	m_lstInfo.InsertString(m_lstInfo.GetCount(), _T("live broadcasting"));
	//set client role in the engine to the CLIENT_ROLE_BROADCASTER.
	m_rtcEngine->setClientRole(agora::rtc::CLIENT_ROLE_BROADCASTER);
	m_lstInfo.InsertString(m_lstInfo.GetCount(), _T("setClientRole broadcaster"));
	return true;
}


//UnInitialize the Agora SDK
void CAgoraMutilVideoSourceDlg::UnInitAgora()
{
	if (m_rtcEngine) {
		if (m_joinChannel) {
			//leave channel
			m_joinChannel = !m_rtcEngine->leaveChannel();
			m_rtcEngine->leaveChannelEx(m_strChannel.data(), m_conn_screen);
		}
		m_bPublishScreen = false;
		//stop preview in the engine.
		m_rtcEngine->stopPreview();
		m_lstInfo.InsertString(m_lstInfo.GetCount(), _T("stopPreview"));
		//disable video in the engine.
		m_rtcEngine->disableVideo();
		m_lstInfo.InsertString(m_lstInfo.GetCount(), _T("disableVideo"));
		if (m_chkRawVideo.GetCheck())
			RegisterVideoFrameObserver(FALSE, nullptr);
		//release engine.
		m_rtcEngine->release(true);
		m_lstInfo.InsertString(m_lstInfo.GetCount(), _T("release rtc engine"));
		m_rtcEngine = NULL;
	}
}

//render local video from SDK local capture.
void CAgoraMutilVideoSourceDlg::RenderLocalVideo()
{
	if (m_rtcEngine) {
		agora::rtc::VideoCanvas canvas;
		canvas.renderMode = media::base::RENDER_MODE_FIT;
		canvas.uid = 0;
		canvas.view = m_videoWnds[0].GetSafeHwnd();
		canvas.sourceType = VIDEO_SOURCE_CAMERA_PRIMARY;
		//setup local video in the engine to canvas.
		m_rtcEngine->setupLocalVideo(canvas);
		
		m_lstInfo.InsertString(m_lstInfo.GetCount(), _T("setupLocalVideo"));
	}
}


//resume window status
void CAgoraMutilVideoSourceDlg::ResumeStatus()
{
	for (int i = 1; i < m_vecVidoeSourceEventHandler.size(); i++)
	{
		delete m_vecVidoeSourceEventHandler.back();
		m_vecVidoeSourceEventHandler.pop_back();
	}
	InitCtrlText();
	m_joinChannel = false;
	m_initialize = false;
	m_bPublishScreen = false;
	m_btnJoinChannel.EnableWindow(TRUE);
}


void CAgoraMutilVideoSourceDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (bShow) {
		//init control text.
		InitCtrlText();
		//update window.
		RenderLocalVideo();
	}
	else {
		//resume window status.
		ResumeStatus();
	}
}


BOOL CAgoraMutilVideoSourceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	RECT rcArea;
	m_staVideoArea.GetClientRect(&rcArea);
	RECT leftArea = rcArea;
	leftArea.right = (rcArea.right - rcArea.left) / 2;
	RECT rightArea = rcArea;
	rightArea.left = (rcArea.right - rcArea.left) / 2;
	
	for (int i = 0; i < this->VIDOE_COUNT; ++i) {
		m_videoWnds[i].Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CRect(0, 0, 1, 1), this, i);
		//set window background color.
		m_videoWnds[i].SetFaceColor(RGB(0x58, 0x58, 0x58));
	}
	m_videoWnds[0].MoveWindow(&leftArea);
	m_videoWnds[1].MoveWindow(&rightArea);

	//camera screen
	ResumeStatus();
	return TRUE;
}


BOOL CAgoraMutilVideoSourceDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CAgoraMutilVideoSourceDlg::StartDesktopShare()
{
	agora::rtc::Rectangle rc;
	ScreenCaptureParameters scp;
	scp.frameRate = 15;
	scp.bitrate = 0;
	HWND hWnd = ::GetDesktopWindow();
	RECT destop_rc;
	::GetWindowRect(hWnd, &destop_rc);
	scp.dimensions.width = destop_rc.right - destop_rc.left;
	scp.dimensions.height = destop_rc.bottom - destop_rc.top;
	m_rtcEngine->startScreenCaptureByScreenRect(rc, rc, scp);
}

BOOL CAgoraMutilVideoSourceDlg::RegisterVideoFrameObserver(BOOL bEnable, IVideoFrameObserver * videoFrameObserver)
{
	agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
	//query interface agora::AGORA_IID_MEDIA_ENGINE in the engine.
	mediaEngine.queryInterface(m_rtcEngine, AGORA_IID_MEDIA_ENGINE);
	int nRet = 0;

	agora::base::AParameter apm(*m_rtcEngine);
	if (mediaEngine.get() == NULL)
		return FALSE;
	if (bEnable) {
		//register agora video frame observer.
		nRet = mediaEngine->registerVideoFrameObserver(videoFrameObserver);
	}
	else {
		//unregister agora video frame observer.
		nRet = mediaEngine->registerVideoFrameObserver(nullptr);
	}
	return nRet == 0 ? TRUE : FALSE;
}

void CAgoraMutilVideoSourceDlg::OnBnClickedButtonJoinchannel()
{
	if (!m_rtcEngine || !m_initialize)
		return;
	CString strInfo;
	CString strChannelName;
	m_edtChannel.GetWindowText(strChannelName);
	std::string szChannelId = cs2utf8(strChannelName);
	if (!m_joinChannel) {
		if (strChannelName.IsEmpty()) {
			AfxMessageBox(_T("Fill channel name first"));
			return;
		}
		//camera
		agora::rtc::ChannelMediaOptions optionsCamera;
		optionsCamera.autoSubscribeAudio = true;
		optionsCamera.autoSubscribeVideo = true;
		optionsCamera.publishAudioTrack = true;
		optionsCamera.publishCameraTrack = true;
		optionsCamera.publishScreenTrack = false;
		optionsCamera.clientRoleType = CLIENT_ROLE_BROADCASTER;


		if (m_chkRawVideo.GetCheck())
			RegisterVideoFrameObserver(TRUE, &m_observer);

		m_rtcEngine->startPreview();

		//join channel in the engine.
		if (0 == m_rtcEngine->joinChannel(GET_APP_TOKEN, szChannelId.data(), 0, optionsCamera)) {
			//strInfo.Format(_T("join channel %s"), strChannelName);
			m_btnJoinChannel.EnableWindow(FALSE);
			m_conn_camera = DEFAULT_CONNECTION_ID;
		}
		m_strChannel = szChannelId;
		conn_id_t conn_id;
		
		StartDesktopShare();
		agora::rtc::ChannelMediaOptions options;
		options.autoSubscribeAudio = false;
		options.autoSubscribeVideo = false;
		options.publishAudioTrack = false;
		options.publishCameraTrack = false;
		CAgoraMultiVideoSourceEventHandler * p = new CAgoraMultiVideoSourceEventHandler;
		p->SetChannelId(m_vecVidoeSourceEventHandler.size());
		p->SetMsgReceiver(GetSafeHwnd());
		m_vecVidoeSourceEventHandler.push_back(p);
		if (0 == m_rtcEngine->joinChannelEx(GET_APP_TOKEN, m_strChannel.c_str(), 0, options, p, &conn_id))
		{
			m_conn_screen = conn_id;
			m_btnJoinChannel.EnableWindow(FALSE);
		}

		m_chkRawVideo.EnableWindow(FALSE);
	}
	else {
		m_rtcEngine->leaveChannel();
		m_rtcEngine->leaveChannelEx(m_strChannel.data(), m_conn_screen);
		m_rtcEngine->stopPreview();
		if (m_chkRawVideo.GetCheck())
			RegisterVideoFrameObserver(FALSE, nullptr);
		m_strChannel = "";
		m_chkRawVideo.EnableWindow(TRUE);
	}
}


void CAgoraMutilVideoSourceDlg::OnBnClickedButtonPublish()
{
	if (!m_bPublishScreen) {
		if (!m_joinChannel) {
			AfxMessageBox(_T("join channel first"));
			return;
		}
		agora::rtc::ChannelMediaOptions options;
		options.autoSubscribeAudio = false;
		options.autoSubscribeVideo = false;
		options.publishScreenTrack = true;
		options.publishAudioTrack = false;
		options.publishCameraTrack = false;
		options.clientRoleType = CLIENT_ROLE_BROADCASTER;
		m_rtcEngine->updateChannelMediaOptions(options, m_conn_screen);
		m_rtcEngine->startPreview();
		VideoCanvas canvas;
		canvas.uid = 0;
		canvas.sourceType = VIDEO_SOURCE_SCREEN_PRIMARY;
		canvas.view = m_videoWnds[1].GetSafeHwnd();
		m_rtcEngine->setupLocalVideo(canvas);
		m_btnPublish.SetWindowText(MultiVideoSourceCtrlUnPublish);
	}
	else {
		agora::rtc::ChannelMediaOptions options;
		options.autoSubscribeAudio = false;
		options.autoSubscribeVideo = false;
		options.publishScreenTrack = false;
		options.publishAudioTrack = false;
		options.publishCameraTrack = false;
		options.clientRoleType = CLIENT_ROLE_BROADCASTER;
		m_rtcEngine->updateChannelMediaOptions(options, m_conn_screen);
		m_rtcEngine->stopPreview();
		m_btnPublish.SetWindowText(MultiVideoSourceCtrlPublish);
		m_videoWnds[1].Invalidate();

	}	
	m_bPublishScreen = !m_bPublishScreen;
}

//EID_JOINCHANNEL_SUCCESS message window handler.
LRESULT CAgoraMutilVideoSourceDlg::OnEIDJoinChannelSuccess(WPARAM wParam, LPARAM lParam)
{
	int cId = (int)lParam;
	CString strChannelName = utf82cs(m_vecVidoeSourceEventHandler[cId]->GetChannelName());
	m_joinChannel = true;
	m_btnJoinChannel.EnableWindow(TRUE);
	m_btnJoinChannel.SetWindowText(commonCtrlLeaveChannel);
	CString strInfo;
	strInfo.Format(_T("join %s success,cid=%u, uid=%u"), strChannelName, cId, wParam);
	m_lstInfo.InsertString(m_lstInfo.GetCount(), strInfo);
	m_videoWnds[0].SetUID(wParam);

	m_rtcEngine->muteRemoteAudioStream((uid_t)wParam, true, cId);
	m_rtcEngine->muteRemoteVideoStream((uid_t)wParam, true, cId);
	return 0;
}

//EID_LEAVE_CHANNEL message window handler.
LRESULT CAgoraMutilVideoSourceDlg::OnEIDLeaveChannel(WPARAM wParam, LPARAM lParam)
{
	if (!m_joinChannel)
		return 0;
	int cId = (int)wParam;
	CString strChannelName = utf82cs(m_vecVidoeSourceEventHandler[cId]->GetChannelName());
	m_btnJoinChannel.SetWindowText(commonCtrlJoinChannel);
	CString strInfo;
	strInfo.Format(_T("leave channel:%s "), strChannelName);
	m_lstInfo.InsertString(m_lstInfo.GetCount(), strInfo);

	if (cId == 0) {
		m_joinChannel = false;
	}
	else {
		delete m_vecVidoeSourceEventHandler[cId];
		m_vecVidoeSourceEventHandler.erase(m_vecVidoeSourceEventHandler.begin() + cId);
		for (int i = cId; i < m_vecVidoeSourceEventHandler.size(); ++i)
		{
			m_vecVidoeSourceEventHandler[cId]->SetChannelId(m_vecVidoeSourceEventHandler[cId]->GetChannelId() - 1);
		}
	}
	return 0;
}

//EID_USER_JOINED message window handler.
LRESULT CAgoraMutilVideoSourceDlg::OnEIDUserJoined(WPARAM wParam, LPARAM lParam)
{
	int cId = (int)lParam;
	/*if (cId == m_conn_camera)
	{
		agora::rtc::VideoCanvas canvas;
		canvas.renderMode = media::base::RENDER_MODE_FIT;
		canvas.uid = wParam;
		canvas.view = m_videoWnds[1].GetSafeHwnd();
		m_rtcEngine->setupRemoteVideo(canvas, m_conn_camera);
	}*/

	CString strChannelName = utf82cs(m_vecVidoeSourceEventHandler[cId]->GetChannelName());
	CString strInfo;
	strInfo.Format(_T("%u joined %s"), wParam, strChannelName);
	m_lstInfo.InsertString(m_lstInfo.GetCount(), strInfo);
	return 0;
}


//EID_USER_OFFLINE message window handler.
LRESULT CAgoraMutilVideoSourceDlg::OnEIDUserOffline(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

//EID_REMOTE_VIDEO_STATE_CHANED message window handler.
LRESULT CAgoraMutilVideoSourceDlg::OnEIDRemoteVideoStateChanged(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

LRESULT CAgoraMutilVideoSourceDlg::OnEIDConnectionStateChanged(WPARAM wParam, LPARAM lParam)
{
	CONNECTION_CHANGED_REASON_TYPE reason = (CONNECTION_CHANGED_REASON_TYPE)wParam;
	if (reason == CONNECTION_CHANGED_INVALID_TOKEN || reason == CONNECTION_CHANGED_TOKEN_EXPIRED ||
		reason == CONNECTION_CHANGED_INVALID_CHANNEL_NAME || reason == CONNECTION_CHANGED_REJECTED_BY_SERVER ||
		reason == CONNECTION_CHANGED_INVALID_APP_ID) {

		CString info = _T("");
		switch (reason)
		{
		case CONNECTION_CHANGED_INVALID_TOKEN:
		case CONNECTION_CHANGED_INVALID_APP_ID:
			info = invalidTokenlError;
			break;
		case CONNECTION_CHANGED_TOKEN_EXPIRED:
			info = invalidTokenExpiredError;
			break;
		case CONNECTION_CHANGED_INVALID_CHANNEL_NAME:
			info = invalidChannelError;
			break;
		case CONNECTION_CHANGED_REJECTED_BY_SERVER:
			info = refusedByServer;
			break;
		default:
			break;
		}

		if (!info.IsEmpty())
			m_lstInfo.InsertString(m_lstInfo.GetCount(), info);

		m_btnJoinChannel.EnableWindow(TRUE);
	}
	return 0;
}
/*
note:
	Join the channel callback.This callback method indicates that the client
	successfully joined the specified channel.Channel ids are assigned based
	on the channel name specified in the joinChannel. If IRtcEngine::joinChannel
	is called without a user ID specified. The server will automatically assign one
parameters:
	channel:channel name.
	uid: user ID。If the UID is specified in the joinChannel, that ID is returned here;
	Otherwise, use the ID automatically assigned by the Agora server.
	elapsed: The Time from the joinChannel until this event occurred (ms).
*/
void CAgoraMultiVideoSourceEventHandler::onJoinChannelSuccess(const char* channel, agora::rtc::uid_t uid, int elapsed)
{
	m_strChannel = channel;
	if (m_hMsgHanlder) {
		::PostMessage(m_hMsgHanlder, WM_MSGID(EID_JOINCHANNEL_SUCCESS), (WPARAM)uid, (LPARAM)m_channelId);
	}
}

/*
note:
	In the live broadcast scene, each anchor can receive the callback
	of the new anchor joining the channel, and can obtain the uID of the anchor.
	Viewers also receive a callback when a new anchor joins the channel and
	get the anchor's UID.When the Web side joins the live channel, the SDK will
	default to the Web side as long as there is a push stream on the
	Web side and trigger the callback.
parameters:
	uid: remote user/anchor ID for newly added channel.
	elapsed: The joinChannel is called from the local user to the delay triggered
	by the callback(ms).
*/
void CAgoraMultiVideoSourceEventHandler::onUserJoined(agora::rtc::uid_t uid, int elapsed)
{
	if (m_hMsgHanlder) {
		::PostMessage(m_hMsgHanlder, WM_MSGID(EID_USER_JOINED), (WPARAM)uid, (LPARAM)m_channelId);
	}
}
/*
note:
	Remote user (communication scenario)/anchor (live scenario) is called back from
	the current channel.A remote user/anchor has left the channel (or dropped the line).
	There are two reasons for users to leave the channel, namely normal departure and
	time-out:When leaving normally, the remote user/anchor will send a message like
	"goodbye". After receiving this message, determine if the user left the channel.
	The basis of timeout dropout is that within a certain period of time
	(live broadcast scene has a slight delay), if the user does not receive any
	packet from the other side, it will be judged as the other side dropout.
	False positives are possible when the network is poor. We recommend using the
	Agora Real-time messaging SDK for reliable drop detection.
parameters:
	uid: The user ID of an offline user or anchor.
	reason:Offline reason: USER_OFFLINE_REASON_TYPE.
*/
void CAgoraMultiVideoSourceEventHandler::onUserOffline(agora::rtc::uid_t uid, agora::rtc::USER_OFFLINE_REASON_TYPE reason)
{
	if (m_hMsgHanlder) {
		::PostMessage(m_hMsgHanlder, WM_MSGID(EID_USER_OFFLINE), (WPARAM)uid, (LPARAM)m_channelId);
	}
}
/*
note:
	When the App calls the leaveChannel method, the SDK indicates that the App
	has successfully left the channel. In this callback method, the App can get
	the total call time, the data traffic sent and received by THE SDK and other
	information. The App obtains the call duration and data statistics received
	or sent by the SDK through this callback.
parameters:
	stats: Call statistics.
*/
void CAgoraMultiVideoSourceEventHandler::onLeaveChannel(const agora::rtc::RtcStats& stats)
{
	if (m_hMsgHanlder) {
		::PostMessage(m_hMsgHanlder, WM_MSGID(EID_LEAVE_CHANNEL), (WPARAM)m_channelId, 0);
	}
}

void CAgoraMultiVideoSourceEventHandler::onRemoteVideoStateChanged(agora::rtc::uid_t uid, agora::rtc::REMOTE_VIDEO_STATE state, agora::rtc::REMOTE_VIDEO_STATE_REASON reason, int elapsed)
{
	if (m_hMsgHanlder) {
		::PostMessage(m_hMsgHanlder, WM_MSGID(EID_REMOTE_VIDEO_STATE_CHANED), (WPARAM)uid, (LPARAM)m_channelId);
	}
}
////////////////////////////////////////////////////////////////////////////
bool CVideoSourceObserver::onScreenCaptureVideoFrame(VideoFrame& videoFrame)
{
	OutputDebugString(_T("###########################################\n"));
	OutputDebugString(_T("onScreenCaptureVideoFrame\n"));
	OutputDebugString(_T("###########################################\n"));
	return true;
}


bool CVideoSourceObserver::onCaptureVideoFrame(VideoFrame& videoFrame)
{
	OutputDebugString(_T("###########################################\n"));
	OutputDebugString(_T("onCaptureVideoFrame\n"));
	OutputDebugString(_T("###########################################\n"));
	return true;
}


void CAgoraMutilVideoSourceDlg::OnBnClickedCheckRawVideo()
{
	
}

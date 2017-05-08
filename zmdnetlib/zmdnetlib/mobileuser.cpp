#include "mobileuser.h"
#include "mediamgr.h"
#include "mobileserver.h"
#include "netserver.h"
#include "zmdcrypt.h"
#include "tcplibdef.h"
#include "CommonDefine.h"
#include "ptz.h"
#include "EncodeManage.h"
extern PARAMETER_MANAGE		*g_cParaManage ;
extern int TARGET_BIT_RATE_TABLE[8][5][5];


CMobileUser::CMobileUser()
{

}

CMobileUser::~CMobileUser()
{

}

void CMobileUser::ResetUser( )
{
    m_bStartSendMedia = 0 ;
    m_mediaSid = INVALID_USERID ;
    m_mediaCmd = 0 ;
    m_sessionHandle = 0 ;
    m_Channel = 0 ;
    m_nUserID = INVALID_USERID ;
    m_alarmID = INVALID_USERID ;
    m_remoteIP = 0 ;
    m_ChannelType = QVGA_CHN_TYPE ;
    m_packetSeq = 0 ;
    m_frameCount = 0 ;
}

void CMobileUser::onClose()
{
    //=====================================
    //关闭用户
    m_sessionHandle = 0 ;
    m_bStartSendMedia = 0 ;

    //如果正在接收视频，关闭媒体会话
    if( INVALID_USERID != m_mediaSid )
    {
        GetMediaMgr()->freeMediaSession( m_Channel , m_ChannelType , m_mediaSid ) ;
        m_mediaSid = INVALID_SESSION ;
        m_bStartSendMedia = 0 ;
    }

    //释放工作会话
    GetMobileServerObj()->freeWorkSession( m_nUserID ) ;

    ResetUser( ) ;
}

bool CMobileUser::onReceive( char *data , int len , int &used )
{
    //从数据流中拆分数据包
    char *start = (char *)data ;
    char *end = (char *)data + len ;

    unsigned int packetSize = 0 ;

    while( start + sizeof( OwspPacketHeader ) < end )
    {
        OwspPacketHeader *pHeader = (OwspPacketHeader *)data ;

        packetSize = ntohl( pHeader->packet_length ) ;
        //内存中的数据长度不包含长度头，此处我们加上长度头
        packetSize += sizeof( packetSize ) ;

        if( packetSize > VGA_FRAME_BUF_SIZE )  
            return false ;

        if( end - start  < packetSize )
        {
            //left data not a complete packet.the used data return to the up.
            used = start - data ;
            return true ;
        }
        if( !onPacketProcesser( start , packetSize ) )
        {
            printf( "onPacketProcesser ret false !!! close the session \r\n" ) ;
            return false ;
        }
        start += packetSize ;
    }
    used = start - data;
    return true ;

}

bool CMobileUser::onIdle()
{
    //是否需要发送媒体数据
    if( m_bStartSendMedia )
    {
        SendVideoLiving( ) ;
    }

    return true ;

}

//-----------------------------------------------
//数据包命令解析
bool CMobileUser::onPacketProcesser( char *data , int len )
{
    OwspPacketHeader *pOwspHeader = (OwspPacketHeader *)data ;
    //检查Owsp序号，以及长度,

    printf( "recv Owsp packet , lent = %d \r\n" , ntohl( pOwspHeader->packet_length ) ) ;

    char *start = (char *)data ;
    char *end = data + len ;

    //跳过OwspPacketHeader
    start += sizeof(OwspPacketHeader) ;

    //开始处理TLV包,有可能会有多个连续TLV包
    while( start + sizeof( TLV_HEADER ) < data + len )
    {
        TLV_HEADER *pHeader = (TLV_HEADER *)start ;
        int tlvLen = pHeader->tlv_len ;
        tlvLen += sizeof( TLV_HEADER ) ;

        if( tlvLen > VGA_FRAME_BUF_SIZE )//256*1024->VGA_FRAME_BUF_SIZE js MODIFY compare
            return false ;

        //处理Tlv包
        if( !processerTlvPacket( start , tlvLen ) )
            return false ;
        //跳过已经处理的tlv包
        start += tlvLen ;
    }

    return (start == data + len );
}
//===========================================
//处理TLV数据包
bool CMobileUser::processerTlvPacket( char *data , int len )
{
    bool bRet = true ;

    TLV_HEADER *pTlvHeader = (TLV_HEADER *)data ;
    char *pakcet = data + sizeof( TLV_HEADER ) ;

    switch( pTlvHeader->tlv_type )
    {
    case TLV_T_VERSION_INFO_REQUEST:
        bRet = onVersionInfoRequest( pakcet , pTlvHeader->tlv_len ) ;
        break;

    case TLV_T_PHONE_INFO_REQUEST:
        bRet = onPhoneInfoRequest( pakcet , pTlvHeader->tlv_len ) ;
        break;

    case TLV_T_LOGIN_REQUEST:
        bRet = onLoginRequest( pakcet , pTlvHeader->tlv_len ) ;
        break;

    case TLV_T_CHANNLE_REQUEST:
        bRet = onChannelRequest( pakcet , pTlvHeader->tlv_len ) ;
        break;

    case TLV_T_CONTROL_REQUEST:
        bRet = onControlRequest( pakcet , pTlvHeader->tlv_len ) ;
        break;

    default :
        printf( "******CMobileUser recv unknow TLV packet!!!****\r\n" ) ;
        break ;

    }

    return bRet ;
}

bool CMobileUser::onVersionInfoRequest( char *data , int len )
{
    printf( "************onVersionInfoRequest len = %d\r\n" , len ) ;



    return true ;
}

bool CMobileUser::onPhoneInfoRequest( char *data , int len )
{
    printf( "************onPhoneInfoRequest len = %d\r\n" , len ) ;
    return true ;
}

bool CMobileUser::onChannelRequest( char *data , int len )
{
    TLV_V_ChannelRequest *pReq = (TLV_V_ChannelRequest *)data ;
    int echo = 0 ;

    printf( "*************onChannelRequest len = %d channel = %d \r\n" , len , pReq->destChannel) ;

    if( m_Channel != pReq->destChannel )
    {
        //释放前期的媒体通道
        GetMediaMgr()->freeMediaSession( m_Channel , m_ChannelType , m_mediaSid ) ;
        //重新申请
        echo = RequestStartVideoLiving( pReq->destChannel ) ;
    }

    if( _RESPONSECODE_SUCC == echo )
    {
        m_Channel = pReq->destChannel ;
    }

    printf( "onChannelRequest echo = %d\r\n" , echo ) ;

    ChanageChannelResponse resp ;
    memset( &resp , 0 , sizeof( resp ));

    resp.ChannelRes.HEADER.tlv_type = TLV_T_CHANNLE_ANSWER;
    resp.ChannelRes.HEADER.tlv_len = sizeof(TLV_V_ChannelResponse);
    resp.ChannelRes.Channel.currentChannel = m_Channel ;
    resp.ChannelRes.Channel.result = echo;

    resp.Format.HEADER.tlv_type = TLV_T_STREAM_FORMAT_INFO;
    resp.Format.HEADER.tlv_len = sizeof(TLV_V_StreamDataFormat);
    resp.Format.DataFormat.videoChannel = m_Channel ;
    resp.Format.DataFormat.dataType = OWSP_SDT_VIDEO_ONLY;
    resp.Format.DataFormat.videoFormat.bitrate = 100000;//SUBSTREAMBITRATE;
    resp.Format.DataFormat.videoFormat.codecId = CODEC_H264 ;
    resp.Format.DataFormat.videoFormat.colorDepth = 24 ;
    resp.Format.DataFormat.videoFormat.framerate = 25 ;//SUBSTREAMFRAMERATE;
    resp.Format.DataFormat.videoFormat.height = 240 ;//SUBSTREAMHIGHT;
    resp.Format.DataFormat.videoFormat.width = 320 ;//SUBSTREAMWIDE;

    resp.PacketHeader.packet_length = htonl( sizeof(ChanageChannelResponse) - sizeof(u_int32));
    resp.PacketHeader.packet_seq = GetSeq() ;

    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;

    return true ;
}

bool CMobileUser::onLoginRequest( char *data , int len )
{
    printf( "***************onLoginRequest() len = %d\r\n" , len ) ;

    if( m_bStartSendMedia )
    {
        printf( "The session already send living media!!!m_bStartSendMedia = %d\r\n" , m_bStartSendMedia ) ;
        return true ;
    }

    TLV_V_LoginRequest *pReq = (TLV_V_LoginRequest *)data ;
    int permit = 0 ;
    int echo = GetNetServerObj()->VerifyUser( pReq->userName, pReq->password , permit ) ;
    if( echo )
    {
        ZmdDeCrypt( pReq->userName , (char *)CRYPT_KEY ) ;
        ZmdDeCrypt( pReq->password , (char *)CRYPT_KEY ) ;
        echo = GetNetServerObj()->VerifyUser( pReq->userName, pReq->password , permit ) ;
    }

    m_Channel = pReq->channel ;

    //检查channel.
    if( m_Channel >= IPC_CLIENT_COUNT || m_Channel < 0 )
        m_Channel = 0 ;

    ///成功则启动视频直播
    if( !echo )
    {
        echo = RequestStartVideoLiving( m_Channel ) ;
    }

    printf( "CMobileUser::onLoginRequest() echo = %d\r\n" , echo ) ;

    //标志开始直播
    if( _RESPONSECODE_SUCC == echo )
    {
        m_frameCount = 0 ;
        m_bStartSendMedia = 1 ;
        SendLoginSuccess( ) ;
    }
    else
    {

        LoginFaileResponse		resp ;

        resp.PacketHeader.packet_length = htonl( sizeof( resp ) - sizeof(u_int32) + sizeof(VersionResponse) + sizeof(LoginResponse));
        resp.PacketHeader.packet_seq = GetSeq() ;
        resp.VersionRes.HEADER.tlv_type = TLV_T_VERSION_INFO_REQUEST;
        resp.VersionRes.HEADER.tlv_len = sizeof(TLV_V_VersionInfoRequest);
        memcpy(&resp.VersionRes.Version, &m_VersionInfoRequest, sizeof(TLV_V_VersionInfoRequest));

        resp.LoginRes.HEADER.tlv_type = TLV_T_LOGIN_ANSWER;
        resp.LoginRes.HEADER.tlv_len = sizeof(TLV_V_LoginResponse);
        resp.LoginRes.Login.result = _RESPONSECODE_SUCC ;
        resp.LoginRes.Login.reserve = 0 ;

        TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;
        return false ;

    }

    return true ;
}

int	CMobileUser::RequestStartVideoLiving( int channel )
{
    if( channel < 0 || channel >= IPC_CLIENT_COUNT )
        return _RESPONSECODE_INVALID_CHANNLE ;


    //获取一个空闲媒体会话
    int mediaid = GetMediaMgr()->getUnuseMediaSession( m_Channel , m_ChannelType ) ;
    if( INVALID_SESSION == mediaid )
    {
        printf( "can't get unuse media session !!\r\n" ) ;
        return _RESPONSECODE_DEVICE_OVERLOAD ;
    }

    printf( "IPC_CLIENT_COUNT = %d , channel = %d , mediaid = %d\r\n" , IPC_CLIENT_COUNT , m_Channel , mediaid ) ;

    m_mediaSid = mediaid ;
	
//	ResetUserData2IFrame(  2 , mediaid );

	return _RESPONSECODE_SUCC ;
}

int CMobileUser::SendLoginSuccess(  )
{
    LoginSuessResponse			SuessResponse;
    CAMERA_PARA  				encode_para;
    u_int32						FirstFrame = 0;
    int							resolution = -1;
    int							bitrate = 0;
    int							framerate = 0;
    int							width = 0;
    int							heigth = 0;
	
    g_cParaManage->GetSysParameter(SYSCAMERA_SET, (void *)&encode_para);

    width = 320;
    heigth = 240;

    int	quality = 0, res = 0, rqfrat = 0, norm = 0;
    framerate = encode_para.m_ChannelPara[m_Channel].m_uSubFrameRate;
    res = encode_para.m_ChannelPara[m_Channel].m_uSubRes;

    framerate = 25;
	
    if((encode_para.m_ChannelPara[m_Channel].m_uChannelValid) == 0)
        return 0;
    if(encode_para.m_ChannelPara[m_Channel].m_uSubEncSwitch == 0)
        return 0;

    quality = encode_para.m_ChannelPara[m_Channel].m_uSubQuality % 5;

    rqfrat = 0;
    if(framerate == 30)
        rqfrat = 0;
    else if(framerate >= 15)
        rqfrat = 1;
    else if(framerate >= 7)
        rqfrat = 2;
    else if(framerate >= 3)
        rqfrat = 3;
    else
        rqfrat = 4;

    //bitrate = TARGET_BIT_RATE_TABLE[res][quality][rqfrat];
    bitrate = 100000 ;

    printf("mobile video para:%d %d %d %d\n", framerate, bitrate, width, heigth);

    memset(&SuessResponse, 0x0, sizeof(LoginSuessResponse));

    SuessResponse.VersionRes.HEADER.tlv_type = TLV_T_VERSION_INFO_REQUEST;
    SuessResponse.VersionRes.HEADER.tlv_len = sizeof(TLV_V_VersionInfoRequest);
    memcpy(&SuessResponse.VersionRes.Version, &m_VersionInfoRequest, sizeof(TLV_V_VersionInfoRequest));

    SuessResponse.DVSInfo.HEADER.tlv_type = TLV_T_DVS_INFO_REQUEST;
    SuessResponse.DVSInfo.HEADER.tlv_len = sizeof(TLV_V_DVSInfoRequest);
    SuessResponse.DVSInfo.info.channleNumber = IPC_CLIENT_COUNT ;
    SuessResponse.DVSInfo.info.equipmentDate.m_day = 11;
    SuessResponse.DVSInfo.info.equipmentDate.m_month = 5;
    SuessResponse.DVSInfo.info.equipmentDate.m_year = 2012;
    memcpy(SuessResponse.DVSInfo.info.companyIdentity , "2Ez0Pm1To2Cd0Oo5", 15);
    memcpy(SuessResponse.DVSInfo.info.equipmentIdentity, "001122144514", 12);
    memcpy(SuessResponse.DVSInfo.info.equipmentName, "Zmodo", 5);
    memcpy(SuessResponse.DVSInfo.info.equipmentVersion, "T_20120511", 10);

    SuessResponse.ChannelRes.HEADER.tlv_type = TLV_T_CHANNLE_ANSWER;
    SuessResponse.ChannelRes.HEADER.tlv_len = sizeof(TLV_V_ChannelResponse);
    SuessResponse.ChannelRes.Channel.currentChannel = m_Channel ;
    SuessResponse.ChannelRes.Channel.result = _RESPONSECODE_SUCC;

    SuessResponse.Format.HEADER.tlv_type = TLV_T_STREAM_FORMAT_INFO;
    SuessResponse.Format.HEADER.tlv_len = sizeof(TLV_V_StreamDataFormat);
    SuessResponse.Format.DataFormat.videoChannel = m_Channel ;
    SuessResponse.Format.DataFormat.dataType = OWSP_SDT_VIDEO_ONLY;
    SuessResponse.Format.DataFormat.videoFormat.bitrate = bitrate;//SUBSTREAMBITRATE;
    SuessResponse.Format.DataFormat.videoFormat.codecId = CODEC_H264;
    SuessResponse.Format.DataFormat.videoFormat.colorDepth = 24;
    SuessResponse.Format.DataFormat.videoFormat.framerate = framerate;//SUBSTREAMFRAMERATE;
    SuessResponse.Format.DataFormat.videoFormat.height = heigth;//SUBSTREAMWIDE;
    SuessResponse.Format.DataFormat.videoFormat.width = width;//SUBSTREAMHIGHT;

    SuessResponse.PacketHeader.packet_length = htonl( sizeof(SuessResponse) - sizeof( u_int32 ) );
    SuessResponse.PacketHeader.packet_seq = GetSeq() ;

    TcpBlockSend( m_sessionHandle , (char *)&SuessResponse , sizeof( SuessResponse ) ) ;

    return true ;
}

bool CMobileUser::onControlRequest( char *data , int len )
{
    printf( "onControlRequest len = %d\r\n" , len ) ;

    TLV_V_ControlRequest *pReq = (TLV_V_ControlRequest *)data ;

    printf("ptz control cmd:%d CH %d ID %ld\n", pReq->cmdCode, pReq->channel, pReq->deviceId);
	
#ifdef APP3518
	switch(pReq->cmdCode)
	{
		case OWSP_ACTION_MD_UP:
			printf( "OWSP_ACTION_MD_UP\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_UP,0,0);
			break;
		case OWSP_ACTION_MD_DOWN:
			printf( "OWSP_ACTION_MD_DOWN\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_DOWN,0,0);
			break;
		case OWSP_ACTION_MD_LEFT:
			printf( "OWSP_ACTION_MD_LEFT\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_LEFT,0,0);
			break;
		case OWSP_ACTION_MD_RIGHT:
			printf( "OWSP_ACTION_MD_RIGHT\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_RIGHT,0,0);
			break;
		case OWSP_ACTION_ZOOMReduce:
			printf( "OWSP_ACTION_ZOOMReduce\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_ZOOMWIDE,0,0);
			break;
		case OWSP_ACTION_ZOOMADD:
			printf( "OWSP_ACTION_ZOOMADD\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_ZOOMTELE,0,0);
			break;
		case OWSP_ACTION_FOCUSADD:
			printf( "OWSP_ACTION_FOCUSADD\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_FOCUSFAR,0,0);
			break;
		case OWSP_ACTION_FOCUSReduce:
			printf( "OWSP_ACTION_FOCUSReduce\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_FOCUSNAER,0,0);
			break;
		case OWSP_ACTION_GOTO_PRESET_POSITION:
			printf( "OWSP_ACTION_GOTO_PRESET_POSITION\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_CALLPRESET,pReq->size,0);
			break;
		case OWSP_ACTION_SET_PRESET_POSITION:
			printf( "OWSP_ACTION_SET_PRESET_POSITION\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_SETPRESET,pReq->size,0);
			break;
		case OWSP_ACTION_CLEAR_PRESET_POSITION:
			printf( "OWSP_ACTION_CLEAR_PRESET_POSITION\r\n" ) ;
			//PTZCtrlAction(pReq->channel,CMD_SETPRESET,pReq->size,0);
			break;
		case OWSP_ACTION_MD_STOP:
			printf( "OWSP_ACTION_MD_STOP\r\n" ) ;
			PTZ_Operate(pReq->channel,CMD_STOP,0,0);
			break;

	}
#endif

    PtzCtrResponse resp ;

    resp.PacketHeader.packet_length = htonl(sizeof(PtzResponse) - sizeof(u_int32) );
    resp.PacketHeader.packet_seq = GetSeq() ;
    resp.ptz.HEADER.tlv_len = sizeof(TLV_V_ControlResponse) ;
    resp.ptz.HEADER.tlv_type = TLV_T_CONTROL_ANSWER;
    resp.ptz.ControlResponse.result = _RESPONSECODE_SUCC;

    TcpBlockSend( m_sessionHandle , (char *)&resp , sizeof( resp ) ) ;

    //===========================================
    //接收到ptz信令后，关闭该连接
    return false ;
}

int CMobileUser::SendVideoLiving( )
{
    unsigned char *data = 0 ;
    FrameInfo info ;
    int datalen = 0 ;

    //从buffermanager获取媒体流
    int getRet = 0 ;

	do
	{
		int streamtpye = 2 ;
		getRet = GetSendNetFrame(m_Channel, streamtpye , m_mediaSid , &data , &info ) ;

        if( getRet != -1 && data )
        {

            if( strncmp( (char *)data , "01wb" , 4 ) == 0 )
                continue ;

            if( info.Flag == 3 )
                continue ;

            int packetLen = sizeof( VideoFrame ) + info.FrmLength - sizeof( VideoFrameHeader ) ;
            VideoFrame	*pVideoFrame = (VideoFrame *)m_FrameBuf ;
            pVideoFrame->PacketHeader.packet_length = htonl( packetLen - sizeof( u_int32 ) );
            pVideoFrame->PacketHeader.packet_seq = GetSeq() ;

            pVideoFrame->FrameInfo.HEADER.tlv_len = sizeof( TLV_V_VideoFrameInfo ) ;
            pVideoFrame->FrameInfo.HEADER.tlv_type = TLV_T_VIDEO_FRAME_INFO ;

            pVideoFrame->FrameInfo.VideoInfo.channelId = m_Channel ;
            pVideoFrame->FrameInfo.VideoInfo.frameIndex = GetFrameCount( ) ;
            pVideoFrame->FrameInfo.VideoInfo.time = info.Pts;

            pVideoFrame->FrameData.HEADER.tlv_len = info.FrmLength - sizeof(VideoFrameHeader);
            if(info.Flag == 1)
                pVideoFrame->FrameData.HEADER.tlv_type = TLV_T_VIDEO_IFRAME_DATA;
            if(info.Flag == 2)
                pVideoFrame->FrameData.HEADER.tlv_type = TLV_T_VIDEO_PFRAME_DATA;

            memcpy( m_FrameBuf + sizeof( VideoFrame ) , data + sizeof( VideoFrameHeader ) ,
                    pVideoFrame->FrameData.HEADER.tlv_len ) ;

            int nSend = TcpBlockSend( m_sessionHandle , (char *)m_FrameBuf , packetLen ) ;
            //发送失败,返回false,通知底层
            //printf( "SendLivingData() !!! len = %d \r\n" , nSend ) ;
            if( nSend <= 0 )
            {
                return false ;
            }

        }
    }
    while( datalen && data );

    return true ;
}







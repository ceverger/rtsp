#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "network.h"
#include "rtsp.h"

void rtspParserGetMethod(const char *message, char *buf, int bufsize)
{
	if(message == NULL || buf == NULL) return;

	bzero(buf, bufsize);

	for(int i = 0; i < bufsize && message[i] != ' '; ++i)
	{
		buf[i] = message[i];
	}
}

void rtspParserGetURI(const char *message, char *buf, int bufsize)
{
	if(message == NULL || buf == NULL) return;
	
	bzero(buf, bufsize);

	const char *pos = message;

	while(*pos++ != ' ');

	for(int i = 0; i < bufsize && pos[i] != ' '; ++i)
	{
		buf[i] = pos[i];
	}
}

void rtspParserGetFilename(const char *uri, char *buf, int bufsize)
{
	if(uri == NULL || buf == NULL) return;
	
	bzero(buf, bufsize);

	char *pos = strstr(uri, "rtsp://");
	if(pos == NULL) return;

	pos += strlen("rtsp://");

	while(*pos != '/' && *pos != '\0') pos++;
	if(*pos == '/') pos++;

	for(int i = 0; i < bufsize && *pos != '\0'; ++i)
	{
		buf[i] = pos[i];
	}
}

int rtspParserGetCSeq(const char *message)
{
    char bufsize = 8;
    char buf[bufsize];
    char *buf_pos = buf;

    char *pos = strstr(message, "CSeq: ");
    if(pos == NULL) return -1;

    while(*pos++ != ' '); 
   
    while(*pos != '\r') 
		*buf_pos++ = *pos++;

    *buf_pos = '\0';

    return strtol(buf, NULL, 10);
}

void rtspParserGetSession(const char *message, char *buf, int bufsize)
{
	if(message == NULL || buf == NULL) return;
	
	bzero(buf, bufsize);

	char *pos = strstr(message, "Session:");
	if(pos == NULL) return;

	while(*pos++ != ' ');
	while(*pos != ';' && *pos != '\r') *buf++ = *pos++;
}

void rtspParserGetClientPorts(const char *message, int *clientRtpPort, int *clientRtcpPort)
{
	char bufsize = 8;
   char buf[bufsize];
   char *buf_pos = buf;

	char *pos = strstr(message, "client_port");
	if(pos == NULL) return;
	
	while(*pos++ != '=');

	while(*pos != '-') *buf_pos++ = *pos++;	
	*buf_pos = '\0';

	*clientRtpPort = strtol(buf, NULL, 10);
	buf_pos = buf;
	pos++;

	while(*pos != '\r' && *pos != ';') *buf_pos++ = *pos++;
	*buf_pos = '\0';

	*clientRtcpPort = strtol(buf, NULL, 10);
}

void rtspParserGetServerPorts(const char *message, int *serverRtpPort, int *serverRtcpPort)
{
	char bufsize = 8;
   char buf[bufsize];
   char *buf_pos = buf;

	char *pos = strstr(message, "server_port");
	if(pos == NULL) return;
	
	while(*pos++ != '=');

	while(*pos != '-') *buf_pos++ = *pos++;	
	*buf_pos = '\0';

	*serverRtpPort = strtol(buf, NULL, 10);
	buf_pos = buf;
	pos++;

	while(*pos != '\r' && *pos != ';') *buf_pos++ = *pos++;
	*buf_pos = '\0';

	*serverRtcpPort = strtol(buf, NULL, 10);
}

int rtspRequestForOption(
								 int cseq, 
                         const char *uri,
								 const char *agent, 
								 char *buf,
								 int bufsize
								)
{
	int ret = 0;
	char *pos = buf;
	int cacheLen = 1024;
	char cache[cacheLen];

	bzero(buf, bufsize);

	ret = snprintf(cache, cacheLen, "OPTIONS %s RTSP/1.0\r\n", uri);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "CSeq: %d\r\n", cseq);
	strncpy(pos, cache, ret);
	pos += ret;	

	ret = snprintf(cache, cacheLen, "User-Agent: %s\r\n\r\n", agent);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = strlen(buf);

	return ret;	
}

int rtspRequestForDescribe(
									int cseq, 
									const char *uri, 
									const char *agent, 
									const char *format,
								   char *buf,
								   int bufsize
								  )
{
	int ret = 0;
	char *pos = buf;
	int cacheLen = 1024;
	char cache[cacheLen];

	bzero(buf, bufsize);

	ret = snprintf(cache, cacheLen, "DESCRIBE %s RTSP/1.0\r\n", uri);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "CSeq: %d\r\n", cseq);
	strncpy(pos, cache, ret);
	pos += ret;	

	ret = snprintf(cache, cacheLen, "User-Agent: %s\r\n", agent);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "Accept: %s\r\n\r\n", format);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = strlen(buf);

	return ret;	
}

int rtspRequestForSetup(
								int cseq,
								const char *uri,
								const char *agent,
								const char *transport,
								const char *type,
								int clientRtpPort,
								int clientRtcpPort,
								char *buf,
								int bufsize		
							  )
{
	int ret;
	char *pos = buf;
	int cacheLen = 1024;
	char cache[cacheLen];

	bzero(buf, bufsize);

	ret = snprintf(cache, cacheLen, "SETUP %s RTSP/1.0\r\n", uri);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "CSeq: %d\r\n", cseq);
	strncpy(pos, cache, ret);
	pos += ret;	

	ret = snprintf(cache, cacheLen, "User-Agent: %s\r\n", agent);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "Transport: %s;%s;client_port=%d-%d\r\n\r\n", transport, type, clientRtpPort, clientRtcpPort);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = strlen(buf);

	return ret;
		
}

int rtspRequestForPlay(
								int cseq, 
								const char *uri,
								const char *agent,
								const char *session,
							   const char *range,
								char *buf,
								int bufsize								
							  )
{
	int ret;
	char *pos = buf;
	int cacheLen = 1024;
	char cache[cacheLen];

	bzero(buf, bufsize);

	ret = snprintf(cache, cacheLen, "PLAY %s RTSP/1.0\r\n", uri);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "CSeq: %d\r\n", cseq);
	strncpy(pos, cache, ret);
	pos += ret;	

	ret = snprintf(cache, cacheLen, "User-Agent: %s\r\n", agent);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "Session: %s\r\n", session);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "Range: %s\r\n\r\n", range);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = strlen(buf);

	return ret;
}

int rtspResponseForOption(
								  int cseq,
								  char *buf, 
								  int bufsize
								 )
{
	int ret;
	char *pos = buf;
	int cacheLen = 1024;
	char cache[cacheLen];

	time_t tm = time(NULL);
	struct tm *gm = gmtime(&tm);

	bzero(buf, bufsize);

	ret = snprintf(cache, cacheLen, "RTSP/1.0 200 OK\r\n");
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "CSeq: %d\r\n", cseq);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = strftime(cache, cacheLen, "Date: %a, %b %d %Y %T GMT\r\n", gm);
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cacheLen, "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER\r\n\r\n");
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);

	ret = strlen(buf);

	return ret;
}


int rtspResponseForDescribe(
									 int cseq,
									 const char *uri,
									 const char *sdp,
									 int sdpLen,
									 const char *serverIP,
									 int serverRtpPort,
									 char *buf, 
									 int bufsize
                           )
{
	int ret;
	int len = 0;
	char *pos = buf;

	int cacheLen = 2048;
	char cache[cacheLen];

	time_t tm = time(NULL);
	struct tm *gm = gmtime(&tm);

	bzero(buf, bufsize);

	ret = snprintf(cache, cacheLen, "RTSP/1.0 200 OK\r\n");
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;
	len += ret;

	ret = snprintf(cache, cacheLen, "CSeq: %d\r\n", cseq);
	strncpy(pos, cache, ret);
	pos += ret;
	len += ret;

	ret = strftime(cache, cacheLen, "Date: %a, %b %d %Y %T GMT\r\n", gm);
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;
	len += ret;

	ret = snprintf(cache, cacheLen, "Content-Base: %s\r\n", uri);
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;
	len += ret;

	ret = snprintf(cache, cacheLen, "Content-Type: application/sdp\r\n");
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;
	len += ret;

	ret = snprintf(cache, cacheLen, "Content-Length: %d\r\n\r\n", sdpLen);
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;
	len += ret;

	strncpy(pos, sdp, sdpLen);
	len += sdpLen;

	return len;
}

int rtspResponseForSetup(
								 int cseq,
								 const char *serverIP,
								 int serverRtpPort,
								 int serverRtcpPort,
								 const char *clientIP,
								 int clientRtpPort,
								 int clientRtcpPort,
								 const char *sessionID,
								 char *buf,
								 int bufsize
                        )
{
	int ret;
	char *pos = buf;

	int cache_len = 1024;
	char cache[cache_len];

	time_t tm = time(NULL);
	struct tm *gm = gmtime(&tm);

	bzero(buf, bufsize);

	ret = snprintf(cache, cache_len, "RTSP/1.0 200 OK\r\n");
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cache_len, "CSeq: %d\r\n", cseq);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = strftime(cache, cache_len, "Date: %a, %b %d %Y %T GMT\r\n", gm);
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(
						cache, 
						cache_len, 
						"Transport: RTP/AVP;mode=PLAY;unicast;source=%s;client_port=%d-%d;server_port=%d-%d\r\n",
						serverIP,
					   clientRtpPort,
					   clientRtcpPort,
						serverRtpPort,
						serverRtcpPort
					  );
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cache_len, "Session: %s\r\n\r\n", sessionID);
	strncpy(pos, cache, ret);

	ret = strlen(buf);

	return ret;
}

int rtspResponsePlay(
							int cseq,
						   const char *uri,
							const char *sessionID,
					      long ssrc,
					      int timestamp,
					      char *buf, 
					      int bufsize
					     )
{
	int ret;
	char *pos = buf;

	int cache_len = 1024;
	char cache[cache_len];

	time_t tm = time(NULL);
	struct tm *gm = gmtime(&tm);

	bzero(buf, bufsize);

	ret = snprintf(cache, cache_len, "RTSP/1.0 200 OK\r\n");
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cache_len, "CSeq: %d\r\n", cseq);
	strncpy(pos, cache, ret);
	pos += ret;

	ret = strftime(cache, cache_len, "Date: %a, %b %d %Y %T GMT\r\n", gm);
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cache_len, "Range: npt=0.000-\r\n");
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);
	pos += ret;

	ret = snprintf(cache, cache_len, "Session: %s\r\n", sessionID);
	if(ret < 0) return -1;
	strncpy(pos, cache, ret);

	return 0;
}

bool rtspMethodIsOptions(const char *message)
{
    return strstr(message, "OPTIONS") == NULL ? false : true; 
}

bool rtspMethodIsDescribe(const char *message)
{
    return strstr(message, "DESCRIBE") == NULL ? false : true; 
}

bool rtspMethodIsSetup(const char *message)
{
    return strstr(message, "SETUP") == NULL ? false : true; 
}

bool rtspMethodIsPlay(const char *message)
{
    return strstr(message, "PLAY") == NULL ? false : true; 
}

bool rtspMethodIsTeardown(const char *message)
{
    return strstr(message, "TEARDOWN") == NULL ? false : true;
}


int rtspCreateSDP(
						const char *serverIP, 
						int serverRtpPort, 
						int framerate, 
						char *buf, 
						int bufsize
					  )
{
	int ret;
	int len = 0;
	char *pos = buf;

	ret = snprintf(pos, bufsize - len, "v=0\r\n");
	if(ret < 0) return -1;
	pos += ret;
	len += ret;

	ret = snprintf(pos, bufsize - len, "o=- 1714468405722913 1 IN IP4 %s\r\n", serverIP);
	if(ret < 0) return -1;
	pos += ret;
	len += ret;

	ret = snprintf(pos, bufsize - len, "s=H.264 Video, streamed by the Rutronica Media Server\r\n");
	if(ret < 0) return -1;
	pos += ret;
	len += ret;

	ret = snprintf(pos, bufsize - len, "m=video 0 RTP/AVP 96\r\n");
	if(ret < 0) return -1;
	pos += ret;
	len += ret;

	ret = snprintf(pos, bufsize - len, "a=rtpmap:96 H264/90000\r\n");
	if(ret < 0) return -1;
	pos += ret;
	len += ret;

	ret = snprintf(pos, bufsize - len, "a=framerate:%d\r\n", framerate);
	if(ret < 0) return -1;
	pos += ret;
	len += ret;

	ret = snprintf(pos, bufsize - len, "c=IN IP4 %s\r\n\r\n\r\n", serverIP);
	if(ret < 0) return -1;
	pos += ret;
	len += ret;
	buf[len] = '\0';

	return len;
}


int rtspReceiveMessage(int rtspSocket, char *buf, int bufsize)
{
	int ret, len = 0;
	char *pos = buf;

	bzero(buf, bufsize);
	
	while(strstr(buf, "\r\n\r\n") == NULL)
	{
		ret = tcpSocketReceive(rtspSocket, pos, bufsize);
		if(ret <= 0) return -1;
		pos += ret;
		len += ret;		
	}

	return len;
}

int rtspSendMessage(int rtspSocket, char *buf, int buflen)
{
	int ret;
	int len = 0;
	char *pos = buf;

	while(len < buflen)
	{
	 	ret = tcpSocketSend(rtspSocket, pos, buflen);
		if(ret <= 0) return -1;
		pos += ret;
		len += ret;
	}

	return len;
}

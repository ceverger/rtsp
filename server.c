#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "network.h"
#include "rtp.h"
#include "h264.h"
#include "rtsp.h"

int h264RtpStream(
						const char *filename, 
						long ssrc, 
						int rtpSocket,
						const char *clientIP, 
						int clientRtpPort
					  );

int main(int argc, char *argv[])
{
	int ret, cseq;
	int serverSocket;

	int clientPort;
	char clientIpaddr[IPV4_ADDRESS_LEN];

	int serverPort = 554;
	char serverIpaddr[IPV4_ADDRESS_LEN];

	long ssrc = 24328698;
	const char *sessionID = "BD587558";
	int rtspSocket, rtpSocket, rtcpSocket;

	int clientRtpPort, clientRtcpPort;
	int serverRtpPort = 13144, serverRtcpPort = 13145;

	int bufsize = 4096;
	char buffer[bufsize];

	int cacheLen = 1024;
	char cache[cacheLen];

	int urilen = 256;
	char uri[urilen];

	int sdpsize = 4096;
	char sdp[sdpsize];

	ret = hostIpaddr(serverIpaddr);
	if(ret < 0) return 1;

	serverSocket = tcpSocketServer(0, serverPort, 1, true);
	if(serverSocket < 0) return 1;

	rtpSocket = udpSocketServer(0, serverRtpPort, true);
	if(rtpSocket < 0) return 1;
	
	rtcpSocket = udpSocketServer(0, serverRtcpPort, true);
	if(rtcpSocket < 0) return 1;
	
	rtspSocket = tcpSocketConnection(serverSocket, clientIpaddr, &clientPort);
	if(rtspSocket < 0) return 1;

	while(1)
	{
		ret = rtspReceiveMessage(rtspSocket, buffer, bufsize);
		if(ret < 0) return 1;
		if(ret == 0) break;

		buffer[ret] = '\0';
		printf("%s", buffer);

		cseq = rtspParserGetCSeq(buffer);
		rtspParserGetURI(buffer, uri, urilen);
		
		if(rtspMethodIsOptions(buffer))
		{
			ret = rtspResponseForOption(cseq, cache, cacheLen);
			if(ret < 0) return 1;
			printf("%s", cache);

			ret = rtspSendMessage(rtspSocket, cache, ret);
			if(ret < 0) return 1;	
		}

		if(rtspMethodIsDescribe(buffer))
		{
			ret = rtspCreateSDP(serverIpaddr, serverRtpPort, 60, sdp, sdpsize);
			ret = rtspResponseForDescribe(cseq, uri, sdp, ret, serverIpaddr, serverRtpPort, cache, cacheLen);
			if(ret < 0) return 1;
			printf("%s", cache);

			ret = rtspSendMessage(rtspSocket, cache, ret);
			if(ret < 0) return 1;			
		}

		if(rtspMethodIsSetup(buffer))
		{
			rtspParserGetClientPorts(buffer, &clientRtpPort, &clientRtcpPort);

			ret = rtspResponseForSetup(
												cseq,
                                    serverIpaddr,
                                    serverRtpPort,
									         serverRtcpPort,
									         clientIpaddr,
									         clientRtpPort,
									         clientRtcpPort,
									         sessionID,
												cache, 
                                    cacheLen
                                  );

			if(ret < 0) return 1;
			printf("%s", cache);

			ret = rtspSendMessage(rtspSocket, cache, ret);
			if(ret < 0) return 1;		
		}

		if(rtspMethodIsPlay(buffer))
		{
			ret = rtspResponsePlay(
										  cseq,
										  uri,
									 	  sessionID,
									 	  ssrc,
									 	  0,
								    	  cache, 
								    	  cacheLen
								   	);

			if(ret < 0) return 1;
			printf("%s", cache);

			ret = rtspSendMessage(rtspSocket, cache, ret);
			if(ret < 0) return 1;	

			ret = h264RtpStream("test.264", ssrc, rtpSocket, clientIpaddr, clientRtpPort);
			if(ret < 0) return 1;
		}
	}
		  
	return 0;
}

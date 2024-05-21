#include <stdbool.h>

#ifndef RTSP_H

void rtspParserGetMethod(const char *message, char *buf, int bufsize);
void rtspParserGetURI(const char *message, char *buf, int bufsize);
void rtspParserGetFilename(const char *uri, char *buf, int bufsize);
int rtspParserGetCSeq(const char *message);

void rtspParserGetClientPorts(
										const char *message, 
										int *clientRtpPort, 
										int *clientRtcpPort
									  );

void rtspParserGetServerPorts(
										const char *message, 
										int *serverRtpPort, 
										int *serverRtcpPort
									  );

void rtspParserGetSession(
								  const char *message,
                          char *buf,
                          int bufsize
                         );

int rtspRequestForOption(
								 int cseq, 
                         const char *uri,
								 const char *agent, 
								 char *buf,
								 int bufsize
								);

int rtspRequestForDescribe(
									int cseq, 
									const char *uri, 
									const char *agent, 
									const char *format,
								   char *buf,
								   int bufsize
								  );

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
							  );

int rtspRequestForPlay(
								int cseq, 
								const char *uri,
								const char *agent,
								const char *session,
							   const char *range,
								const char *uri,
								int seq,
								int rtptime,
								char *buf,
								int bufsize								
							  );

int rtspResponseForOption(
								  int cseq,
								  char *buf, 
								  int bufsize
								 );

int rtspResponseForDescribe(
									 int cseq,
									 const char *uri,
									 const char *sdp,
									 int sdpLen,
									 const char *serverIP,
									 int serverRtpPort,
									 char *buf, 
									 int bufsize
                           );

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
                        );

int rtspResponsePlay(
							int cseq,
						   const char *uri,
							const char *sessionID,
					      long ssrc,
					      int timestamp,
					      char *buf, 
					      int bufsize
					     );

bool rtspMethodIsOptions(const char *method);
bool rtspMethodIsDescribe(const char *method);
bool rtspMethodIsSetup(const char *method);
bool rtspMethodIsPlay(const char *method);
bool rtspMethodIsTeardown(const char *method);

int rtspCreateSDP(
						const char *serverIP, 
						int serverRtpPort, 
						int framerate, 
						char *buf, 
						int bufsize
					  );

int rtspReceiveMessage(int rtspSocket, char *buf, int bufsize);
int rtspSendMessage(int rtspSocket, char *buf, int len);

#endif // RTSP_H

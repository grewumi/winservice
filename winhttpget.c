/*
 * Winhttp get ����
*/
#include <stdio.h>      
#include <winsock2.h>      
#define  MAXBUFLEN  20480      
#define  HTTPADDLEN 50      
#define  TIMEWAIT   2000      
#pragma comment(lib,"ws2_32.lib")     
SOCKET Global[1000];      
          
DWORD WINAPI Proxy( LPVOID pSocket);      
int ParseHttpRequest(char *SourceBuf,int DataLen,void *ServerAddr);      
          
int main(int argc,char *argv[]){      
    SOCKET MainSocket;      
    struct sockaddr_in Host;      
    WSADATA WsaData;      
    int AddLen,i;      
          
    //��ʼ��      
    if(WSAStartup(MAKEWORD(2,2),&WsaData) < 0){      
        printf("��ʼ��ʧ��n");      
        return 1;      
    }      
    //����socket      
    MainSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);      
    if(MainSocket == SOCKET_ERROR){      
        printf("socket��������n");     
        return 1;     
    }      
    Host.sin_family = AF_INET;      
    Host.sin_port = htons(80);      
    Host.sin_addr.s_addr = inet_addr("180.149.134.17");      
    printf("���ڹ���n");      
	return 0;     
}      
DWORD WINAPI Proxy( LPVOID pSocket){      
    SOCKET ClientSocket;      
    char  ReceiveBuf[MAXBUFLEN];      
    int  DataLen;      
    struct sockaddr_in  ServerAddr;      
    SOCKET  ProxySocket;      
    int i = 0;      
    int time = TIMEWAIT;      
          
    //�õ������еĶ˿ں���Ϣ      
    ClientSocket = (SOCKET)pSocket;      
    //���ܵ�һ��������Ϣ      
    memset(ReceiveBuf,0,MAXBUFLEN);      
    DataLen = recv(ClientSocket,ReceiveBuf,MAXBUFLEN,0);      
          
    if(DataLen == SOCKET_ERROR){      
        printf("����n");      
        closesocket(ClientSocket);      
        return 0;     
    }      
    if(DataLen == 0){      
        closesocket(ClientSocket);      
        return 0;     
    }          
    //����������Ϣ���������������ַ      
    if( ParseHttpRequest(ReceiveBuf,DataLen,(void *)&ServerAddr) < 0){      
        closesocket(ClientSocket);      
        goto error;      
    }      
   //�����µ�socket�����ͷ�������������      
   ProxySocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);      
   //���ó�ʱʱ��      
   setsockopt(ProxySocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&time,sizeof(time));      
   if(ProxySocket == SOCKET_ERROR)      
   {      
       printf("�˿ڴ�������n");      
       return 0;     
   }      
   if(connect(ProxySocket,(SOCKADDR *)&ServerAddr,sizeof(ServerAddr)) == SOCKET_ERROR)      
   {      
       //printf("���ӷ���������");      
       goto error;      
   }      
   //��ʼ�������ݴ��䴦��      
   //���͵���������      
   if(send(ProxySocket,ReceiveBuf,DataLen,0) == SOCKET_ERROR)      
   {      
       //printf("���ݷ��ʹ���");      
       goto error;      
          
   }      
       //�ӷ������˽�������      
   while(DataLen > 0)      
   {      
       memset(ReceiveBuf,0,MAXBUFLEN);      
              
       if((DataLen = recv(ProxySocket,ReceiveBuf,MAXBUFLEN,0)) <= 0)      
       {      
           //    printf("���ݽ��ܴ���");      
           break;      
                      
       }      
       else 
           //���͵��ͻ���      
           if(send(ClientSocket,ReceiveBuf,DataLen,0) < 0)      
           {      
               //    printf("���ݷ��ʹ���");      
                   break;      
           }      
                  
   }      
error:      
    closesocket(ClientSocket);      
    closesocket(ProxySocket);      
    return 0;      
}      
int ParseHttpRequest(char *SourceBuf,int DataLen,void *ServerAddr){      
          
    char *HttpHead = "http://";      
    char *FirstLocation = NULL;      
    char *LastLocation = NULL;      
    char *PortLocation = NULL;      
    char ServerName[HTTPADDLEN];      
    char PortString[10];      
    int NameLen;      
    struct hostent *pHost;      
    struct sockaddr_in *pServer = (struct sockaddr_in*)ServerAddr;      
    //ȡ��http://��λ��      
    FirstLocation = strstr(SourceBuf,HttpHead) + strlen(HttpHead);      
    //ȡ��/��λ��      
    printf("%sn",FirstLocation);     
    LastLocation=strstr(FirstLocation,"/");      
          
    //�õ�http://��/֮��ķ�����������      
              
    memset(ServerName,0,HTTPADDLEN);      
    memcpy(ServerName,FirstLocation,LastLocation - FirstLocation);      
          
    //��Щ����£�����ĵ�ַ�д��ж˿ںŸ�ʽΪ����+ �˿ںš���      
    //ȡ�� ����λ��      
    PortLocation = strstr(ServerName,":");      
          
              
    //���server�ṹ      
    pServer->sin_family = AF_INET;      
    //��url���ƶ��˷������˿�      
    if(PortLocation != NULL)      
    {      
        NameLen = PortLocation - ServerName -1;      
        memset(PortString,0,10);      
        memcpy(PortString,PortLocation + 1,NameLen);      
        pServer->sin_port = htons((u_short)atoi(PortString));      
        *PortLocation = 0;          
    }      
    else//��url�У�û���ƶ��������˿�      
    {      
        pServer->sin_port=htons(80);      
    }      
          
    if(NameLen > HTTPADDLEN)      
   {      
       printf("����������̫��n");      
       return -1;      
   }      
              
   //�õ���������Ϣ      
   //�����ַ��Ϣ����IP��ַ(192.168.0.1)����ʽ���ֵ�      
   if(ServerName[0] >= '0' && ServerName[0] <= '9')      
   {      
                  
       pServer->sin_addr.s_addr = inet_addr(ServerName);      
   }      
   //����������ʽ���ֵ�(www.sina.com.cn)      
   else 
   {      
       pHost = (struct hostent *)gethostbyname(ServerName);      
       if(!pHost)      
       {      
           printf("ȡ��������Ϣ����n");      
           printf("%sn",ServerName);      
           return -1;      
       }      
       memcpy(&pServer->sin_addr,pHost->h_addr_list[0],sizeof(pServer->sin_addr));      
   }      
              
   return 0;      
}


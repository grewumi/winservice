/*
 * Winhttp get 请求
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
          
    //初始化      
    if(WSAStartup(MAKEWORD(2,2),&WsaData) < 0){      
        printf("初始化失败n");      
        return 1;      
    }      
    //创建socket      
    MainSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);      
    if(MainSocket == SOCKET_ERROR){      
        printf("socket创建错误n");     
        return 1;     
    }      
    Host.sin_family = AF_INET;      
    Host.sin_port = htons(80);      
    Host.sin_addr.s_addr = inet_addr("180.149.134.17");      
    printf("正在工作n");      
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
          
    //得到参数中的端口号信息      
    ClientSocket = (SOCKET)pSocket;      
    //接受第一次请求信息      
    memset(ReceiveBuf,0,MAXBUFLEN);      
    DataLen = recv(ClientSocket,ReceiveBuf,MAXBUFLEN,0);      
          
    if(DataLen == SOCKET_ERROR){      
        printf("错误n");      
        closesocket(ClientSocket);      
        return 0;     
    }      
    if(DataLen == 0){      
        closesocket(ClientSocket);      
        return 0;     
    }          
    //处理请求信息，分离出服务器地址      
    if( ParseHttpRequest(ReceiveBuf,DataLen,(void *)&ServerAddr) < 0){      
        closesocket(ClientSocket);      
        goto error;      
    }      
   //创建新的socket用来和服务器进行连接      
   ProxySocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);      
   //设置超时时间      
   setsockopt(ProxySocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&time,sizeof(time));      
   if(ProxySocket == SOCKET_ERROR)      
   {      
       printf("端口创建错误n");      
       return 0;     
   }      
   if(connect(ProxySocket,(SOCKADDR *)&ServerAddr,sizeof(ServerAddr)) == SOCKET_ERROR)      
   {      
       //printf("连接服务器错误");      
       goto error;      
   }      
   //开始进行数据传输处理      
   //发送到服务器端      
   if(send(ProxySocket,ReceiveBuf,DataLen,0) == SOCKET_ERROR)      
   {      
       //printf("数据发送错误");      
       goto error;      
          
   }      
       //从服务器端接受数据      
   while(DataLen > 0)      
   {      
       memset(ReceiveBuf,0,MAXBUFLEN);      
              
       if((DataLen = recv(ProxySocket,ReceiveBuf,MAXBUFLEN,0)) <= 0)      
       {      
           //    printf("数据接受错误");      
           break;      
                      
       }      
       else 
           //发送到客户端      
           if(send(ClientSocket,ReceiveBuf,DataLen,0) < 0)      
           {      
               //    printf("数据发送错误");      
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
    //取得http://的位置      
    FirstLocation = strstr(SourceBuf,HttpHead) + strlen(HttpHead);      
    //取得/的位置      
    printf("%sn",FirstLocation);     
    LastLocation=strstr(FirstLocation,"/");      
          
    //得到http://和/之间的服务器的名称      
              
    memset(ServerName,0,HTTPADDLEN);      
    memcpy(ServerName,FirstLocation,LastLocation - FirstLocation);      
          
    //有些情况下，请求的地址中带有端口号格式为“：+ 端口号”；      
    //取得 ：的位置      
    PortLocation = strstr(ServerName,":");      
          
              
    //填充server结构      
    pServer->sin_family = AF_INET;      
    //在url中制定了服务器端口      
    if(PortLocation != NULL)      
    {      
        NameLen = PortLocation - ServerName -1;      
        memset(PortString,0,10);      
        memcpy(PortString,PortLocation + 1,NameLen);      
        pServer->sin_port = htons((u_short)atoi(PortString));      
        *PortLocation = 0;          
    }      
    else//在url中，没有制定服务器端口      
    {      
        pServer->sin_port=htons(80);      
    }      
          
    if(NameLen > HTTPADDLEN)      
   {      
       printf("服务器名字太长n");      
       return -1;      
   }      
              
   //得到服务器信息      
   //如果地址信息是以IP地址(192.168.0.1)的形式出现的      
   if(ServerName[0] >= '0' && ServerName[0] <= '9')      
   {      
                  
       pServer->sin_addr.s_addr = inet_addr(ServerName);      
   }      
   //以域名的形式出现的(www.sina.com.cn)      
   else 
   {      
       pHost = (struct hostent *)gethostbyname(ServerName);      
       if(!pHost)      
       {      
           printf("取得主机信息错误n");      
           printf("%sn",ServerName);      
           return -1;      
       }      
       memcpy(&pServer->sin_addr,pHost->h_addr_list[0],sizeof(pServer->sin_addr));      
   }      
              
   return 0;      
}


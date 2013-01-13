/*
 *  Windows服务
*/
#include <windows.h>
#include <stdio.h>
/*
 * 时间间隔和文件路径
*/
#define SLEEP_TIME 5000 
#define LOGFILE "F:\\memstatus.txt"
/*
 * 日志写入函数,成功返回0
*/
int WriteToLog(char *str){
	FILE *log;
	log = fopen(LOGFILE,"a+");
	if(!log)
		return -1;
	fprintf(log,"%s\n",str);
	fclose(log);
	return 0;
}
/*
 * 全局变量
*/
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void ServiceMain(int argc,char *argv[]);
void ControlHandler(DWORD request);
int InitService();

/*
 * 创建服务分派表和控制分派机
*/
void main(){
	/*
	 * 服务分派表结构SERVICE_TABLE_ENTRY,分别有两个域
	 * lpServiceName:服务名
	 * lpServiceProc:服务函数指针
	 * 服务分派表最后一项必须是主服务名和函数,如果就一个服务,都为NULL
	*/
	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = "MemoryStatus";
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;
	
	//启动服务的控制分派机线程
	StartServiceCtrlDispatcher(ServiceTable);
}
//初始化服务
int InitService(){
	int result;
	result = WriteToLog("Monitoring started.");
	return result;
}
void ServiceMain(int argc,char *argv[]){
	int error,result;
	MEMORYSTATUS memory;
	ServiceStatus.dwServiceType = SERVICE_WIN32;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | \
									  SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;
	hStatus = RegisterServiceCtrlHandler("MemoryStatus",\
						(LPHANDLER_FUNCTION)ControlHandler);
	if(hStatus == (SERVICE_STATUS_HANDLE)0){
		//注册控制处理错误
		return;
	}
	//初始化服务
	error = InitService();
	if(error){
		//服务初始化失败,终止服务
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode = -1;
		SetServiceStatus(hStatus,&ServiceStatus);
		//退出ServiceMain
		return;
	}
	//初始化成功则向SCM报告状态
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus,&ServiceStatus);

	//循环读取内存
	while(ServiceStatus.dwCurrentState == SERVICE_RUNNING){
		char buffer[16];
		GlobalMemoryStatus(&memory);
		sprintf(buffer,"%d",memory.dwAvailPhys);
		OutputDebugString(buffer);
		result = WriteToLog(buffer);
		if(result){
			ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			ServiceStatus.dwWin32ExitCode = -1;
			SetServiceStatus(hStatus,&ServiceStatus);
			return;
		}
		Sleep(SLEEP_TIME);
	}
	return;
}
void ControlHandler(DWORD request){
	switch(request){
		case SERVICE_CONTROL_STOP:
			OutputDebugString("Montitoring stopped.");
			WriteToLog("Montitoring stopped");
			ServiceStatus.dwWin32ExitCode = 0;
			ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			SetServiceStatus(hStatus,&ServiceStatus);
			return;
		case SERVICE_CONTROL_SHUTDOWN:
			OutputDebugString("Montitoring stopped.");
			WriteToLog("Montitoring stopped");
			ServiceStatus.dwWin32ExitCode = 0;
			ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			SetServiceStatus(hStatus,&ServiceStatus);
			return;
		default:
			break;
	}
	//返回当前状态
	SetServiceStatus(hStatus,&ServiceStatus);
	return;
}

/*
 *  Windows����
*/
#include <windows.h>
#include <stdio.h>
/*
 * ʱ�������ļ�·��
*/
#define SLEEP_TIME 5000 
#define LOGFILE "F:\\memstatus.txt"
/*
 * ��־д�뺯��,�ɹ�����0
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
 * ȫ�ֱ���
*/
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void ServiceMain(int argc,char *argv[]);
void ControlHandler(DWORD request);
int InitService();

/*
 * ����������ɱ�Ϳ��Ʒ��ɻ�
*/
void main(){
	/*
	 * ������ɱ�ṹSERVICE_TABLE_ENTRY,�ֱ���������
	 * lpServiceName:������
	 * lpServiceProc:������ָ��
	 * ������ɱ����һ����������������ͺ���,�����һ������,��ΪNULL
	*/
	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = "MemoryStatus";
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;
	
	//��������Ŀ��Ʒ��ɻ��߳�
	StartServiceCtrlDispatcher(ServiceTable);
}
//��ʼ������
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
		//ע����ƴ������
		return;
	}
	//��ʼ������
	error = InitService();
	if(error){
		//�����ʼ��ʧ��,��ֹ����
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode = -1;
		SetServiceStatus(hStatus,&ServiceStatus);
		//�˳�ServiceMain
		return;
	}
	//��ʼ���ɹ�����SCM����״̬
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus,&ServiceStatus);

	//ѭ����ȡ�ڴ�
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
	//���ص�ǰ״̬
	SetServiceStatus(hStatus,&ServiceStatus);
	return;
}

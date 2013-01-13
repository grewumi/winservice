/**
 * Author: Gan
 * Date: 2007-9-27
 * Name: main.c
 */
#include <stdio.h>
#include <windows.h>

#define SVC_SLEEP_TIME 1000
#define SVC_LOG_FILENAME "c:\\svc.log"

SERVICE_STATUS svc_st;
SERVICE_STATUS_HANDLE svc_hd;

void ServiceMain(int argc, char **argv);
void ControlHandler(DWORD request);
int svc_init(void);

/**
 * Write Log file
 */
int svc_write_log(const char *str)
{
    FILE *fp;

    fp = fopen(SVC_LOG_FILENAME, "a+");
    if (fp == NULL)
    {
      printf("file open error! \n");
      return (-1);
    }

    fprintf(fp, "%s\n", str);
    fclose(fp);

    return (0);
}

/**
 * Initialize Service
 */
int svc_init(void)
{
    int ret;

    printf("svc_init called. \n");

    ret = svc_write_log("svc_init called, started. ");
    return (ret);
}

void ControlHandler(DWORD request)
{
    switch (request)
    {
      case SERVICE_CONTROL_STOP:
      case SERVICE_CONTROL_SHUTDOWN:
        printf("Monitoring Started.\n");
        svc_write_log("Monitoring started.");

        svc_st.dwWin32ExitCode = 0;
        svc_st.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(svc_hd, &svc_st);
        return;

      default:
        break;
    }

    SetServiceStatus(svc_hd, &svc_st);
}

/**
 * Service Main Function
 */
void ServiceMain(int argc, char **argv)
{
    char str[32];
    MEMORYSTATUS mem;

    svc_st.dwServiceType = SERVICE_WIN32;
    svc_st.dwCurrentState = SERVICE_START_PENDING;
    svc_st.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    svc_st.dwWin32ExitCode = 0;
    svc_st.dwServiceSpecificExitCode = 0;
    svc_st.dwCheckPoint = 0;
    svc_st.dwWaitHint = 0;

    printf("ServiceMain Called. \n");

    svc_hd = RegisterServiceCtrlHandler("MemoryStatus", (LPHANDLER_FUNCTION)ControlHandler);
    if (svc_hd == (SERVICE_STATUS_HANDLE)0)
    {
      printf("RegisterServiceCtlHandler error. \n");
      return;
    }

    if (svc_init())
    {
      svc_st.dwCurrentState = SERVICE_STOPPED;
      svc_st.dwWin32ExitCode = 0;
      SetServiceStatus(svc_hd, &svc_st);
      return;
    }

    svc_st.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(svc_hd, &svc_st);

    while (svc_st.dwCurrentState == SERVICE_RUNNING)
    {
      GlobalMemoryStatus(&mem);
      sprintf(str, "%d", mem.dwAvailPhys);

      printf(str);

      if (svc_write_log(str))
      {
        svc_st.dwCurrentState = SERVICE_STOPPED;
        svc_st.dwWin32ExitCode = -1;
        SetServiceStatus(svc_hd, &svc_st);
        return;
      }

      Sleep(SVC_SLEEP_TIME);
    }
}

int main(int argc, char *argv[])
{
    SERVICE_TABLE_ENTRY svc_tab[2];

    printf("Main Test started.\n");

    svc_tab[0].lpServiceName = "MemoryStatus";
    svc_tab[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

    svc_tab[1].lpServiceName = NULL;
    svc_tab[1].lpServiceProc = NULL;

    StartServiceCtrlDispatcher(svc_tab);

    return 0;
}


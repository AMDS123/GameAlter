#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <TlHelp32.h>
#include <sys\timeb.h>

#define KONEK 1024
#define KPAGE 4*KONEK
#define KONEG KONEK*KONEK*KONEK
#define KFILELEN 60

BOOL CompareAPage(DWORD dwBaseAddr,DWORD dwValue);
BOOL FindFirst(DWORD dwValue);
BOOL FindNext(DWORD dwValue);
BOOL WriteMemory(DWORD dwValue);
void ShowAddList(void);
void editValue(DWORD dwId);
void showAllProcess();
BOOL closeProcess(DWORD dwId);
void showMenu();
DWORD getProcessId();
DWORD GetBaseAddress(DWORD dwPID);

DWORD g_dwAddList[KPAGE] = {0};
DWORD g_dwCount = 0;
HANDLE g_hProcess = NULL;
DWORD g_dwId = 0;

int main(int argc,char *argv[])
{
	UINT uIndex = 0;
	DWORD dwId;
	while(1)
	{
		showMenu();
		scanf("%d",&uIndex);
		switch (uIndex)
		{
		case 1:
			showAllProcess();
			break;
		case 2:
			editValue(getProcessId());
			break;
		case 3:
			closeProcess(getProcessId());
			break;
		case 4:
			system("pause");
			return 0;
			break;
		case 5:
			DWORD dwValue = GetBaseAddress(getProcessId());
			printf("��ַ��%#08x\n",dwValue);
			break;
		}
		system("pause");
	}
	
	system("pause");
	return 0;
}

void showMenu()
{
	system("cls");
	printf("1.�����б�\n");
	printf("2.�޸��ڴ�\n");
	printf("3.��������\n");
	printf("4.�˳�ϵͳ\n");
	printf("5.�õ���ַ\n");
	printf("������ѡ��");
}

DWORD getProcessId()
{
	DWORD dwId;
	printf("���������ID:");
	scanf("%d",&dwId);
	g_dwId = dwId;
	return dwId;
}


void editValue(DWORD dwId)
{
	g_hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwId);
	if (!g_hProcess)
	{
		printf("�򿪽���%sʧ��\n",dwId);
		return;
	}

	DWORD dwValue;
	printf("�������һ�ε�ֵ:");
	scanf("%d",&dwValue);

	FindFirst(dwValue);

	ShowAddList();

	printf("������ڶ��ε�ֵ��");
	scanf("%d",&dwValue);

	FindNext(dwValue);

	ShowAddList();

	printf("������Ҫ��ֵ:");
	scanf("%d",&dwValue);
	WriteMemory(dwValue);
}

BOOL CompareAPage(DWORD dwBaseAddr,DWORD dwValue)
{
	BYTE bytes[KPAGE];
	if (!ReadProcessMemory(g_hProcess,(LPCVOID)dwBaseAddr,bytes,KPAGE,NULL))
	{
		//printf("��ȡ�ڴ�ʧ��\n");
		return FALSE;
	}

	DWORD *pdw = (DWORD*)bytes;

	for (int i=0;i<KONEK;i++)
	{
		if (pdw[i] == dwValue)
		{
			g_dwAddList[g_dwCount++] = dwBaseAddr + i*sizeof(DWORD);
		}		
	}
	return TRUE;
}

BOOL FindFirst(DWORD dwValue)
{
	OSVERSIONINFO vi = {sizeof(vi)};
	GetVersionEx(&vi);
	DWORD dwBase;

	if (vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		printf("Windows 98\n");
		dwBase = 4 * KONEK * KONEK;
	}
	else if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		printf("Windows NT\n");
		dwBase = 64 * KONEK;
	}
	g_dwCount = 0;

	DWORD dwOld = 0;
	DWORD dwNew = 0;

	struct timeb start;
	struct timeb end;

	printf("������...\n%%%02d",0.0);

	ftime(&start);

	//dwBase = GetBaseAddress(g_dwId);
	for (;dwBase < 2 * KONEG;dwBase+=KPAGE)
	{
		dwNew = dwBase/(KONEG/50);
		if (dwNew != dwOld)
		{
			printf("\b\b%02d",dwNew);
			dwOld = dwNew;
		}
		
		CompareAPage(dwBase,dwValue);
	}
	ftime(&end);
	printf("\b\b100\n�������\n");
	
	printf("��ʱ%d����\n",(end.time-start.time)*1000+ end.millitm-start.millitm);

	return TRUE;
}

BOOL FindNext(DWORD dwValue)
{
	DWORD dwCount = 0;
	DWORD dwValue1 = 0;

	for (int i=0;i<g_dwCount;i++)
	{
		if (!ReadProcessMemory(g_hProcess,(LPCVOID)g_dwAddList[i],&dwValue1,sizeof(DWORD),NULL))
		{
			//printf("��ȡ�ڴ�ʧ��\n");
			return FALSE;
		}
		if (dwValue1 == dwValue)
		{
			g_dwAddList[dwCount++] = g_dwAddList[i];
		}
	}
	g_dwCount = dwCount;

	return TRUE;
}

BOOL WriteMemory(DWORD dwValue)
{
	for (int i=0;i<g_dwCount;i++)
	{
		if (!WriteProcessMemory(g_hProcess,(LPVOID)g_dwAddList[i],(LPCVOID)&dwValue,sizeof(DWORD),NULL))
		{
			return FALSE;
		}
		
	}	
	return TRUE;
}


void ShowAddList(void)
{
	printf("��ַ�б�...\n");
	for (int i=0;i<g_dwCount;i++)
	{
		printf("%#010x\n",g_dwAddList[i]);
	}
}

BOOL closeProcess(DWORD dwId)
{
	BOOL bRet = FALSE;
	HANDLE hHandle = OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwId);

	if (hHandle != NULL)
	{
		bRet = TerminateProcess(hHandle,0);
	}
	CloseHandle(hHandle);
	return bRet;
}

void showAllProcess()
{
	PROCESSENTRY32 pc;
	pc.dwSize = sizeof(pc);

	HANDLE dProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	if (INVALID_HANDLE_VALUE == dProcessSnap)
	{
		printf("��ý���ʧ��");
		system("pause");
		return ;
	}

	BOOL bMore = Process32First(dProcessSnap,&pc);

	while (bMore)
	{
		printf("����ID:%4d | �������ƣ�%s\n",pc.th32ProcessID,pc.szExeFile);
		bMore = Process32Next(dProcessSnap,&pc);
	}

	CloseHandle(dProcessSnap);

}

//////////////////////////////////////////////////////////////////////////
//   �������ܣ� ��ȡexeģ��ļ��ص�ַ
//   ��   ���� dwPID�����̵�pid�� 
//   �� �� ֵ�� ����exeģ���ַ��
//////////////////////////////////////////////////////////////////////////
DWORD GetBaseAddress(DWORD dwPID)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;
	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwPID );
	if( hModuleSnap == INVALID_HANDLE_VALUE )
	{
		printf("ʧ��!");
		return 0;
	}

	me32.dwSize = sizeof( MODULEENTRY32 );
	
	if( !Module32First( hModuleSnap, &me32 ) )
	{
		CloseHandle( hModuleSnap );           // clean the snapshot object
		return 0;
	}
	DWORD Value = (DWORD)me32.modBaseAddr;
	CloseHandle( hModuleSnap );
	return Value;
}
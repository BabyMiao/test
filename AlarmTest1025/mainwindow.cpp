#include "mainwindow.h"
#include <QBoxLayout>


void CALLBACK MessageCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
  int i;
  NET_DVR_ALARMINFO struAlarmInfo;
  memcpy(&struAlarmInfo, pAlarmInfo, sizeof(NET_DVR_ALARMINFO));
  switch(lCommand)
  {
  case COMM_ALARM:
      {
          switch (struAlarmInfo.dwAlarmType)
          {
          case 3: //移动侦测报警
               for (i=0; i<16; i++)   //#define MAX_CHANNUM   16  //最大通道数
               {
                   if (struAlarmInfo.dwChannel[i] == 1)
                   {
                       printf("发生移动侦测报警的通道号 %d\n", i+1);
                   }
               }
          break;
          default:
          break;
          }
       }
  break;
  default:
  break;
  }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    init();
}

MainWindow::~MainWindow()
{

}

void MainWindow::init()
{
    m_testBtn = new QPushButton(tr("test"));

    QHBoxLayout *hlay = new QHBoxLayout;
    hlay->addWidget(m_testBtn);

    QWidget *widget = new QWidget;
    widget->setLayout(hlay);

    setCentralWidget(widget);

    connect(m_testBtn,SIGNAL(clicked(bool)),this,SLOT(testslot()));

}

void MainWindow::testslot()
{
    //---------------------------------------
    // 初始化
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    //---------------------------------------
    // 注册设备
    LONG lUserID;
    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "221.178.208.46"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "ad53937301"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0)
    {
        printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return;
    }

    //设置报警回调函数
    NET_DVR_SetDVRMessageCallBack_V30(MessageCallback, NULL);

    //启用布防
    LONG lHandle;

    //布防参数
    NET_DVR_SETUPALARM_PARAM  struAlarmParam={0};
    struAlarmParam.dwSize=sizeof(struAlarmParam);
    struAlarmParam.byAlarmInfoType=0;
    //设备是否支持新的报警信息通过登录返回的NET_DVR_DEVICEINFO_V30中参数bySupport1 & 0x80判断

    lHandle = NET_DVR_SetupAlarmChan_V41(lUserID, & struAlarmParam);
    if (lHandle < 0)
    {
        printf("NET_DVR_SetupAlarmChan_V41 error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return;
    }

    Sleep(5000);
    //撤销布防上传通道
    if (!NET_DVR_CloseAlarmChan_V30(lHandle))
    {
        printf("NET_DVR_CloseAlarmChan_V30 error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return;
    }

    //注销用户
    NET_DVR_Logout(lUserID);
    //释放SDK资源
    NET_DVR_Cleanup();
    return;

}

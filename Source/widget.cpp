#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Widget)
{
	ui->setupUi(this);

	QObject::connect (ui->Clear,SIGNAL(clicked()),this,SLOT(ClearLeaderboard()));
	QObject::connect (ui->Load,SIGNAL(clicked()),this,SLOT(LoadLeaderboard()));
	QObject::connect (ui->Save,SIGNAL(clicked()),this,SLOT(SaveLeaderboard()));

	MyServer=new QTcpServer(this);
	MySocket=new QTcpSocket(this);
	StartServer();
	LoadLeaderboard ();
	DisplayLeaderboard ();
}

Widget::~Widget()
{
	SaveLeaderboard();
	delete ui;
}

void Widget::DisplayOnConsole(QString msg)
{
	QString OutFileName=QString::number(QDateTime::currentDateTime ().date ().year ())+"_"+QString::number(QDateTime::currentDateTime ().date ().month())+"_"+QString::number(QDateTime::currentDateTime ().date ().day())+".txt";
	QFile ofile(OutFileName);
	ofile.open(QIODevice::Append);
	QTextStream fout(&ofile);

	QString datestr="["+QDateTime::currentDateTime ().time ().toString ()+"] ";

	ui->Console->setHtml(ui->Console->toHtml()+datestr+msg);
	fout<<datestr<<msg<<"\n";

	ofile.close();
}

void Widget::StartServer(){
	MyServer->close();
	delete MyServer;
	MyServer=new QTcpServer(this);
	MyServer->listen (QHostAddress::Any,PORT);

	QObject::connect(MyServer,SIGNAL(newConnection()),this,SLOT(NewConnect()));
	DisplayOnConsole("Started Leaderboard Server on Port:"+QString::number(MyServer->serverPort(),10));
}

void Widget::NewConnect (){
	MySocket->abort();
	delete MySocket;
	MySocket=MyServer->nextPendingConnection();
	DisplayOnConsole("New Connect received! IP:"+MySocket->peerAddress().toString ());
	QObject::connect(MySocket,SIGNAL(readyRead()),this,SLOT(ReadFromClient()));
}

void Widget::ReadFromClient(){
	QByteArray recvByte=MySocket->readAll();
	QString recvStr=QVariant(recvByte).toString();
	if(recvStr=="request"){
		SendToClient();
	}else{
		ReadData(recvStr);
	}
}

void Widget::SendToClient(){
	QString sendstr,procstr;

	procstr=QString::number(FloorList.count(),10)+"-";

	for(int i=0;i<FloorList.count();++i){
		sendstr=QString::number(i+1,10)+".     "+QString::number(FloorList[i].floor,10)+"      "+FloorList[i].name;
		procstr+=QString::number(sendstr.size(),10)+"-"+sendstr+"-";
	}
	for(int i=0;i<PlatformList.count();++i){
		sendstr=QString::number(i+1,10)+".     "+QString::number(PlatformList[i].platform,10)+"      "+PlatformList[i].name;
		procstr+=QString::number(sendstr.size(),10)+"-"+sendstr+"-";
	}

	MySocket->write(procstr.toStdString ().c_str (),strlen (procstr.toStdString ().c_str ()));

	DisplayOnConsole("Send Leaderboard to client. IP:"+MySocket->peerAddress().toString ());
}

void Widget::SaveLeaderboard (){
			
	QString procstr;
	
	procstr=QString::number(FloorList.count(),10)+"-";
	for(int i=0;i<FloorList.count();++i){
		procstr+=QString::number(FloorList[i].floor,10)+"-"+QString::number(FloorList[i].name.size(),10)+"-"+FloorList[i].name+"-";
	}
		
	QString FloorFileName="Floor.leb";
	QFile Floorfile(FloorFileName);
	
	QTextStream floorout(&Floorfile);
	Floorfile.open(QIODevice::WriteOnly);
	floorout<<procstr;
	Floorfile.close();
	procstr.clear();
	
	/*-----platform-----*/
	
	procstr=QString::number(PlatformList.count(),10)+"-";
	for(int i=0;i<PlatformList.count();++i){
		procstr+=QString::number(PlatformList[i].platform,10)+"-"+QString::number(PlatformList[i].name.size(),10)+"-"+PlatformList[i].name+"-";
	}
	
	QString PlatformFileName="Platform.leb";
	QFile Platformfile(PlatformFileName);
	Platformfile.open(QIODevice::WriteOnly);
	
	QTextStream platout(&Platformfile);
	platout<<procstr;
	Platformfile.close();
	
}

void Widget::LoadLeaderboard (){
	QString FloorStr,PlatformStr;

	QString FloorFileName="Floor.leb";
	QFile Floorfile(FloorFileName);
	QTextStream floorin(&Floorfile);
	Floorfile.open(QIODevice::ReadOnly);
	floorin>>FloorStr;
	Floorfile.close();

	QString PlatformFileName="Platform.leb";
	QFile Platformfile(PlatformFileName);
	Platformfile.open(QIODevice::ReadOnly);
	QTextStream platin(&Platformfile);
	platin>>PlatformStr;
	Platformfile.close();

	int F_LeaCount=FloorStr.split("-")[0].toInt();
	FloorStr.remove(0,FloorStr.split("-")[0].size ()+1);
	for(int i=0;i<F_LeaCount;++i){
		int floor=FloorStr.split("-")[0].toInt();
		FloorStr.remove(0,FloorStr.split("-")[0].size ()+1);
		int charCount=FloorStr.split("-")[0].toInt();
		FloorStr.remove(0,FloorStr.split("-")[0].size ()+1);
		QString name=FloorStr.left(charCount);
		FloorStr.remove(0,charCount+1);

		person_floor newFloor(floor,name);
		FloorList.push_back(newFloor);
	}

	int P_LeaCount=PlatformStr.split("-")[0].toInt();
	PlatformStr.remove(0,PlatformStr.split("-")[0].size ()+1);
	for(int i=0;i<P_LeaCount;++i){
		int platform=PlatformStr.split("-")[0].toInt();
		PlatformStr.remove(0,PlatformStr.split("-")[0].size ()+1);
		int charCount=PlatformStr.split("-")[0].toInt();
		PlatformStr.remove(0,PlatformStr.split("-")[0].size ()+1);
		QString name=PlatformStr.left(charCount);
		PlatformStr.remove(0,charCount+1);

		person_platform newplatform(platform,name);
		PlatformList.push_back(newplatform);
	}

	DisplayLeaderboard ();
}
void Widget::ClearLeaderboard (){
	FloorList.clear ();
	PlatformList.clear ();
	DisplayLeaderboard ();
}

bool FloorLessThan(const person_floor &p1,const person_floor &p2){
	return p1.floor > p2.floor;
}

bool PlatLessThan(const person_platform &p1,const person_platform &p2){
	return p1.platform > p2.platform;
}

void Widget::ReadData (QString msg){
	DisplayOnConsole("Read from client. IP:"+MySocket->peerAddress().toString ());
	
	int recvFloor=msg.split("F")[0].toInt();
	msg.remove(0,msg.split("F")[0].size ()+1);
	int recvPlat=msg.split("P")[0].toInt();
	msg.remove(0,msg.split("P")[0].size ()+1);

	QString recvName=msg;

	person_floor newfloor(recvFloor,recvName);
	person_platform newplat(recvPlat,recvName);


	FloorList.push_back(newfloor);
	PlatformList.push_back(newplat);

	qSort(FloorList.begin(),FloorList.end(),FloorLessThan);
	qSort(PlatformList.begin(),PlatformList.end(),PlatLessThan);

	DisplayLeaderboard();
	
	SendToClient();
}

void Widget::DisplayLeaderboard(){
	ui->Floor->clear();
	ui->Platform->clear();
	
	for(int i=0;i<FloorList.count();++i){
		ui->Floor->setHtml(ui->Floor->toHtml()+QString::number(i+1)+".    "+QString::number(FloorList[i].floor)+"   "+FloorList[i].name);
	}
	for(int i=0;i<PlatformList.count();++i){
		ui->Platform->setHtml(ui->Platform->toHtml()+QString::number(i+1)+".    "+QString::number(PlatformList[i].platform)+"   "+PlatformList[i].name);
	}
}

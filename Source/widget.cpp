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
}

Widget::~Widget()
{
	MyServer->close();
	delete MyServer;
	MySocket->abort();
	delete MySocket;
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
		sendstr=QString::number(i+1,10)+". "+QString::number(FloorList[i].floor,10)+" "+FloorList[i].name;
		procstr+=QString::number(sendstr.size(),10)+"-"+sendstr+"-";
	}
	for(int i=0;i<PlatformList.count();++i){
		sendstr=QString::number(i+1,10)+". "+QString::number(PlatformList[i].platform,10)+" "+PlatformList[i].name;
		procstr+=QString::number(sendstr.size(),10)+"-"+sendstr+"-";
	}

	MySocket->write(procstr.toStdString ().c_str (),strlen (procstr.toStdString ().c_str ()));

	DisplayOnConsole("Send Leaderboard to client. IP:"+MySocket->peerAddress().toString ());
}

void Widget::SaveLeaderboard (){

}

void Widget::LoadLeaderboard (){

}
void Widget::ClearLeaderboard (){

}

bool FloorLessThan(const person_floor &p1,const person_floor &p2){
	return p1.floor > p2.floor;
}

bool PlatLessThan(const person_platform &p1,const person_platform &p2){
	return p1.platform > p2.platform;
}

void Widget::ReadData (QString msg){
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

	SendToClient();
}

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include <QFile>
#include <QtAlgorithms>

#define PORT 5000

namespace Ui {
	class Widget;
}

class person_floor{
	public:
		person_floor(int f,QString n){
			floor=f;
			name=n;
		}
		int floor;
		QString name;
};

class person_platform{
	public:
		person_platform(int f,QString n){
			platform=f;
			name=n;
		}
		int platform;
		QString name;
};

class Widget : public QWidget
{
		Q_OBJECT

	public:
		explicit Widget(QWidget *parent = 0);
		~Widget();

	private:
		Ui::Widget *ui;

		QTcpServer *MyServer;
		QTcpSocket *MySocket;

		QList<person_floor> FloorList;
		QList<person_platform> PlatformList;

		void AddFloor(person_floor input);
		void AddPlatform(person_platform input);
		void DisplayOnConsole(QString msg);
		void ReadData(QString msgrecvF);
		void SendData();


	private slots:
		void SaveLeaderboard();
		void LoadLeaderboard();
		void ClearLeaderboard();
		void StartServer();

		void NewConnect();
		void ReadFromClient();
		void SendToClient();

};

#endif // WIDGET_H

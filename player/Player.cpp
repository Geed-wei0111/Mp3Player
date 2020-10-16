//#include <iostream>
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>
#include <graphics.h>
#include <io.h>
#include <string>
#include <vector>
#pragma comment(lib,"winmm.lib")

//������ǰ��״̬
enum State{Play, Pause, Stop};

struct Time
{
	int Ms;

	int Minute;
	int Second;
	Time()
	{
		Ms = 0;
		Minute = 0;
		Second = 0;
	}
	void setTime(int ms)
	{
		this->Ms = ms;
		Ms = Ms / 1000;
		Minute = Ms / 60;
		Second = Ms % 60;
	}
};

class Player
{
public:
	Player()
	{
		memset(&this->chVolume, 0, sizeof(chVolume));
		memset(&this->length, 0, sizeof(length));
		memset(&this->length2, 0, sizeof(length2));

		//���ؽ��������ͼƬ
		loadimage(&NEXT, "IMAGE", "next");
		loadimage(&BEFORE, "IMAGE", "before");
		loadimage(&PAUSE, "IMAGE", "pause");
		loadimage(&PLAY, "IMAGE", "play");
		loadimage(&BACKGROUD, "IMAGE", "background");
		loadimage(&PLUS, "IMAGE", "plus");
		loadimage(&MINOR, "IMAGE", "minor");

		//��ʼ����������Ϣ

			//�洢�ļ���Ϣ�����֣�
			_finddata_t fileInfo;
			//�ļ����У���������������ļ����У�����Ϣ
			long fileHandle;
			//���������ļ�����string������
			int i = 0;
			std::string name;
			if ((fileHandle = _findfirst("./songs/*", &fileInfo)) == -1L) {
				;
			}
			else {
				do
				{
					name = fileInfo.name;
					if (name.find(".mp3", 0) <= name.length() || name.find(".wav", 0) <= name.length())
					{
						i++;
						std::string head = "./songs/";
						SongLocation.push_back(head+fileInfo.name);
						SongNames.push_back(fileInfo.name);
					}
				} while (_findnext(fileHandle, &fileInfo) == 0);
			}
			_findclose(fileHandle);

		this->SongCount = i;
		this->SongPosition = 0;
		this->State = Stop;
		this->Volume = 500;
	}
	
	void Draw()
	{
		//���ƽ���
		putimage(0, 0, &BACKGROUD);
		putimage(120, 285, &BEFORE);
		putimage(248, 285, &NEXT);

		putimage(180, 350, &PLUS);
		putimage(210, 350, &MINOR);
		
		if (State == Stop || State == Pause)
			putimage(180, 280, &PLAY);
		else
			putimage(180, 280, &PAUSE);
		//�����ǰ���ڲ���ĳ�׸�����ʾ���Ž���
		if (State != Stop)
		{
			this->showTime();
			this->getlength();
			this->getposition();

			setcolor(LIGHTBLUE);
			setfillcolor(LIGHTBLUE);
			int num = atoi(this->length) * 275 / atoi(this->length2);
			rectangle(75, 240, 350, 250);
			fillrectangle(75, 240, 75+num, 250);

			buffer = "��ǰ���ڲ��ţ�" + SongNames[SongPosition];
			setcolor(BLACK);
			outtextxy(0, 0, buffer.c_str());
		}
	}
	void Control()
	{
		MOUSEMSG msg;
		if (MouseHit())
			msg = GetMouseMsg();
		else
			goto Label;
		if (msg.uMsg == WM_LBUTTONDOWN)
		{
			if (msg.x > 120 && msg.x < 172 && msg.y>280 && msg.y < 332)//��һ��
			{
				if (SongPosition > 0)
				{
					this->stop();
					this->close();

					SongPosition--;

					this->open();
					this->play();
					this->setvolume();

					State = Play;
				}
				else
					this->Tips(0);
			}
			else if (msg.x > 248 && msg.x < 300 && msg.y>285 && msg.y < 337)//��һ��
			{
				if (SongPosition < SongCount - 1)
				{
					this->stop();
					this->close();

					SongPosition++;

					this->open();
					this->play();
					this->setvolume();

					State = Play;
				}
				else
					this->Tips(0);
			}
			else if (msg.x > 180 && msg.x < 250 && msg.y>280 && msg.y < 340 && State == Play)//��ͣ
			{
				this->pause();
				State = Pause;
			}
			else if (msg.x > 180 && msg.x < 250 && msg.y>280 && msg.y < 340 && State != Play)//����or����
			{
				if (SongNames.size() == 0)
				{
					this->Tips(0);
					return ;
				}
				if (State == Stop)
				{
					this->open();
					this->play();
					this->setvolume();
				}
				else
					this->resume();
				State = Play;
			}
			else if (msg.x > 180 && msg.x < 210 && msg.y > 350 && msg.y < 370)//��������
			{
				Volume += 100;
				this->setvolume();
			}
			else if (msg.x > 210 && msg.x < 240 && msg.y > 350 && msg.y < 370)//��С����
			{
				Volume -= 100;
				this->setvolume();
			}
		}
		if (msg.uMsg == WM_RBUTTONDBLCLK)//�رղ�����
		{
			this->Tips(1);
			system("PAUSE");
			if (State != Stop)
			{
				this->stop();
				this->close();
			}
			EndBatchDraw();
			closegraph();
			exit(0);
		}
Label:
		FlushMouseMsgBuffer();
	}
	//������ʱ����
	void absDelay(int delay)
	{
		static int curtime = GetTickCount();
		static int pretime = GetTickCount();
		while (curtime - pretime < delay)
		{
			curtime = GetTickCount();
			Sleep(1);
		}
		pretime = curtime;
	}
	void Check()
	{
		if (State == Play)
		{
			//�������һ��֤���Ѿ�������ϣ�����״̬
			if (atoi(length) == atoi(length2))
			{
				this->stop();
				this->close();
				State = Stop;
			}
		}
	}
private:
	//��������
	int SongCount;
	//��N�׸���
	int SongPosition;
	//����״̬
	State State;
	//���и�������Ϣ
	std::vector<std::string> SongLocation;
	std::vector<std::string> SongNames;
	//�滭����
	IMAGE NEXT, BEFORE, PAUSE, PLAY, BACKGROUD, PLUS, MINOR;
	//�ַ�������
	std::string buffer;
	//�������ų���
	Time Time;
	//��ȡ�������ȵĻ��洮
	char length[64];
	char length2[64];
	//��ǰ���Ÿ������������洮
	char chVolume[64];
	//��ǰ���Ÿ���������
	int Volume;

	//�����ַ�������
	void open()
	{
		buffer = "open ";
		buffer += SongLocation[SongPosition];
		mciSendString(buffer.c_str(), 0, 0, 0);
	}
	void close()
	{
		buffer = "close ";
		buffer += SongLocation[SongPosition];
		mciSendString(buffer.c_str(), 0, 0, 0);
	}
	void play()
	{
		buffer = "play ";
		buffer += SongLocation[SongPosition];
		mciSendString(buffer.c_str(), 0, 0, 0);
	}
	void resume()
	{
		buffer = "resume ";
		buffer += SongLocation[SongPosition];
		mciSendString(buffer.c_str(), 0, 0, 0);
	}
	void pause()
	{
		buffer = "pause ";
		buffer += SongLocation[SongPosition];
		mciSendString(buffer.c_str(), 0, 0, 0);
	}
	void stop()
	{
		buffer = "stop ";
		buffer += SongLocation[SongPosition];
		mciSendString(buffer.c_str(), 0, 0, 0);
	}

	void getposition()
	{
		//��ȡ������ǰ����
		buffer = "status " + SongLocation[SongPosition] + " position";
		mciSendString(buffer.c_str(), length, sizeof(length), 0);
		Time.setTime(atoi(length));
	}
	void getlength()
	{
		//��ȡ�����ܽ���
		buffer = "status " + SongLocation[SongPosition] + " length";
		mciSendString(buffer.c_str(), length2, sizeof(length2), 0);
	}
	void getvolume()
	{
		buffer = "status " + SongLocation[SongPosition] + " volume";
		mciSendString(buffer.c_str(), chVolume, sizeof(chVolume), 0);
		Volume = atoi(chVolume);
	}
	void setvolume()
	{
		itoa(Volume, chVolume, 10);
		buffer = "setaudio " + SongLocation[SongPosition] + " volume to " + chVolume;
		mciSendString(buffer.c_str(), 0, 0, 0);
	}
	//��ʾʱ��
	void showTime()
	{
		//���֣��봦����ַ���
		char ch;
		if (Time.Minute < 10)
		{
			ch = Time.Minute + '0';
			buffer = ch;
		}
		else
		{
			ch = Time.Minute / 10 + '0';
			buffer = ch;
			ch = Time.Minute % 10 + '0';
			buffer += ch;
		}
		buffer += ':';
		if (Time.Second < 10)
		{
			ch = (Time.Second + '0');
			buffer += '0';
			buffer += ch;
		}
		else
		{
			ch = Time.Second / 10 + '0';
			buffer += ch;
			ch = Time.Second % 10 + '0';
			buffer += ch;
		}
		//������ɫȥ��
		setbkmode(TRANSPARENT);
		setcolor(BLACK);
		outtextxy(195, 255, buffer.c_str());
	}
	//��ʾ
	void Tips(int n)
	{
		HWND hwnd = GetHWnd();
		if (n == 0)
		{
			MessageBox(hwnd, "No more songs.", "û�и�����.", MB_OK);
		}
		else if (n == 1)
		{
			MessageBox(hwnd, "��ȷ��Ҫ�˳���", "��ȷ��Ҫ�˳���", MB_OKCANCEL);
		}
	}
};

int main()
{
	initgraph(640, 480);
//	initgraph(640, 480, SHOWCONSOLE);
	Player p;
	BeginBatchDraw();
	while (1)
	{
		p.Draw();
		p.Check();
		p.Control();
		FlushBatchDraw();
		p.absDelay(16);
	}
	return 0;
}
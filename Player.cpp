//#include <iostream>
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>
#include <graphics.h>
#include <io.h>
#include <string>
#include <vector>
#pragma comment(lib,"winmm.lib")

//歌曲当前的状态
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

		//加载界面所需的图片
		loadimage(&NEXT, "IMAGE", "next");
		loadimage(&BEFORE, "IMAGE", "before");
		loadimage(&PAUSE, "IMAGE", "pause");
		loadimage(&PLAY, "IMAGE", "play");
		loadimage(&BACKGROUD, "IMAGE", "background");
		loadimage(&PLUS, "IMAGE", "plus");
		loadimage(&MINOR, "IMAGE", "minor");

		//开始检索歌曲信息

			//存储文件信息（名字）
			_finddata_t fileInfo;
			//文件（夹）句柄，用来保存文件（夹）的信息
			long fileHandle;
			//用来保存文件名的string类容器
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
		//绘制界面
		putimage(0, 0, &BACKGROUD);
		putimage(120, 285, &BEFORE);
		putimage(248, 285, &NEXT);

		putimage(180, 350, &PLUS);
		putimage(210, 350, &MINOR);
		
		if (State == Stop || State == Pause)
			putimage(180, 280, &PLAY);
		else
			putimage(180, 280, &PAUSE);
		//如果当前正在播放某首歌则显示播放进度
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

			buffer = "当前正在播放：" + SongNames[SongPosition];
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
			if (msg.x > 120 && msg.x < 172 && msg.y>280 && msg.y < 332)//上一曲
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
			else if (msg.x > 248 && msg.x < 300 && msg.y>285 && msg.y < 337)//下一曲
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
			else if (msg.x > 180 && msg.x < 250 && msg.y>280 && msg.y < 340 && State == Play)//暂停
			{
				this->pause();
				State = Pause;
			}
			else if (msg.x > 180 && msg.x < 250 && msg.y>280 && msg.y < 340 && State != Play)//继续or播放
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
			else if (msg.x > 180 && msg.x < 210 && msg.y > 350 && msg.y < 370)//增大音量
			{
				Volume += 100;
				this->setvolume();
			}
			else if (msg.x > 210 && msg.x < 240 && msg.y > 350 && msg.y < 370)//减小音量
			{
				Volume -= 100;
				this->setvolume();
			}
		}
		if (msg.uMsg == WM_RBUTTONDBLCLK)//关闭播放器
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
	//绝对延时函数
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
			//如果两数一样证明已经播放完毕，更改状态
			if (atoi(length) == atoi(length2))
			{
				this->stop();
				this->close();
				State = Stop;
			}
		}
	}
private:
	//歌曲个数
	int SongCount;
	//第N首歌曲
	int SongPosition;
	//歌曲状态
	State State;
	//所有歌曲的信息
	std::vector<std::string> SongLocation;
	std::vector<std::string> SongNames;
	//绘画缓存
	IMAGE NEXT, BEFORE, PAUSE, PLAY, BACKGROUD, PLUS, MINOR;
	//字符串缓存
	std::string buffer;
	//歌曲播放长度
	Time Time;
	//获取歌曲长度的缓存串
	char length[64];
	char length2[64];
	//当前播放歌曲的音量缓存串
	char chVolume[64];
	//当前播放歌曲的音量
	int Volume;

	//发送字符串函数
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
		//获取歌曲当前进度
		buffer = "status " + SongLocation[SongPosition] + " position";
		mciSendString(buffer.c_str(), length, sizeof(length), 0);
		Time.setTime(atoi(length));
	}
	void getlength()
	{
		//获取歌曲总进度
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
	//显示时间
	void showTime()
	{
		//将分，秒处理成字符串
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
		//将背景色去掉
		setbkmode(TRANSPARENT);
		setcolor(BLACK);
		outtextxy(195, 255, buffer.c_str());
	}
	//提示
	void Tips(int n)
	{
		HWND hwnd = GetHWnd();
		if (n == 0)
		{
			MessageBox(hwnd, "No more songs.", "没有歌曲了.", MB_OK);
		}
		else if (n == 1)
		{
			MessageBox(hwnd, "你确定要退出吗？", "你确定要退出吗？", MB_OKCANCEL);
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
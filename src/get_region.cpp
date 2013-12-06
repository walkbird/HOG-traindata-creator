#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <io.h>
#include <time.h>
#include <cstdio>

using namespace std;
using namespace cv;

//文件目录
string src_dir;
string output_dir;

Rect select;
bool select_flag=false;
Point origin;
Mat src_img;
Mat sel_img;

string winTitle;
string imgId;

string selTitle = "64*128";
time_t timestamp = 0;
int timeCoverCnt = 0;
string savedName;

bool doLockXY = true;

void onMouse(int event,int x,int y,int,void*)
{
    if(select_flag)
    {
        select.x=MIN(origin.x,x);//不一定要等鼠标弹起才计算矩形框，而应该在鼠标按下开始到弹起这段时间实时计算所选矩形框
        select.y=MIN(origin.y,y);
		select.height=abs(y-origin.y);
		select.width=abs(x-origin.x);//算矩形宽度和高度
        select&=Rect(0,0,src_img.cols,src_img.rows);//保证所选矩形框在视频显示区域之内
		if (doLockXY){
			if (select.width >= select.height / 2)
				select.width = select.height / 2;
			else 
				select.height = select.width * 2;
		}
		Mat show_img;
		src_img.copyTo(show_img);
		rectangle(show_img,select.tl(),select.br(),cvScalar(0,255,0,0), 2, 8, 0 );
		imshow(winTitle,show_img);
    }
    if(event==CV_EVENT_LBUTTONDOWN)
    {
        select_flag=true;//鼠标按下的标志赋真值
        origin=Point(x,y);//保存下来单击是捕捉到的点
        select=Rect(x,y,0,0);//这里一定要初始化，宽和高为(0,0)是因为在opencv中Rect矩形框类内的点是包含左上角那个点的，但是不含右下角那个点
    }
    else if(event==CV_EVENT_LBUTTONUP)
    {
		select_flag=false;
		if (select.width == 0 || select.height == 0)
			return;
        
		//显示所选内容
		Mat temp = src_img(select);
		resize(temp,sel_img,Size(64,128));
		namedWindow(selTitle);
		imshow(selTitle, sel_img);
		waitKey(1);

		//生成文件名：
		ostringstream sname;
		time_t time_now = time(NULL);
		if (timestamp == 0 || timestamp != time_now) timeCoverCnt = 0;
		else timeCoverCnt++;
		timestamp = time_now;
		sname << output_dir << "/" << imgId << "_" << timestamp << "_" << timeCoverCnt << ".jpg";
		savedName = sname.str();

		//保存图片
		imwrite(savedName,sel_img);
		cout<< savedName << " 截图保存，按“d”放弃保存" << endl; 
    }
}

//获取目录下文件列表
void getFiles( string path, vector<string>& files ) {  
    //文件句柄    
    long   hFile   =   0;    
    //文件信息    
    struct _finddata_t fileinfo;    
  
    string p;  
  
    if   ((hFile   =   _findfirst(p.assign(path).append("/*").c_str(),&fileinfo))   !=   -1)  {    
  
        do  {    
            //如果是目录,迭代之  
            //如果不是,加入列表  
            if   ((fileinfo.attrib   &   _A_SUBDIR)) {    
                if   (strcmp(fileinfo.name,".")   !=   0   &&   strcmp(fileinfo.name,"..")   !=   0)    
                    getFiles(   p.assign(path).append("/").append(fileinfo.name), files   );    
            }  else  {    
                files.push_back(   p.assign(path).append("/").append(fileinfo.name)  );  
            }    
        }   while   (_findnext(   hFile,   &fileinfo   )   ==   0);    
  
        _findclose(hFile);    
    }  
}  

int main(int argc, char **argv)
{
	if (argc != 3){
		cerr<<"get_region.exe src_dir output_dir"<<endl;
		return -1;
	}

	cout << "按键说明: "<< endl;
	cout << "      n - 下一张图片" << endl;
	cout << "      p - 前一张图片" << endl;
	cout << "      g - 按编号跳转到第n张图片" << endl;
	cout << "      s - 按名字跳转到图片" << endl;
	cout << "      l - 切换锁定截图选取框比例为2:1" << endl;
	cout << "      d - 删除当前保存的截取框内图片" << endl;
	cout << "      ESC - 退出" << endl;

	src_dir = string(argv[1]);
	output_dir = string(argv[2]);

	vector<string>   files;   

	getFiles(src_dir, files);

	for (int it = 0; it < files.size(); )
	{
		src_img = imread(files[it]);
		ostringstream winTitle_b;
		winTitle_b << it << " - " << files[it];
		winTitle = string(winTitle_b.str());
		string tmp_imgId = files[it].substr(0,files[it].rfind("."));
		imgId = tmp_imgId.substr(tmp_imgId.find("/")+1);
		namedWindow(winTitle);
		imshow(winTitle,src_img);

		//设置回调函数
		setMouseCallback(winTitle,onMouse,0);

		char ch = waitKey(0);
		switch(ch)
		{
		case 'n':
			it++;
			break;
		case 'p':
			it--;
			break;
		case 'g':
			do {
			cout<<"jump to num(0 - "<< files.size() - 1 << " ): "<<flush;
			cin>>it;
			}while(it >= files.size() || it < 0 );
			break;
		case 's':
			{
				cout<<"jump to filename:"<<flush;
				string filename;
				cin >> filename;
				string filePath = src_dir + "/" + filename;
				int j = 0;
				for (;j<files.size();++j)
				{
					if (files[j] == filePath)
					{
						it = j;
						break;
					}
				}
				if (j == files.size())
				{
					cout<< "文件没有找到，注意不需要带源目录" << endl; 
				}
			}
			break;
		case 'l':
			doLockXY = !doLockXY;
			cout<< "锁定XY比例：" << doLockXY << endl;
			break;
		case 'd':
			//删除最近截取的图片
			if (savedName != ""){
				cout<< savedName << " 放弃保存"<<endl;
				remove(savedName.c_str());
				savedName = "";
				destroyWindow(selTitle);
			}
			
			break;
		case 27:
			return 0;
		}
		destroyWindow(winTitle);
	}
	return 0;
}
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

//�ļ�Ŀ¼
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
        select.x=MIN(origin.x,x);//��һ��Ҫ����굯��ż�����ο򣬶�Ӧ������갴�¿�ʼ���������ʱ��ʵʱ������ѡ���ο�
        select.y=MIN(origin.y,y);
		select.height=abs(y-origin.y);
		select.width=abs(x-origin.x);//����ο�Ⱥ͸߶�
        select&=Rect(0,0,src_img.cols,src_img.rows);//��֤��ѡ���ο�����Ƶ��ʾ����֮��
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
        select_flag=true;//��갴�µı�־����ֵ
        origin=Point(x,y);//�������������ǲ�׽���ĵ�
        select=Rect(x,y,0,0);//����һ��Ҫ��ʼ������͸�Ϊ(0,0)����Ϊ��opencv��Rect���ο����ڵĵ��ǰ������Ͻ��Ǹ���ģ����ǲ������½��Ǹ���
    }
    else if(event==CV_EVENT_LBUTTONUP)
    {
		select_flag=false;
		if (select.width == 0 || select.height == 0)
			return;
        
		//��ʾ��ѡ����
		Mat temp = src_img(select);
		resize(temp,sel_img,Size(64,128));
		namedWindow(selTitle);
		imshow(selTitle, sel_img);
		waitKey(1);

		//�����ļ�����
		ostringstream sname;
		time_t time_now = time(NULL);
		if (timestamp == 0 || timestamp != time_now) timeCoverCnt = 0;
		else timeCoverCnt++;
		timestamp = time_now;
		sname << output_dir << "/" << imgId << "_" << timestamp << "_" << timeCoverCnt << ".jpg";
		savedName = sname.str();

		//����ͼƬ
		imwrite(savedName,sel_img);
		cout<< savedName << " ��ͼ���棬����d����������" << endl; 
    }
}

//��ȡĿ¼���ļ��б�
void getFiles( string path, vector<string>& files ) {  
    //�ļ����    
    long   hFile   =   0;    
    //�ļ���Ϣ    
    struct _finddata_t fileinfo;    
  
    string p;  
  
    if   ((hFile   =   _findfirst(p.assign(path).append("/*").c_str(),&fileinfo))   !=   -1)  {    
  
        do  {    
            //�����Ŀ¼,����֮  
            //�������,�����б�  
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

	cout << "����˵��: "<< endl;
	cout << "      n - ��һ��ͼƬ" << endl;
	cout << "      p - ǰһ��ͼƬ" << endl;
	cout << "      g - �������ת����n��ͼƬ" << endl;
	cout << "      s - ��������ת��ͼƬ" << endl;
	cout << "      l - �л�������ͼѡȡ�����Ϊ2:1" << endl;
	cout << "      d - ɾ����ǰ����Ľ�ȡ����ͼƬ" << endl;
	cout << "      ESC - �˳�" << endl;

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

		//���ûص�����
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
					cout<< "�ļ�û���ҵ���ע�ⲻ��Ҫ��ԴĿ¼" << endl; 
				}
			}
			break;
		case 'l':
			doLockXY = !doLockXY;
			cout<< "����XY������" << doLockXY << endl;
			break;
		case 'd':
			//ɾ�������ȡ��ͼƬ
			if (savedName != ""){
				cout<< savedName << " ��������"<<endl;
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
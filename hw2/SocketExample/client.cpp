#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <fstream>

#include "opencv2/opencv.hpp"

#define BUFF_SIZE 1024

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{

    int localSocket, remoteSocket, recved, port;

    struct timeval tv;

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    if (argc < 2)
    {
        cout << "Command not found.\n";
    }
    else
    {
        port = atoi(argv[1]);
    }

    localSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (localSocket == -1)
    {
        printf("Fail to create a socket.\n");
        return 0;
    }
    int index_folder = getpid();
    stringstream ss;
    ss << index_folder;
    string defaultPath = "./";
    string Client_index;
    ss >> Client_index;
    string folderPath = defaultPath + "Client_" + Client_index + "_Folder";

    if (0 != access(folderPath.c_str(), 0))
    {
        // if this folder not exist, create a new one.
        mkdir(folderPath.c_str(), 0777);
    }

    struct sockaddr_in info;
    bzero(&info, sizeof(info));

    info.sin_family = PF_INET;
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(port);

    char Message[BUFF_SIZE] = {};

    int err = connect(localSocket, (struct sockaddr *)&info, sizeof(info));
    if (err == -1)
    {
        printf("Connection error\n");
        return 0;
    }

    while (1)
    {
        fd_set master_socks;
        FD_ZERO(&master_socks);
        FD_SET(localSocket, &master_socks);
        char receiveMessage[BUFF_SIZE] = {};
        int sent;
        cout << "Enter commands:\n";
        scanf("%s", Message);

        if (strncmp("ls", Message, 2) == 0)
        {

            sent = send(localSocket, Message, strlen(Message), 0);
            while (1)
            {
                tv.tv_sec = 3;
                tv.tv_usec = 0;
                int newrv = select(localSocket + 1, &master_socks, NULL, NULL, &tv);
                if (newrv == 0)
                {
                    cout << "timeout, newrv= " << newrv << endl;

                    break;
                }
                else if ((recved = recv(localSocket, receiveMessage, sizeof(char) * BUFF_SIZE, 0)) == -1)
                {
                    cerr << "recv failed, received bytes = " << recved << endl;
                }
                cout << receiveMessage << endl;
            }
        }
        else if (strncmp("play", Message, 4) == 0)
        {
            int QUIT = 0;

            sent = send(localSocket, Message, strlen(Message), MSG_WAITALL);
            bzero(Message, sizeof(char) * BUFF_SIZE);
            sleep(1);
            cin >> Message; //video file name
            sent = send(localSocket, Message, strlen(Message), MSG_WAITALL);

            // get the resolution of the video
            Mat imgClient;

            bzero(receiveMessage, sizeof(char) * BUFF_SIZE);
            recved = recv(localSocket, receiveMessage, sizeof(char) * BUFF_SIZE, 0);
            int width = atoi(receiveMessage);

            bzero(receiveMessage, sizeof(char) * BUFF_SIZE);
            recved = recv(localSocket, receiveMessage, sizeof(char) * BUFF_SIZE, 0);
            int height = atoi(receiveMessage);

            cout << width << ", " << height << endl;

            //allocate container to load frames
            imgClient = Mat::zeros(height, width, CV_8UC3);
            int imgSize = imgClient.total() * imgClient.elemSize();
            cout << imgSize << endl;
            // ensure the memory is continuous (for efficiency issue.)
            if (!imgClient.isContinuous())
            {
                imgClient = imgClient.clone();
            }

            while (1)
            {
                tv.tv_sec = 3;
                tv.tv_usec = 0;
                int newrv = select(localSocket + 1, &master_socks, NULL, NULL, &tv);
                cout << newrv << endl;
                if (newrv == 0 || waitKey(33.3333) == 27)
                {
                    cout << "timeout, newrv= " << newrv << endl;
                    destroyAllWindows();
                    break;
                }
                else if ((recved = recv(localSocket, imgClient.data, imgSize, MSG_WAITALL)) == -1)
                {
                    cerr << "recv failed, received bytes = " << recved << endl;
                }

                //cout << "recv byte: " << recved << endl;
                startWindowThread();
                imshow("Video", imgClient);
                //Press ESC on keyboard to exit
                // notice: this part is necessary due to openCV's design.
                // waitKey means a delay to get the next frame.
                //char c = (char)waitKey(33.3333);
                /*if (waitKey(33.3333) == 27)
                {
                    destroyAllWindows();
                    break;
                }*/
            }
            //send(localSocket, (void *)QUIT, sizeof(QUIT), 0);
        }
        else if (strncmp("put", Message, 3) == 0)
        {
            sent = send(localSocket, Message, strlen(Message), 0);
            char filename[BUFF_SIZE] = {};
            cin >> filename; //file name
            sleep(1);
            sent = send(localSocket, filename, strlen(filename), 0);
        }
        else if (strncmp("get", Message, 3) == 0)
        {
            long cnt_count = 0;
            sent = send(localSocket, Message, strlen(Message), 0);
            char filename[BUFF_SIZE] = {};
            cin >> filename; //file name
            sleep(1);
            sent = send(localSocket, filename, strlen(filename), 0);
            stringstream ss, s1, s2;
            ss << filename;
            string filename_tmp;
            ss >> filename_tmp;
            s1 << (folderPath + "/" + filename_tmp);
            char file_path[BUFF_SIZE] = {};
            string File_path;
            s1 >> File_path;
            cout << File_path << endl;
            fstream ff(File_path.c_str(), ios::out | ios::binary);
            //FILE* file=fopen(File_path.c_str(),"wb");
            sleep(1);
            char filesize[BUFF_SIZE] = {};
            recv(localSocket, filesize, BUFF_SIZE, 0);
            long long FILESIZE = atoi(filesize);
            cout << "get filesize: " << FILESIZE << endl;

            while (1)
            {
                tv.tv_sec = 3;
                tv.tv_usec = 0;
                int newrv = select(localSocket + 1, &master_socks, NULL, NULL, &tv);
                char ch[BUFF_SIZE + 5] = {};
                if (newrv == 0)
                {
                    cout << "timeout, newrv= " << newrv << endl;

                    break;
                }
                else if ((recved = recv(localSocket, ch, BUFF_SIZE, 0)) == -1)
                {
                    cerr << "recv failed, received bytes = " << recved << endl;
                }
                for (int cnt = 0; /*(ch[cnt] != '\0') &&*/ cnt < 1024 && cnt + cnt_count <= FILESIZE;)
                {
                    ff.write(&ch[cnt++], 1);
                    ff.flush();
                }
                cnt_count += 1024;

                //ff.write(ch, sizeof(ch));
            }
            string command = "sed -e 's/\r//g' " + File_path + " > " + File_path + "1";
            //char *comm[50] = {};
            /*
            cout << command << endl;
            ff.close();
            system(command.c_str());
            command ="rm "+File_path;
            system(command.c_str());
            command="cp "+File_path+"1 "+File_path;
            system(command.c_str());
            command ="rm "+File_path+"1";
            system(command.c_str());
            */
            ff.close();
        }
        else
        {
            cout << "Command not found.\n";
        }
    }

    printf("close Socket\n");
    close(localSocket);
    return 0;
}

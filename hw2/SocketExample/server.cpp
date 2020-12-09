#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "opencv2/opencv.hpp"

#define BUFF_SIZE 1024

using namespace std;
using namespace cv;
int main(int argc, char **argv)
{

    /*setting start*/
    int localSocket, port = 4097;
    int remoteSocket = -2;

    if (argc < 2)
    {
        cout << "Command not found.\n";
    }
    else
    {
        port = atoi(argv[1]);
    }

    int recved;

    struct sockaddr_in localAddr, remoteAddr;

    int addrLen = sizeof(struct sockaddr_in);

    localSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (localSocket == -1)
    {
        printf("socket() call failed!!");
        return 0;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(port);

    char Message[BUFF_SIZE] = {};

    if (bind(localSocket, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0)
    {
        printf("Can't bind() socket");
        return 0;
    }

    listen(localSocket, 3);

    /*setting end*/

    while (1)
    {
        int sent;
        std::cout << "Waiting for connections...\n"
                  << "Server Port:" << port << std::endl;

        if (remoteSocket == -2)
            remoteSocket = accept(localSocket, (struct sockaddr *)&remoteAddr, (socklen_t *)&addrLen);

        if (remoteSocket < 0)
        {
            printf("accept failed!");
            return 0;
        }

        std::cout << "Connection accepted" << std::endl;

        /*receive command*/
        char receiveMessage[BUFF_SIZE] = {};

        bzero(receiveMessage, sizeof(char) * BUFF_SIZE);
        if ((recved = recv(remoteSocket, receiveMessage, sizeof(char) * BUFF_SIZE, 0)) < 0)
        {
            cout << "recv failed, with received bytes = " << recved << endl;
            break;
        }
        else if (recved == 0)
        {
            cout << "<end>\n";
            break;
        }
        printf("word len: %d: %s\n", recved, receiveMessage);

        /*check commands*/
        if (strncmp("ls", receiveMessage, 2) == 0)
        {
            cout << "recived: " << receiveMessage << "\n";
            // sent = send(remoteSocket,Message,strlen(Message),0);
        }
        else if (strncmp("play", receiveMessage, 4) == 0)
        {

            // server

            Mat imgServer;
            bzero(receiveMessage, sizeof(char) * BUFF_SIZE);

            recv(remoteSocket, receiveMessage, sizeof(char) * BUFF_SIZE, 0);
            cout << "videoname: " << receiveMessage << "\n";
            VideoCapture cap("./tmp.mpg");

            // get the resolution of the video
            int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
            int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
            cout << width << ", " << height << endl;
            

            //allocate container to load frames

            imgServer = Mat::zeros(height, width, CV_8UC3);

            sprintf(Message, "%d", width);
            sent = send(remoteSocket, Message, strlen(Message), 0);
            bzero(Message, sizeof(char) * BUFF_SIZE);

            sprintf(Message, "%d", height);
            sent = send(remoteSocket, Message, strlen(Message), 0);
            int imgSize = imgServer.total() * imgServer.elemSize();
            // ensure the memory is continuous (for efficiency issue.)
            if (!imgServer.isContinuous())
            {
                imgServer = imgServer.clone();
            }

            while (1)
            {
                //get a frame from the video to the container on server.
                cap >> imgServer;

                if ((sent = send(remoteSocket, imgServer.data, imgSize, 0)) < 0)
                {
                    cerr << "bytes = " << sent << endl;
                    char tmp[1]='E';
                    send(remoteSocket, tmp, 1, 0);
                    break;
                }
            }
        }
        else if (strncmp("put", receiveMessage, 3) == 0)
        {
        }
        else if (strncmp("get", receiveMessage, 3) == 0)
        {
        }
        else if (strncmp("close", receiveMessage, 5) == 0)
        {
            close(remoteSocket);
            remoteSocket = -2;
            cout << "close Socket.\n\n";
        }
        else
        {
            cout << "Command not found.\n";
        }
    }
    return 0;
}

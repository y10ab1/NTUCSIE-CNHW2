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
    int fdmax = 2;
    fd_set master_socks, command_socks;
    FD_ZERO(&master_socks);
    FD_ZERO(&command_socks);
    int status[100] = {0};
    VideoCapture capglocal;

    struct timeval tv;

    tv.tv_sec = 1;
    tv.tv_usec = 0;

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
    FD_SET(localSocket, &master_socks);
    fdmax = localSocket;

    /*setting end*/

    while (1)
    {
        command_socks = master_socks;

        //todo : select
        fdmax = (remoteSocket > fdmax) ? remoteSocket : fdmax;

        select(fdmax + 1, &command_socks, NULL, NULL, &tv);

        int sent;
        std::cout << "Waiting for connections...\n"
                  << "Server Port:" << port << std::endl;
        /*
        if (remoteSocket < 0)
        {
            printf("accept failed!");
            return 0;
        }
        */

        for (int i = 3; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &command_socks))
            {
                if (i == localSocket)
                { //new socket
                    if (remoteSocket = accept(localSocket, (struct sockaddr *)&remoteAddr, (socklen_t *)&addrLen) != -1)
                    {
                        FD_SET(remoteSocket, &master_socks);
                        fdmax = (remoteSocket > fdmax) ? remoteSocket : fdmax;
                        cout << "Connection accepted" << endl;
                    }
                    else
                    {
                        perror("accept failed!\n");
                        return 0;
                    }
                }
                else
                {
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
                        cout << "<socket closed>\n";
                        break;
                    }
                    else
                    {
                        printf("word len, command: %d: %s\n", recved, receiveMessage);

                        /*check commands*/
                        if (strncmp("ls", receiveMessage, 2) == 0)
                        {
                            cout << "recived: " << receiveMessage << "\n";
                            status[i] = 1;
                            // sent = send(remoteSocket,Message,strlen(Message),0);
                        }
                        else if (strncmp("play", receiveMessage, 4) == 0)
                        {
                            status[i] = 2;

                            // server

                            Mat imgServer;
                            bzero(receiveMessage, sizeof(char) * BUFF_SIZE);

                            recv(remoteSocket, receiveMessage, sizeof(char) * BUFF_SIZE, 0);
                            cout << "videoname: " << receiveMessage << "\n";
                            //VideoCapture cap("./tmp.mpg");
                            VideoCapture cap(receiveMessage);
                            capglobal=cap;

                            // get the resolution of the video
                            int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
                            int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
                            cout << width << ", " << height << endl;

                            //allocate container to load frames

                            imgServer = Mat::zeros(height, width, CV_8UC3);

                            sprintf(Message, "%d", width);
                            sent = send(remoteSocket, Message, strlen(Message), 0);
                            bzero(Message, sizeof(char) * BUFF_SIZE);
                            sleep(1);
                            sprintf(Message, "%d", height);
                            sent = send(remoteSocket, Message, strlen(Message), 0);
                            int imgSize = imgServer.total() * imgServer.elemSize();
                            // ensure the memory is continuous (for efficiency issue.)
                            if (!imgServer.isContinuous())
                            {
                                imgServer = imgServer.clone();
                            }
                            /*
                            while (1)
                            {
                                //get a frame from the video to the container on server.
                                cap >> imgServer;

                                if ((sent = send(remoteSocket, imgServer.data, imgSize, 0)) < 0)
                                {
                                    cerr << "bytes = " << sent << endl;

                                    break;
                                }
                                cout << "sent bytes: " << sent << endl;
                            }*/
                        }
                        else if (strncmp("put", receiveMessage, 3) == 0)
                        {
                            status[i] = 3;
                        }
                        else if (strncmp("get", receiveMessage, 3) == 0)
                        {
                            status[i] = 4;
                        }
                        else if (strncmp("close", receiveMessage, 5) == 0)
                        {
                            //status[i] = 5;
                            close(remoteSocket);
                            //remoteSocket = -2;
                            cout << "close Socket.\n\n";
                        }
                        else
                        {
                            cout << "Command not found.\n";
                        }
                    }
                }
            }

            if (FD_ISSET(i, &master_socks) && status[i] != 0)
            {
                switch (status[i])
                {
                case 1:
                    /* ls */
                    cout << "executing ls\n";
                    break;
                case 2:
                    /* play */
                    capglobal >> imgServer;

                    if ((sent = send(remoteSocket, imgServer.data, imgSize, 0)) < 0)
                    {
                        cerr << "bytes = " << sent << endl;

                        break;
                    }
                    cout << "sent bytes: " << sent << endl;

                    break;
                case 3:
                    /* put */
                    cout << "executing put\n";
                    break;
                case 4:
                    /* get */
                    cout << "executing get\n";
                    break;

                default:
                    break;
                }
            }
        }
    }
    return 0;
}


/* void command(int sockfd)
{
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
        //VideoCapture cap("./tmp.mpg");
        VideoCapture cap(receiveMessage);

        // get the resolution of the video
        int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        cout << width << ", " << height << endl;

        //allocate container to load frames

        imgServer = Mat::zeros(height, width, CV_8UC3);

        sprintf(Message, "%d", width);
        sent = send(remoteSocket, Message, strlen(Message), 0);
        bzero(Message, sizeof(char) * BUFF_SIZE);
        sleep(1);
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

                break;
            }
            cout << "sent bytes: " << sent << endl;
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
} */

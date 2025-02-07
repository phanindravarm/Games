#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <thread>
#include <mutex>
#include <map>
using namespace std;

mutex mut;
string player1Move = "";
string player2Move = "";
string result = "";
string playerMove = "";
int player1Socket = 0;
int player2Socket = 0;
int playerSocket = 0;
string gameResult(string player1Move, string player2Move)
{
    return (player1Move == player2Move) ? "Tie" : (player1Move == "Rock" && player2Move == "Paper")   ? "Player 2 Wins"
                                              : (player1Move == "Paper" && player2Move == "Scissors") ? "Player 2 Wins"
                                              : (player1Move == "Scissors" && player2Move == "Rock")  ? "Player 2 Wins"
                                                                                                      : "Player 1 Wins";
}

void resetGame()
{
    cout << "reset" << endl;
    player1Move = "";
    player2Move = "";

    player1Socket = 0;
    player2Socket = 0;
}

void handleRequest(int clientSocket)
{
    mut.lock();
    if (player1Socket == clientSocket)
    {
        cout << "exited at first" << endl;
        mut.unlock();
        return;
    }
    char request[1024];
    if (recv(clientSocket, request, sizeof(request), 0) < 0)
    {
        cout << "Error in receiving request" << endl;
        return;
    }

    stringstream ss(request);
    string method, path;
    ss >> method >> path;

    string response = "HTTP/1.1 200 OK\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Content-Type: text/plain\r\n\r\n";

    // mut.lock();
    cout << "after locking" << endl;
    path = path.substr(1);
    int slash = path.find('/');
    playerMove = path.substr(0, slash);

    playerSocket = clientSocket;
    cout << "player Socket : " << playerSocket << endl;

    if (!player1Socket)
    {
        player1Socket = playerSocket;
        cout << "entered 2" << endl;
        player1Move = playerMove;

        cout << "player1Move : " << player1Move << endl;

        // response += " Waiting for player 2 Move";
        string hey = response + " Waiting for player 2 Move";
        // if (send(player1Socket, hey.c_str(), hey.length(), 0) < 0)
        // {
        //     cout << "error in sending " << endl;
        // }
        cout << "closing in 2" << endl;
        // close(player1Socket);
        mut.unlock();
        cout << " after unlocking in 2" << endl;
        return;
    }
    else if (player2Socket != clientSocket)
    {
        player2Socket = playerSocket;
        cout << "entered 3" << endl;
        player2Move = playerMove;

        result = gameResult(player1Move, player2Move);
        // response += result;
        cout << "player2Move : " << player2Move << endl;
        string hey = response + result;
        cout << "player 1 socket in 3 : " << player1Socket << endl;
        cout << "player 2 socket in 3 : " << player2Socket << endl;

        if (send(player1Socket, hey.c_str(), hey.length(), 0) < 0)
        {
            cout << "Error in sending for player 1" << endl;
        }

        if (send(player2Socket, hey.c_str(), hey.length(), 0) < 0)
        {
            cout << "Error in sending for player 2" << endl;
        }
        cout << "closing 1 in 3" << endl;
        close(player1Socket);
        cout << "closing 2 in 3" << endl;
        close(player2Socket);
        cout << "after unlocking at 3" << endl;
        resetGame();
        mut.unlock();
        return;
    }
}

int main()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Error creating socket");
        return -1;
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int bindInteger = ::bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindInteger < 0)
    {
        perror("Error binding");
        return 0;
    }
    listen(serverSocket, 5);
    cout << "Server is listening on port 8080..." << endl;

    while (true)
    {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0)
        {
            cout << "Error in accepting client connection" << endl;
            return -1;
        }
        cout << "client socket : " << clientSocket << endl;
        cout << "player 1 socket before  : " << player1Socket << endl;
        cout << "player 2 socket before  : " << player2Socket << endl;
        thread t(handleRequest, clientSocket);
        t.detach();
    }

    close(serverSocket);
    return 0;
}
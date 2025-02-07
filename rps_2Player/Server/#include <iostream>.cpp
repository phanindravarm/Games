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
string player1Id = "";
string player2Id = "";
bool player1Turn = true;
bool player2Turn = false;
int player1Socket = 0;
int player2Socket = 0;
string result = "";
map<int, pair<string, bool>> playerMoves;

string gameResult(string player1Move, string player2Move)
{
    return (player1Move == player2Move) ? "Tie" : (player1Move == "Rock" && player2Move == "Paper")   ? "Player 2 Wins"
                                              : (player1Move == "Paper" && player2Move == "Scissors") ? "Player 2 Wins"
                                              : (player1Move == "Scissors" && player2Move == "Rock")  ? "Player 2 Wins"
                                                                                                      : "Player 1 Wins";
}

void resetGame()
{
    player1Move = "";
    player2Move = "";
    player1Turn = true;
    player2Turn = false;
}

void handleRequest(int clientSocket)
{
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

    mut.lock();
    if (player1Move.empty())
    {
        player1Socket = clientSocket;
        cout << "entered 2" << endl;
        player2Turn = true;
        player1Turn = false;
        path = path.substr(1);
        int slash = path.find('/');
        player1Move = path.substr(0, slash);
        player1Id = path.substr(slash + 1);
        cout << "player1Move : " << player1Move << endl;
        cout << "player1Id : " << player1Id << endl;
        response += " Waiting for player 2 Move";
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    else if (player2Move.empty())
    {

        cout << "entered 3" << endl;
        player2Turn = false;
        path = path.substr(1);
        int slash = path.find('/');
        player2Move = path.substr(0, slash);
        player2Id = path.substr(slash + 1);
        result = gameResult(player1Move, player2Move);
        response += player2Move + " " + result;
        cout << "player2Move : " << player2Move << endl;
        cout << "player2Id : " << player2Id << endl;
        send(clientSocket, response.c_str(), response.length(), 0);
        resetGame();
    }

    close(clientSocket);
    mut.unlock();
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
    listen(serverSocket, 1);
    cout << "Server is listening on port 8080..." << endl;

    while (true)
    {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0)
        {
            cout << "Error in accepting client connection" << endl;
            return -1;
        }
        // cout << "Client connected with socket: " << clientSocket << endl;
        thread t(handleRequest, clientSocket);
        t.detach();
    }

    close(serverSocket);
    return 0;
}
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
bool player1Turn = true;
bool player2Turn = false;
int player1Socket = 0;
int player2Socket = 0;
string result = "";
map<int, pair<string, bool>> playerMoves; // Stores player moves and their state (whether they are ready)

string gameResult(string player1Move, string player2Move)
{
    return (player1Move == player2Move) ? "tie" : (player1Move == "Rock" && player2Move == "Paper")   ? "Player 2 Wins"
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
    cout << "request :" << request << endl;
    stringstream ss(request);
    string method, path;
    ss >> method >> path;

    string response = "HTTP/1.1 200 OK\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Content-Type: text/plain\r\n\r\n";

    mut.lock();
    if (player1Turn)
    {
        player1Socket = clientSocket;
        player1Move = path.substr(1); // Remove the leading slash
        player1Turn = false;
        player2Turn = true;
        response += "Waiting for player 2 Move";
    }
    else if (player2Turn)
    {
        player2Socket = clientSocket;
        player2Move = path.substr(1); // Remove the leading slash
        result = gameResult(player1Move, player2Move);

        // Send the result to both players
        string resultMessage = player2Move + " " + result;
        send(player1Socket, resultMessage.c_str(), resultMessage.length(), 0);
        send(player2Socket, resultMessage.c_str(), resultMessage.length(), 0);

        resetGame(); // Reset the game after sending the result
    }

    // Long-polling: Keep the connection open until a result is available
    while (player1Turn || player2Turn)
    {
        // Keep waiting for the next player move
        usleep(100000); // Sleep for a while to prevent high CPU usage
    }

    if (send(clientSocket, response.c_str(), response.length(), 0) < 0)
    {
        perror("Error sending");
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
        thread t(handleRequest, clientSocket);
        t.detach();
    }

    close(serverSocket);
    return 0;
}

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <mutex>
#include <thread>
#include <fcntl.h>
#include <set>
#include <map>
using namespace std;

int socket1, socket2, socketcs;
const int PORT1 = 8081;                             // my port for communication
const int PORT2 = 8080;                             // port of computer 1 for communication
const int PORT3 = 9097;                             // port of computer 1 for file access
const std::string COMPUTER1_IP = "10.10.58.241";       // Ip of computer 1
mutex socket1_mutex, socket2_mutex, queue_mutex, reply_mutex, lclock_mutex;
bool debug = true;

set<pair<int, int>> queue;
vector<int> reply(4, 0);
int lclock = 0;
int my_id = 2;
vector<int> ids = {1, 2, 3};
map<int, int> socket_to_id;

int connect_socket_1() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create a socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Set up the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT1);

    // Bind the socket to the server address
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Binding failed\n";
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server listening on port " << PORT1 << std::endl;

    // Accept incoming connections
    if ((socket1 = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
        std::cerr << "Accept failed\n";
        return 1;
    }

    std::cout << "Connection accepted from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;

    return 0;
}

int connect_socket_2() {
    struct sockaddr_in serverAddr;

    // Create a socket
    if ((socket2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Set up the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT2);
    serverAddr.sin_addr.s_addr = inet_addr(COMPUTER1_IP.c_str());

    // Connect to the server
    if (connect(socket2, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }

    std::cout << "Connected to server at " << COMPUTER1_IP << ":" << PORT2 << std::endl;

    return 0;
}

int connect_socket_cs() {
    struct sockaddr_in serverAddr;

    // Create a socket
    if ((socketcs = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Set up the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT3);
    serverAddr.sin_addr.s_addr = inet_addr(COMPUTER1_IP.c_str());

    // Connect to the server
    if (connect(socketcs, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }

    std::cout << "Connected for cs " << COMPUTER1_IP << ":" << PORT2 << std::endl;

    return 0;
}

// Function to deserialize a byte array into a vector<int>
std::vector<int> deserializeVector(const char* data, size_t dataSize) {
    std::vector<int> vec(dataSize / sizeof(int));
    std::memcpy(vec.data(), data, dataSize);
    return vec;
}

// Function to serialize a vector<int> into a byte array
std::vector<char> serializeVector(const std::vector<int>& vec) {
    std::vector<char> data(sizeof(int) * vec.size());
    std::memcpy(data.data(), vec.data(), data.size());
    return data;
}

bool send_message(int socket, vector<int>& v) {
    // Serialize vector into byte array
    std::vector<char> serializedData = serializeVector(v);

    // Send serialized data over the socket
    if (send(socket, serializedData.data(), serializedData.size(), 0) < 0) {
        std::cerr << "Send failed\n";
        return false;
    }

    return true;
}

vector<int> receive_message(int socket) {
    char recvBuffer[1024];

    // Make the socket non-blocking
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Error in fcntl F_GETFL: " << strerror(errno) << std::endl;
        return {};
    }
    flags |= O_NONBLOCK;
    if (fcntl(socket, F_SETFL, flags) == -1) {
        std::cerr << "Error in fcntl F_SETFL: " << strerror(errno) << std::endl;
        return {};
    }

    // Receive serialized data
    int bytesRead = recv(socket, recvBuffer, sizeof(recvBuffer), 0);
    if (bytesRead < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Receive failed\n";
        }
        return {};
    }

    // Deserialize data into vector<int>
    std::vector<int> receivedVector = deserializeVector(recvBuffer, bytesRead);

    return receivedVector;
}

void sender() {
    while (1) {
        lclock_mutex.lock();
        queue_mutex.lock();
        socket1_mutex.lock();
        socket2_mutex.lock();
        lclock++;
        if (debug) {
            cout << "D: Inserting to queue ("<< lclock<< ","<< my_id<< ")\n"; 
        }
        queue.insert({lclock, my_id});
        vector<int> v1 = {1, lclock, my_id};
        if (debug) {
            cout << "D: Sending Reqeust for CS to 1\n";
            cout << "D: Sending Request for CS to 2\n";
        }
        send_message(socket1, v1);
        send_message(socket2, v1);
        lclock_mutex.unlock();
        queue_mutex.unlock();
        socket1_mutex.unlock();
        socket2_mutex.unlock();
        while (1) {
            bool reply_received = false;
            reply_mutex.lock();
            if (reply[1] != 0 && reply[3] != 0) {
                reply_received = true;
                reply[0] = 0;
                reply[1] = 0;
                reply[2] = 0;
                reply[3] = 0;
            }
            reply_mutex.unlock();
            if (reply_received) {
                cout << "I: replied receive \n";
                break;
            }
            sleep(1);
        }
        while (1) {
            bool my_process_in_front = false;
            queue_mutex.lock();
            if ((*queue.begin()).second == my_id) {
                my_process_in_front = true;
            }
            queue_mutex.unlock();
            if (my_process_in_front) {
                break;
            }
            sleep(1);
        }
        // cs
        cout << "I: accessing critical section\n";
        vector<int> vcs = {1, 2, 3};
        srand(time(0));
        vcs[0] = rand() % 100;
        vcs[1] = 20 + (rand() % 100);
        vcs[2] = 40 + (rand() % 100);
        cout << "I: writing "<< vcs[0]<< " "<< vcs[1]<< " "<< vcs[2]<< " into file\n";
        send_message(socketcs, vcs);
        sleep(3);
        lclock_mutex.lock();
        socket1_mutex.lock();
        socket2_mutex.lock();
        lclock++;
        vector<int> v2 = {3, lclock, my_id};
        if (debug) {
            cout << "D: Sending Release message to 2\n";
            cout << "D: Sending Release message to 3\n";
        }
        send_message(socket1, v2);
        send_message(socket2, v2);
        if (debug) {
            cout << "D: Removing from my queue ("<< (*queue.begin()).first<< ","<<(*queue.begin()).second<< ")\n";
        }
        queue.erase(queue.begin());        
        lclock_mutex.unlock();
        socket1_mutex.unlock();
        socket2_mutex.unlock();
        sleep(1);
    }
}

void listen_on_socket1() {
    while (1) {
        socket1_mutex.lock();
        vector<int> v = receive_message(socket1);
        socket1_mutex.unlock();
        if (v.size() > 0) {
            lclock_mutex.lock();
            lclock = (max(lclock, v[1])) + 1;
            lclock_mutex.unlock();
            if (v[0] == 1) {
                socket1_mutex.lock();
                queue_mutex.lock();
                queue.insert({v[1], v[2]});
                vector<int> v1 = {2, lclock, my_id};
                send_message(socket1, v1);
                socket1_mutex.unlock();
                queue_mutex.unlock();
            } else if (v[0] == 2) {
                reply_mutex.lock();
                reply[v[2]] = 1;
                reply_mutex.unlock();
            } else {
                queue_mutex.lock();
                queue.erase(queue.begin());
                queue_mutex.unlock();
            }
        }
        // sleep(1);
    }
}

void listen_on_socket2() {
    while (1) {
        socket2_mutex.lock();
        vector<int> v = receive_message(socket2);
        socket2_mutex.unlock();
        if (v.size() > 0) {
            lclock_mutex.lock();
            lclock = (max(lclock, v[1])) + 1;
            lclock_mutex.unlock();
            if (v[0] == 1) {
                socket2_mutex.lock();
                queue_mutex.lock();
                queue.insert({v[1], v[2]});
                vector<int> v1 = {2, lclock, my_id};
                send_message(socket2, v1);
                socket2_mutex.unlock();
                queue_mutex.unlock();
            } else if (v[0] == 2) {
                reply_mutex.lock();
                reply[v[2]] = 1;
                reply_mutex.unlock();
            } else {
                queue_mutex.lock();
                queue.erase(queue.begin());
                queue_mutex.unlock();
            }
        }
        // sleep(1);
    }
}

void receiver() {
    thread listneer1(listen_on_socket1);
    thread listneer2(listen_on_socket2);
    listneer1.detach();
    listneer2.detach();
}

int main() {
    connect_socket_2();
    sleep(1);
    connect_socket_1();
    sleep(14);
    connect_socket_cs();
    socket_to_id[socket1] = 3;
    socket_to_id[socket2] = 1;
    thread receiverThread(receiver);
    sleep(15);
    thread senderThread(sender);

    senderThread.join();
    receiverThread.join();
    return 0;
}
/* C++ standard include files first */
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <mutex>
#include <cstdio>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>

using namespace std;

/* C system include files next */
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>

/* C standard include files next */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

/* your own include last */
#include "my_socket.h"
#include "my_readwrite.h"
#include "my_timestamp.h"
#include "utils.h"
#include "Connection.h"
#include "md5-calc.h"

static int master_socket_fd = (-1); /* there is nothing wrong with using a global variable */

void consoleFunction(std::mutex &ioMutex, vector<unique_ptr<Connection>> &connections)
{
    enum Command
    {
        STATUS,
        QUIT,
        HELP,
        CLOSE,
        DIAL
    };
    const string help = "Available commands are:\tclose #\n\n\thelp\n\tstatus\n\tquit\n\tdial # percent\n";
    const string closeFailure = "Invalid or inactive connection number: #\n";
    string command;
    unordered_map<string, Command> commands = {{"status", STATUS},
                                               {"quit", QUIT},
                                               {"close", CLOSE},
                                               {"dial", DIAL}};
    char streamBuf[1024];
    while (master_socket_fd != -1)
    {
        cout << '>';
        int connectionToClose = -1;
        int percent = 100;
        std::getline(cin, command);
        if (command.empty())
        {
            continue;
        }
        else if (strncmp(command.c_str(), "close ", 6) == 0)
        {
            char *endptr;
            connectionToClose = strtol(command.c_str() + 6, &endptr, 10);
            if ((command.c_str() + 6) == endptr)
            {
                cout << help;
                continue;
            }
            command = "close";
        }
        else if (strncmp(command.c_str(), "dial", 4) == 0)
        {
            connectionToClose = -1;
            if (sscanf(command.c_str(), "dial %d %d", &connectionToClose, &percent) != 2)
            {
                cout << "Invalid percent. The command syntax is \" dial #percent \".  Please try again.\n";
                continue;
            }
            if (percent > 100 || percent < 1)
            {
                cout << "Dial value is out of range (it must be >=1 and <= 100)\n";
                continue;
            }
            command = "dial";
        }
        auto result = commands.find(command);
        if (result == commands.end())
        {
            cout << help;
        }
        else
        {
            switch (result->second)
            {
            case STATUS:
            {
                bool empty = true;
                struct timeval t2;
                gettimeofday(&t2, NULL);
                vector<int> openConnections;
                {
                    lock_guard<mutex> ioGuard(ioMutex);
                    for (auto &connection : connections)
                    {
                        if (connection->socket_fd >= 0)
                        {
                            if (empty)
                            {
                                empty = false;
                                cout << "The following connections are active:\n";
                            }
                            double r = ((double)connection->MAXR / (double)connection->p) * ((double)connection->dial / 100.0f);
                            sprintf(streamBuf, "[%d]\tClient: %s\n\tPath: %s\n\tContent-Length:%d\n\tStart-Time: %s\n\tSharper-Params: P=%d, MAXR=%d tokens/s, DIAL=%d%%, rate=%.2f KB/s\n\tSent: %d bytes (%.2f%%), time elapsed: %.2f sec\n", connection->conn_number, connection->IPPORT.c_str(), connection->path.c_str(), connection->total, connection->startTime.c_str(), connection->p, connection->MAXR, connection->dial, r, connection->Kb_sent, (double)connection->Kb_sent / (double)connection->total * 100, timestamp_diff_in_seconds(&connection->start, &t2));
                            cout << streamBuf;
                        }
                    }
                }
                if (empty)
                {
                    cout << "No active connections.\n";
                }
                break;
            }
            case QUIT:
            {

                shutdown(master_socket_fd, SHUT_RDWR);
                close(master_socket_fd);
                master_socket_fd = -1;
                break;
            }
            case DIAL:
            {
                bool success = false;
                lock_guard<mutex> ioGuard(ioMutex);
                for (auto &connection : connections)
                {
                    if (connection->conn_number == connectionToClose)
                    {
                        success = true;
                        connection->dial = percent;
                        double r = ((double)connection->MAXR / (double)connection->p) * ((double)connection->dial / 100.0f);
                        sprintf(streamBuf, "Dial for connection %d at %d%%.  Token rate at %.3f tokens/s.  Data rate at %.3f KB/s.\n", connectionToClose, connection->dial, (double)connection->MAXR * percent / 100.0f, r);
                        cout << streamBuf;
                    }
                }
                if (!success)
                {
                    cout << "connection not found\n";
                }
                break;
            }
            case CLOSE:
            {
                lock_guard<mutex> ioGuard(ioMutex);
                bool success = false;
                for (auto &connection : connections)
                {
                    if (connection->conn_number == connectionToClose)
                    {
                        if (connection->socket_fd >= 0)
                        {
                            shutdown(connection->socket_fd, SHUT_RDWR);
                            connection->socket_fd = -2;
                            sprintf(streamBuf, "[%d]\tClose connection requested.\n", connectionToClose);
                            cout << streamBuf;
                            success = true;
                            break;
                        }
                    }
                }
                if (!success)
                {
                    cout << closeFailure;
                }
            }
            }
        }
    }
    {
        lock_guard<mutex> ioGuard(ioMutex);
        for (auto &connection : connections)
        {
            if (connection->socket_fd >= 0)
            {
                shutdown(connection->socket_fd, SHUT_RDWR);
                connection->socket_fd = -2;
            }
        }
    }
}

void reaperFunction(std::mutex &ioMutex, vector<unique_ptr<Connection>> &connections, ostream &log)
{
    char streamBuf[1024];
    while (true)
    {
        usleep(250);
        {
            lock_guard<mutex> ioGuard(ioMutex);
            if (master_socket_fd == -1 && connections.empty())
            {
                break;
            }
            else
            {
                for (int i = 0; i < connections.size(); i++)
                {
                    auto &connection = connections[i];
                    if (connection->socket_fd == -1)
                    {
                        connection->thread_ptr->join();
                        sprintf(streamBuf, "[%d]\tReaper has joined with connection thread\n", connection->conn_number);
                        log << GetTimeFormatted() << streamBuf << '\n';
                        connections[i].swap(connections.back());
                        connections.pop_back();
                        i--;
                    }
                }
            }
        }
    }
}

/**
 * This is the function you need to change to change the behavior of your server!
 *
 * @param newsockfd - socket that can be used to "talk" (i.e., read/write) to the client.
 */
void talk_to_client(std::mutex &ioMutex, ostream &log, Connection *connection, const string &rootDir, INIFile &config)
{
    {
        lock_guard<mutex> ioGuard(ioMutex);
    }
    connection->startTime = GetTimeFormatted();
    gettimeofday(&(connection->start), NULL);
    int newsockfd = connection->socket_fd;
    int id = connection->conn_number;
    string clientIP = get_ip_and_port_for_server(newsockfd, 0);
    char streamBuf[1024];
    int bytes_received = 0;
    string line;
    string request;
    bool not_closed = true;
    while (not_closed)
    {
        string header;
        bytes_received = read_a_line(newsockfd, request);
        if (bytes_received < 2)
        {
            break;
        }
        header += ("\t[" + to_string(id) + "]\t" + request);
        while (true)
        {
            bytes_received = read_a_line(newsockfd, line);
            header += ("\t[" + to_string(id) + "]\t" + line);
            if (bytes_received < 2)
            {
                shutdown(newsockfd, SHUT_RDWR);
                close(newsockfd);
                return;
            }
            if (line.size() == 2)
                break;
        }
        auto params = SplitSanjay(request, ' ');
        if (params[0].compare("GET") || params[2].compare(0, 6, "HTTP/1"))
        {
            shutdown(newsockfd, SHUT_RDWR);
            close(newsockfd);
            return;
        }
        string contentType = "application/octet-stream";
        if (hasEnding(params[1], ".html"))
            contentType = "text/html";
        auto path = rootDir + params[1];
        {
            lock_guard<mutex> ioGuard(ioMutex);
            sprintf(streamBuf, " REQUEST[%d]: %s, uri=%s\n", id, clientIP.c_str(), params[1].c_str());
            log << GetTimeFormatted() << streamBuf
                << header;
        }
        const string notFound = "HTTP/1.1 404 Not Found\r\nServer: lab4a\r\nContent-Type: text / html\r\nContent-Length: 63\r\nContent-MD5: 5b7e68429c49c66e88489a80d9780025\r\n\r\n<html><head></head><body><h1>404 Not Found</h1></body></html>\r\n";
        auto size = get_file_size(path);
        if (size < 1)
        {
            better_write(newsockfd, notFound.c_str(), notFound.length());
            {
                sprintf(streamBuf, " RESPONSE[%d]: %s, status=404\n", id, clientIP.c_str());
                lock_guard<mutex> ioGuard(ioMutex);
                log << GetTimeFormatted() << streamBuf;
                stringstream ss(notFound);
                while (getline(ss, line))
                {
                    log << '\t' << '[' << id << ']' << '\t' << line << endl;
                    if (line.size() == 1)
                        break;
                }
            }
        }
        else
        {
            connection->path = move(params[1]);
            auto extention = connection->path.substr(connection->path.find_last_of(".") + 1);
            if (config.find(extention) != config.end())
            {
                connection->MAXR = stoi(config[extention]["MAXR"]);
                connection->p = stoi(config[extention]["P"]);
                connection->dial = stoi(config[extention]["DIAL"]);
            }
            double r = ((double)connection->MAXR / (double)connection->p) * ((double)connection->dial / 100.0f);
            string foundHeader = "HTTP/1.1 200 OK\r\nServer: pa2 (davidz@usc.edu)\r\nContent-Type: " + contentType + "\r\nContent-Length:" + to_string(size) + "\r\nContent-MD5: " + GetMd5(path) +
                                 "\r\n\r\n";
            {
                sprintf(streamBuf, " RESPONSE[%d]: %s, status=200, P=%d, MAXR=%d tokens/s, DIAL=%d%%, rate=%.2f KB/s\n", id, clientIP.c_str(), connection->p, connection->MAXR, connection->dial, r);
                lock_guard<mutex> ioGuard(ioMutex);
                log << GetTimeFormatted() << streamBuf;
                stringstream ss(foundHeader);
                while (getline(ss, line))
                {
                    log << '\t' << '[' << id << ']' << '\t' << line << endl;
                    if (line.size() == 1)
                        break;
                }
            }
            better_write(newsockfd, foundHeader.c_str(), foundHeader.length());
            char buf[1024];
            auto fd = open(path.c_str(), O_RDONLY);
            auto X = size;
            connection->total = size;
            double p = (double)connection->p;
            struct timeval t1;
            gettimeofday(&t1, NULL);
            auto b1 = p;
            while (X > 0)
            {
                r = ((double)connection->MAXR / (double)connection->p) * ((double)connection->dial / 100.0f);
                int bytes_to_read = (X > 1024) ? 1024 : X;
                int bytes_actually_read = read(fd, buf, bytes_to_read);
                if (bytes_actually_read <= 0)
                {
                    shutdown(newsockfd, SHUT_RDWR);
                    close(newsockfd);
                    return;
                }
                better_write(newsockfd, buf, bytes_actually_read);
                X -= bytes_actually_read;
                {
                    lock_guard<mutex> ioGuard(ioMutex);
                    connection->Kb_sent += bytes_actually_read;
                    if (connection->socket_fd == -2 || master_socket_fd == -1)
                    {
                        not_closed = false;
                        break;
                    }
                }
                bool no_enough = true;
                while (no_enough)
                {
                    struct timeval t2;
                    gettimeofday(&t2, NULL);
                    auto n = (int)(r * (timestamp_diff_in_seconds(&t1, &t2)));
                    if ((n > 1) || (b1 == p && b1 - p + n >= p))
                    {
                        add_seconds_to_timestamp(&t1, (((double)1.0) / ((double)r)), &t1);
                        b1 = p;
                        no_enough = false;
                    }
                    else
                    {
                        b1 = 0;
                        struct timeval t3;
                        add_seconds_to_timestamp(&t1, (((double)1.0) / ((double)r)), &t3);
                        auto timetosleep = timestamp_diff_in_seconds(&t2, &t3);
                        auto usec = timetosleep * 1000000;
                        if (usec > 0)
                            usleep(std::min((int)usec, 250000));
                    }
                }
                {
                    lock_guard<mutex> ioGuard(ioMutex);
                    connection->Kb_sent++;
                    if (connection->socket_fd == -2 || master_socket_fd == -1)
                    {
                        not_closed = false;
                        break;
                    }
                }
            }
        }
    }
    {
        lock_guard<mutex> ioGuard(ioMutex);
        if (connection->socket_fd >= 0)
            shutdown(newsockfd, SHUT_RDWR);
        connection->socket_fd = -1;
        close(connection->orig_socket_fd);
        sprintf(streamBuf, "[%d]\tConnection closed with client at %s\n", id, clientIP.c_str());
        log << GetTimeFormatted() << streamBuf << '\n'
            << flush;
        return;
    }
}

void echo_server(string port, const string &rootDir, ostream &log, const string &pidFileName, INIFile &config)
{
    master_socket_fd = create_master_socket(port);
    auto MAXR = stoi(config["*"]["MAXR"]);
    auto P = stoi(config["*"]["P"]);
    auto DIAL = stoi(config["*"]["DIAL"]);
    if (master_socket_fd != (-1))
    {
        string s = get_ip_and_port_for_server(master_socket_fd, 1);
        log << GetTimeFormatted() << "[SERVER]\tstarted at " << s << '\n';
        int id = 1;
        vector<unique_ptr<Connection>> connections;
        std::mutex ioMutex;
        auto console = new thread(&consoleFunction, ref(ioMutex), ref(connections));
        auto reaper = new thread(&reaperFunction, ref(ioMutex), ref(connections), std::ref(log));
        while (1)
        {
            int newsockfd = my_accept(master_socket_fd);
            if (newsockfd != (-1))
            {
                {
                    lock_guard<mutex> ioGuard(ioMutex);
                    connections.emplace_back(make_unique<Connection>(id++, newsockfd, nullptr, P, MAXR, DIAL));
                    connections.back()->thread_ptr.reset(new thread(&talk_to_client, std::ref(ioMutex), std::ref(log), connections.back().get(), std::ref(rootDir), std::ref(config)));
                }
            }
            else
            {
                break;
            }
        }
        console->join();
        reaper->join();
        while (!connections.empty())
        {
            connections.back()->thread_ptr->join();
            connections.pop_back();
        }
        log << GetTimeFormatted() << "[SERVER]\tstopped at " << s << '\n';
    }
}

/**
 * This is the function you need to change to change the behavior of your server!
 * Returns non-zero if succeeds.
 * Otherwise, return 0;
 *
 * @param argc - number of arguments in argv.
 * @param argv - array of argc C-strings, must only use array index >= 0 and < argc.
 */
static void process_options(int argc, char *argv[])
{
    INIFile config = ParseINI(argv[1]);
    auto &section = config["startup"];
    if (section.find("logfile") != section.end())
    {
        ofstream log;
        if (FileExist(section["logfile"]))
        {
            log.open(section["logfile"], std::ios_base::app);
        }
        else
        {
            log.open(section["logfile"]);
        }
        echo_server(section["port"], section["rootdir"], log, section["pidfile"], config);
    }
    else
    {
        echo_server(section["port"], section["rootdir"], cout, section["pidfile"], config);
    }
}

int main(int argc, char *argv[])
{
    process_options(argc, argv);
    return 0;
}

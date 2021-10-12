/* C++ standard include files first */
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

/* C system include files next */
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

/* C standard include files next */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

/* your own include last */
#include "my_socket.h"
#include "my_readwrite.h"
#include "utils.h"
#include "md5-calc.h"

static int master_socket_fd = (-1); /* there is nothing wrong with using a global variable */

static void usage()
{
    cerr << "wrong arguments" << endl;
    exit(-1);
}

/**
 * This is the function you need to change to change the behavior of your server!
 *
 * @param newsockfd - socket that can be used to "talk" (i.e., read/write) to the client.
 */
static int talk_to_client(int newsockfd, const string &rootDir, ostream &log)
{
    int bytes_received = 0;
    string line;
    string request;
    while (true)
    {
        string header;
        bytes_received = read_a_line(newsockfd, request);
        if (bytes_received < 2)
        {
            break;
        }
        header += ('\t' + request);
        while (true)
        {
            bytes_received = read_a_line(newsockfd, line);
            header += ('\t' + line);
            if (bytes_received < 2)
            {
                shutdown(newsockfd, SHUT_RDWR);
                close(newsockfd);
                return 1;
            }
            if (line.size() == 2)
                break;
        }
        auto params = SplitSanjay(request, ' ');
        if (params[0].compare("GET") || params[2].compare(0, 6, "HTTP/1"))
        {
            shutdown(newsockfd, SHUT_RDWR);
            close(newsockfd);
            return 1;
        }
        string contentType = "application/octet-stream";
        if (hasEnding(params[1], ".html"))
            contentType = "text/html";
        auto path = rootDir + "/" + params[1];
        const string notFound = "HTTP/1.1 404 Not Found\r\nServer: lab4a\r\nContent-Type: text / html\r\nContent-Length: 63\r\nContent-MD5: 5b7e68429c49c66e88489a80d9780025\r\n\r\n<html><head></head><body><h1>404 Not Found</h1></body></html>\r\n";
        auto size = get_file_size(path);
        log << GetTimeFormatted() << " REQUEST: " << get_ip_and_port_for_server(newsockfd, 0) << ", uri=" + params[1] << endl
            << header;
        log << GetTimeFormatted() << " RESPONSE: " << get_ip_and_port_for_server(newsockfd, 0) << ", status=";
        if (size < 1)
        {
            log << "404" << endl;
            better_write(newsockfd, notFound.c_str(), notFound.length());
            stringstream ss(notFound);
            while (getline(ss, line))
            {
                if (line.size() == 1)
                    break;
                log << '\t' << line << endl;
            }
        }
        else
        {
            log << "200" << endl;
            string foundHeader = "HTTP/1.1 200 OK\r\nServer: pa2 (davidz@usc.edu)\r\nContent-Type: " + contentType + "\r\nContent-Length:" + to_string(size) + "\r\nContent-MD5: " + GetMd5(path) +
                                 "\r\n\r\n";
            stringstream ss(foundHeader);
            while (getline(ss, line))
            {
                if (line.size() == 1)
                    break;
                log << '\t' << line << endl;
            }
            better_write(newsockfd, foundHeader.c_str(), foundHeader.length());
            char buf[1024];
            auto fd = open(path.c_str(), O_RDONLY);
            auto X = size;
            while (X > 0)
            {
                int bytes_to_read = (X > 1024) ? 1024 : X;
                int bytes_actually_read = read(fd, buf, bytes_to_read);
                if (bytes_actually_read <= 0)
                {
                    shutdown(newsockfd, SHUT_RDWR);
                    close(newsockfd);
                    return 0;
                }
                better_write(newsockfd, buf, bytes_actually_read);
                X -= bytes_actually_read;
                usleep(250000);
            }
        }
        log.flush();
    }
    log << GetTimeFormatted() << " CLOSE: " << get_ip_and_port_for_server(newsockfd, 0) << endl;
    shutdown(newsockfd, SHUT_RDWR);
    close(newsockfd);
    return 1;
}

/**
 * This is the function you need to change to change the behavior of your client!
 *
 * @param client_socket_fd - socket that can be used to "talk" (i.e., read/write) to the server.
 */
static void talk_to_user_and_server(int client_socket_fd, string uri, string outputFile, string host, string port)
{
    string request = "GET /" + uri + " HTTP/1.1\r\nUser-Agent: pa2 davidz@usc.edu\r\nAccept: */*\r\nHost: " + host + ":" + port + "\r\n\r\n";
    better_write(client_socket_fd, request.c_str(), request.size());
    string printOut = "\tGET " + uri + " HTTP/1.1\r\n \tUser-Agent: pa2 davidz@usc.edu\r\n \tAccept: */*\r\n \tHost: " + host + ":" + port + "\r\n \t\r\n";
    cerr << printOut;
    int bytes_received = 0;
    string line;
    string status;
    int size = 0;
    bytes_received = read_a_line(client_socket_fd, status);
    cerr << '\t' << status;
    while (true)
    {
        bytes_received = read_a_line(client_socket_fd, line);
        cerr << '\t' << line;
        auto keyValue = SplitSanjay(line, ':');
        if (!keyValue[0].compare("Content-Length"))
        {
            size = stoi(keyValue[1]);
        }
        if (line.size() == 2)
            break;
    }
    if (size == 0)
    {
        cout << "No Content-Length in response header when downloading " + uri + " from server at " + get_ip_and_port_for_client(client_socket_fd, 0) << "  Program terminated." << endl;
        exit(-1);
    }
    else
    {
        auto params = SplitSanjay(status, ' ');
        if (params.size() > 1)
        {
            char buf[1024];
            auto fd = open(outputFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
            auto X = size;
            float Avg = 0;
            int Bc = 0;
            auto start = chrono::steady_clock::now();
            float counter = 0.0f;
            while (X > 0)
            {
                int bytes_to_read = (X > 1024) ? 1024 : X;
                int bytes_actually_read = read(client_socket_fd, buf, bytes_to_read);
                if (bytes_actually_read <= 0)
                {
                    shutdown(client_socket_fd, SHUT_RDWR);
                    close(client_socket_fd);
                    return;
                }
                better_write(fd, buf, bytes_actually_read);
                X -= bytes_actually_read;
                Bc += bytes_actually_read;
                auto currentTime = chrono::steady_clock::now();
                std::chrono::duration<double> seconds = currentTime - start;
                auto count = seconds.count();
                start = currentTime;
                Avg = 0.15 * bytes_actually_read / count + 0.85 * Avg;
                counter += count;
                std::cerr << "\r\e[K" << std::flush;
                fprintf(stderr, "%dbytes of %dbytes (%.*f%%) downloaded at %.*fb/s (EWMA), time elapsed: %.*fs", Bc, size, 2, Bc * 100.0f / size, 2, Avg, 2, counter);
                cerr << flush;
            }
        }
        cout << endl
             << uri << " saved into " << outputFile << endl
             << endl;
    }
}

void echo_client(string host, string port, int argc, char *argv[])
{
    int client_socket_fd = create_client_socket_and_connect(host, port);
    if (client_socket_fd == (-1))
    {
        cerr << "Cannot connect to " << host << ":" << port << endl;
        exit(-1);
    }
    else
    {
        string client_ip_and_port = get_ip_and_port_for_client(client_socket_fd, 1);
        string server_ip_and_port = get_ip_and_port_for_client(client_socket_fd, 0);
        cerr << "pa2 client at " << client_ip_and_port << " is connected to server at " << server_ip_and_port << endl;
    }
    for (auto i = 0; i < argc; i += 2)
        talk_to_user_and_server(client_socket_fd, argv[i], argv[i + 1], host, port);
    shutdown(client_socket_fd, SHUT_RDWR);
    close(client_socket_fd);
}

void echo_server(string port, const string &rootDir, ostream &log, const string &pidFileName)
{
    master_socket_fd = create_master_socket(port);
    if (master_socket_fd != (-1))
    {
        log << GetTimeFormatted() << " START: "
            << "port=" << port << ", rootdir=\'" << rootDir + '\'' << endl;
        ofstream pidFile(pidFileName);
        pidFile << (int)getpid() << endl;
        while (1)
        {
            int newsockfd = my_accept(master_socket_fd);
            if (newsockfd != (-1))
            {
                log << GetTimeFormatted() << " CONNECT: " << get_ip_and_port_for_server(newsockfd, 0) << endl;
                if (!talk_to_client(newsockfd, rootDir, log))
                {
                    shutdown(master_socket_fd, SHUT_RDWR);
                    close(master_socket_fd);
                    break;
                }
            }
        }
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
    if (strcmp(argv[1], "-c"))
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
            echo_server(section["port"], section["rootdir"], log, section["pidfile"]);
        }
        else
        {
            echo_server(section["port"], section["rootdir"], cout, section["pidfile"]);
        }
    }
    else if (argc >= 6)
    {
        echo_client(argv[2], argv[3], argc - 4, argv + 4);
    }
    else
    {
        usage();
    }
}

int main(int argc, char *argv[])
{
    process_options(argc, argv);
    return 0;
}

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>
#include <csignal>

#include <cpprest/json.h>
#include <cpprest/http_client.h>

#ifdef __WIN32__
#include <winsock2.h>
#include <io.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#define CHUNK_SIZE 10240

using namespace web;
using namespace web::http;
using namespace web::http::client;

bool server_termination = false;
int socket_fd;


// Http request to main server POST /crossing.
//request - json: {"license_plate": "x", "id_parking_lot":x, "crossing_type": "entry"/"exit", "auth_token":"x"}
bool make_request(http_client& client, const json::value& json_body) {
    bool open = false;
    try {
        client.request(methods::POST, "crossing", json_body)
        .then([&open](pplx::task<http_response> responseTask)
        {
            http_response response = responseTask.get();
            return response.extract_json();
        })
        .then([&open](pplx::task<json::value> jsonTask)
        {
           try {
              json::value results = jsonTask.get();
              open = results.at("open").as_bool();
              std::string message = results.at("message").as_string();
              std::cout << "Open command: " << open << std::endl << "Message: " << message << std::endl << std::endl << std::endl;
           }
           catch (json::json_exception& e) {
              std::cout << e.what() << std::endl;
              open = false;
           }
        }).wait();                // waiting for task termination
    }
    catch (http_exception& e) {
        std::cout << e.what() << std::endl;
    }
    return open;
}


// Receive image from socket client, process license plate (alpr) and reply to client.
void receive_image(const int connection_socket, http_client& client, const struct sockaddr_in& client_address, const std::vector<json::object>& clients) {
    int total_received_size = 0, image_size = 0, write_size, read_size, packet_index = 1;
    char image_array[CHUNK_SIZE];
    std::string image_name;

    // client identity check
    std::vector<json::object>::const_iterator it;
    it = std::find_if(std::begin(clients), std::end(clients), [&client_address](const json::object& json_obj)
    {
        return (json_obj.at("ip").as_string() == std::string(inet_ntoa(client_address.sin_addr)) &&
                json_obj.at("port").as_string() == std::to_string(ntohs(client_address.sin_port)));
    });

    if (it == std::end(clients)) {
        std::cout << "Client not recognized. Closing socket connection..." << std::endl;
        return;
    }

    std::cout << "Established new connection with client: " << inet_ntoa(client_address.sin_addr) << " on port: "
          << ntohs(client_address.sin_port) << std::endl;

    image_name = std::to_string(it->at("id_parking_lot").as_number().to_int64()) + "_" + it->at("crossing_type").as_string();

    while (!server_termination) {

        // Get image_size of the image
        do {
            read_size = read(connection_socket, &image_size, sizeof(int));
        }
        while (read_size == -1);

        if (read_size == 0)       // other end (client) closed the socket
            break;

        std::cout << "Image size: " << image_size << " bytes" << std::endl;

        //Send verification signal
        do {
            write_size = write(connection_socket, "1", 2);
        }
        while (write_size == -1);

        std::ofstream image ("../plates/" + image_name + ".jpeg", std::ofstream::binary);
        if(!image.is_open()) {
            std::cout << "Error has occurred. Image file could not be opened" << std::endl;
            break;
        }

        //Loop while the entire file is not received
        while(total_received_size < image_size) {

            do {
                read_size = read(connection_socket,image_array, CHUNK_SIZE);
            }
            while(read_size == -1);

            std::cout << "Packet number received: " << packet_index << std::endl;
            std::cout << "Packet size: " << read_size << " bytes" << std::endl;

            write_size = image.tellp();   //current position

            try {
                image.write(image_array, read_size);
            }
            catch(std::ofstream::failure& e) {
                std::cout << e.what() << std::endl;
                break;
            }

            write_size = (int) image.tellp() - write_size;        // how many bytes have been written

            std::cout << "Written image size: " << (int) write_size << " bytes" << std::endl;

            if(read_size != write_size) {
                std::cout << "Error in image writing." << std::endl;
                break;
            }

            //Increment the total number of bytes read
            total_received_size += read_size;
            packet_index++;
            std::cout << "Total received image size: " << total_received_size << " bytes" << std::endl;
        }

        image.close();

        if (total_received_size < image_size) {
            std::cout << std::endl << "Error with the received image!" << std::endl << std::endl;
            continue;
        }

        std::cout << std::endl << "Image successfully received!" << std::endl << std::endl;

        // Image processing
        std::string str = "alpr -j ../plates/" + image_name + ".jpeg" + " > ../plates/" + image_name + ".json";
        const char *command = str.c_str();

        std::cout << "Processing license plate..." << std::endl;
        if(system(command) != 0) {
            std::cout << "Error in processing!" << std::endl;
            continue;
        }
        else {
            try {
                std::ifstream f("../plates/" + image_name + ".json");
                std::stringstream s;
                web::json::value v;

                if (f) {
                    s << f.rdbuf();
                    f.close();

                    v = web::json::value::parse(s);

                    std::string license_plate = v.at("results")[0]["plate"].as_string();
                    std::cout << "Processing completed!" << std::endl;
                    std::cout << "License_plate: " << license_plate << std::endl;

                    json::value json_body = json::value::object(true);
                    json_body["license_plate"] = json::value::string(U(license_plate));
                    json_body["id_parking_lot"] = json::value::number(it->at("id_parking_lot").as_number().to_int64());
                    json_body["crossing_type"] = json::value::string(it->at("crossing_type").as_string());
                    json_body["auth_token"] = json::value::string("UHJvY2Vzc2luZ1NlcnZlcg==");

                    bool open = make_request(client, json_body);

                    do {
                        write_size = write(connection_socket, open?"true":"false", 10);
                    }
                    while(write_size == -1);
                }
            }
            catch (web::json::json_exception& e) {
                std::cout << "Error parsing JSON: ";
                std::cout << e.what() << std::endl;
            }
        }
    }
    close(connection_socket);
}


// Read json configuration file containing clients allowed to connect to the processing server.
void read_json_clients(std::vector<json::object>& clients, std::string path) {
    try {
        std::ifstream f(path);
        std::stringstream s;
        web::json::value v;

        if (f) {
            s << f.rdbuf();
            f.close();

            v = web::json::value::parse(s);

            json::array json_clients = v.as_array();

            json::array::iterator it;
            for (it = json_clients.begin(); it != json_clients.end(); ++it) {
                clients.push_back(it->as_object());
            }
        }
    }
    catch (web::json::json_exception excep) {
        std::cout << "ERROR Parsing JSON: ";
        std::cout << excep.what();
    }
}


void termination_handling(int sig) {
    std::cout << std::endl << "Server is shutting down ..." << std::endl;
    server_termination = true;
    close(socket_fd);
}


int main(int argc, const char *argv[])
{
    int conn_socket_fd;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len;

    signal(SIGINT, termination_handling);

    // https config
    http_client_config client_config;

    client_config.set_ssl_context_callback(
    [&](boost::asio::ssl::context &ctx)
    {
        ctx.load_verify_file("../utility/upark_server.crt");
    });

    http_client client("https://localhost:50050/apis/", client_config);


    // read json configuration file
    std::vector<json::object> clients;
    read_json_clients(clients, "../utility/clients.json");


    // 1. Socket creation
    // socket(int domain, int type, int protocol)
    socket_fd =  socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        std::cout << "Error on socket opening!" << std::endl;
        exit(1);
    }

    // 2. Socket bind
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(50051);

    if (bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        std::cout << "Bind failed!" << std::endl;
        exit(1);
    }

    // 3. Socket listen
    if (listen(socket_fd, 10) == -1) {
        std::cout << "Listen failed!" << std::endl;
        exit(1);
    }

    std::cout << "Server is listening on address: " << inet_ntoa(server_address.sin_addr) << ":"
          << ntohs(server_address.sin_port) << std::endl;

    // 4. Socket accepts connections
    std::vector<std::tuple<std::thread, int>> sockets_opened;

    client_address_len = sizeof(client_address);

    while(!server_termination) {
        std::cout << std::endl << "Waiting for a new connection ..." << std::endl;

        conn_socket_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_address_len);
        if (conn_socket_fd == -1) {
            std::cout << std::endl << "Accept new connection failed!" << std::endl;
            continue;
        }
        sockets_opened.push_back(
            std::make_tuple(std::thread(receive_image, conn_socket_fd, std::ref(client), std::ref(client_address), std::ref(clients)),
            conn_socket_fd)
        );
    }

    //wait for threads termination
    std::cout << "Waiting for threads to finish..." << std::endl;
    for (std::tuple<std::thread, int>& socket : sockets_opened) {
        shutdown(std::get<1>(socket), SHUT_RDWR);
        std::get<0>(socket).join();
    }

    close(socket_fd);
    std::cout << "Bye!" << std::endl;
    return 0;
}

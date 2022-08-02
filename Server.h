#pragma once
#include <evhttp.h>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <iostream>
#include <atomic>
#include <future>
#include <array>
#include <thread>
#include <chrono>

/*
 * The class for handling HTTP requests with an image attached
 * The image that comes in the body of the request
 * is mirrored horizontally and sent back
 */
class Server {
public:
    /*
     * Constructs a new server and binds it on the specified port and address
     */
    Server(uint16_t new_port = 5875, const char* new_ip = "127.0.0.1");

    /*
     * The method starts the server
     */
    void processing_requests();

    /*
     * The method terminates the server
     */
    void turn_off();
private:
    std::unique_ptr<event_base, decltype(&event_base_free)> base;
    std::unique_ptr<evhttp, decltype(&evhttp_free)> http;
    std::atomic_bool is_working = true;
};

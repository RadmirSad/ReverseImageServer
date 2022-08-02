#include "Server.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

Server::Server(uint16_t new_port, const char* new_ip):
// Initializing an event base and an HTTP server
base(event_base_new(), &event_base_free), http(evhttp_new(base.get()), &evhttp_free) {
    if(!base)
        throw std::logic_error("Event base wasn't initialized");
    if(!http)
        throw std::logic_error("HTTP server wasn't initialized");
    std::cout << "New ip: " << new_ip << ", new port: " << new_port << std::endl;

    // Binding it on the specified port and ip
    if(evhttp_bind_socket(http.get(), new_ip, new_port))
        throw std::logic_error("Error with binding server socket");
}

void Server::processing_requests() {
    std::cout << "Listening..." << std::endl;
    auto action = [] (evhttp_request *req, void *)// image mirroring function
    {
        // open request's input buffer
        auto in_buf = evhttp_request_get_input_buffer(req);
        if(!in_buf)
            return;

        // make unique variable name
        std::string filename = "image" + std::to_string(reinterpret_cast<long>(req)) + ".jpg";
        FILE* inp_file = fopen(filename.c_str(), "wb");

        // save the captured image
        if(evbuffer_write(in_buf, fileno(inp_file)) == -1)
            std::cerr << "Zero bytes(" << std::endl;
        fclose(inp_file);

        // load jpeg as a pointer, save image parameters
        // an image looks like:
        // 0: [byte, byte, byte], 1: [byte, byte, byte], ..., width * height - 1: [byte, byte, byte]
        //    | <-components-> |
        int width = 0, height = 0, component_number = 0;
        std::unique_ptr<unsigned char, decltype(&stbi_image_free)> data(
                stbi_load(filename.c_str(),&width,
                          &height, &component_number, 0), &stbi_image_free);
        if(data) {
            // divide an image into four parts with corresponding pointers
            size_t delta = height / 4, real_width = width * component_number;
            delta *= real_width;
            std::array<unsigned char*, 5> steps{
                data.get(),
                data.get() + delta,
                data.get() + delta * 2,
                data.get() + delta * 3,
                data.get() + real_width * height
            };
            std::array<std::future<void>, 4> funcs;

            // reverse each part of an image
            for(size_t i = 0; i < 4; ++i)
                funcs[i] = std::async([&steps, i, component_number, real_width]{
                    // to reverse one row of an image two pixels should be swapped
                    // first_pixel: [byte, byte, byte]
                    //                 ^     ^     ^
                    //                 |     |     |
                    //                 v     v     v
                    // last_pixel:  [byte, byte, byte]
                    // etc.
                    for(size_t row = 0, row_cnt = (steps[i + 1] - steps[i]) / real_width; row < row_cnt; ++row)
                        for(unsigned char* left_pixel = row * real_width + steps[i],
                                * middle = left_pixel + real_width/ 2,
                                * right_pixel = left_pixel + real_width - component_number;
                        left_pixel < middle - component_number / 2;
                        left_pixel += component_number, right_pixel -= component_number)
                            for(int ind = 0; ind < component_number; ++ind)
                                std::swap(left_pixel[ind], right_pixel[ind]);
                });

            for(auto& func: funcs)
                func.get();

            // save a mirrored image
            stbi_write_jpg(filename.c_str(), width, height, component_number, data.get(), 100);
        }

        auto out_buf = evhttp_request_get_output_buffer(req);
        if (!out_buf)
            return;

        // open a result
        FILE* out_file = fopen(filename.c_str(), "rb");

        // write it into an output buffer
        evbuffer_add_file(out_buf, fileno(out_file), 0, -1);
        fclose(out_file);

        // delete an image and send reply
        remove(filename.c_str());
        evhttp_send_reply(req, HTTP_OK, "", out_buf);
    };

    // set a callback for all requests
    evhttp_set_gencb(http.get(), action, nullptr);

    // the server is waiting some requests while it works
    while(is_working) {
        event_base_loop(base.get(), EVLOOP_NONBLOCK);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void Server::turn_off() {
    is_working = false;
}
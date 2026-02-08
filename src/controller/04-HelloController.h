#ifndef HELLO_CONTROLLER
#define HELLO_CONTROLLER

#include "03-IHelloController.h"
#include <random>

/* @RestController */
/* @RequestMapping("/hello") */
class HelloController final : public IHelloController {
public:

    /* @PostMapping("/greet") */
    HelloResponseDto hello(/* @RequestBody */ HelloRequestDto /* request */) override {
        HelloResponseDto response;
        
        // Generate two random strings
        StdString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        Int length = 10;
        
        #ifdef ARDUINO
            // Arduino version using random()
            StdString str1, str2;
            for (Int i = 0; i < length; i++) {
                str1 += chars[random(0, chars.length())];
                str2 += chars[random(0, chars.length())];
            }
            response.randomString1 = str1;
            response.randomString2 = str2;
        #else
            // Desktop version using std::random
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, chars.length() - 1);
            StdString str1, str2;
            for (Int i = 0; i < length; i++) {
                str1 += chars[dis(gen)];
                str2 += chars[dis(gen)];
            }
            response.randomString1 = str1;
            response.randomString2 = str2;
        #endif
        
        return response;
    }
};

#endif


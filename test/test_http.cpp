//
// Created by liucxi on 2022/11/23.
//

#include "http/http.h"
#include <iostream>

using namespace luwu;

void test_request() {
    http::HttpRequest req;
    req.setMethod(http::HttpMethod::GET);
    req.setVersion(0x11);
    req.setPath("/search");
    req.setQuery("q=url+%E5%8F%82%E6%95%B0%E6%9E%84%E9%80%A0&aqs=chrome..69i57.8307j0j7&ie=UTF-8");
    req.setHeader("Accept", "text/plain");
    req.setHeader("Content-Type", "application/x-www-form-urlencoded");
    req.setHeader("Cookie", "yummy_cookie=choco;tasty_cookie=strawberry");
    req.setBody("title=test&sub%5B1%5D=1&sub%5B2%5D=2"); // title=test&sub[1]=1&sub[2]=2

    std::cout << req.toString() << std::endl;
    std::cout << std::endl;
    std::cout << req.getParam("q") << std::endl;
    std::cout << req.getParam("aqs") << std::endl;
    std::cout << req.getParam("ie") << std::endl;
    std::cout << req.getParam("title") << std::endl;
    std::cout << req.getParam("sub[1]") << std::endl;
    std::cout << req.getParam("sub[2]") << std::endl;
    std::cout << req.getHeader("Accept") << std::endl;
    std::cout << req.getHeader("Content-Type") << std::endl;
    std::cout << req.getCookie("yummy_cookie") << std::endl;
    std::cout << req.getCookie("tasty_cookie") << std::endl;
}

void test_response() {
    http::HttpResponse rsp;
    rsp.setVersion(0x11);
    rsp.setStatus(http::HttpStatus::OK);
    rsp.setHeader("Content-Type", "text/html");
    rsp.setBody("<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<title>hello world</title>"
                "</head>"
                "<body><p>hello world</p></body>"
                "</html>");
    rsp.setCookie("cookie1", "value1", 0, "/");
    rsp.dump(std::cout);
}

int main() {
    test_request();
    std::cout << "====================" << std::endl;
    test_response();
    return 0;
}

//
// Created by liucxi on 2022/11/24.
//

#include "servlet.h"
#include <fnmatch.h>
#include <utility>
#include <iostream>

namespace luwu {
    namespace http {
        // region # html and css
        static const std::string not_found_page = "<!DOCTYPE html>\n" /* NOLINT */
                                                  "<html>\n"
                                                  "<head>\n"
                                                  "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
                                                  "    <title>404</title>\n"
                                                  "\n"
                                                  "    <style>\n"
                                                  "        html, body {\n"
                                                  "            height: 100%;\n"
                                                  "            min-height: 450px;\n"
                                                  "            font-size: 32px;\n"
                                                  "            font-weight: 500;\n"
                                                  "            color: #5d7399;\n"
                                                  "            margin: 0;\n"
                                                  "            padding: 0;\n"
                                                  "            border: 0;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .content {\n"
                                                  "            height: 100%;\n"
                                                  "            position: relative;\n"
                                                  "            z-index: 1;\n"
                                                  "            background-color: #d2e1ec;\n"
                                                  "            background-image: -webkit-linear-gradient(top, #bbcfe1 0%, #e8f2f6 80%);\n"
                                                  "            background-image: linear-gradient(to bottom, #bbcfe1 0%, #e8f2f6 80%);\n"
                                                  "            overflow: hidden;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .snow {\n"
                                                  "            position: absolute;\n"
                                                  "            top: 0;\n"
                                                  "            left: 0;\n"
                                                  "            pointer-events: none;\n"
                                                  "            z-index: 20;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .main-text {\n"
                                                  "            padding: 20vh 20px 0 20px;\n"
                                                  "            text-align: center;\n"
                                                  "            line-height: 2em;\n"
                                                  "            font-size: 5vh;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .main-text h1 {\n"
                                                  "            font-size: 45px;\n"
                                                  "            line-height: 48px;\n"
                                                  "            margin: 0;\n"
                                                  "            padding: 0;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .main-text-a {\n"
                                                  "            height: 32px;\n"
                                                  "            margin-left: auto;\n"
                                                  "            margin-right: auto;\n"
                                                  "            text-align: center;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .main-text-a a {\n"
                                                  "            font-size: 16px;\n"
                                                  "            text-decoration: none;\n"
                                                  "            color: #0066CC;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .main-text-a a:hover {\n"
                                                  "            color: #000;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .home-link {\n"
                                                  "            font-size: 0.6em;\n"
                                                  "            font-weight: 400;\n"
                                                  "            color: inherit;\n"
                                                  "            text-decoration: none;\n"
                                                  "            opacity: 0.6;\n"
                                                  "            border-bottom: 1px dashed rgba(93, 115, 153, 0.5);\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .home-link:hover {\n"
                                                  "            opacity: 1;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .ground {\n"
                                                  "            height: 160px;\n"
                                                  "            width: 100%;\n"
                                                  "            position: absolute;\n"
                                                  "            bottom: 0;\n"
                                                  "            left: 0;\n"
                                                  "            background: #f6f9fa;\n"
                                                  "            box-shadow: 0 0 10px 10px #f6f9fa;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .ground:before, .ground:after {\n"
                                                  "            content: '';\n"
                                                  "            display: block;\n"
                                                  "            width: 250px;\n"
                                                  "            height: 250px;\n"
                                                  "            position: absolute;\n"
                                                  "            top: -62.5px;\n"
                                                  "            z-index: -1;\n"
                                                  "            background: transparent;\n"
                                                  "            -webkit-transform: scaleX(0.2) rotate(45deg);\n"
                                                  "            transform: scaleX(0.2) rotate(45deg);\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .ground:after {\n"
                                                  "            left: 50%;\n"
                                                  "            margin-left: -166.66667px;\n"
                                                  "            box-shadow: -340px 260px 15px #8193b2, -620px 580px 15px #8193b2, -900px 900px 15px #b0bccf, -1155px 1245px 15px #b4bed1, -1515px 1485px 15px #8193b2, -1755px 1845px 15px #8a9bb8, -2050px 2150px 15px #91a1bc, -2425px 2375px 15px #bac4d5, -2695px 2705px 15px #a1aec6, -3020px 2980px 15px #8193b2, -3315px 3285px 15px #94a3be, -3555px 3645px 15px #9aa9c2, -3910px 3890px 15px #b0bccf, -4180px 4220px 15px #bac4d5, -4535px 4465px 15px #a7b4c9, -4840px 4760px 15px #94a3be;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .ground:before {\n"
                                                  "            right: 50%;\n"
                                                  "            margin-right: -166.66667px;\n"
                                                  "            box-shadow: 325px -275px 15px #b4bed1, 620px -580px 15px #adb9cd, 925px -875px 15px #a1aec6, 1220px -1180px 15px #b7c1d3, 1545px -1455px 15px #7e90b0, 1795px -1805px 15px #b0bccf, 2080px -2120px 15px #b7c1d3, 2395px -2405px 15px #8e9eba, 2730px -2670px 15px #b7c1d3, 2995px -3005px 15px #9dabc4, 3285px -3315px 15px #a1aec6, 3620px -3580px 15px #8193b2, 3880px -3920px 15px #aab6cb, 4225px -4175px 15px #9dabc4, 4510px -4490px 15px #8e9eba, 4785px -4815px 15px #a7b4c9;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .mound {\n"
                                                  "            margin-top: -80px;\n"
                                                  "            font-weight: 800;\n"
                                                  "            font-size: 180px;\n"
                                                  "            text-align: center;\n"
                                                  "            color: #dd4040;\n"
                                                  "            pointer-events: none;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .mound:before {\n"
                                                  "            content: '';\n"
                                                  "            display: block;\n"
                                                  "            width: 600px;\n"
                                                  "            height: 200px;\n"
                                                  "            position: absolute;\n"
                                                  "            left: 50%;\n"
                                                  "            margin-left: -300px;\n"
                                                  "            top: 50px;\n"
                                                  "            z-index: 1;\n"
                                                  "            border-radius: 100%;\n"
                                                  "            background-color: #e8f2f6;\n"
                                                  "            background-image: -webkit-linear-gradient(top, #dee8f1, #f6f9fa 60px);\n"
                                                  "            background-image: linear-gradient(to bottom, #dee8f1, #f6f9fa 60px);\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .mound:after {\n"
                                                  "            content: '';\n"
                                                  "            display: block;\n"
                                                  "            width: 28px;\n"
                                                  "            height: 6px;\n"
                                                  "            position: absolute;\n"
                                                  "            left: 50%;\n"
                                                  "            margin-left: -150px;\n"
                                                  "            top: 68px;\n"
                                                  "            z-index: 2;\n"
                                                  "            background: #dd4040;\n"
                                                  "            border-radius: 100%;\n"
                                                  "            -webkit-transform: rotate(-15deg);\n"
                                                  "            transform: rotate(-15deg);\n"
                                                  "            box-shadow: -56px 12px 0 1px #dd4040, -126px 6px 0 2px #dd4040, -196px 24px 0 3px #dd4040;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .mound_text {\n"
                                                  "            -webkit-transform: rotate(6deg);\n"
                                                  "            transform: rotate(6deg);\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .mound_spade {\n"
                                                  "            display: block;\n"
                                                  "            width: 35px;\n"
                                                  "            height: 30px;\n"
                                                  "            position: absolute;\n"
                                                  "            right: 50%;\n"
                                                  "            top: 42%;\n"
                                                  "            margin-right: -250px;\n"
                                                  "            z-index: 0;\n"
                                                  "            -webkit-transform: rotate(35deg);\n"
                                                  "            transform: rotate(35deg);\n"
                                                  "            background: #dd4040;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .mound_spade:before, .mound_spade:after {\n"
                                                  "            content: '';\n"
                                                  "            display: block;\n"
                                                  "            position: absolute;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .mound_spade:before {\n"
                                                  "            width: 40%;\n"
                                                  "            height: 30px;\n"
                                                  "            bottom: 98%;\n"
                                                  "            left: 50%;\n"
                                                  "            margin-left: -20%;\n"
                                                  "            background: #dd4040;\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        .mound_spade:after {\n"
                                                  "            width: 100%;\n"
                                                  "            height: 30px;\n"
                                                  "            top: -55px;\n"
                                                  "            left: 0%;\n"
                                                  "            box-sizing: border-box;\n"
                                                  "            border: 10px solid #dd4040;\n"
                                                  "            border-radius: 4px 4px 20px 20px;\n"
                                                  "        }\n"
                                                  "    </style>\n"
                                                  "\n"
                                                  "</head>\n"
                                                  "\n"
                                                  "<body translate=\"no\">\n"
                                                  "<div class=\"content\">\n"
                                                  "    <canvas class=\"snow\" id=\"snow\" width=\"1349\" height=\"400\"></canvas>\n"
                                                  "    <div class=\"ground\">\n"
                                                  "        <div class=\"mound\">\n"
                                                  "            <div class=\"mound_text\">404</div>\n"
                                                  "            <div class=\"mound_spade\"></div>\n"
                                                  "        </div>\n"
                                                  "    </div>\n"
                                                  "</div>\n"
                                                  "\n"
                                                  "\n"
                                                  "<script>\n"
                                                  "    (function () {\n"
                                                  "        function ready(fn) {\n"
                                                  "            if (document.readyState != 'loading') {\n"
                                                  "                fn();\n"
                                                  "            } else {\n"
                                                  "                document.addEventListener('DOMContentLoaded', fn);\n"
                                                  "            }\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        function makeSnow(el) {\n"
                                                  "            var ctx = el.getContext('2d');\n"
                                                  "            var width = 0;\n"
                                                  "            var height = 0;\n"
                                                  "            var particles = [];\n"
                                                  "\n"
                                                  "            var Particle = function () {\n"
                                                  "                this.x = this.y = this.dx = this.dy = 0;\n"
                                                  "                this.reset();\n"
                                                  "            }\n"
                                                  "\n"
                                                  "            Particle.prototype.reset = function () {\n"
                                                  "                this.y = Math.random() * height;\n"
                                                  "                this.x = Math.random() * width;\n"
                                                  "                this.dx = (Math.random() * 1) - 0.5;\n"
                                                  "                this.dy = (Math.random() * 0.5) + 0.5;\n"
                                                  "            }\n"
                                                  "\n"
                                                  "            function createParticles(count) {\n"
                                                  "                if (count != particles.length) {\n"
                                                  "                    particles = [];\n"
                                                  "                    for (var i = 0; i < count; i++) {\n"
                                                  "                        particles.push(new Particle());\n"
                                                  "                    }\n"
                                                  "                }\n"
                                                  "            }\n"
                                                  "\n"
                                                  "            function onResize() {\n"
                                                  "                width = window.innerWidth;\n"
                                                  "                height = window.innerHeight;\n"
                                                  "                el.width = width;\n"
                                                  "                el.height = height;\n"
                                                  "\n"
                                                  "                createParticles((width * height) / 10000);\n"
                                                  "            }\n"
                                                  "\n"
                                                  "            function updateParticles() {\n"
                                                  "                ctx.clearRect(0, 0, width, height);\n"
                                                  "                ctx.fillStyle = '#f6f9fa';\n"
                                                  "\n"
                                                  "                particles.forEach(function (particle) {\n"
                                                  "                    particle.y += particle.dy;\n"
                                                  "                    particle.x += particle.dx;\n"
                                                  "\n"
                                                  "                    if (particle.y > height) {\n"
                                                  "                        particle.y = 0;\n"
                                                  "                    }\n"
                                                  "\n"
                                                  "                    if (particle.x > width) {\n"
                                                  "                        particle.reset();\n"
                                                  "                        particle.y = 0;\n"
                                                  "                    }\n"
                                                  "\n"
                                                  "                    ctx.beginPath();\n"
                                                  "                    ctx.arc(particle.x, particle.y, 5, 0, Math.PI * 2, false);\n"
                                                  "                    ctx.fill();\n"
                                                  "                });\n"
                                                  "\n"
                                                  "                window.requestAnimationFrame(updateParticles);\n"
                                                  "            }\n"
                                                  "\n"
                                                  "            onResize();\n"
                                                  "            updateParticles();\n"
                                                  "        }\n"
                                                  "\n"
                                                  "        ready(function () {\n"
                                                  "            var canvas = document.getElementById('snow');\n"
                                                  "            makeSnow(canvas);\n"
                                                  "        });\n"
                                                  "    })();\n"
                                                  "</script>\n"
                                                  "\n"
                                                  "</body>\n"
                                                  "</html>";
        // endregion

        HttpServlet::HttpServlet(std::string name, handle_func func)
            : ServletBase(std::move(name)), func_(std::move(func)){
        }

        int HttpServlet::handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session) {
            if (func_) {
                return func_(req, rsp, session);
            }
            return 0;
        }

        int ServletNotFound::handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session) {
            rsp->setStatus(HttpStatus::NOT_FOUND);
            rsp->setHeader("Content-Type", "text/html");
            rsp->setBody(not_found_page);
            return 0;
        }

        WSServlet::WSServlet(std::string name, handle_func handel, callback on_connect, callback on_close)
                : ServletBase(std::move(name)), handel_(std::move(handel))
                , on_connect_(std::move(on_connect)), on_close_(std::move(on_close)) {
        }

        int WSServlet::onConnect(HttpRequest::ptr req, WSConnection::ptr conn) {
            if (on_connect_) {
                return on_connect_(req, conn);
            }
            return 0;
        }

        int WSServlet::onClose(HttpRequest::ptr req, WSConnection::ptr conn) {
            if (on_close_) {
                return on_close_(req, conn);
            }
            return 0;
        }

        int WSServlet::handle(HttpRequest::ptr req, WSFrameMessage::ptr msg, WSConnection::ptr conn) {
            if (handel_) {
                return handel_(req, msg, conn);
            }
            return 0;
        }

        ServletDispatch::ServletDispatch(std::string name)
            : name_(std::move(name)), default_(new ServletNotFound){
        }

        int ServletDispatch::handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session) {
            auto servlet = getMatchedServlet(req->getPath());
            if (servlet) {
                return std::dynamic_pointer_cast<HttpServlet>(servlet)->handle(req, rsp, session);
            } else {
                return getDefaultServlet()->handle(req, rsp, session);
            }
        }

        int ServletDispatch::handle(HttpRequest::ptr req, WSFrameMessage::ptr rsp, WSConnection::ptr session) {
            auto servlet = getMatchedServlet(req->getPath());
            if (servlet) {
                return std::dynamic_pointer_cast<WSServlet>(servlet)->handle(req, rsp, session);
            }
            return -1;
        }

        ServletBase::ptr ServletDispatch::getExactServlet(const std::string &uri) {
            RWMutex::ReadLock lock(mutex_);
            auto it = exact_.find(uri);
            return it == exact_.end() ? nullptr : it->second;
        }

        ServletBase::ptr ServletDispatch::getFuzzyServlet(const std::string &uri) {
            RWMutex::ReadLock lock(mutex_);
            for (auto &f : fuzzy_) {
                if (!fnmatch(f.first.c_str(), uri.c_str(), 0)) {
                    return f.second;
                }
            }
            return nullptr;
        }

        ServletBase::ptr ServletDispatch::getMatchedServlet(const std::string &uri) {
            auto servlet1 = getExactServlet(uri);
            if (servlet1) {
                return servlet1;
            }
            auto servlet2 = getFuzzyServlet(uri);
            if (servlet2) {
                return servlet2;
            }
            return nullptr;
        }

        void ServletDispatch::addExactServlet(const std::string &uri, ServletBase::ptr servlet) {
            RWMutex::WriteLock lock(mutex_);
            exact_[uri] = std::move(servlet);
        }

        void ServletDispatch::addExactHttpServlet(const std::string &uri, HttpServlet::handle_func func) {
            addExactServlet(uri, std::make_shared<HttpServlet>(uri, std::move(func)));
        }

        void ServletDispatch::addExactWSServlet(const std::string &uri, WSServlet::handle_func func,
                                                WSServlet::callback on_connect, WSServlet::callback on_close) {
            addExactServlet(uri, std::make_shared<WSServlet>(uri, std::move(func),
                                 std::move(on_connect), std::move(on_close)));
        }

        void ServletDispatch::addFuzzyServlet(const std::string &uri, ServletBase::ptr servlet) {
            delFuzzyServlet(uri);
            RWMutex::WriteLock lock(mutex_);
            fuzzy_.emplace_back(uri, std::move(servlet));
        }

        void ServletDispatch::addFuzzyHttpServlet(const std::string &uri, HttpServlet::handle_func func) {
            addFuzzyServlet(uri, std::make_shared<HttpServlet>(uri, std::move(func)));
        }

        void ServletDispatch::addFuzzyWSServlet(const std::string &uri, WSServlet::handle_func func,
                                                WSServlet::callback on_connect, WSServlet::callback on_close) {
            addFuzzyServlet(uri, std::make_shared<WSServlet>(uri, std::move(func),
                                 std::move(on_connect), std::move(on_close)));
        }

        void ServletDispatch::delExactServlet(const std::string &uri) {
            RWMutex::WriteLock lock(mutex_);
            exact_.erase(uri);
        }

        void ServletDispatch::delFuzzyServlet(const std::string &uri) {
            RWMutex::WriteLock lock(mutex_);
            for (auto it = fuzzy_.begin(); it != fuzzy_.end(); ++it) {
                if (it->first == uri) {
                    fuzzy_.erase(it);
                    break;
                }
            }
        }
    }
}

<h1 align="center">Welcome to luwu 👋</h1>
<p>
  <img alt="version:1.0" src="https://img.shields.io/badge/version-1.0.0-blue" />
  <img alt="license:MIT" src="https://img.shields.io/badge/license-MIT-brightgreen" />
</p>

> a high-performance server framework

### 🏠 [Homepage](https://github.com/liucxi04/luwu1.0)

## Install

```sh
git clone git@github.com:liucxi04/luwu1.0.git
cd luwu1.0
sh autobuild.sh
```

## Usage
you can use this framework like this:
```cpp
// for log
#include <logger.h>
// for fiber
#inlcude <fiber.h>
```

## Note
the server system is Ubuntu 20.04, using Ubuntu 22.04's default configuration can be a bit problematic.

autobuild.sh should use bash, you can use `sudo dpkg-reconfigure dash` change from dash to bash.

## Logs
| module name     | content                                   | start date | estimated time | actual time | end date   |
| --------------- | ----------------------------------------- | ---------- | -------------- | ----------- | ---------- |
| ready           | build a project                           | 2022.10.31 | 1              | 1           | 2022.10.31 |
| logger          | log4cpp、sylar                            | 2022.11.01 | 3              | 3           | 2022.11.03 |
| fiber           | basic、sylar                              | 2022.11.04 | 3              | 2           | 2022.11.05 |
| thread          | basic                                     | 2022.11.06 | 2              | 1           | 2022.11.06 |
| fiber scheduler | ucontext_t、sylar                         | 2022.11.07 | 3              | 3           | 2022.11.09 |
| clock           | timer、sylar                              | 2022.11.10 | 2              | 2           | 2022.11.11 |
| reactor         | channel、epoll、reactor                   | 2022.11.14 | 3              | 2           | 2022.11.15 |
| net basic       | fd、hook、address、socket                 | 2022.11.16 | 3              | 4           | 2022.11.19 |
| byte array      | muduo、encode                             | 2022.11.20 | 2              | 1           | 2022.11.20 |
| tcp server      | basic                                     | 2022.11.21 | 1              | 1           | 2022.11.21 |
| http            | http、parser、servlet、connection、server | 2022.11.22 | 4              | 4           | 2022.11.25 |
| websocket       | servlet、connection、server               | 2022.11.28 | 3              | 3           | 2022.11.30 |

## Author
* Website: [liucxi](http://blog.liucxi.xyz)
* Github: [@liucxi04](https://github.com/liucxi04)

## Show your support

Give a ⭐️ if this project helped you!

## 📝 License

Copyright © 2022 [liucxi](https://github.com/liucxi04).<br />
This project is [MIT](https://github.com/liucxi04/luwu1.0/blob/main/LICENSE) licensed.

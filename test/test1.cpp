#include <iostream>
#include <thread>
#include <sstream>
#include <memory>

void thread_work1(std::string str) {
    std::cout << "str is " << str << std::endl;
}

class background_task {
public:
    void operator()() {
        std::cout << "background_task called" << std::endl;
    }
};

struct func {
    int& _i;
    func(int & i): _i(i){}
    void operator()() {
        for (int i = 0; i < 3; i++) {
            _i = i;
            std::cout << "_i is " << _i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};
void oops() {
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread(myfunc);
    functhread.detach();
}


void use_join() {
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread((myfunc));
    functhread.join();
}

void catch_exception() {
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread{myfunc};
    try {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch(std::exception& e) {
        functhread.join();
        throw;
    }

    functhread.join();
}

class thread_guard {
private:
    std::thread& _t;
public:
    explicit thread_guard(std::thread& t) : _t(t) {}
    ~thread_guard() {
        if(_t.joinable()) {
            _t.join();
        }
    }

    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};

void auto_guard() {
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);

    std::cout << "auto guard finished" << std::endl;
}

int main() {
//    std::string hellostr = "hello world!";
//    std::thread t1(thread_work1, hellostr);
//    t1.join();
//
//    std::thread t2((background_task()));
//    t2.join();
//    std::thread t3{background_task()};
//    t3.join();
//
//    std::thread t4([](std::string str) {
//        std::cout << "str is " << str << std::endl;
//    }, hellostr);
//    t4.join();
//
//    oops();
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//    use_join();
//
//    catch_exception();
    auto_guard();

    return 0;
}

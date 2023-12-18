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

void print_str(int i, std::string const& s) {
    std::cout << "i is" << i << " str is " << s << std::endl;
}

void danger_oops(int som_param) {
    char buffer[1024];
    sprintf(buffer, "%i", som_param);
    std::thread t(print_str, 3, buffer);
    t.detach();
    std::cout << "danger oops finished " << std::endl;
}

void safe_oops(int some_param) {
    char buffer[1024];
    sprintf(buffer, "%i", some_param);
    std::thread t(print_str, 3, std::string(buffer));
    t.detach();
}

void change_param(int& param) {
    param++;
}

void ref_oops(int some_param) {
    std::cout << "before change, param is " << some_param << std::endl;
    std::thread t2(change_param, std::ref(some_param));
    t2.join();
    std::cout << "after change, param is " << some_param << std::endl;
}

class X {
public:
    void do_lengthy_work() {
        std::cout << "do_lengthy_work " << std::endl;
    }
};

void bind_class_oops() {
    X my_x;
    std::thread t(&X::do_lengthy_work, &my_x);
    t.join();
}

void deal_unique(std::unique_ptr<int> p) {
    std::cout << "unique ptr data is " << *p << std::endl;
    (*p)++;

    std::cout << "after unique ptr data is " << *p << std::endl;
}

void move_oops() {
    auto p = std::make_unique<int>(100);
    std::thread t(deal_unique, std::move(p));
    t.join();
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
//    auto_guard();

//    safe_oops(3);
//    std::this_thread::sleep_for(std::chrono::seconds(1));

    ref_oops(100);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}

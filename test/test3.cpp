#include <iostream>
#include <mutex>
#include <thread>
#include <stack>

std::mutex mtx1;
int shared_data = 100;

void use_lock() {
    while(true) {
        mtx1.lock();
        ++shared_data;
        std::cout << "current thread is " << std::this_thread::get_id() << std::endl;
        std::cout << "shared data is " << shared_data << std::endl;
        mtx1.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

template<typename T>
class threadsafe_stack1 {
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack1() {}
    threadsafe_stack1(const threadsafe_stack1& other) {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack1& operator=(const threadsafe_stack1&) = delete;

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    // 问题代码
    T pop() {
        std::lock_guard<std::mutex> lock(m);
        auto element = data.top();
        data.pop();
        return element;
    }

    // 危险
    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

void test_threadsafe_stack1() {
    threadsafe_stack1<int> safe_stack;
    safe_stack.push(1);

    std::thread t1([&safe_stack](){
        if(!safe_stack.empty()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            safe_stack.pop();
        }
    });

    std::thread t2([&safe_stack]() {
        if(!safe_stack.empty()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            safe_stack.pop();
        }
    });

    t1.join();
    t2.join();
}

struct empty_stack : std::exception {
    const char* what() const throw();
};

template<typename T>
class threadsafe_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
        //if(data.empty()) throw empty_stack();
        if(data.empty()) return nullptr;
        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }
    void pop(T& value) {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        value = data.top();
        data.pop();
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

void test_lock() {
    std::thread t1(use_lock);

    std::thread t2([]() {
        while(true) {
            mtx1.lock();
            --shared_data;
            std::cout << "current thread is " << std::this_thread::get_id() << std::endl;
            std::cout << "shared data is " << shared_data << std::endl;
            mtx1.unlock();
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });

    std::thread t3([](){
        while(true) {
            {
                std::lock_guard<std::mutex> lk_guard(mtx1);
                --shared_data;
                std::cout << "current thread is " << std::this_thread::get_id() << std::endl;
                std::cout << "shared data is " << shared_data << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });

    t1.join();
    t2.join();
    t3.join();
};

std::mutex t_lock1;
std::mutex t_lock2;
int m_1 = 0;
int m_2 = 1;

void dead_lock1() {
    while(true) {
        std::cout << "dead_lock1 begin " << std::endl;
        t_lock1.lock();
        m_1 = 1024;
        t_lock2.lock();
        m_2 = 2048;
        t_lock2.unlock();
        t_lock1.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::cout << "dead_lock2 end " << std::endl;
    }
};

void dead_lock2() {
    while(true) {
        std::cout << "dead_lock2 begin " << std::endl;
        t_lock2.lock();
        m_2 = 2048;
        t_lock1.lock();
        m_1 = 1024;
        t_lock1.unlock();
        t_lock2.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::cout << "dead_lock2 end " << std::endl;
    }
};

void atomic_lock1() {
    std::cout << "lock1 begin lock" << std::endl;
    t_lock1.lock();
    m_1 = 1024;
    t_lock1.unlock();
    std::cout << "lock1 end lock" << std::endl;
}

void atomic_lock2() {
    std::cout << "lock2 begin lock" << std::endl;
    t_lock2.lock();
    m_2 = 2048;
    t_lock2.unlock();
    std::cout << "lock2 end lock" << std::endl;
}

void safe_lock1() {
    while(true) {
        atomic_lock1();
        atomic_lock2();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void safe_lock2() {
    while(true) {
        atomic_lock2();
        atomic_lock1();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void test_dead_lock() {
    std::thread t1(dead_lock1);
    std::thread t2(dead_lock2);
    t1.join();
    t2.join();
}

void test_safe_lock() {
    std::thread t1(safe_lock1);
    std::thread t2(safe_lock2);
    t1.join();
    t2.join();
}

class som_big_object {
private:
    int _data;
public:
    som_big_object(int data) : _data(data) {}
    som_big_object(const som_big_object& b2) : _data(b2._data) {
        //_data = b2._data;
    }
    som_big_object(som_big_object&& b2) : _data(std::move(b2._data)) {}

    friend std::ostream& operator << (std::ostream& os, const som_big_object& big_obj) {
        os << big_obj._data;
        return os;
    }

    som_big_object& operator=(const som_big_object& b2) {
        if(this == &b2) {
            return *this;
        }
        _data = b2._data;
        return *this;
    }

    friend void swap(som_big_object& b1, som_big_object& b2) {
        som_big_object temp = std::move(b1);
        b1 = std::move(b2);
        b2 = std::move(temp);
    }
};

class big_object_mgr {
private:
    std::mutex _mtx;
    som_big_object _obj;
public:
    big_object_mgr(int data = 0) : _obj(data) {}
    void printinfo() {
        std::cout << "current obj data is " << _obj << std::endl;
    }
    friend void danger_swap(big_object_mgr& objm1, big_object_mgr& objm2);
    friend void safe_swap(big_object_mgr& objm1, big_object_mgr& objm2);
    friend void safe_swap_scope(big_object_mgr& objm1, big_object_mgr& objm2);
};

void danger_swap(big_object_mgr& objm1, big_object_mgr& objm2) {
    std::cout << "thread [ " << std::this_thread::get_id() << " ] begin" << std::endl;
    if(&objm1 == &objm2) {
        return;
    }
    std::lock_guard<std::mutex> guard1(objm1._mtx);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::lock_guard<std::mutex> guard2(objm2._mtx);
    swap(objm1._obj, objm2._obj);
    std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
}

void test_danger_swap() {
    big_object_mgr objm1(5);
    big_object_mgr objm2(100);

    std::thread t1(danger_swap, std::ref(objm1), std::ref(objm2));
    std::thread t2(danger_swap, std::ref(objm2), std::ref(objm1));
    t1.join();
    t2.join();

    objm1.printinfo();
    objm2.printinfo();
}

void safe_swap(big_object_mgr& objm1, big_object_mgr& objm2) {
    std::cout <<"thread [ " << std::this_thread::get_id() << " ] begin" << std::endl;
    if(&objm1 == &objm2) {
        return;
    }
    std::lock(objm1._mtx, objm2._mtx);
    std::lock_guard<std::mutex> guard1(objm1._mtx, std::adopt_lock);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::lock_guard<std::mutex> guard2(objm2._mtx, std::adopt_lock);

    swap(objm1._obj, objm2._obj);
    std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
}

void safe_swap_scope(big_object_mgr& objm1, big_object_mgr& objm2) {
    std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
    if(&objm1 == &objm2) {
        return;
    }
    std::scoped_lock guard(objm1._mtx, objm2._mtx);
    swap(objm1._obj, objm2._obj);
    std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
}

void test_safe_swap() {
    big_object_mgr objm1(5);
    big_object_mgr objm2(100);

    std::thread t1(safe_swap, std::ref(objm1), std::ref(objm2));
    std::thread t2(safe_swap, std::ref(objm2), std::ref(objm1));
    t1.join();
    t2.join();

    objm1.printinfo();
    objm2.printinfo();
}

class hierarchical_mutex {
public:
    explicit hierarchical_mutex(unsigned long value) : _hierarchy_value(value), _previous_hierarchy_value(0) {}
    hierarchical_mutex(const hierarchical_mutex&) = delete;
    hierarchical_mutex& operator=(const hierarchical_mutex&) = delete;
    void lock() {
        check_for_hierarchy_violation();
        _internal_mutex.lock();
        update_hierarchy_value();
    }

    void unlock() {
        if(_this_thread_hierarchy_value != _hierarchy_value) {
            throw std::logic_error("mutex hierarchy violated");
        }
        _this_thread_hierarchy_value = _previous_hierarchy_value;
        _internal_mutex.unlock();
    }

    bool try_look() {
        check_for_hierarchy_violation();
        if(!_internal_mutex.try_lock()) {
            return false;
        }
        update_hierarchy_value();
        return true;
    }
private:
    std::mutex _internal_mutex;
    unsigned long const _hierarchy_value;
    unsigned long _previous_hierarchy_value;
    static thread_local unsigned long _this_thread_hierarchy_value;

    void check_for_hierarchy_violation() {
        if(_this_thread_hierarchy_value <= _hierarchy_value) {
            throw std::logic_error("mutex hierarchy violated");
        }
    }

    void update_hierarchy_value() {
        _previous_hierarchy_value = _this_thread_hierarchy_value;
        _this_thread_hierarchy_value = _hierarchy_value;
    }
};

thread_local unsigned long hierarchical_mutex::_this_thread_hierarchy_value(ULONG_MAX);

void test_hierarchy_lock() {
    hierarchical_mutex hmtx1(1000);
    hierarchical_mutex hmtx2(500);
    std::thread t1([&hmtx1, &hmtx2]() {
        hmtx1.lock();
        hmtx2.lock();
        hmtx2.unlock();
        hmtx1.unlock();
    });
}

int main() {
    test_lock();
    //test_threadsafe_stack1();
}
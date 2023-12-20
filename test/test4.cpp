#include <iostream>
#include <mutex>
#include <map>
#include <shared_mutex>

std::mutex mtx;
int shared_data = 0;

void use_unique() {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "lock success" << std::endl;
    ++shared_data;
    lock.unlock();
}

void owns_lock() {
    std::unique_lock<std::mutex> lock(mtx);
    ++shared_data;
    if(lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;
    } else {
        std::cout << "doesn't own lock" << std::endl;
    }

    lock.unlock();
    if(lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;
    } else {
        std::cout << "doesn't own lock" << std::endl;
    }
}

void defer_lock() {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    lock.lock();
    lock.unlock();
}

void use_own_defer() {
    std::unique_lock<std::mutex> lock(mtx);

    if(lock.owns_lock()) {
        std::cout << "Main thread has the lock." << std::endl;
    } else {
        std::cout << " Main thread does not have the lock." << std::endl;
    }

    std::thread t([](){
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if(lock.owns_lock()) {
            std::cout << "Thread has the lock." << std::endl;
        } else {
            std::cout << "Thread does not have the lock." << std::endl;
        }
        lock.unlock();
    });

    t.join();
}

void use_own_adopt() {
    mtx.lock();
    std::unique_lock<std::mutex> lock(mtx, std::adopt_lock);
    if(lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;
    } else {
        std::cout << "does not have the lock" << std::endl;
    }
    lock.unlock();
}

int a = 10;
int b = 99;
std::mutex mtx1;
std::mutex mtx2;

void safe_swap() {
    std::lock(mtx1, mtx2);
    std::unique_lock<std::mutex> lock1(mtx1, std::adopt_lock);
    std::unique_lock<std::mutex> lock2(mtx2, std::adopt_lock);
    std::swap(a, b);
}

void safe_swap2() {
    std::unique_lock<std::mutex> lock1(mtx1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(mtx2, std::defer_lock);

    std::lock(lock1, lock2);
    std::swap(a, b);
}

std::unique_lock<std::mutex> get_lock() {
    std::unique_lock<std::mutex> lock(mtx);
    ++shared_data;
    return lock;
}

void use_return() {
    std::unique_lock<std::mutex> lock(get_lock());
    ++shared_data;
}

void precision_lock() {
    std::unique_lock<std::mutex> lock(mtx);
    ++shared_data;
    lock.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    lock.lock();
    ++shared_data;
}

class DNService {
public:
    DNService();
    std::string QueryDNS(std::string dnsname) {
        std::shared_lock<std::shared_mutex> shared_locks(_shared_mtx);
        auto iter = _dns_info.find(dnsname);
        if (iter != _dns_info.end()) return iter->second;
        return "";
    }
    void AddDNSInfo(std::string dnsname, std::string dnsentry) {
        std::lock_guard<std::shared_mutex>  guard_locks(_shared_mtx);
        _dns_info.insert(std::make_pair(dnsname, dnsentry));
    }

private:
    std::map<std::string, std::string> _dns_info;
    mutable std::shared_mutex  _shared_mtx;
};
#ifndef DISK_H
#define DISK_H

#include "parameters.h"

class Block {
public:
    ObjBlock data;
    Block *next;

    Block(ObjBlock value) : data(value), next(nullptr) {}
    void to_string() {
        data.to_string();
    }
};

enum Action {
    Jump,
    Pass,
    Read,
    Abort
};

struct TokenReturn {
    Block* temp;
    Block* curr;
    int read_id;
    int steps;
    int tokens;
    Action action;

    TokenReturn() : temp(nullptr), curr(nullptr), read_id(-1), steps(0), tokens(0) {}
    TokenReturn(Block* ptr, Block* _curr, int id, int step, int token, Action _action) : temp(ptr), curr(_curr), read_id(id), steps(step), tokens(token), action(_action) {}
    void action_to_string() {
        switch (action) {
            case Jump: std::cout << "Jump";
                       break;
            case Pass: std::cout << "Pass";
                       break;
            case Read: std::cout << "Read";
                       break;
            case Abort: std::cout << "Abort";
                       break;
            default: std::cout << "UNKNOWN";
        }
    }
    void to_string() {
        std::cout << "Read " << read_id;
        std::cout << " 执行计划: ";
        action_to_string();
        std::cout << ", 消耗token: " << tokens;
        std::cout << ", 移位: " << steps;
        std::cout << ", 磁头之前的block: ";
        curr->to_string();
        std::cout << ", 磁头当前的block: ";
        temp->to_string();
        std::cout << std::endl;
    }
};

class Disk {
public:
    Block *curr;
    Block *head;
    int length;
    int size;
    int max_token;

    bool isRead; // 磁头的上一个动作是不是read
    int prev_token; // 上一个动作（是read）消耗的token

    Disk(int len, int _token);
    // ~Disk();

    void to_string();
    void compress();
    void insert(ObjBlock& value);
    void remove(int& id);
    // void simple_read(int& id);
    void simple_read(const int& read_id, const int& time, const int& obj_id, const int& disk_id, ReadQueue& simple_read);
    TokenReturn token_read(SimpleRead& task);
    bool isfull();
    int spaceLeft();
    int getLocation(Block* block);
    int avgDist();
};

class StorageManager {
public:
    std::vector<Disk> disks;
    std::unordered_map<int, uint16_t> obj_bitmap;

    StorageManager(int disk_count, int block_count, int total_token);
    void insert(Object& value);
    void remove(int& removed_id);
    void simple_read(const int& read_id, const int& read_time, const int& obj_id, ReadWaitList& simple_read_result);
    void token_read();
    void to_string();
    void map_to_string(int obj_id);
    void map_to_string();
    std::vector<int> chooseDisk(int num, int part);
};


struct diskdistCompare {
    bool operator()(Disk& d1, Disk& d2) {
        return d1.avgDist() > d2.avgDist();
    }
};

using DiskQueue = std::priority_queue<Disk, std::vector<Disk>, diskdistCompare>;

#endif
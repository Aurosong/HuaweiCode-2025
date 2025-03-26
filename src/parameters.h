#ifndef PARAMETER_H
#define PARAMETER_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <map>
#include <cmath>
#include <algorithm>
#include <functional>

struct ObjBlock {
    int objId;
    int total_size;
    int part;
    int tag;
    
    ObjBlock() : objId(-1), total_size(-1), part(-1), tag(-1) {}
    ObjBlock(int id, int size, int _part, int _tag) : objId(id), total_size(size), part(_part), tag(_tag) {}
    ObjBlock(const ObjBlock& other) : objId(other.objId), total_size(other.total_size), part(other.part), tag(other.tag) {}
    void to_string() {
        std::cout << "id:" << objId;
        std::cout << " size:" << total_size;
        std::cout << " part:" << part;
        // std::cout << " tag:" << tag;
    }
    
    bool operator == (const ObjBlock& other) {
        return objId == other.objId && part == other.part && tag == other.tag && total_size == other.total_size;
    }

    bool operator != (const ObjBlock& other) {
        return objId != other.objId || part != other.part;
    }
};

struct Object {
    int id;
    int tag;
    int size;

    Object(int _id, int _tag, int _size) : id(_id), tag(_tag), size(_size) {}
    Object(const Object& other) : id(other.id), tag(other.tag), size(other.size) {}
    void to_string() {
        std::cout << "object id: " << id;
        std::cout << "   tag: " << tag;
        std::cout << "   size: " << size << std::endl;
    }

    bool operator == (const Object& other) {
        return id == other.id && tag == other.tag && size == other.size;
    }

    bool operator != (const Object& other) {
        return id != other.id || tag != other.tag || size != other.size;
    }
};

// simple read 读取到一个disk中一个block的数据
struct SimpleRead {
    int read_id;
    int time;
    int disk_id;
    int dist;
    int priority;
    ObjBlock block;
    // std::vector<std::pair<ObjBlock, int>> block_msg; // <block, distance>

    SimpleRead(int readId, int _time, int diskId, int _dist, ObjBlock _block) : read_id(readId), disk_id(diskId), dist(_dist),
                                                                                block(_block), time(_time) {
        priority = _time + _dist;
    };

    void update_priority() {
        priority = time + dist;
    }
    
    void to_string() {
        std::cout << "read " << read_id << " in tiemstamp " << time;
        std::cout << ", block msg: ";
        block.to_string();
        std::cout << ", distance: " << dist;
        std::cout << ", priority: " << priority << std::endl;
    }
};

struct TokenRead {
    int read_id; // 该read请求的id
    int time; // 获取到该read的时间
    int size; // 该object的size;
    std::vector<SimpleRead> read_msg; // 提前通过simple read获取信息
    std::vector<bool> isready; // 该object是否读取完成的标记

    TokenRead(int _id, int _time);
};

struct distCompare {
    bool operator()(const SimpleRead& a, const SimpleRead& b) {
        return a.dist > b.dist;
    };
};

struct timeCompare {
    bool operator()(const SimpleRead& a, const SimpleRead& b) {
        return a.time > b.time;
    };
};

struct combCompare {
    bool operator()(const SimpleRead& a, const SimpleRead& b) {
        return a.priority > b.priority;
    };
};

using ReadQueue = std::priority_queue<SimpleRead, std::vector<SimpleRead>, combCompare>;
using ReadWaitList = std::unordered_map<int, ReadQueue>;

#endif
#ifndef DISK_C
#define DISK_C

#include "disk_manager.h"

Disk::Disk(int len, int _token) : length(len), size(0), isRead(false), prev_token(0), max_token(_token) {
    if (len > 0) {
        ObjBlock empty_block(-1, -1, -1, -1);
        curr = new Block(empty_block); // 先创建一个头节点
        Block *node = curr;
        for (int i = 1; i < length; ++i) {
            node -> next = new Block(empty_block);
            node = node -> next;
        }
        node -> next = curr; // 形成循环
    }
}

// Disk::~Disk() {
//     if (curr == nullptr)
//         return;
//     Block *curr = curr;
//     Block *nextNode;
//     do {
//         nextNode = curr -> next;
//         delete curr;
//         curr = nextNode;
//     } while (curr != curr);
// }

void Disk::to_string() {
    Block *finder = curr;
    do {
        finder -> to_string();
        std::cout << " | ";
        finder = finder -> next;
    } while (finder != curr);
    std::cout << std::endl << std::endl << std::endl;
}

void Disk::compress() {
    Block *fast = curr->next;
    Block *slow = curr;
    int count = 0;
    int gap = 0;
    ObjBlock empty_block(-1, -1, -1, -1);
    while(slow != fast) {
        if(fast->data == empty_block) {
            fast = fast->next;
            count += 1;
            gap += 1;
        } else {
            if(gap > 0) {
                slow->next = fast;
                slow = fast;
                fast = fast->next;
                gap = 0;
            } else {
                slow = slow->next;
                fast = fast->next;
            }
        }
        if(fast == curr) {
            if(fast->data == empty_block) {

            }
            break;
        }
    }
    for(count; count > 0; count--) {
        slow->next->data = empty_block;
        slow = slow->next;
    }
}

void Disk::insert(ObjBlock& value) {
    if (size >= length) {
        std::cout << "List is full. Cannot insert." << std::endl;
        return;
    }
    ObjBlock empty_block(-1, -1, -1, -1);
    Block* finder = curr;
    if (finder -> data == empty_block) {
        finder -> data = value;
    } else {
        while (finder -> next -> data != empty_block) {
            finder = finder -> next;
        }
        finder -> next -> data = value;
    }
    size += 1;
    finder = nullptr;
}

void Disk::remove(int& id) {
    ObjBlock empty_block(-1, -1, -1, -1);
    Block *finder = curr;

    while(finder -> next != curr) {
        if(finder -> data.objId == id) {
            finder -> data = empty_block;
            size -= 1;
        }
        finder = finder -> next;
    }
    finder = nullptr;
}

void Disk::simple_read(const int& read_id, const int& time, const int& obj_id, const int& disk_id, ReadQueue& simple_read) {
    Block* finder = curr;
    int dist = 0;
    while(finder -> next != curr) {
        if(finder->data.objId == obj_id) {
            SimpleRead simple_block(read_id, time, disk_id, dist, finder->data);
            simple_read.emplace(simple_block);
        }
        finder = finder->next;
        dist += 1;
    }
    if(finder->next == curr && finder->data.objId == obj_id) {
        SimpleRead simple_block(read_id, time, disk_id, dist, finder->data);
        simple_read.emplace(simple_block);
    }
}

TokenReturn Disk::token_read(SimpleRead& task) {
    int steps = 0;
    int token = 0;
    Block* temp = curr;
    Action action;
    // 如果dist = 0，磁头正好在数据的位置，read
    if(task.dist == 0) {
        if(isRead) {
            prev_token = std::max(16, static_cast<int>(std::ceil(prev_token * 0.8)));
        } else {
            prev_token = 64;
        }
        isRead = true;
        temp = temp->next;
        steps = 1;
        action = Read;
    } else {
        // 如果距离大于G，且token够用，jump，反之，暂时抛弃这个read，进行下一个
        if(task.dist >= max_token) {
            if(token >= max_token) {
                // jump
                for(int i = task.dist; i > 0; i--) {
                    temp = temp->next;
                    isRead = false;
                    prev_token = max_token;
                }
                steps = task.dist;
                action = Jump;
            } else {
                // 暂时放弃这个read，降低优先级
                // 优先级降低的幅度需要测试
                task.priority -= 10;
                prev_token = 0;
                action = Abort;
            }
        } else {
            // 距离小于G，做一次pass
            temp = temp->next;
            isRead = false;
            prev_token = 1;
            steps = 1;
            action = Pass;
        }
    }
    token += prev_token;
    return TokenReturn(temp, steps, token, action);
}

bool Disk::isfull() {
    return size == length;
}

int Disk::spaceLeft() {
    return length - size;
}

StorageManager::StorageManager(int disk_count, int block_count, int total_token) {
    this -> disks.reserve(disk_count);
    for(int i = 0; i < disk_count; i++) {
        Disk dManager(block_count, total_token);
        this -> disks.push_back(dManager);
    }
}

void StorageManager::insert(Object& value) {
    int split = value.size;
    uint16_t value_bitmap = 0;
    std::vector<int> in_disk = chooseDisk(3, value.size);

    for(int i = 0; i < 3; i++) {
        for(int ii = 0; ii < split; ii++) {
            ObjBlock obj_block(value.id, value.size, ii, value.tag);
            disks.at(in_disk.at(i)).insert(obj_block);
        }
        value_bitmap = 1 << in_disk.at(i) | value_bitmap;
    }
    obj_bitmap.emplace(value.id, value_bitmap);
}

void StorageManager::remove(int& removed_id) {
    // map_to_string(removed_id);
    uint16_t id_map = obj_bitmap.at(removed_id);
    int count = 0;
    for(int i = 0; i < 16; i++) {
        if((id_map & 1) == 1) {
            disks.at(i).remove(removed_id);
            count += 1;
        }
        id_map = id_map >> 1;
        if(count >= 3) break;
    }
    obj_bitmap.erase(removed_id);
    if(count < 3) std::cout << "error in remove, not 3 disk found" << std::endl;
}

void StorageManager::simple_read(const int& read_id, const int& read_time, const int& obj_id, ReadWaitList& simple_read_result) {
    uint16_t id_map = obj_bitmap.at(obj_id);
    // map_to_string(obj_id);

    for(int i = 0; i < 16; i++) {
        if(id_map & 1 == 1) {
            ReadQueue disk_read_result;
            disks.at(i).simple_read(read_id, read_time, obj_id, i, disk_read_result);

            // std::cout << "Simple Read Msg: ";
            // std::cout << "Read id: " << read_id << std::endl;

            if(simple_read_result.find(i) == simple_read_result.end()) {
                simple_read_result.emplace(i, disk_read_result);
            } else {
                while(!disk_read_result.empty()) {
                    simple_read_result.at(i).emplace(disk_read_result.top());
                    disk_read_result.pop();
                }
            }
        }
        id_map = id_map >> 1;
    }
}

void StorageManager::map_to_string(int obj_id) {
    uint16_t local_bitmap = obj_bitmap.at(obj_id);
    std::cout << "id: " << obj_id << ", bitmap: ";
    for(int i = 15; i >=0; --i) {
        std::cout << ((local_bitmap >> i) & 1);
    }
    std::cout << std::endl;
}

void StorageManager::to_string() {
    // map_to_string();
    for(int i = 0; i < disks.size(); i++) {
        std::cout << "disk index: " << i;
        Disk disk = disks.at(i);
        std::cout << " length: " << disk.length << "; size: " << disk.size << std::endl;
        disk.to_string();
    }
}

std::vector<int> StorageManager::chooseDisk(int num, int part) {
    std::vector<int> result;
    result.reserve(num);
    for(int i = 0; i < disks.size(); i++) {
        if(!disks.at(i).isfull() && disks.at(i).spaceLeft() >= part) {
            result.push_back(i);
        }
        if(result.size() == num) break;
    }
    return result;
}

#endif
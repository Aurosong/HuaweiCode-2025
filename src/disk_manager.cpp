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
    head = curr;
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
    std::cout << "Disk length: " << length << ", size: " << size << ", isRead: " << isRead << ", prev token: " << prev_token << std::endl;
    Block *finder = head;
    ObjBlock empty_block(-1, -1, -1, -1);
    int dist = 0;
    do {
        if(finder == curr) {
            std::cout << " -- curr --> ";
        }
        if(finder->data != empty_block) {
            finder -> to_string();
            std::cout << " dist: " << dist << " | ";
        }
        // finder -> to_string();
        // std::cout << " | ";
        finder = finder -> next;
        dist += 1;
    } while (finder != head);
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
    Block* finder = head;
    int dist = 0;

    while(finder->data != empty_block) {
        finder = finder->next;
        dist += 1;
    }

    finder->data = value;
    std::cout << dist + 1 << " ";

    // if (finder -> data == empty_block) {
    //     finder -> data = value;
    //     std::cout << dist + 1 << " ";
    // } else {
    //     while (finder -> next -> data != empty_block) {
    //         finder = finder -> next;
    //         dist += 1;
    //     }
    //     finder -> next -> data = value;
    //     std::cout << dist + 1 << " ";
    // }
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
        token = prev_token;
        isRead = true;
        temp = temp->next;
        steps = 1;
        action = Read;
    } else {
        // 如果距离大于G，且token够用，jump，反之，暂时抛弃这个read，进行下一个
        prev_token = -1;
        isRead = false;
        if(task.dist >= max_token) {
            // jump

            // std::cout << std::endl << "开始执行jump！" << std::endl;
            // std::cout << "当前block：";
            // temp->to_string();
            // std::cout << std::endl << "目标block：";
            // task.block.to_string();
            // std::cout << std::endl << "距离：" << task.dist << std::endl;


            for(int i = task.dist; i > 0; i--) {
                temp = temp->next;
            }
            token = max_token;
            steps = task.dist;
            action = Jump;
            // if(token == max_token) {
            //     // jump
            //     for(int i = task.dist; i > 0; i--) {
            //         temp = temp->next;
            //         isRead = false;
            //         prev_token = max_token;
            //     }
            //     steps = task.dist;
            //     action = Jump;
            // } else {
            //     // 暂时放弃这个read，降低优先级
            //     // 优先级降低的幅度需要测试
            //     task.priority -= 10;
            //     prev_token = 0;
            //     action = Abort;
            // }
        } else {
            // 距离小于G，做一次pass
            temp = temp->next;
            token = 1;
            steps = 1;
            action = Pass;
        }
    }
    return TokenReturn(temp, curr, task.read_id, steps, token, action);
}

bool Disk::isfull() {
    return size == length;
}

int Disk::spaceLeft() {
    return length - size;
}

int Disk::getLocation(Block* block) {
    Block* temp = head;
    int loc = 1;
    while(temp != block) {
        temp = temp->next;
        loc += 1;
    }
    return loc;
}

int Disk::avgDist() {
    ObjBlock empty_block(-1, -1, -1, -1);
    int distsum = 0;
    int dist = 0;
    Block *finder = curr;

    while(finder -> next != curr) {
        if(finder -> data != empty_block) {
            distsum += dist;
        }
        dist += 1;
        finder = finder -> next;
    }
    return distsum;
}

StorageManager::StorageManager(int disk_count, int block_count, int total_token) {
    this -> disks.reserve(disk_count);
    for(int i = 0; i < disk_count; i++) {
        Disk dManager(block_count, total_token);
        this -> disks.push_back(dManager);
    }
}

void StorageManager::insert(Object& value) {
    if(value.id > 0) {
        std::cout << value.id << std::endl;
    } else {
        std::cerr << "Not a valid object id!" << std::endl;
    }

    int split = value.size;
    uint16_t value_bitmap = 0;
    std::vector<int> in_disk = chooseDisk(3, value.size);

    // std::cout << "insert obj id: " << value.id << std::endl;
    for(int i = 0; i < 3; i++) {
        std::cout << in_disk.at(i) + 1 << " ";
        for(int ii = 0; ii < split; ii++) {
            ObjBlock obj_block(value.id, value.size, ii, value.tag);
            disks.at(in_disk.at(i)).insert(obj_block);
        }
        value_bitmap = 1 << in_disk.at(i) | value_bitmap;
        std::cout << std::endl;
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

void StorageManager::map_to_string() {
    for(auto pair : obj_bitmap) {
        int obj_id = pair.first;
        uint16_t local_bitmap = pair.second;
        std::cout << "id: " << obj_id << ", bitmap: ";
        for(int i = 15; i >=0; --i) {
            std::cout << ((local_bitmap >> i) & 1);
        }
        std::cout << std::endl;
    }
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

std::vector<int> StorageManager::chooseDisk(int num, int part) { // num = 3
    std::vector<int> result;
    result.reserve(num);

    // 选择策略：按顺序选择三个有空位的disk
    // for(int i = 0; i < disks.size(); i++) {
    //     if(!disks.at(i).isfull() && disks.at(i).spaceLeft() >= part) {
    //         result.push_back(i);
    //     }
    //     if(result.size() == num) break;
    // }

    // 选择策略：选择元素最少的三个disk
    for(int i = 0; i < disks.size(); i++) {
        if(result.size() < 3) {
            result.push_back(i);
        } else {
            int curr_max_index = 0;
            for(int j = 0; j < 3; j++) {
                if(disks.at(result.at(j)).size > disks.at(curr_max_index).size) {
                    curr_max_index = j;
                }
            }
            if(disks.at(i).size < disks.at(curr_max_index).size) {
                result.at(curr_max_index) = i;
            }
        }
    }

    return result;
}

#endif
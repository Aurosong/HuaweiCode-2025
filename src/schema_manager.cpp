#ifndef SCHEMA_C
#define SCHEMA_C

#include "schema_manager.h"


Schema::Schema() : sManager(0, 0, 0) {
    std::cin >> TIME_STAMP_COUNT >> TAG_COUNT >> DISK_COUNT >> BLOCK_COUNT >> TOKEN_COUNT;
    sManager = StorageManager(DISK_COUNT, BLOCK_COUNT, TOKEN_COUNT);
    token = TOKEN_COUNT;
    timestamp = TIME_STAMP_COUNT;

    write_time = 0;
    delete_time = 0;
    read_time = 0;
    simple_time = 0;
    token_time = 0;
    dist_update_time = 0;
    block_update_time = 0;

    for(int i = 0; i < TAG_COUNT; i++) {
        int tag = i + 1;
        std::vector<int> delete_timestamps;
        delete_timestamps.reserve(std::ceil((double)TIME_STAMP_COUNT / 1800.0));
        for(int j = 0; j < std::ceil((double)TIME_STAMP_COUNT / 1800.0); j++) {
            int delete_sum_in_timestamp;
            std::cin >> delete_sum_in_timestamp;
            delete_timestamps.push_back(delete_sum_in_timestamp);
        }
        delete_counter.emplace(tag, delete_timestamps);
    }

    for(int i = 0; i < TAG_COUNT; i++) {
        int tag = i + 1;
        std::vector<int> write_timestamps;
        write_timestamps.reserve(std::ceil((double)TIME_STAMP_COUNT / 1800.0));
        for(int j = 0; j < std::ceil((double)TIME_STAMP_COUNT / 1800.0); j++) {
            int write_sum_in_timestamp;
            std::cin >> write_sum_in_timestamp;
            write_timestamps.push_back(write_sum_in_timestamp);
        }
        write_counter.emplace(tag, write_timestamps);
    }

    for(int i = 0; i < TAG_COUNT; i++) {
        int tag = i + 1;
        std::vector<int> read_timestamps;
        read_timestamps.reserve(std::ceil((double)TIME_STAMP_COUNT / 1800.0));
        for(int j = 0; j < std::ceil((double)TIME_STAMP_COUNT / 1800.0); j++) {
            int read_sum_in_timestamp;
            std::cin >> read_sum_in_timestamp;
            read_timestamps.push_back(read_sum_in_timestamp);
        }
        read_counter.emplace(tag, read_timestamps);
    }

    for(int i = 0; i < DISK_COUNT; i++) {
        ReadQueue empty_queue;
        read_time_waitList.emplace(i, empty_queue);
    }

    std::cout << "OK" << std::endl;

    // initialize_to_string();
}

void Schema::initialize_to_string() {
    std::cout << "time stamp count: " << TIME_STAMP_COUNT << std::endl;
    std::cout << "tag count: " << TAG_COUNT << std::endl;
    std::cout << "disk count: " << DISK_COUNT << std::endl;
    std::cout << "block count: " << BLOCK_COUNT << std::endl;
    std::cout << "token count: " << TOKEN_COUNT << std::endl;
    std::cout << "==========================" << std::endl;
    for(auto pair : delete_counter) {
        int tag = pair.first;
        auto summer = pair.second;
        std::cout << "Tag: " << tag << std::endl;
        for(auto sum : summer) {
            std::cout << sum << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "==========================" << std::endl;
    for(auto pair : write_counter) {
        int tag = pair.first;
        auto summer = pair.second;
        std::cout << "Tag: " << tag << std::endl;
        for(auto sum : summer) {
            std::cout << sum << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "==========================" << std::endl;
    for(auto pair : read_counter) {
        int tag = pair.first;
        auto summer = pair.second;
        std::cout << "Tag: " << tag << std::endl;
        for(auto sum : summer) {
            std::cout << sum << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "disk number: " << sManager.disks.size() << std::endl;
    // sManager.map_to_string();
    // for(auto disk : sManager.disks) {
    //     disk.to_string();
    // }
}

void Schema::handle_timeStamp(int stamp) {
    int current_timestamp;
    std::string notion;
    std::cin >> notion >> current_timestamp;
    if(current_timestamp != stamp) {
        std::cerr << "Timestamp not aligned!" << std::endl;
    } else {
        std::cout << "TIMESTAMP " << stamp << std::endl;
    }

    TimerClock tc;

    handle_delete(stamp);

    delete_time += tc.second();
    tc.tick();

    std::cout.flush();
    handle_write(stamp);

    write_time += tc.second();
    tc.tick();

    // handle_simple_read(stamp);
    std::cout.flush();

    handle_token_read(stamp);

    read_time += tc.second();

    std::cout.flush();

    // sManager.to_string();

    // std::cout << "================================================================" << std::endl;
    // std::cout << "================================================================" << std::endl;
}

void Schema::handle_delete(int stamp) {
    int operation_count;
    std::cin >> operation_count;
    
    std::set<int> cancle_read_id;
    for(int i = 0; i < operation_count; i++) {
        int delete_id;
        std::cin >> delete_id;
        sManager.remove(delete_id);
        waitList_delete_update(delete_id, cancle_read_id);
    }
    std::cout << cancle_read_id.size() << std::endl;
    for(auto readId : cancle_read_id) {
        std::cout << readId << std::endl;
    }
}

void Schema::handle_write(int stamp) {
    int operation_count;
    std::cin >> operation_count;
    
    for(int i = 0; i < operation_count; i++) {
        int obj_id, obj_size, obj_tag;
        std::cin >> obj_id >> obj_size >> obj_tag;
        Object object(obj_id, obj_tag, obj_size);
        sManager.insert(object);
    }
}

void Schema::handle_token_read(int stamp) {
    report_list.clear();
    int operation_count;
    std::cin >> operation_count;

    TimerClock tc;

    // 存储当前时间片内输入的查询
    for(int i = 0; i < operation_count; i++) {
        int read_id, read_obj_id;
        std::cin >> read_id >> read_obj_id;

        tc.tick();
        sManager.simple_read(read_id, stamp, read_obj_id, read_time_waitList);
        simple_time += tc.second();

        std::vector<ObjBlock> empty_blocks;
        read_finish_container.emplace(read_id, empty_blocks);
    }

    // if(stamp == 435) {
    //     waitList_to_string(stamp);
    // }

    // 处理当前时间片时剩下的查询
    std::vector<int> token_counter(DISK_COUNT, TOKEN_COUNT); // token计数器，下标为disk ID，value为剩余的token
    for(auto disk : read_time_waitList) {
        int disk_id = disk.first;
        bool isJump = false;
        while(token_counter.at(disk_id) > 0 && !read_time_waitList.at(disk_id).empty()) {
            TokenReturn token_return;
            SimpleRead read_task = read_time_waitList.at(disk_id).top();

            // sManager返回一个token read方案（token_return），由Schema决定该方案能否被提交

            tc.tick();
            token_return = sManager.disks.at(disk_id).token_read(read_task);
            token_time += tc.second();
            if(token_counter.at(disk_id) >= token_return.tokens) {
                // 能够完成任务，提交所有修改
                token_counter.at(disk_id) -= token_return.tokens;
                sManager.disks.at(disk_id).curr = token_return.temp;

                if(token_return.action == Read) {

                    // std::cout << std::endl << ", Read Block: ";
                    // token_return.curr->to_string();
                    // std::cout << " ,dist: ";
                    // std::cout << sManager.disks.at(disk_id).getLocation(token_return.curr);
                    // std::cout << std::endl;

                    sManager.disks.at(disk_id).isRead = true;
                    read_time_waitList.at(disk_id).pop();
                    read_finish_container.at(token_return.read_id).push_back(token_return.curr->data);
                    if(read_finish_container.at(token_return.read_id).size() == token_return.curr->data.total_size) {
                        checkFinish(token_return.read_id);
                    }

                    // sManager.to_string();
                    // token_return.to_string();
                    tc.tick();
                    waitList_block_update(token_return.read_id, token_return.curr->data);
                    block_update_time += tc.second();

                    // finishList_to_string(stamp);
                }

                tc.tick();
                waitList_dist_update(token_return.steps, disk_id);
                dist_update_time += tc.second();

                if(token_return.action == Jump) {
                    int jump_location = sManager.disks.at(disk_id).getLocation(token_return.temp);
                    std::cout << "j " << jump_location << std::endl;
                    isJump = true;
                    continue;
                } else {
                    switch (token_return.action) {
                        case Pass: std::cout << "p";
                                   break;
                        case Read: std::cout << "r";
                                   break;
                        default: std::cout << "error";
                    }
                }
            } else {
                // token不够完成当前任务
                token_counter.at(disk_id) = 0;
                sManager.disks.at(disk_id).isRead = false;
                // std::cout << "token不够执行下一个任务！" << std::endl;
            }
            // waitList_to_string(stamp);
        }
        if(!isJump) {
            std::cout << "#" << std::endl;
        }
    }

    std::cout << report_list.size() << std::endl;
    for(auto report : report_list) {
        std::cout << report << std::endl;
    }
}

/*
在disk操作之后，磁头位置变化，waitlist中所有相对距离都会变动
此函数用来更新waitlist中的距离，让其根据磁头变动适当改变
*/
void Schema::waitList_dist_update(int dist, int disk_id) {
    ReadQueue disk_queue = read_time_waitList.at(disk_id);
    ReadQueue temp;

    dist = dist % BLOCK_COUNT;
    while(!disk_queue.empty()) {
        SimpleRead simple_read = disk_queue.top();
        disk_queue.pop();

        if (simple_read.dist >= dist) {
            simple_read.dist -= dist;
        } else {
            simple_read.dist = BLOCK_COUNT - dist + simple_read.dist;
        }
        simple_read.update_priority();
        temp.emplace(simple_read);
    }
    read_time_waitList.at(disk_id) = temp;
}

/*
当有未读完的项目被delete时，该read被取消
此函数用来删除并汇报这些被取消的read
*/
void Schema::waitList_delete_update(int id, std::set<int>& cancle_read_id) {
    int cancle_count = 0;
    for(auto pair : read_time_waitList) {
        int disk_id = pair.first;
        ReadQueue readQueue = pair.second;
        ReadQueue temp;
        while(!readQueue.empty()) {
            SimpleRead simple_read = readQueue.top();
            readQueue.pop();
            if(simple_read.block.objId != id) {
                temp.emplace(simple_read);
            } else {
                cancle_read_id.emplace(simple_read.read_id);
            }
        }
        read_time_waitList.at(disk_id) = temp;
    }
}

/*
当一个block被某个disk完成了读时，其他disk的waitlist中可能还有这个block的读任务
此函数用来删除这些冗余的读任务
*/
void Schema::waitList_block_update(const int& read_id, ObjBlock& block) {
    // std::cout << "Block msg: ";
    // block.to_string();
    // std::cout << std::endl;
    // sManager.map_to_string();

    int obj_id = block.objId;
    uint16_t disk_bitmap = sManager.obj_bitmap.at(obj_id);

    for(int i = 0; i < 10; i++) {
        if(disk_bitmap & 1 == 1) {
            ReadQueue readQueue = read_time_waitList.at(i);
            ReadQueue temp;
            while(!readQueue.empty()) {
                SimpleRead simple_read = readQueue.top();
                readQueue.pop();
                if(simple_read.block != block || simple_read.read_id != read_id) {
                    temp.emplace(simple_read);
                }
            }
            read_time_waitList.at(i) = temp;
        }
        disk_bitmap = disk_bitmap >> 1;
    }

    // for(auto pair : read_time_waitList) {
    //     int disk_id = pair.first;
    //     ReadQueue readQueue = pair.second;
    //     ReadQueue temp;
    //     while(!readQueue.empty()) {
    //         SimpleRead simple_read = readQueue.top();
    //         readQueue.pop();
    //         if(simple_read.block != block || simple_read.read_id != read_id) {
    //             temp.emplace(simple_read);
    //         }
    //     }
    //     read_time_waitList.at(disk_id) = temp;
    // }
}

void Schema::waitList_finish_update(const int& read_id) {
    for(auto pair : read_time_waitList) {
        int disk_id = pair.first;
        ReadQueue readQueue = pair.second;
        ReadQueue temp;
        while(!readQueue.empty()) {
            SimpleRead simple_read = readQueue.top();
            readQueue.pop();
            if(simple_read.read_id != read_id) {
                temp.emplace(simple_read);
            }
        }
        read_time_waitList.at(disk_id) = temp;
    }
}

void Schema::checkFinish(const int& read_id) {
    std::vector<ObjBlock> list = read_finish_container.at(read_id);
    if(list.empty()) {
        std::cerr << "Error for a empty finish list!" << std::endl;
        return;
    }
    int objId = list.at(0).objId;
    int size = list.at(0).total_size;
    std::set<int> checker;
    for(int i = 0; i < size; i++) {
        checker.emplace(i);
    }
    for(auto obj_block : list) {
        checker.erase(obj_block.part);
    }
    if(checker.empty()) {
        // std::cout << "Read " << read_id << " 已完成所有读取！" << std::endl;
        // waitList_finish_update(read_id);
        report_list.push_back(read_id);
        read_finish_container.erase(read_id);
    } else {
        std::cerr << "Error: Read" << read_id << " 验证未通过！" << std::endl;
        return;
    }
}

void Schema::waitList_add() {

}

void Schema::waitList_remove(int stamp, int obj_id) {
    // std::vector<SimpleRead> temp;
    // while(!read_time_waitList.empty()) {
    //     SimpleRead simple_read = read_time_waitList.top();
    //     read_time_waitList.pop();
        
    //     // if(simple_read.time)

    //     temp.push_back(simple_read);
    // }
    // for(auto& entry : temp) {
    //     read_time_waitList.push(entry);
    // }
}

void Schema::time_to_string() {
    std::cout << std::endl << "运行时间总览：" << std::endl;
    std::cout << "Delete Time: " << delete_time << std::endl;
    std::cout << "Write Time: " << write_time << std::endl;
    std::cout << "Total Read Time: " << read_time << std::endl;
    std::cout << "Simple Read Time: " << simple_time << std::endl;
    std::cout << "Token Read Time: " << token_time << std::endl;
    std::cout << "Waitlist dist update Time: " << dist_update_time << std::endl;
    std::cout << "Waitlist block update Time: " << block_update_time << std::endl;
}

void Schema::waitList_to_string(int stamp) {
    std::cout << "wait list in timestamp " << stamp << std::endl;

    if(read_time_waitList.size() == 0) {
        std::cout << "Wait List is Empty!" << std::endl;
    } else {
        for(auto pair : read_time_waitList) {
            std::cout << "disk " << pair.first << std::endl;
            std::vector<SimpleRead> temp;
            while(!pair.second.empty()) {
                SimpleRead simple_read = pair.second.top();
                pair.second.pop();
                simple_read.to_string();
                temp.push_back(simple_read);
            }
            for(auto& entry : temp) {
                pair.second.push(entry);
            }
        }
        std::cout << std::endl;
    }
}

void Schema::finishList_to_string(int stamp) {
    std::cout << "Finish container in timestamp " << stamp << std::endl;
    for(auto read : read_finish_container) {
        std::cout << "Read ID: " << read.first << ", finish blocks: ";
        if(read.second.size() == 0) {
            std::cout << "null "; 
        } else {
            for(auto objb : read.second) {
                objb.to_string();
            }
        }
        std::cout << std::endl;
    }
}

#endif
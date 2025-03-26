#ifndef SCHEMA_C
#define SCHEMA_C

#include "schema_manager.h"


Schema::Schema() : sManager(0, 0, 0) {
    std::cin >> TIME_STAMP_COUNT >> TAG_COUNT >> DISK_COUNT >> BLOCK_COUNT >> TOKEN_COUNT;
    sManager = StorageManager(DISK_COUNT, BLOCK_COUNT, TOKEN_COUNT);
    token = TOKEN_COUNT;
    timestamp = TIME_STAMP_COUNT;

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

    handle_delete(stamp);
    handle_write(stamp);
    // handle_simple_read(stamp);

    sManager.to_string();

    handle_token_read(stamp);

    sManager.to_string();

    std::cout << "================================================================" << std::endl;
    std::cout << "================================================================" << std::endl;
}

void Schema::handle_delete(int stamp) {
    int operation_count;
    std::cin >> operation_count;

    for(int i = 0; i < operation_count; i++) {
        int delete_id;
        std::cin >> delete_id;
        sManager.remove(delete_id);
        waitList_update_delete(delete_id);
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
    int operation_count;
    std::cin >> operation_count;

    // 存储当前时间片内输入的查询
    for(int i = 0; i < operation_count; i++) {
        int read_id, read_obj_id;
        std::cin >> read_id >> read_obj_id;
        sManager.simple_read(read_id, stamp, read_obj_id, read_time_waitList);
    }

    waitList_to_string(stamp);

    std::vector<int> token_counter(DISK_COUNT, TOKEN_COUNT); // token计数器，下表为disk ID，value为剩余的token
    for(auto disk : read_time_waitList) {
        int disk_id = disk.first;
        while(token_counter.at(disk_id) > 0 && !read_time_waitList.at(disk_id).empty()) {
            TokenReturn token_return;
            SimpleRead read_task = read_time_waitList.at(disk_id).top();
            token_return = sManager.disks.at(disk_id).token_read(read_task);
            // token_return.to_string();
            if(token_counter.at(disk_id) >= token_return.tokens) {
                // 能够完成任务，提交所有修改
                token_counter.at(disk_id) -= token_return.tokens;
                sManager.disks.at(disk_id).curr = token_return.temp;

                if(token_return.action == Read) {
                    read_time_waitList.at(disk_id).pop();
                }
                waitList_update(token_return.steps, disk_id);

                // waitList_to_string(stamp);

                std::cout << "Disk " << disk_id << " 的Read " << read_task.read_id << " 执行了 ";
                token_return.action_to_string();
                std::cout << ", 消耗Token: " << token_return.tokens;
                std::cout << ", 指针移动: " << token_return.steps;
                std::cout << ", 剩余token: " << token_counter.at(disk_id) << std::endl;
            } else {
                // token不够完成当前任务
                token_counter.at(disk_id) = 0;
                std::cout << "token不够执行下一个任务！" << std::endl;
            }
            waitList_to_string(stamp);
        }
    }
}

void Schema::handle_simple_read(int stamp) {
    // int operation_count;
    // std::cin >> operation_count;
    // // 存储当前时间片内输入的查询
    // for(int i = 0; i < operation_count; i++) {
    //     int read_id, read_obj_id;
    //     std::cin >> read_id >> read_obj_id;
    //     std::vector<SimpleRead> simple_read_result;
    //     sManager.simple_read(read_id, stamp, read_obj_id, simple_read_result);
    
    //     waitList_to_string(stamp);
    //     std::cout << std::endl << std::endl;
    // }
}

void Schema::waitList_update(int dist, int disk_id) {
    // std::vector<SimpleRead> temp;
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

void Schema::waitList_update_delete(int id) {
    for(auto pair : read_time_waitList) {
        int disk_id = pair.first;
        ReadQueue readQueue = pair.second;
        ReadQueue temp;
        while(!readQueue.empty()) {
            SimpleRead simple_read = readQueue.top();
            readQueue.pop();
            if(simple_read.block.objId != id) {
                temp.emplace(simple_read);
            }
        }
        read_time_waitList.at(disk_id) = temp;
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


#endif
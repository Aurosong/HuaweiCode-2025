#ifndef SCHEMA_H
#define SCHEMA_H

#include "disk_manager.h"

// auto distCompare = [](const SimpleRead& a, const SimpleRead& b) {
//     return a.dist > b.dist;
// };

// auto timeCompare = [](const SimpleRead& a, const SimpleRead& b) {
    
// };

class Schema {
public:
    int TIME_STAMP_COUNT; // T: number of timestamps: T + 105
    int TAIL_TIME_STAMP = 150;
    int TAG_COUNT; // M: categories of tags
    int DISK_COUNT; // N: number of disks
    int BLOCK_COUNT; // V: blocks in one disk
    int TOKEN_COUNT; // G: Token usable per timestamp

    StorageManager sManager;
    std::vector<TokenRead> readlist;
    int timestamp;
    int token;

    ReadWaitList read_time_waitList;

    std::map<int, std::vector<int>> delete_counter;
    std::map<int, std::vector<int>> write_counter;
    std::map<int, std::vector<int>> read_counter;
    
    Schema();

    // handle time stamps
    void handle_timeStamp(int stamp);
    void handle_delete(int stamp);
    void handle_write(int stamp);
    void handle_token_read(int stamp);
    void handle_simple_read(int stamp);
    void waitList_update(int dist, int disk_id);
    void waitList_update_delete(int id);

    // handle waitlist
    void waitList_add();
    void waitList_remove(int stamp, int obj_id);

    // visualization functions
    void initialize_to_string();
    void waitList_to_string(int stamp);
};

#endif
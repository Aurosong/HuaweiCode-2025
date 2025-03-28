#ifndef SCHEMA_H
#define SCHEMA_H

#include "disk_manager.h"

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

    double write_time;
    double delete_time;
    double read_time;
    double simple_time;
    double token_time;
    double dist_update_time;
    double block_update_time;

    std::vector<int> report_list;
    ReadWaitList read_time_waitList;
    std::unordered_map<int, std::vector<ObjBlock>> read_finish_container;

    std::map<int, std::vector<int>> delete_counter;
    std::map<int, std::vector<int>> write_counter;
    std::map<int, std::vector<int>> read_counter;
    
    Schema();

    // handle time stamps
    void handle_timeStamp(int stamp);
    void handle_delete(int stamp);
    void handle_write(int stamp);
    void handle_token_read(int stamp);
    void waitList_dist_update(int dist, int disk_id);
    void waitList_delete_update(int id, std::set<int>& cancle_read_id);
    void waitList_block_update(const int& read_id, ObjBlock& block);
    void waitList_finish_update(const int& read_id);
    void checkFinish(const int& read_id);

    // handle waitlist
    void waitList_add();
    void waitList_remove(int stamp, int obj_id);

    // visualization functions
    void time_to_string();
    void initialize_to_string();
    void waitList_to_string(int stamp);
    void finishList_to_string(int stamp);
};

#endif
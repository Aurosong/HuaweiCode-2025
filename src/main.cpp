
#include "schema_manager.h"

int main() {
    Schema schema;
    int n = 0;

    while(n < schema.TIME_STAMP_COUNT) {
        schema.handle_timeStamp(++n);
        // if(n <= 10) {
        //     std::cout << "time stamp: " << n << std::endl;
        //     schema.sManager.to_string();
        // }
    }

    // while(n < schema.TIME_STAMP_COUNT + schema.TAIL_TIME_STAMP) {
    //     schema.handle_timeStamp(++n);
    //     // if(n <= 10) {
    //     //     std::cout << "time stamp: " << n << std::endl;
    //     //     schema.sManager.to_string();
    //     // }
    // }
    return 0;
}
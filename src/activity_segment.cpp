#include <chrono>
#include <cstdint>
#include "activity_segment.h"


ActivitySegment begin_activity_segment(
    const ActiveWindowSnapshot& snapshot,
    bool idle,
    std::chrono::system_clock::time_point started_at
)
{
    ActivitySegment segment;
    segment.started_at = started_at;
    segment.ended_at = started_at;
    segment.process_id = snapshot.process_id;
    segment.window_title = snapshot.window_title;
    segment.process_name = snapshot.process_name;
    segment.idle = idle;
    return segment;
}

void finish_activity_segment(
    ActivitySegment& segment,
    std::chrono::system_clock::time_point ended_at
){
    segment.ended_at = ended_at;
}

std::int64_t activity_duration_ms(const ActivitySegment& segment)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(segment.ended_at - segment.started_at)
            .count();
}
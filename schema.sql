CREATE TABLE IF NOT EXISTS activity_segments (
    id INTEGER PRIMARY KEY,
    started_at_ms INTEGER NOT NULL,
    ended_at_ms INTEGER NOT NULL,
    process_id INTEGER NOT NULL,
    process_name TEXT NOT NULL,
    window_title TEXT NOT NULL,
    idle INTEGER NOT NULL
);
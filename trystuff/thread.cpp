// g++ -o thread thread.cpp && ./thread

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

constexpr int NUM_TRACKS = 4;

// std::mutex mtx;
// std::condition_variable cv;
// std::atomic<bool> stop_flag(false);
// // std::atomic<int> ready_count(0);

// bool sampling[NUM_TRACKS] = { false };

// void track_worker(int track_id)
// {
//     while (!stop_flag) {
//         // Wait for a signal to generate the next sample
//         {
//             std::unique_lock lock(mtx);
//             cv.wait(lock, [track_id] { return sampling[track_id]; });
//         }

//         int randMs = rand() % 1000;
//         printf("Track %d generating sample in %d ms\n", track_id, randMs);
//         std::this_thread::sleep_for(std::chrono::milliseconds(randMs));
//         printf("Track %d finished in %d ms\n", track_id, randMs);

//         // Notify that this track is done
//         {
//             // std::unique_lock<std::mutex> lock(mtx);
//             sampling[track_id] = false;
//             cv.notify_all();
//         }
//     }
// }

// void resetThreads()
// {
//     for (int i = 0; i < NUM_TRACKS; ++i) {
//         sampling[i] = true;
//     }
// }

// int main()
// {
//     std::vector<std::thread> threads;

//     // Launch track threads
//     for (int i = 0; i < NUM_TRACKS; ++i) {
//         threads.emplace_back(track_worker, i);
//     }

//     for (int loop = 0; loop < 5; ++loop) {
//         // Notify threads to process the next sample
//         {
//             // std::unique_lock lock(mtx);
//             resetThreads();
//             cv.notify_all();
//         }

//         printf("Wait for all threads to finish generating their samples in Loop %d\n", loop);

//         // Wait for all threads to finish generating their samples
//         {
//             std::unique_lock lock(mtx);
//             cv.wait(lock, [] {
//                 bool ready = true;
//                 for (int i = 0; i < NUM_TRACKS; ++i) {
//                     ready &= !sampling[i];
//                 }
//                 return ready;
//             });
//         }

//         printf("All threads finished generating their samples in Loop %d\n", loop);
//     }

//     // Stop the worker threads
//     stop_flag = true;
//     resetThreads();
//     cv.notify_all();

//     // Join threads
//     for (auto& t : threads) {
//         t.join();
//     }

//     return 0;
// }

std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> stop_flag(false);
// std::atomic<int> ready_count(0);

bool sampling[NUM_TRACKS] = { true };

void resetThreads()
{
    for (int i = 0; i < NUM_TRACKS; ++i) {
        if (sampling[i]) {
            return;
        }
    }
    printf("Resetting threads\n");
    for (int i = 0; i < NUM_TRACKS; ++i) {
        sampling[i] = true;
    }
    cv.notify_all();
}

void track_worker(int track_id)
{
    while (!stop_flag) {
        std::unique_lock lock(mtx);
        cv.wait(lock, [track_id] { return sampling[track_id]; });
        lock.unlock();

        int randMs = rand() % 1000;
        printf("Track %d generating sample in %d ms\n", track_id, randMs);
        std::this_thread::sleep_for(std::chrono::milliseconds(randMs));
        printf("Track %d finished in %d ms\n", track_id, randMs);

        // std::unique_lock<std::mutex> lock(mtx);
        sampling[track_id] = false;
        resetThreads();
    }
}

int main()
{
    std::vector<std::thread> threads;

    // Launch track threads
    for (int i = 0; i < NUM_TRACKS; ++i) {
        threads.emplace_back(track_worker, i);
    }

    // Join threads
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
/** Description:
This program is designed to simulate a system where multiple tasks, referred to as "tracks," must work together in a synchronized manner. Think of it like a four-person relay team where every runner must complete their leg before the whole team can start the next lap.

The core idea is to launch four independent streams of activity, called "threads," that run in parallel.

To manage this complex timing and prevent errors, the system relies on specialized tools:
1.  **Locks (Mutexes):** These ensure that only one track is changing shared status information at any given moment.
2.  **Signals (Condition Variables):** These allow the tracks to "sleep" and wait efficiently until they receive a specific instruction to start working.

**How it Works:**
The four tracks start in a waiting state. When the system is ready, they are signaled to begin. Each track performs a simulated, time-consuming job (represented by a random pause).

Once a track finishes its job, it updates its status. The system constantly monitors the status of all tracks. As soon as the very last track completes its task, the entire group is "reset," and all four tracks are immediately signaled to begin the next work cycle simultaneously. This creates a continuous, self-regulating loop where the workers coordinate their starts based on the completion of the entire team.

sha: 48b65acb1b1a4809fc6345ca4ad39c17fecc9bd10ae287c9aa6c696b3429248a 
*/
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
// g++ -o thread thread.cpp && ./thread

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

constexpr int NUM_TRACKS = 4;

std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> stop_flag(false);
std::atomic<int> ready_count(0);

void track_worker(int track_id)
{
    while (!stop_flag) {
        // Wait for a signal to generate the next sample
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [track_id] { return ready_count == 0; });
        }

        int randMs = rand() % 1000;
        printf("Track %d generating sample in %d ms\n", track_id, randMs);
        std::this_thread::sleep_for(std::chrono::milliseconds(randMs));
        printf("Track %d finished in %d ms\n", track_id, randMs);

        // Notify that this track is done
        {
            std::unique_lock<std::mutex> lock(mtx);
            ready_count++;
            if (ready_count == NUM_TRACKS) {
                cv.notify_all(); // Signal the main thread
            }
        }
    }
}

int main()
{
    std::vector<std::thread> threads;

    // Launch track threads
    for (int i = 0; i < NUM_TRACKS; ++i) {
        threads.emplace_back(track_worker, i);
    }

    for (int loop = 0; loop < 5; ++loop) {
        // Notify threads to process the next sample
        {
            std::unique_lock<std::mutex> lock(mtx);
            ready_count = 0;
            cv.notify_all();
        }

        printf("Wait for all threads to finish generating their samples in Loop %d\n", loop);

        // Wait for all threads to finish generating their samples
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return ready_count == NUM_TRACKS; });
        }

        printf("All threads finished generating their samples in Loop %d\n", loop);
    }

    // Stop the worker threads
    stop_flag = true;
    ready_count = 0;
    cv.notify_all();

    // Join threads
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
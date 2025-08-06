// main.cpp version for MCU IDE, no need to use FreeRTOS in this file
// only change timing functions, keep all logic
#ifdef USE_FREERTOS
	#include "freeRTOS/FreeRTOS.h"
	#include "freeRTOS/task.h"
	#include "esp_task_wdt.h"
#else
#include <thread>
#include <chrono>
#endif

#include <functional>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>

// Include from BlockB++
#include "music_data_handler.hpp"
#include "data_receive_handler.hpp"

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


inline void portable_sleep_ms(uint32_t ms) {
#ifdef USE_FREERTOS
    vTaskDelay(pdMS_TO_TICKS(ms));
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

// Cross-platform time wrapper
inline uint32_t portable_uptime_ms() {
#ifdef USE_FREERTOS
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
#else
    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
#endif
}


class RandomMusicDataGenerator {
private:
    static constexpr int valid_genre_ids[] = {0, 1, 2, 3, 4};
    static constexpr int num_genres = sizeof(valid_genre_ids) / sizeof(valid_genre_ids[0]);
    
    // Random number generator
    mutable std::mt19937 rng_;
    mutable std::uniform_real_distribution<float> float_dist_;
    mutable std::uniform_int_distribution<int> int_dist_;
    
public:
    RandomMusicDataGenerator() : rng_(std::chrono::steady_clock::now().time_since_epoch().count()) {}
    
    // Helper function to generate random float in range
    float random_float(float min, float max) const {
        return std::uniform_real_distribution<float>(min, max)(rng_);
    }
    
    // Helper function to generate random int in range
    int random_int(int min, int max) const {
        return std::uniform_int_distribution<int>(min, max)(rng_);
    }
    
    // Helper function to generate random choice from array
    int random_choice(const int* choices, int count) const {
        int index = random_int(0, count - 1);
        return choices[index];
    }
    
    // Helper function for Gaussian distribution
    float random_gauss(float mean, float stddev) const {
        std::normal_distribution<float> gauss_dist(mean, stddev);
        return gauss_dist(rng_);
    }

    MusicAnalyzedData generate_random_data() const {
        MusicAnalyzedData data = {};
        
        // EQ levels: random between -40 and 0 dB
        data.allpass_dB = random_float(-40.0f, 0.0f);
        data.LPF200_dB = random_float(-40.0f, 0.0f);
        data.BPF500_dB = random_float(-40.0f, 0.0f);
        data.BPF2000_dB = random_float(-40.0f, 0.0f);
        data.BPF4000_dB = random_float(-40.0f, 0.0f);
        data.HPF6000_dB = random_float(-40.0f, 0.0f);
        
        // Genre ID: random choice from valid genres (0-4)
        data.genreID = random_choice(valid_genre_ids, num_genres);
        
        // Surround score: 0 or 1
        data.surround_score = random_int(0, 1);
        
        // Beat: 0 or 1 (boolean as int)
        data.beat = random_int(0, 1);
        
        // Tempo: between 60 and 180 BPM
        data.tempo = random_float(60.0f, 180.0f);
        
        // Tempo confidence: between 0 and 1
        data.tempo_confidence = random_float(0.0f, 1.0f);
        
        return data;
    }
    
    MusicAnalyzedData simulate_beat_pattern(float beat_probability = 0.25f) const {
        MusicAnalyzedData data = generate_random_data();
        
        // Create more realistic beat pattern
        data.beat = (random_float(0.0f, 1.0f) < beat_probability) ? 1 : 0;
        
        // Make EQ levels change more naturally using Gaussian distribution
        data.allpass_dB = std::fmax(-40.0f, std::fmin(0.0f, random_gauss(-20.0f, 10.0f)));
        data.LPF200_dB = std::fmax(-40.0f, std::fmin(0.0f, random_gauss(-20.0f, 10.0f)));
        data.BPF500_dB = std::fmax(-40.0f, std::fmin(0.0f, random_gauss(-20.0f, 10.0f)));
        data.BPF2000_dB = std::fmax(-40.0f, std::fmin(0.0f, random_gauss(-20.0f, 10.0f)));
        data.BPF4000_dB = std::fmax(-40.0f, std::fmin(0.0f, random_gauss(-20.0f, 10.0f)));
        data.HPF6000_dB = std::fmax(-40.0f, std::fmin(0.0f, random_gauss(-20.0f, 10.0f)));
        
        // Add some "excitement" occasionally (5% chance of drop/exciting moment)
        if (random_float(0.0f, 1.0f) < 0.05f) {
            // Boost bass and overall volume
            data.allpass_dB = random_float(-10.0f, 0.0f);  // Boost overall
            data.LPF200_dB = random_float(-10.0f, 0.0f);   // Boost bass
            data.beat = 1;  // Force beat
            
            std::cout << "*** EXCITING MOMENT! Bass boost applied ***" << std::endl;
        }
        
        return data;
    }
    
    void print_data_info(const MusicAnalyzedData& data, int frame_count) const {
        // Print status every ~100 frames (similar to Python version)
        if (frame_count % 100 == 0) {
            std::cout << "Frame " << frame_count 
                      << ": EQ[" << std::fixed << std::setprecision(1)
                      << data.allpass_dB << "," << data.LPF200_dB << "," << data.BPF500_dB << ","
                      << data.BPF2000_dB << "," << data.BPF4000_dB << "," << data.HPF6000_dB << "] "
                      << "Genre:" << data.genreID << " Beat:" << data.beat 
                      << " Tempo:" << data.tempo << " Conf:" << std::setprecision(2) << data.tempo_confidence 
                      << std::endl;
        }
    }
};

void create_default_segments(std::shared_ptr<LightEffect> effect, int count = 10) {
    // Python: def create_default_segments(self, effect: LightEffect, count: int = 10):
    for (int i = 1; i <= count; i++) {
        // Python: segment = LightSegment(
        //     segment_ID=i,
        //     color=[i % 6, i % 6, i % 6, i % 6],
        //     transparency=DEFAULT_TRANSPARENCY,
        //     length=DEFAULT_LENGTH,
        //     move_speed=DEFAULT_MOVE_SPEED,
        //     move_range=DEFAULT_MOVE_RANGE,
        //     initial_position=DEFAULT_INITIAL_POSITION,
        //     is_edge_reflect=DEFAULT_IS_EDGE_REFLECT,
        //     dimmer_time=DEFAULT_DIMMER_TIME,
        //     dimmer_time_ratio=1.0
        // )
        auto segment = std::make_shared<LightSegment>(
            i,                                      // segment_ID
            std::vector<int>{i % 6, i % 6, i % 6, i % 6}, // color
            std::vector<float>{1.0f},              // transparency (DEFAULT_TRANSPARENCY)
            std::vector<int>{10},                  // length (DEFAULT_LENGTH)
            1.0f,                                  // move_speed (DEFAULT_MOVE_SPEED)
            std::vector<int>{0, 100},              // move_range (DEFAULT_MOVE_RANGE)
            0.0f,                                  // initial_position (DEFAULT_INITIAL_POSITION)
            true,                                  // is_edge_reflect (DEFAULT_IS_EDGE_REFLECT)
            std::vector<int>{0, 100, 200, 100, 500}, // dimmer_time (DEFAULT_DIMMER_TIME)
            1.0f                                   // dimmer_time_ratio
        );
        
        // Python: segment.gradient = False
        // Python: segment.fade = False
        // Note: Set these if your LightSegment class has these properties
        // segment->set_gradient(false);
        // segment->set_fade(false);

        // Python: effect.add_segment(i, segment)
        effect->add_segment(i, segment);

        std::cout << "Created default segment " << i << std::endl;
    }
}

void create_default_effects(std::shared_ptr<LightScene> scene, int num_effects = 8) {
    // Python: def create_default_effects(self, scene: LightScene, num_effects: int = 8):
    for (int effect_id = 1; effect_id <= num_effects; effect_id++) {
        // Python: effect = LightEffect(
        //     effect_ID=effect_id,
        //     led_count=DEFAULT_LED_COUNT,
        //     fps=DEFAULT_FPS
        // )
        auto effect = std::make_shared<LightEffect>(
            effect_id,          // effect_ID
            300,               // led_count (DEFAULT_LED_COUNT)
            20                 // fps (DEFAULT_FPS)
        );
        
        // Python: self.create_default_segments(effect)
        create_default_segments(effect);

        // Python: scene.add_effect(effect_id, effect)
        scene->add_effect(effect_id, effect);

        std::cout << "Created default effect " << effect_id << " with 10 segments" << std::endl;
    }
}

std::shared_ptr<LightScene> initialize_default_scene() {
    // Python: self.light_scenes = {1: LightScene(scene_ID=1)}
    auto scene = std::make_shared<LightScene>(1); // Create scene with ID 1

    // Python: scene = self.light_scenes[1]
    // Python: self.create_default_effects(scene)
    create_default_effects(scene);
    
    // Set current effect ID to first effect (like Python does)
    auto effects = scene->get_effects();
    if (!effects.empty()) {
        scene->set_current_effect_id(effects.begin()->first);
        std::cout << "Set current effect ID to: " << effects.begin()->first << std::endl;
    }
    
    std::cout << "Initialized default scene 1 with " << effects.size() << " effects" << std::endl;

    return scene;
}

extern "C" void app_main(void)
{
    std::cout << "Yamaha Lighting System Starting..." << std::endl;
#ifdef USE_FREERTOS
    std::cout << "Running on freeRTOS" << std::endl;
    
   
#else
    std::cout << "Running on standard C++ environment" << std::endl;
#endif
    std::cout << "Using random music data generation (based on random_osc_sender.py)" << std::endl;

    MusicDataHandler music_handler;

    // IMPORTANT: Initialize default scene with effects and segments (like Python)
    auto scene = initialize_default_scene();
    DataReceiveHandler data_receive_handler({{1, scene}}); // Pass scene with ID 1
    
    data_receive_handler.register_output_callback_function([](const BlockCOutputData &data) {
        // Check if this is LED binary data
        if (data.address.find("/light/serial/") == 0) {
            if (std::holds_alternative<std::vector<uint8_t>>(data.payload)) {
                auto binary_data = std::get<std::vector<uint8_t>>(data.payload);

                std::cout << data.address << std::endl;
            }
        } else {
            std::cout << "Received data: address=" << data.address << std::endl;
        }
    });
    
    music_handler.register_callback_function([&data_receive_handler](const BlockBOutputData &data) {
        BlockCInputData input_data;
        input_data.address = data.address;
        input_data.data = data.data;
        data_receive_handler.handle_input_data(input_data);
    });

    RandomMusicDataGenerator generator;
    int frame_count = 0;
    uint32_t start_time = portable_uptime_ms();
    
    // Target FPS and frame timing
    constexpr float TARGET_FPS = 20.0f;
    constexpr uint32_t TARGET_FRAME_TIME_MS = static_cast<uint32_t>(1000.0f / TARGET_FPS); // 50ms per frame
    
    // Performance tracking
    uint32_t total_execution_time = 0;
    uint32_t total_sleep_time = 0;
    uint32_t max_frame_time = 0;
    uint32_t min_frame_time = UINT32_MAX;

    std::cout << "Target FPS: " << TARGET_FPS << " (" << TARGET_FRAME_TIME_MS << "ms per frame)" << std::endl;

    // Add debug output to verify scene initialization
    std::cout << "=== Scene Initialization Check ===" << std::endl;
    // This would require adding a debug method to DataReceiveHandler to check scene state
    // For now, just proceed with the main loop

    while (1) {
        uint32_t frame_start_time = portable_uptime_ms();
        
        // Generate realistic music data (similar to Python's simulate_beat_pattern)
        MusicAnalyzedData music_data = generator.simulate_beat_pattern();
        
        // Print periodic status
        generator.print_data_info(music_data, frame_count);
        
        // Process the music data
        music_handler.music_data_handle(music_data);
        
        // Send LED binary data every frame (since we're at 20 FPS, this is appropriate)
        data_receive_handler.send_led_binary_data();
        
        // Calculate execution time for this frame
        uint32_t frame_end_time = portable_uptime_ms();
        uint32_t frame_execution_time = frame_end_time - frame_start_time;
        
        // Track performance statistics
        total_execution_time += frame_execution_time;
        max_frame_time = std::max(max_frame_time, frame_execution_time);
        min_frame_time = std::min(min_frame_time, frame_execution_time);
        
        // Calculate how long to sleep to maintain target FPS
        uint32_t sleep_time = 0;
        if (frame_execution_time < TARGET_FRAME_TIME_MS) {
            sleep_time = TARGET_FRAME_TIME_MS - frame_execution_time;
            total_sleep_time += sleep_time;
            portable_sleep_ms(sleep_time);
        } else {
            // Frame took longer than target - warn about performance issue
            if (frame_count % 100 == 0) {  // Don't spam warnings
                std::cout << "WARNING: Frame " << frame_count 
                          << " took " << frame_execution_time << "ms (target: " 
                          << TARGET_FRAME_TIME_MS << "ms) - FPS will be lower than target!" << std::endl;
            }
        }
        
        frame_count++;
        
        // Print detailed statistics every 1000 frames
        if (frame_count % 1000 == 0) {
            uint32_t total_elapsed_ms = portable_uptime_ms() - start_time;
            float actual_fps = (float)frame_count / ((float)total_elapsed_ms / 1000.0f);
            float avg_execution_time = (float)total_execution_time / frame_count;
            float avg_sleep_time = (float)total_sleep_time / frame_count;
            float cpu_utilization = ((float)total_execution_time / total_elapsed_ms) * 100.0f;
            
            std::cout << "=== Performance Stats (Frame " << frame_count << ") ===" << std::endl;
            std::cout << "  Total time: " << total_elapsed_ms << "ms" << std::endl;
            std::cout << "  Actual FPS: " << std::fixed << std::setprecision(1) << actual_fps 
                      << " (target: " << TARGET_FPS << ")" << std::endl;
            std::cout << "  Avg execution time: " << std::setprecision(2) << avg_execution_time 
                      << "ms (avg sleep: " << avg_sleep_time << "ms)" << std::endl;
            std::cout << "  Frame time range: " << min_frame_time << "-" << max_frame_time << "ms" << std::endl;
            std::cout << "  CPU utilization: " << std::setprecision(1) << cpu_utilization << "%" << std::endl;
            
            // Check if we're meeting target FPS
            if (actual_fps < TARGET_FPS * 0.95f) {  // 5% tolerance
                std::cout << "  ⚠️  FPS below target - consider optimizing code!" << std::endl;
            } else if (actual_fps > TARGET_FPS * 1.05f) {
                std::cout << "  ℹ️  FPS above target - timing accuracy issue?" << std::endl;
            } else {
                std::cout << "  ✅ FPS on target" << std::endl;
            }
            std::cout << "================================================" << std::endl;
            
            // Reset min/max for next period
            min_frame_time = UINT32_MAX;
            max_frame_time = 0;
        }
        
        // Print brief status every 100 frames
        else if (frame_count % 100 == 0) {
            uint32_t elapsed_ms = portable_uptime_ms() - start_time;
            float current_fps = (float)frame_count / ((float)elapsed_ms / 1000.0f);
            std::cout << "Frame " << frame_count << " - FPS: " << std::fixed << std::setprecision(1) 
                      << current_fps << " (exec: " << frame_execution_time << "ms, sleep: " 
                      << sleep_time << "ms)" << std::endl;
        }
    }
    
 //   return 0;
}

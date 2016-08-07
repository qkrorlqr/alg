#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

struct TTimer {
    using TClock = std::chrono::steady_clock;
    using TTimeUnit = std::chrono::microseconds;
    using TTime = TClock::time_point;
    using TTimeDiff = TTimeUnit::rep;

    TTime Time;

    TTimer()
        : Time(TClock::now())
    {
    }

    TTimeDiff Get() {
        auto diff = std::chrono::duration_cast<TTimeUnit>(TClock::now() - Time);
        Time = TClock::now();
        return diff.count();
    }
};

using TType = uint32_t;
using TData = std::vector<TType>;
const uint32_t RADIX_SIZE = 4;
const size_t BUCKET_COUNT = 1 << RADIX_SIZE;
const uint32_t END_POS = sizeof(TType) * 8 / RADIX_SIZE;
using TBuckets = TData[BUCKET_COUNT];

template <uint32_t pos>
void BucketSort(TBuckets& buckets, TData& data) {
    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
        buckets[i].clear();
    }

    for (const auto& x : data) {
        uint32_t r = (x >> (pos * RADIX_SIZE)) & (BUCKET_COUNT - 1);
        buckets[r].push_back(x);
    }

    size_t idx = 0;
    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
        for (const auto& x : buckets[i]) {
            data[idx++] = x;
        }
    }
}

template <uint32_t pos>
void BucketSortIteration(TBuckets& buckets, TData& data) {
    BucketSort<pos>(buckets, data);
    BucketSortIteration<pos + 1>(buckets, data);
}

template <>
void BucketSortIteration<END_POS>(TBuckets&, TData&) {
}

void RadixSort(TData& data) {
    TBuckets buckets;
    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
        buckets[i].reserve(2 * data.size() / BUCKET_COUNT);
    }
    BucketSortIteration<0>(buckets, data);
}

int main(int argc, const char** argv) {
    std::default_random_engine generator;
    std::uniform_int_distribution<TType> distribution(0, std::numeric_limits<TType>::max());

    size_t dataSize = argc > 1 ? std::atol(argv[1]) : 1000000;

    TData data;
    for (size_t i = 0; i < dataSize; ++i) {
        data.push_back(distribution(generator));
    }

    TData data1 = data;
    TData data2 = data;

    TTimer timer;

    std::sort(data1.begin(), data1.end());
    auto stdSortTime = timer.Get();

    RadixSort(data2);
    auto radixSortTime = timer.Get();

    for (size_t i = 0; i < data.size(); ++i) {
        if (data1[i] != data2[i]) {
            std::cerr << "mismatch at pos " << i << ": " << data1[i] << " != " << data2[i] << std::endl;
        }
    }

    std::cout << "std::sort time: " << stdSortTime << std::endl;
    std::cout << "RadixSort time: " << radixSortTime << std::endl;

    return 0;
}

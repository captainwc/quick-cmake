#ifndef SK_UTILS_RANDOM_UTILS_H
#define SK_UTILS_RANDOM_UTILS_H

#include <random>
#include <vector>

#include "macro.h"

namespace sk::utils {

#define RANDPOOL sk::utils::RandomUtil::getInstance()

// Sigleton
class RandomUtil {
public:
    int    getRandomInt(int lower = 0, int upper = 100);
    double getRandomDouble(double lower = 0.0, double upper = 100.0);

    std::string         getRandomString(size_t length, const std::string& charset = lowerAlpha);
    std::vector<int>    getRandomIntVector(size_t size, int lower = 0, int upper = 100);
    std::vector<double> getRandomDoubleVector(size_t size, double lower = 0.0, double upper = 100.0);

    static RandomUtil& getInstance() {
        static RandomUtil pool;
        return pool;
    }

    RandomUtil(const RandomUtil&)            = delete;
    RandomUtil& operator=(const RandomUtil&) = delete;
    RandomUtil(RandomUtil&& p)               = default;
    RandomUtil& operator=(RandomUtil&& p)    = default;
    ~RandomUtil()                            = default;

    static const std::string upperAlpha;
    static const std::string lowerAlpha;
    static const std::string alpha;
    static const std::string upperAlphaNumeric;
    static const std::string lowerAlphaNumeric;

private:
    RandomUtil() = default;
    std::mt19937                           _gen{std::random_device{}()};
    std::uniform_int_distribution<int>     _intDistro{0, 10000};
    std::uniform_real_distribution<double> _doubleDistro{0.0, 1.0};
};

inline const std::string RandomUtil::upperAlpha{"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
inline const std::string RandomUtil::lowerAlpha{"abcdefghijklmnopqrstuvwxyz"};
inline const std::string RandomUtil::alpha{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};
inline const std::string RandomUtil::upperAlphaNumeric{"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
inline const std::string RandomUtil::lowerAlphaNumeric{"abcdefghijklmnopqrstuvwxyz0123456789"};

inline int RandomUtil::getRandomInt(int lower, int upper) {
    return _intDistro(_gen) % (upper - lower + 1) + lower;
}

inline double RandomUtil::getRandomDouble(double lower, double upper) {
    return _doubleDistro(_gen) * (upper - lower) + lower;
}

inline std::vector<int> RandomUtil::getRandomIntVector(size_t size, int lower, int upper) {
    std::vector<int> vc(size);
    for (int i = 0; i < size; i++) {
        vc[i] = getRandomInt(lower, upper);
    }
    return vc;
}

inline std::vector<double> RandomUtil::getRandomDoubleVector(size_t size, double lower, double upper) {
    std::vector<double> vc(size);
    for (int i = 0; i < size; i++) {
        vc[i] = getRandomDouble(lower, upper);
    }
    return vc;
}

inline std::string RandomUtil::getRandomString(size_t length, const std::string& charset) {
    auto        len = charset.length();
    std::string ret;
    for (int i = 0; i < length; i++) {
        ret += charset[getRandomInt(0, len - 1)];
    }
    return ret;
}

}  // namespace sk::utils

#endif  // SK_UTILS_RANDOM_UTILS_H
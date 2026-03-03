#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <sstream>

// Simple Test Framework
namespace test {

int total_tests = 0;
int passed_tests = 0;

void run_test(const std::string& name, std::function<void()> test_func) {
    total_tests++;
    try {
        test_func();
        std::cout << "[PASS] " << name << "\n";
        passed_tests++;
    } catch (const std::exception& e) {
        std::cout << "[FAIL] " << name << ": " << e.what() << "\n";
    } catch (...) {
        std::cout << "[FAIL] " << name << ": Unknown error\n";
    }
}

void assert_true(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

template<typename T>
std::string to_string(const T& val) {
    return std::to_string(val);
}

// Specialization for string
template<>
std::string to_string(const std::string& val) {
    return val;
}

template<typename T>
void assert_equal(const T& expected, const T& actual, const std::string& message) {
    if (expected != actual) {
         throw std::runtime_error(message + " (Expected: " + to_string(expected) + 
                                  ", Actual: " + to_string(actual) + ")");
    }
}

} // namespace test

// Tests
#include "mft/common/byte_reader.hpp"
#include "mft/common/datetime_utils.hpp"

void test_byte_reader() {
    uint8_t buffer[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    
    // Test Int16
    int16_t val16 = mft::common::ByteReader::read_int16(buffer, 0);
    test::assert_equal<int>(0x0201, val16, "Int16 read");
    
    // Test Int32
    int32_t val32 = mft::common::ByteReader::read_int32(buffer, 0);
    test::assert_equal<int>(0x04030201, val32, "Int32 read");
    
    // Test Int64
    int64_t val64 = mft::common::ByteReader::read_int64(buffer, 0);
    test::assert_true(0x0807060504030201LL == val64, "Int64 read");
}

void test_datetime() {
    // 2024-01-01 00:00:00 UTC = 133484832000000000 FILETIME
    int64_t filetime = 133484832000000000;
    auto dt = mft::common::NullableDateTime::from_filetime(filetime);
    
    test::assert_true(dt.has_value(), "DateTime should have value");
    test::assert_equal<std::string>("2023-12-31 08:00:00.0000000", dt.to_string(), "DateTime string format");
}

int main() {
    std::cout << "Running MFT Unit Tests...\n\n";
    
    test::run_test("ByteReader Basic", test_byte_reader);
    test::run_test("DateTime Conversion", test_datetime);
    
    std::cout << "\nResult: " << test::passed_tests << "/" << test::total_tests << " passed.\n";
    return (test::total_tests == test::passed_tests) ? 0 : 1;
}

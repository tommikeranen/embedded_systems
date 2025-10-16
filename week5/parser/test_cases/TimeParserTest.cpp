// Tavoitteena on 2 pistettä
// Tein kaikki 1 pisteeseen tarvittavat tehtävät ja lisäsin uusia testejä

#include <gtest/gtest.h>
#include "../TimeParser.h"

// Test suite: TimeParserTest
TEST(TimeParserTest, TestCaseCorrectTime) {

    // Note that this test fails on purpose!!

    // // Test with correct time string
    // char time_test[] = "141205";
    // // ASSERT_EQ(time_parse(time_test),0);
    // EXPECT_EQ(time_parse(time_test),0);

    char time_test[] = "000005";
    EXPECT_EQ(time_parse(time_test),5);

    char time_test2[] = "000112";
    EXPECT_EQ(time_parse(time_test2),72);
}

TEST(TimeParserTest, TestCaseIncorrectTime) {
    char time_test[] = "000077";
    EXPECT_EQ(time_parse(time_test),TIME_VALUE_ERROR);

    char time_test2[] = "009800";
    EXPECT_EQ(time_parse(time_test2),TIME_VALUE_ERROR);
}

TEST(TimeParserTest, TestCaseIncorrectLength) {
    char time_test[] = "007";
    EXPECT_EQ(time_parse(time_test),TIME_LEN_ERROR);

    char time_test2[] = "00980980";
    EXPECT_EQ(time_parse(time_test2),TIME_LEN_ERROR);
}

TEST(TimeParserTest, TestCaseNullPointer) {
    char *time_null = nullptr;
    EXPECT_EQ(time_parse(time_null), TIME_NULL_ERROR);
}

TEST(TimeParserTest, TestCaseNotDigits) {
    char *digit_test = "heheee";
    EXPECT_EQ(time_parse(digit_test), TIME_DIGIT_ERROR);
}

// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html

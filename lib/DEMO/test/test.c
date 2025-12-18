#include "test.h"
#include "log.h"


void test(void)
{
    // 测试日志
    LOG_INFO("This is a test log");
    LOG_WARNING("This is a test log");
    LOG_ERROR("This is a test log");
    LOG_DEBUG("This is a test log");


    // 测试字符串
    char *str = "Hello, World!";
}
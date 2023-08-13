#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include <tbox/base/log_output.h>

#include "raw_proto.h"

namespace tbox {
namespace jsonrpc {

TEST(RawProto, sendRequest) {
    LogOutput_Enable();

    RawProto proto;
    Proto::Callbacks cbs;

    int count = 0;
    cbs.recv_request_cb = \
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, Json());
            ++count;
        };
    cbs.recv_respond_cb = \
        [&] (int id, int errcode, const Json &js_result) { ++count; };
    cbs.send_data_cb = \
        [&] (const void *data_ptr, size_t data_size) {
            proto.onRecvData(data_ptr, data_size);
        };

    proto.setCallbacks(cbs);

    proto.sendRequest(1, "test");
    EXPECT_EQ(count, 1);

    LogOutput_Disable();
}

TEST(RawProto, sendRequestWithParams) {
    Json js_send_params = {
        {"a", 123},
        {"b", {"hello", "world", "!"}},
    };
    LogOutput_Enable();

    RawProto proto;
    Proto::Callbacks cbs;

    int count = 0;
    cbs.recv_request_cb = \
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, js_send_params);
            ++count;
        };
    cbs.recv_respond_cb = \
        [&] (int id, int errcode, const Json &js_result) { ++count; };
    cbs.send_data_cb = \
        [&] (const void *data_ptr, size_t data_size) {
            proto.onRecvData(data_ptr, data_size);
        };

    proto.setCallbacks(cbs);

    proto.sendRequest(1, "test", js_send_params);
    EXPECT_EQ(count, 1);

    LogOutput_Disable();
}

TEST(RawProto, sendResult) {
    Json js_send_result = {
        {"a", 123},
        {"b", {"hello", "world", "!"}},
    };
    LogOutput_Enable();

    RawProto proto;
    Proto::Callbacks cbs;

    int count = 0;
    cbs.recv_request_cb = \
        [&] (int id, const std::string &method, const Json &js_params) { ++count; };
    cbs.recv_respond_cb = \
        [&] (int id, int errcode, const Json &js_result) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(js_result, js_send_result);
            ++count;
        };
    cbs.send_data_cb = \
        [&] (const void *data_ptr, size_t data_size) {
            proto.onRecvData(data_ptr, data_size);
        };

    proto.setCallbacks(cbs);

    proto.sendResult(1, js_send_result);
    EXPECT_EQ(count, 1);

    LogOutput_Disable();
}

TEST(RawProto, sendError) {
    LogOutput_Enable();

    RawProto proto;
    Proto::Callbacks cbs;

    int count = 0;
    cbs.recv_request_cb = \
        [&] (int id, const std::string &method, const Json &js_params) { ++count; };
    cbs.recv_respond_cb = \
        [&] (int id, int errcode, const Json &) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(errcode, -1000);
            ++count;
        };
    cbs.send_data_cb = \
        [&] (const void *data_ptr, size_t data_size) {
            proto.onRecvData(data_ptr, data_size);
        };

    proto.setCallbacks(cbs);

    proto.sendError(1, -1000);
    EXPECT_EQ(count, 1);

    LogOutput_Disable();
}

TEST(RawProto, RecvUncompleteData) {
    LogOutput_Enable();

    RawProto proto;
    Proto::Callbacks cbs;

    int count = 0;
    cbs.recv_request_cb = \
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, Json());
            ++count;
        };

    proto.setCallbacks(cbs);

    const char *str_1 = R"({"id":1,"meth)";
    const char *str_2 = R"({"id":1,"method":"test","jsonrpc":"2.0"})";
    EXPECT_EQ(proto.onRecvData(str_1, ::strlen(str_1)), 0);
    EXPECT_EQ(proto.onRecvData(str_2, ::strlen(str_2)), ::strlen(str_2));

    EXPECT_EQ(count, 1);
    LogOutput_Disable();
}

}
}
#include <stdlib.h>
#include <stdio.h>
#include <pomelo.h>
#include <stdbool.h>

#include "test_common.h"

#define SESSION_DATA "{\"Data\":{\"animal\":2}}"

static pc_client_t *g_client = NULL;

typedef struct {
    bool called;
    char *data;
} session_cb_data_t;

static void
event_cb(pc_client_t* client, int ev_type, void* ex_data, const char* arg1, const char* arg2)
{
    Unused(client); Unused(arg1); Unused(arg2);
    bool *connected = (bool*)ex_data;
    *connected = true;
    assert_int(ev_type, ==, PC_EV_CONNECTED);
}

static void
set_session_request_cb(const pc_request_t* req, const char* resp)
{
    bool *called = (bool*)pc_request_ex_data(req);
    *called = true;
    assert_string_equal(resp, SUCCESS_RESP);
    assert_not_null(req);
}

static void
get_session_request_cb(const pc_request_t* req, const char* resp)
{
    session_cb_data_t *scd = (session_cb_data_t*)pc_request_ex_data(req);
    scd->called = true;
    assert_string_equal(resp, scd->data);
    assert_not_null(req);
}

void
do_test_session_persistence(pc_client_config_t *config, int port)
{
    pc_client_init_result_t res = pc_client_init(NULL, config);
    g_client = res.client;
    assert_int(res.rc, ==, PC_RC_OK);

    bool connected = false;
    int handler_id = pc_client_add_ev_handler(g_client, event_cb, &connected, NULL);
    assert_int(handler_id, !=, PC_EV_INVALID_HANDLER_ID);

    assert_int(pc_client_connect(g_client, LOCALHOST, port, NULL), ==, PC_RC_OK);
    SLEEP_SECONDS(1);

    assert_true(connected);

    session_cb_data_t session_cb_data;
    session_cb_data.called = false;
    session_cb_data.data = EMPTY_RESP;

    // TODO: why passing NULL msg is an invalid argument?
    assert_int(pc_request_with_timeout(g_client, "connector.getsessiondata", NULL, NULL, REQ_TIMEOUT, get_session_request_cb, NULL), ==, PC_RC_INVALID_ARG);

    // Get empty session and check that the callback was called.
    assert_int(pc_request_with_timeout(g_client, "connector.getsessiondata", "{}", &session_cb_data, REQ_TIMEOUT, get_session_request_cb, NULL), ==, PC_RC_OK);
    SLEEP_SECONDS(1);
    assert_true(session_cb_data.called);

    session_cb_data.called = false;
    session_cb_data.data = SESSION_DATA;

    // Set session and check that the callback was called.
    bool set_session_cb_called = false;
    assert_int(pc_request_with_timeout(g_client, "connector.setsessiondata", SESSION_DATA, &set_session_cb_called, REQ_TIMEOUT, set_session_request_cb, NULL), ==, PC_RC_OK);
    SLEEP_SECONDS(1);
    assert_true(set_session_cb_called);

    // Get session and check that the callback was called.
    assert_int(pc_request_with_timeout(g_client, "connector.getsessiondata", "{}", &session_cb_data, REQ_TIMEOUT, get_session_request_cb, NULL), ==, PC_RC_OK);
    SLEEP_SECONDS(1);
    assert_true(session_cb_data.called);

    session_cb_data.called = false;
    assert_int(pc_request_with_timeout(g_client, "connector.getsessiondata", "{}", &session_cb_data, REQ_TIMEOUT, get_session_request_cb, NULL), ==, PC_RC_OK);
    SLEEP_SECONDS(1);
    assert_true(session_cb_data.called);

    pc_client_disconnect(g_client);
    pc_client_rm_ev_handler(g_client, handler_id);
    pc_client_cleanup(g_client);
}

static MunitResult
test_persistence(const MunitParameter params[], void *data)
{
    Unused(data); Unused(params);
    pc_client_config_t config = PC_CLIENT_CONFIG_TEST;
    // Test with TCP
    do_test_session_persistence(&config, g_test_server.tcp_port);
    // Test with TLS
    config.transport_name = PC_TR_NAME_UV_TLS;
    assert_int(tr_uv_tls_set_ca_file("../../../test/server/fixtures/ca.crt", NULL), ==, PC_RC_OK);
    do_test_session_persistence(&config, g_test_server.tls_port);

    return MUNIT_OK;
}

static MunitTest tests[] = {
    {"/persistence", test_persistence, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

const MunitSuite session_suite = {
    "/session", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

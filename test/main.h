#ifndef __MAIN_H__
#define __MAIN_H__

#define NUM_SUITES 9

extern const MunitSuite pc_client_suite;
extern const MunitSuite tcp_suite;
extern const MunitSuite tls_suite;
extern const MunitSuite session_suite;
extern const MunitSuite reconnection_suite;
extern const MunitSuite compression_suite;
extern const MunitSuite kick_suite;
extern const MunitSuite request_suite;
extern const MunitSuite notify_suite;

#endif // __MAIN_H__

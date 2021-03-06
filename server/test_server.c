/* This file is part of the Project Athena Zephyr Notification System.
 * It contains the server unit tests.
 *
 *	Created by:	Karl Ramm
 *
 *	Copyright (c) 1987,1988,1991 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h".
 */

#include <stdio.h>
#include <stdlib.h>

#include "zserver.h"

int ulogin_add_user(ZNotice_t *notice, Exposure_type exposure,
                    struct sockaddr_in *who);
Exposure_type ulogin_remove_user(ZNotice_t *notice,
                                 struct sockaddr_in *who,
                                 int *err_return);
int ulogin_find(char *user, struct in_addr *host,
                      unsigned int port);
int ulogin_find_user(char *user);
void ulogin_flush_user(ZNotice_t *notice);
extern Location *locations;

#define TEST(EXP) \
    do { \
        printf("%s:%d: %s: ", __FILE__, __LINE__, #EXP); \
        fflush(stdout); \
        if (EXP) {        \
            puts("PASS"); \
        } else {          \
            puts("FAIL"); \
            failures++; \
        } \
        fflush(stdout); \
    } while (0)

#define V(EXP)                                  \
    do { \
        printf("%s:%d: %s\n", __FILE__, __LINE__, #EXP); \
        fflush(stdout); \
        EXP; \
    } while (0)


int failures = 0;

void test_uloc(void);

int
main(int argc, char **argv)
{
    puts("Zephyr server testing");
    puts("");

    test_uloc();

    exit(!(failures == 0));
}

void
test_uloc(void)
{
    ZNotice_t z1, z2, z0, z4;
    String *s1, *s2, *s0, *s4;
    struct sockaddr_in who1, who2, who3, who0, who4;
    int ret;

    puts("uloc storage routines");

    TEST(ulogin_find_user("nonexistent") == -1);

    /* fake up just enough */
    who1.sin_family = AF_INET;
    who1.sin_port = 1;
    who1.sin_addr.s_addr = INADDR_LOOPBACK;

    z1.z_class_inst = "user1";
    z1.z_port = 1;
    z1.z_message = "here\0now\0this\0";
    z1.z_message_len = 14;

    s1 = make_string(z1.z_class_inst, 0);

    TEST(ulogin_add_user(&z1, NET_ANN, &who1) == 0);
    TEST(ulogin_find_user("user1") != -1);

    who2.sin_family = AF_INET;
    who2.sin_port = 2;
    who2.sin_addr.s_addr = INADDR_LOOPBACK;

    z2.z_class_inst = "user2";
    z2.z_port = 2;
    z2.z_message = "here\0now\0this\0";
    z2.z_message_len = 14;

    s2 = make_string(z2.z_class_inst, 0);

    TEST(ulogin_add_user(&z2, NET_ANN, &who2) == 0);
    TEST(ulogin_find_user("user2") != -1);
    TEST(locations[ulogin_find_user("user1")].user == s1);
    TEST(locations[ulogin_find_user("user2")].user == s2);
    TEST(ulogin_add_user(&z1, NET_ANN, &who1) == 0);
    TEST(locations[ulogin_find_user("user1")].user == s1);
    TEST(locations[ulogin_find_user("user2")].user == s2);

    who3.sin_family = AF_INET;
    who3.sin_port = 3;
    who3.sin_addr.s_addr = INADDR_LOOPBACK;

    TEST(ulogin_find("user1", &who3.sin_addr, 3) == -1);

    who0.sin_family = AF_INET;
    who0.sin_port = 3;
    who0.sin_addr.s_addr = INADDR_LOOPBACK;

    z0.z_class_inst = "user0";
    z0.z_port = 3;
    z0.z_message = "here\0now\0this\0";
    z0.z_message_len = 14;

    s0 = make_string(z0.z_class_inst, 0);

    TEST(ulogin_add_user(&z0, NET_ANN, &who0) == 0);
    TEST(ulogin_find_user("user0") != -1);
    TEST(locations[ulogin_find_user("user1")].user == s1);
    TEST(locations[ulogin_find_user("user2")].user == s2);

    TEST(ulogin_remove_user(&z0, &who0, &ret) == NET_ANN && ret == 0);
    /* 1 = NOLOC */
    TEST(ulogin_remove_user(&z0, &who0, &ret) == NONE && ret == 1);

    TEST(ulogin_add_user(&z0, NET_ANN, &who0) == 0);
    TEST(ulogin_remove_user(&z1, &who0, &ret) == NET_ANN && ret == 0);

    V(ulogin_flush_user(&z0));
    TEST(ulogin_find_user("user0") == -1);

    TEST(ulogin_add_user(&z0, NET_ANN, &who0) == 0);
    TEST(ulogin_add_user(&z1, NET_ANN, &who1) == 0);
    V(ulogin_flush_user(&z1));
    TEST(ulogin_find_user("user1") == -1);

    who4.sin_family = AF_INET;
    who4.sin_port = 4;
    who4.sin_addr.s_addr = INADDR_ANY;

    z4.z_class_inst = "user4";
    z4.z_port = 4;
    z4.z_message = "here\0now\0this\0";
    z4.z_message_len = 14;

    s4 = make_string(z4.z_class_inst, 0);

    TEST(ulogin_add_user(&z4, NET_ANN, &who4) == 0);

    V(uloc_flush_client(&who2));
    TEST(locations[ulogin_find_user("user0")].user == s0);
    TEST(ulogin_find_user("user1") == -1);
    TEST(ulogin_find_user("user2") == -1);
    TEST(locations[ulogin_find_user("user4")].user == s4);

    V(uloc_hflush(&who0.sin_addr));
    TEST(ulogin_find_user("user0") == -1);
    TEST(ulogin_find_user("user1") == -1);
    TEST(ulogin_find_user("user2") == -1);
    TEST(locations[ulogin_find_user("user4")].user == s4);
}

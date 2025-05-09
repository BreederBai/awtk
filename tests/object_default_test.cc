﻿#include "tkc/utils.h"
#include "tkc/object_default.h"
#include "gtest/gtest.h"
#include <stdlib.h>
#include <string>

using std::string;

static ret_t event_dump(void* ctx, event_t* e) {
  string& str = *(string*)ctx;

  if (e->type == EVT_PROP_WILL_CHANGE || e->type == EVT_PROP_CHANGED) {
    prop_change_event_t* evt = (prop_change_event_t*)e;
    str += evt->name;
    str += ":";
  } else if (e->type == EVT_CMD_WILL_EXEC || e->type == EVT_CMD_EXECED ||
             e->type == EVT_CMD_CAN_EXEC) {
    cmd_exec_event_t* evt = (cmd_exec_event_t*)e;
    str += evt->name;
    str += ":";

    if (e->type == EVT_CMD_CAN_EXEC && tk_str_eq(evt->name, "ok")) {
      evt->can_exec = TRUE;
      return RET_STOP;
    }
  } else if (e->type == EVT_DESTROY) {
    str += "destroy:";
  }

  return RET_OK;
}

static ret_t visit_dump(void* ctx, const void* data) {
  string& str = *(string*)ctx;
  const named_value_t* nv = (named_value_t*)data;

  str += nv->name;

  return RET_OK;
}

TEST(ObjectDefault, events) {
  value_t v;
  string log;
  object_t* obj = object_default_create();
  object_default_t* o = OBJECT_DEFAULT(obj);

  emitter_on((emitter_t*)o, EVT_PROP_WILL_CHANGE, event_dump, &log);
  emitter_on((emitter_t*)o, EVT_PROP_CHANGED, event_dump, &log);
  emitter_on((emitter_t*)o, EVT_DESTROY, event_dump, &log);

  ASSERT_EQ(object_set_prop(obj, "6", value_set_int(&v, 50)), RET_OK);
  ASSERT_EQ(object_set_prop(obj, "8", value_set_int(&v, 50)), RET_OK);

  object_unref(obj);

  ASSERT_EQ(log, "6:6:8:8:destroy:");
}

TEST(ObjectDefault, basic) {
  value_t v;
  string log;
  object_t* obj = object_default_create();
  object_default_t* o = OBJECT_DEFAULT(obj);

  ASSERT_EQ(o->props.size, 0u);

  ASSERT_EQ(object_set_prop(obj, "5", value_set_int(&v, 50)), RET_OK);
  ASSERT_EQ(o->props.size, 1u);
  ASSERT_EQ(object_get_prop(obj, "5", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 50);

  ASSERT_EQ(object_set_prop(obj, "5", value_set_int(&v, 51)), RET_OK);
  ASSERT_EQ(o->props.size, 1u);
  ASSERT_EQ(object_get_prop(obj, "5", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 51);

  ASSERT_EQ(object_set_prop(obj, "6", value_set_int(&v, 60)), RET_OK);
  ASSERT_EQ(o->props.size, 2u);
  ASSERT_EQ(object_get_prop(obj, "6", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 60);

  ASSERT_EQ(object_set_prop(obj, "6", value_set_int(&v, 61)), RET_OK);
  ASSERT_EQ(o->props.size, 2u);
  ASSERT_EQ(object_get_prop(obj, "6", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 61);

  ASSERT_EQ(object_set_prop(obj, "4", value_set_int(&v, 40)), RET_OK);
  ASSERT_EQ(o->props.size, 3u);
  ASSERT_EQ(object_get_prop(obj, "4", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 40);

  ASSERT_EQ(object_set_prop(obj, "4", value_set_int(&v, 41)), RET_OK);
  ASSERT_EQ(o->props.size, 3u);
  ASSERT_EQ(object_get_prop(obj, "4", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 41);

  ASSERT_EQ(object_set_prop(obj, "2", value_set_int(&v, 20)), RET_OK);
  ASSERT_EQ(o->props.size, 4u);
  ASSERT_EQ(object_get_prop(obj, "2", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 20);

  ASSERT_EQ(object_set_prop(obj, "2", value_set_int(&v, 21)), RET_OK);
  ASSERT_EQ(o->props.size, 4u);
  ASSERT_EQ(object_get_prop(obj, "2", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 21);

  ASSERT_EQ(object_set_prop(obj, "3", value_set_int(&v, 30)), RET_OK);
  ASSERT_EQ(o->props.size, 5u);
  ASSERT_EQ(object_get_prop(obj, "3", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 30);

  ASSERT_EQ(object_set_prop(obj, "3", value_set_int(&v, 31)), RET_OK);
  ASSERT_EQ(o->props.size, 5u);
  ASSERT_EQ(object_get_prop(obj, "3", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 31);

  ASSERT_EQ(object_set_prop(obj, "9", value_set_int(&v, 90)), RET_OK);
  ASSERT_EQ(o->props.size, 6u);
  ASSERT_EQ(object_get_prop(obj, "9", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 90);

  ASSERT_EQ(object_set_prop(obj, "9", value_set_int(&v, 91)), RET_OK);
  ASSERT_EQ(o->props.size, 6u);
  ASSERT_EQ(object_get_prop(obj, "9", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 91);

  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "234569");

  ASSERT_EQ(object_remove_prop(obj, "3"), RET_OK);
  ASSERT_EQ(o->props.size, 5u);
  ASSERT_EQ(object_get_prop(obj, "3", &v), RET_NOT_FOUND);
  ASSERT_EQ(object_remove_prop(obj, "3"), RET_NOT_FOUND);

  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "24569");

  ASSERT_EQ(object_remove_prop(obj, "9"), RET_OK);
  ASSERT_EQ(o->props.size, 4u);
  ASSERT_EQ(object_get_prop(obj, "9", &v), RET_NOT_FOUND);
  ASSERT_EQ(object_remove_prop(obj, "9"), RET_NOT_FOUND);
  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "2456");

  ASSERT_EQ(object_remove_prop(obj, "4"), RET_OK);
  ASSERT_EQ(o->props.size, 3u);
  ASSERT_EQ(object_get_prop(obj, "4", &v), RET_NOT_FOUND);
  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "256");

  ASSERT_EQ(object_remove_prop(obj, "2"), RET_OK);
  ASSERT_EQ(o->props.size, 2u);
  ASSERT_EQ(object_get_prop(obj, "2", &v), RET_NOT_FOUND);
  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "56");

  ASSERT_EQ(object_remove_prop(obj, "5"), RET_OK);
  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "6");

  ASSERT_EQ(object_remove_prop(obj, "6"), RET_OK);
  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "");

  object_unref(obj);
}

static ret_t visit_remove_odd(void* ctx, const void* data) {
  const named_value_t* nv = (named_value_t*)data;
  int32_t n = tk_atoi(nv->name);

  return n % 2 ? RET_REMOVE : RET_OK;
}

static ret_t visit_remove_all(void* ctx, const void* data) {
  return RET_REMOVE;
}

static void test_busy(object_t* obj) {
  value_t v;
  ASSERT_EQ(object_set_prop(obj, "a", value_set_int(&v, 1)), RET_BUSY);
  ASSERT_EQ(object_get_prop(obj, "a", &v), RET_BUSY);
  ASSERT_EQ(object_remove_prop(obj, "a"), RET_BUSY);
}

static ret_t visit_test_busy(void* ctx, const void* data) {
  object_t* obj = (object_t*)(ctx);

  test_busy(obj);

  return RET_OK;
}

TEST(ObjectDefault, visis_remove) {
  value_t v;
  string log;
  object_t* obj = object_default_create();
  object_default_t* o = OBJECT_DEFAULT(obj);

  ASSERT_EQ(object_set_prop(obj, "2", value_set_int(&v, 2)), RET_OK);
  ASSERT_EQ(object_set_prop(obj, "1", value_set_int(&v, 1)), RET_OK);

  ASSERT_EQ(object_set_prop(obj, "4", value_set_int(&v, 4)), RET_OK);
  ASSERT_EQ(object_set_prop(obj, "3", value_set_int(&v, 3)), RET_OK);

  ASSERT_EQ(object_set_prop(obj, "6", value_set_int(&v, 6)), RET_OK);
  ASSERT_EQ(object_set_prop(obj, "5", value_set_int(&v, 5)), RET_OK);

  ASSERT_EQ(o->props.size, 6u);
  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "123456");

  object_foreach_prop(obj, visit_remove_odd, &log);
  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "246");
  ASSERT_EQ(o->props.size, 3u);

  object_foreach_prop(obj, visit_remove_all, &log);
  log = "";
  object_foreach_prop(obj, visit_dump, &log);
  ASSERT_EQ(log, "");
  ASSERT_EQ(o->props.size, 0u);

  object_foreach_prop(obj, visit_test_busy, obj);

  object_unref(obj);
}

TEST(ObjectDefault, random) {
  value_t v;
  string log;
  char name[32];
  int32_t i = 0;
  int32_t n = 1000;

  object_t* obj = object_default_create();

  srandom(time(0));

  for (i = 0; i < n; i++) {
    int32_t num = tk_abs((int32_t)random());
    tk_snprintf(name, sizeof(name), "%d", num);
    ASSERT_EQ(object_set_prop(obj, name, value_set_int(&v, num)), RET_OK);
    ASSERT_EQ(object_get_prop(obj, name, &v), RET_OK);
    ASSERT_EQ(value_int(&v), num);
  }

  object_unref(obj);
}

TEST(ObjectDefault, set_name) {
  object_t* obj = object_default_create();

  object_set_name(obj, "abc");
  ASSERT_EQ(obj->name, string("abc"));

  object_set_name(obj, "abc123");
  ASSERT_EQ(obj->name, string("abc123"));

  object_unref(obj);
}

TEST(ObjectDefault, set_prop_int) {
  object_t* obj = object_default_create();

  ASSERT_EQ(object_set_prop_int(obj, "int", 123), RET_OK);
  ASSERT_EQ(object_get_prop_int(obj, "int", 0), 123);

  object_unref(obj);
}

TEST(ObjectDefault, set_prop_float) {
  object_t* obj = object_default_create();

  ASSERT_EQ(object_set_prop_float(obj, "float", 123), RET_OK);
  ASSERT_EQ(object_get_prop_float(obj, "float", 0), 123);

  object_unref(obj);
}

TEST(ObjectDefault, copy_prop) {
  object_t* src = object_default_create();
  object_t* obj = object_default_create();

  ASSERT_EQ(object_set_prop_float(src, "float", 123), RET_OK);
  ASSERT_EQ(object_get_prop_float(src, "float", 0), 123);

  ASSERT_EQ(object_copy_prop(obj, src, "float"), RET_OK);
  ASSERT_EQ(object_get_prop_float(obj, "float", 0), 123);

  ASSERT_NE(object_copy_prop(obj, src, "not exist"), RET_OK);

  object_unref(src);
  object_unref(obj);
}

TEST(ObjectDefault, set_prop_str) {
  object_t* obj = object_default_create();

  ASSERT_EQ(object_set_prop_str(obj, "str", "123"), RET_OK);
  ASSERT_EQ(string(object_get_prop_str(obj, "str")), string("123"));

  object_unref(obj);
}

TEST(ObjectDefault, has_prop) {
  object_t* obj = object_default_create();

  ASSERT_EQ(object_set_prop_float(obj, "a", 123), RET_OK);
  ASSERT_EQ(object_has_prop(obj, "a"), TRUE);
  ASSERT_EQ(object_has_prop(obj, "A"), FALSE);

  object_unref(obj);
}

TEST(ObjectDefault, expr_number) {
  value_t v;
  object_t* obj = object_default_create();

  ASSERT_EQ(object_set_prop_float(obj, "aa", 123), RET_OK);
  ASSERT_EQ(object_set_prop_float(obj, "bb", 456), RET_OK);
  ASSERT_EQ(object_eval(obj, "$aa+$bb", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 123 + 456);

  ASSERT_EQ(object_eval(obj, "($aa+$bb)*2", &v), RET_OK);
  ASSERT_EQ(value_int(&v), (123 + 456) * 2);

  object_unref(obj);
}

TEST(ObjectDefault, expr_str) {
  value_t v;
  object_t* obj = object_default_create();

  ASSERT_EQ(object_set_prop_str(obj, "aa", "123"), RET_OK);
  ASSERT_EQ(object_set_prop_str(obj, "bb", "abc"), RET_OK);
  ASSERT_EQ(object_eval(obj, "$aa+$bb", &v), RET_OK);
  ASSERT_EQ(string(value_str(&v)), string("123abc"));

  value_reset(&v);
  object_unref(obj);
}

TEST(ObjectDefault, clone) {
  value_t v;
  object_t* clone = NULL;
  object_t* obj = object_default_create();

  ASSERT_EQ(object_set_prop_str(obj, "aa", "123"), RET_OK);
  ASSERT_EQ(object_set_prop_str(obj, "bb", "abc"), RET_OK);

  clone = object_clone(obj);
  ASSERT_EQ(object_eval(clone, "$aa+$bb", &v), RET_OK);
  ASSERT_EQ(string(value_str(&v)), string("123abc"));

  value_reset(&v);
  object_unref(obj);
  object_unref(clone);
}

TEST(ObjectDefault, path) {
  value_t v;
  object_t* root = object_default_create();
  object_t* obja = object_default_create();
  object_t* obja1 = object_default_create();
  object_t* obja2 = object_default_create();
  object_t* objb = object_default_create();
  object_t* objb1 = object_default_create();
  object_t* objb2 = object_default_create();

  ASSERT_EQ(object_set_prop_object(root, "a", obja), RET_OK);
  ASSERT_EQ(object_set_prop_object(root, "b", objb), RET_OK);

  ASSERT_EQ(object_set_prop_int(obja, "value", 123), RET_OK);
  ASSERT_EQ(object_get_prop_by_path(root, "a.value", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 123);
  ASSERT_EQ(object_set_prop_by_path(root, "a.value", value_set_int(&v, 789)), RET_OK);
  ASSERT_EQ(object_get_prop_int(obja, "value", 0), 789);

  ASSERT_EQ(object_set_prop_int(objb, "value", 123), RET_OK);
  ASSERT_EQ(object_get_prop_by_path(root, "b.value", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 123);
  ASSERT_EQ(object_set_prop_by_path(root, "b.value", value_set_int(&v, 789)), RET_OK);
  ASSERT_EQ(object_get_prop_int(objb, "value", 0), 789);

  ASSERT_EQ(object_set_prop_object(obja, "1", obja1), RET_OK);
  ASSERT_EQ(object_set_prop_int(obja1, "value", 456), RET_OK);
  ASSERT_EQ(object_get_prop_by_path(root, "a.1.value", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 456);
  ASSERT_EQ(object_set_prop_by_path(root, "a.1.value", value_set_int(&v, 123)), RET_OK);
  ASSERT_EQ(object_get_prop_int(obja1, "value", 0), 123);

  ASSERT_EQ(object_set_prop_object(obja, "2", obja2), RET_OK);
  ASSERT_EQ(object_set_prop_int(obja2, "value", 56), RET_OK);
  ASSERT_EQ(object_get_prop_by_path(root, "a.2.value", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 56);
  ASSERT_EQ(object_set_prop_by_path(root, "a.2.value", value_set_int(&v, 123)), RET_OK);
  ASSERT_EQ(object_get_prop_int(obja2, "value", 0), 123);

  ASSERT_EQ(object_set_prop_object(objb, "1", objb1), RET_OK);
  ASSERT_EQ(object_set_prop_int(objb1, "value", 56), RET_OK);
  ASSERT_EQ(object_get_prop_by_path(root, "b.1.value", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 56);
  ASSERT_EQ(object_set_prop_by_path(root, "b.1.value", value_set_int(&v, 123)), RET_OK);
  ASSERT_EQ(object_get_prop_int(objb1, "value", 0), 123);

  ASSERT_EQ(object_set_prop_object(objb, "2", objb2), RET_OK);
  ASSERT_EQ(object_set_prop_int(objb2, "value", 156), RET_OK);
  ASSERT_EQ(object_get_prop_by_path(root, "b.2.value", &v), RET_OK);
  ASSERT_EQ(value_int(&v), 156);
  ASSERT_EQ(object_set_prop_by_path(root, "b.2.value", value_set_int(&v, 123)), RET_OK);
  ASSERT_EQ(object_get_prop_int(objb2, "value", 0), 123);

  ASSERT_EQ(object_get_prop_by_path(root, "a.a.value", &v), RET_NOT_FOUND);
  ASSERT_EQ(object_get_prop_by_path(root, "b.a.value", &v), RET_NOT_FOUND);
  ASSERT_EQ(object_get_prop_by_path(root, "c.value", &v), RET_NOT_FOUND);
  ASSERT_EQ(object_set_prop_by_path(root, "a.a.value", value_set_int(&v, 123)), RET_NOT_FOUND);
  ASSERT_EQ(object_set_prop_by_path(root, "b.a.value", value_set_int(&v, 123)), RET_NOT_FOUND);
  ASSERT_EQ(object_set_prop_by_path(root, "c.value", value_set_int(&v, 123)), RET_NOT_FOUND);

  object_unref(root);
  object_unref(obja);
  object_unref(obja1);
  object_unref(obja2);
  object_unref(objb);
  object_unref(objb1);
  object_unref(objb2);
}

TEST(ObjectDefault, cmd_events) {
  string log;
  object_t* obj = object_default_create();
  object_default_t* o = OBJECT_DEFAULT(obj);

  emitter_on((emitter_t*)o, EVT_CMD_CAN_EXEC, event_dump, &log);
  emitter_on((emitter_t*)o, EVT_CMD_WILL_EXEC, event_dump, &log);
  emitter_on((emitter_t*)o, EVT_CMD_EXECED, event_dump, &log);
  emitter_on((emitter_t*)o, EVT_DESTROY, event_dump, &log);

  ASSERT_EQ(object_can_exec(obj, "6", NULL), FALSE);
  ASSERT_EQ(object_can_exec(obj, "ok", NULL), TRUE);
  ASSERT_NE(object_exec(obj, "8", NULL), RET_OK);

  object_unref(obj);

  ASSERT_EQ(log, "6:ok:8:8:destroy:");
}

TEST(ObjectDefault, cmd_events_by_path) {
  string log;
  uint32_t i;
  object_t* root = object_default_create();
  object_t* obja = object_default_create();
  object_t* obja1 = object_default_create();
  object_t* obja2 = object_default_create();
  object_t* objs[] = {root, obja, obja1, obja2};

  ASSERT_EQ(object_set_prop_object(root, "a", obja), RET_OK);
  ASSERT_EQ(object_set_prop_object(obja, "1", obja1), RET_OK);
  ASSERT_EQ(object_set_prop_object(obja, "2", obja2), RET_OK);

  for (i = 0; i < ARRAY_SIZE(objs); i++) {
    emitter_on((emitter_t*)objs[i], EVT_CMD_CAN_EXEC, event_dump, &log);
    emitter_on((emitter_t*)objs[i], EVT_CMD_WILL_EXEC, event_dump, &log);
    emitter_on((emitter_t*)objs[i], EVT_CMD_EXECED, event_dump, &log);
    emitter_on((emitter_t*)objs[i], EVT_DESTROY, event_dump, &log);
  }

  ASSERT_EQ(object_can_exec_by_path(root, "6", NULL), FALSE);
  ASSERT_EQ(object_can_exec_by_path(root, "ok", NULL), TRUE);
  ASSERT_NE(object_exec_by_path(root, "8", NULL), RET_OK);

  ASSERT_EQ(object_can_exec_by_path(root, "a.6", NULL), FALSE);
  ASSERT_EQ(object_can_exec_by_path(root, "a.ok", NULL), TRUE);
  ASSERT_NE(object_exec_by_path(root, "a.8", NULL), RET_OK);

  ASSERT_EQ(object_can_exec_by_path(root, "a.1.6", NULL), FALSE);
  ASSERT_EQ(object_can_exec_by_path(root, "a.1.ok", NULL), TRUE);
  ASSERT_NE(object_exec_by_path(root, "a.1.8", NULL), RET_OK);

  ASSERT_EQ(object_can_exec_by_path(root, "a.2.6", NULL), FALSE);
  ASSERT_EQ(object_can_exec_by_path(root, "a.2.ok", NULL), TRUE);
  ASSERT_NE(object_exec_by_path(root, "a.2.8", NULL), RET_OK);

  object_unref(root);
  object_unref(obja);
  object_unref(obja1);
  object_unref(obja2);

  ASSERT_EQ(log, "6:ok:8:8:6:ok:8:8:6:ok:8:8:6:ok:8:8:destroy:destroy:destroy:destroy:");
}

TEST(ObjectDefault, size) {
  object_t* obj = object_default_create();

  ASSERT_EQ(object_get_prop_int(obj, OBJECT_PROP_SIZE, 0), 0);
  ASSERT_EQ(object_set_prop_str(obj, "a", "123"), RET_OK);
  ASSERT_EQ(object_get_prop_int(obj, OBJECT_PROP_SIZE, 0), 1);
  ASSERT_EQ(object_set_prop_str(obj, "b", "abc"), RET_OK);
  ASSERT_EQ(object_get_prop_int(obj, OBJECT_PROP_SIZE, 0), 2);

  ASSERT_STREQ(object_get_prop_str(obj, "[0]"), "123");
  ASSERT_STREQ(object_get_prop_str(obj, "[1]"), "abc");

  object_unref(obj);
}

TEST(ObjectDefault, sub) {
  object_t* obj = object_default_create();
  object_t* a = object_default_create();
  object_t* b = object_default_create();

  object_set_prop_str(obj, "name", "root");

  object_set_prop_str(a, "name", "aaa");
  object_set_prop_int(a, "age", 100);

  object_set_prop_str(b, "name", "bbb");
  object_set_prop_int(b, "age", 200);

  ASSERT_EQ(object_set_prop_object(obj, "a", a), RET_OK);
  ASSERT_EQ(object_set_prop_object(obj, "bbb", b), RET_OK);

  ASSERT_STREQ(object_get_prop_str(obj, "name"), "root");
  ASSERT_STREQ(object_get_prop_str(obj, "a.name"), "aaa");
  ASSERT_STREQ(object_get_prop_str(obj, "bbb.name"), "bbb");

  ASSERT_EQ(object_remove_prop(obj, "a.name"), RET_OK);
  ASSERT_EQ(object_get_prop_int(obj, "a.age", 0), 100);

  ASSERT_EQ(object_remove_prop(obj, "bbb.name"), RET_OK);
  ASSERT_EQ(object_get_prop_int(obj, "bbb.age", 0), 200);

  ASSERT_EQ(object_remove_prop(obj, "bbb.age"), RET_OK);
  ASSERT_EQ(object_get_prop_int(obj, "bbb.age", 0), 0);

  ASSERT_EQ(object_remove_prop(obj, "name"), RET_OK);

  ASSERT_EQ(object_set_prop_str(obj, "a.name", "AAA"), RET_OK);
  ASSERT_STREQ(object_get_prop_str(obj, "a.name"), "AAA");

  object_unref(obj);
  object_unref(a);
  object_unref(b);
}

TEST(ObjectDefault, insert1) {
  value_t v;
  string log;
  char name[32];
  int32_t i = 0;
  int32_t n = 1000;
  object_t* obj = object_default_create();
  object_default_t* o = OBJECT_DEFAULT(obj);

  ASSERT_EQ(o->props.size, 0u);

  for (i = 0; i < n; i++) {
    tk_snprintf(name, sizeof(name), "name%u", i);
    ASSERT_EQ(object_set_prop(obj, name, value_set_int(&v, i)), RET_OK);
    ASSERT_EQ(object_get_prop_int(obj, name, 0), i);
  }
  ASSERT_EQ(object_get_prop_int(obj, OBJECT_PROP_SIZE, 0), n);

  object_default_clear_props(obj);
  ASSERT_EQ(object_get_prop_int(obj, OBJECT_PROP_SIZE, 0), 0);
  for (i = n; i > 0; i--) {
    tk_snprintf(name, sizeof(name), "name%u", i);
    ASSERT_EQ(object_set_prop(obj, name, value_set_int(&v, i)), RET_OK);
    ASSERT_EQ(object_get_prop_int(obj, name, 0), i);
  }
  ASSERT_EQ(object_get_prop_int(obj, OBJECT_PROP_SIZE, 0), n);

  object_default_clear_props(obj);
  ASSERT_EQ(object_get_prop_int(obj, OBJECT_PROP_SIZE, 0), 0);
  for (i = n; i > 0; i--) {
    tk_snprintf(name, sizeof(name), "name%u", i);
    ASSERT_EQ(object_set_prop(obj, name, value_set_int(&v, i)), RET_OK);
    ASSERT_EQ(object_get_prop_int(obj, name, 0), i);
  }
  ASSERT_EQ(object_get_prop_int(obj, OBJECT_PROP_SIZE, 0), n);

  OBJECT_UNREF(obj);
}

TEST(ObjectDefault, disable_path) {
  object_t* obj = object_default_create_ex(FALSE);

  ASSERT_EQ(object_set_prop_int(obj, "tom.age", 123), RET_OK);
  ASSERT_EQ(object_get_prop_int(obj, "tom.age", 0), 123);

  object_unref(obj);
}

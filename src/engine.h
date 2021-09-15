//
// Created by dell on 2021/4/8.
//


#pragma once
#include <ibus.h>

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "getHan.h"

#include "scope_guard.hpp"
#include <tensorflow/c/c_api.h>
#include "tf_utils.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include <QApplication>
#include "xykeyboardfilter.h"
#include "xyvirtualkeyboard.h"
#include "xyskin.h"
#include <QIcon>

#include <QDebug>
#define IBUS_TYPE_MY_ENGINE	\
	(ibus_my_engine_get_type ())

GType   ibus_my_engine_get_type    (void);

typedef struct _IBusMyEngine IBusMyEngine;
typedef struct _IBusMyEngineClass IBusMyEngineClass;
using namespace rapidjson;
using namespace std;
using namespace cv;
//using namespace tensorflow;
//using namespace ::tensorflow::ops;

struct _IBusMyEngine {
    IBusEngine parent;

    /* members */
    GString *preedit;  //提示词
    gint cursor_pos;   //光标位置
    //GtkWidget * drawpanel;
    IBusLookupTable *table;  //提示板
};

struct _IBusMyEngineClass {
    IBusEngineClass parent;
};
gboolean ibus_my_engine_commit_preedit (IBusMyEngine *enchant);
void ibus_my_engine_update      (IBusMyEngine      *enchant);

typedef void (* ibus_engine_callback)(IBusEngine *engine);

/* functions prototype */
static void ibus_my_engine_class_init   (IBusMyEngineClass  *klass);//初始化
static void ibus_my_engine_init     (IBusMyEngine       *engine);
static void ibus_my_engine_destroy      (IBusMyEngine       *engine);//摧毁
static gboolean
ibus_my_engine_process_key_event //获取键盘信息
        (IBusEngine             *engine,
         guint                   keyval,
         guint                   keycode,
         guint                   modifiers);
static void ibus_my_engine_focus_in    (IBusEngine             *engine);
static void ibus_my_engine_focus_out   (IBusEngine             *engine);
static void ibus_my_engine_reset       (IBusEngine             *engine);
static void ibus_my_engine_enable      (IBusMyEngine             *engine);
static void ibus_my_engine_disable     (IBusMyEngine             *engine);
static void ibus_my_set_cursor_location (IBusEngine             *engine,
                                             gint                    x,
                                             gint                    y,
                                             gint                    w,
                                             gint                    h);
static void ibus_my_engine_set_capabilities
        (IBusEngine             *engine,
         guint                   caps);
static void ibus_my_engine_page_up     (IBusEngine             *engine);
static void ibus_my_engine_page_down   (IBusEngine             *engine);
static void ibus_my_engine_cursor_up   (IBusEngine             *engine);
static void ibus_my_engine_cursor_down (IBusEngine             *engine);
static void ibus_my_property_activate  (IBusEngine             *engine,
                                             const gchar            *prop_name,
                                             gint                    prop_state);
static void ibus_my_engine_property_show
        (IBusEngine             *engine,
         const gchar            *prop_name);
static void ibus_my_engine_property_hide
        (IBusEngine             *engine,
         const gchar            *prop_name);
void ibus_my_engine_commit_string
        (IBusMyEngine      *enchant,
         const gchar            *string);
void
ibus_my_engine_update_preedit 
        (IBusMyEngine *enchant);

IBusMyEngine* get_engine();

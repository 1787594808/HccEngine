//
// Created by dell on 2021/4/8.
//

#include "engine.h"
#include <string.h>
typedef struct _IBusMyEngine IBusMyEngine;
typedef struct _IBusMyEngineClass IBusMyEngineClass;

struct _IBusMyEngine {
    IBusEngine parent;

    /* members */
    GString *preedit;  //提示词
    gint cursor_pos;   //光标位置

    IBusLookupTable *table;  //提示板
};

struct _IBusMyEngineClass {
    IBusEngineClass parent;
};

/* functions prototype */
static void	ibus_my_engine_class_init	(IBusMyEngineClass	*klass);//初始化
static void	ibus_my_engine_init		(IBusMyEngine		*engine);
static void	ibus_my_engine_destroy		(IBusMyEngine		*engine);//摧毁
static gboolean
ibus_my_engine_process_key_event //获取键盘信息
        (IBusEngine             *engine,
         guint               	 keyval,
         guint               	 keycode,
         guint               	 modifiers);
static void ibus_my_engine_focus_in    (IBusEngine             *engine);
static void ibus_my_engine_focus_out   (IBusEngine             *engine);
static void ibus_my_engine_reset       (IBusEngine             *engine);
static void ibus_my_engine_enable      (IBusEngine             *engine);
static void ibus_my_engine_disable     (IBusEngine             *engine);
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

static void ibus_my_engine_commit_string
        (IBusMyEngine      *enchant,
         const gchar            *string);
static void ibus_my_engine_update      (IBusMyEngine      *enchant);

//static MyEngineBroker *broker = NULL;
//static MyEngineDict *dict = NULL;


G_DEFINE_TYPE (IBusMyEngine, ibus_my_engine, IBUS_TYPE_ENGINE)//绑定初始化

static void ibus_my_engine_class_init (IBusMyEngineClass *klass)//class初始化  绑定各函数
{
	g_print("class_init_start\n");
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_my_engine_destroy;//绑定destory函数

    engine_class->process_key_event = ibus_my_engine_process_key_event;//绑定事件函数
	g_print("class_init_end\n");
}

static void ibus_my_engine_init (IBusMyEngine *enchant)//引擎初始化  更新显示内容
{
    //if (broker == NULL) {
       // broker = enchant_broker_init ();
       // dict = enchant_broker_request_dict (broker, "en");
    //}
	g_print("engine_init_start\n");
    enchant->preedit = g_string_new ("");
    enchant->cursor_pos = 0;

    enchant->table = ibus_lookup_table_new (9, 0, TRUE, TRUE);// 大小 光标位置 是否可见 是否循环
    g_object_ref_sink (enchant->table);
	g_print("engine_init_end\n");
}



static void
ibus_my_engine_destroy (IBusMyEngine *enchant)//引擎销毁
{
    if (enchant->preedit) {
        g_string_free (enchant->preedit, TRUE);//释放
        enchant->preedit = NULL;
    }

    if (enchant->table) {
        g_object_unref (enchant->table);
        enchant->table = NULL;
    }

    ((IBusObjectClass *) ibus_my_engine_parent_class)->destroy ((IBusObject *)enchant);//摧毁其父亲
}

static void
ibus_my_engine_update_lookup_table (IBusMyEngine *enchant)
{
  //  gchar ** sugs;
    gint n_sug, i;
    gboolean retval;

    if (enchant->preedit->len == 0) {//预输入为0
        ibus_engine_hide_lookup_table ((IBusEngine *) enchant);//隐藏托盘
        return;
    }

    ibus_lookup_table_clear (enchant->table);//清空托盘

    //查询建议单词
//    sugs = enchant_dict_suggest (dict,
//                                 enchant->preedit->str,
//                                 enchant->preedit->len,
//                                 &n_sug);
//
//    n_sug=3;
//    sugs={""};
//    if (sugs == NULL || n_sug == 0) {//为0隐藏托盘
//        ibus_engine_hide_lookup_table ((IBusEngine *) enchant);
//        return;
//    }

    n_sug=3;
    gchar sugs[300];
	gchar add[100];
	//g_print("stringss");
    strcpy(sugs,enchant->preedit->str);
    strcpy(add,sugs);
    for (i = 0; i < n_sug; i++) {
        ibus_lookup_table_append_candidate (enchant->table, ibus_text_new_from_string (sugs));//附加一个短语
        strcat(sugs,add);
    }
	//g_print("stringend");
    ibus_engine_update_lookup_table ((IBusEngine *) enchant, enchant->table, TRUE);//更新
    /*TODO 测试*/
    ibus_engine_show_lookup_table       ((IBusEngine *)enchant); //显示
//    if (sugs)
//        enchant_dict_free_suggestions (dict, sugs);

}

static void
ibus_my_engine_update_preedit (IBusMyEngine *enchant)//更新预输入
{
    IBusText *text;
    gint retval;

    text = ibus_text_new_from_static_string (enchant->preedit->str);
    text->attrs = ibus_attr_list_new ();//创建属性列表

    ibus_attr_list_append (text->attrs,//将ibus属性追加到text的属性列表 下划线
                           ibus_attr_underline_new (IBUS_ATTR_UNDERLINE_SINGLE, 0, enchant->preedit->len));
	
    ibus_my_engine_update_lookup_table (enchant);
//    if (enchant->preedit->len > 0) { //若长度大于0
//        retval = enchant_dict_check (dict, enchant->preedit->str, enchant->preedit->len);//匹配是否有相同的
//        if (retval != 0) {
//            ibus_attr_list_append (text->attrs,
//                                   ibus_attr_foreground_new (0xff0000, 0, enchant->preedit->len));//设置RED颜色
//        }
//    }

    ibus_engine_update_preedit_text ((IBusEngine *)enchant,text,enchant->cursor_pos,TRUE);

}


static gboolean ibus_my_engine_commit_preedit (IBusMyEngine *enchant) //将得到的数据进行输出
{
    if (enchant->preedit->len == 0)
        return FALSE;

    ibus_my_engine_commit_string (enchant, enchant->preedit->str);//转到输入函数
    g_string_assign (enchant->preedit, "");
    enchant->cursor_pos = 0;

    ibus_my_engine_update (enchant);

    return TRUE;
}

static void ibus_my_engine_commit_string (IBusMyEngine *enchant, const gchar *string)
{
    IBusText *text;
    text = ibus_text_new_from_static_string (string);
    ibus_engine_commit_text ((IBusEngine *)enchant, text); //输出
}


static void ibus_my_engine_update(IBusMyEngine * enchant) {
    ibus_my_engine_update_preedit(enchant);//更新
    //ibus_engine_hide_lookup_table((IBusEngine * ) enchant);  //隐藏
}


#define is_alpha(c) (((c) >= IBUS_a && (c) <= IBUS_z) || ((c) >= IBUS_A && (c) <= IBUS_Z))
//处理键盘事件
static gboolean ibus_my_engine_process_key_event (IBusEngine *engine, guint keyval, guint keycode, guint modifiers)
{
    IBusText *text;
    IBusMyEngine *enchant = (IBusMyEngine *)engine;

    if (modifiers & IBUS_RELEASE_MASK) //密钥被释放
        return FALSE;

    modifiers &= (IBUS_CONTROL_MASK | IBUS_MOD1_MASK);

    if (modifiers == IBUS_CONTROL_MASK && keyval == IBUS_s)
    {
        ibus_my_engine_update_lookup_table (enchant);
        return TRUE;
    }

    if (modifiers != 0) {
        if (enchant->preedit->len == 0)
                return FALSE;
        else
                return TRUE;
    }


    switch (keyval) {
            case IBUS_space://空格
                g_string_append (enchant->preedit, " ");
                return ibus_my_engine_commit_preedit (enchant);
            case IBUS_Return://回车
                return ibus_my_engine_commit_preedit (enchant);
            case IBUS_Escape://ESC
                if (enchant->preedit->len == 0)
                    return FALSE;
                g_string_assign (enchant->preedit, "");//变成预选内容变为0
                enchant->cursor_pos = 0;
                ibus_my_engine_update (enchant);
                    return TRUE;

            case IBUS_Left:
                if (enchant->preedit->len == 0)
                    return FALSE;
                if (enchant->cursor_pos > 0) {
                    enchant->cursor_pos --;
                    ibus_my_engine_update (enchant);
                }
                return TRUE;

            case IBUS_Right:
                if (enchant->preedit->len == 0)
                    return FALSE;
                if (enchant->cursor_pos < enchant->preedit->len)
                {
                    enchant->cursor_pos ++;
                    ibus_my_engine_update (enchant);
                }
                return TRUE;

            case IBUS_Up:
                if (enchant->preedit->len == 0)
                    return FALSE;
                if (enchant->cursor_pos != 0)
                {
                    enchant->cursor_pos = 0;
                    ibus_my_engine_update (enchant);
                }
                return TRUE;
           case IBUS_Down:
                if (enchant->preedit->len == 0)
                     return FALSE;

                if (enchant->cursor_pos != enchant->preedit->len)
                {
                    enchant->cursor_pos = enchant->preedit->len;
                    ibus_my_engine_update (enchant);
                }

                return TRUE;

           case IBUS_BackSpace:
                if (enchant->preedit->len == 0)
                        return FALSE;
                if (enchant->cursor_pos > 0)
                {
                    enchant->cursor_pos --;
                    g_string_erase (enchant->preedit, enchant->cursor_pos, 1);
                    ibus_my_engine_update (enchant);
                }
                return TRUE;
           case IBUS_Delete:

                if (enchant->preedit->len == 0)
                     return FALSE;
                if (enchant->cursor_pos < enchant->preedit->len)
                {
                    g_string_erase (enchant->preedit, enchant->cursor_pos, 1);
                    ibus_my_engine_update (enchant);
                }
                return TRUE;
    }

    if (is_alpha (keyval))
    {
        g_string_insert_c (enchant->preedit,enchant->cursor_pos,keyval);
        enchant->cursor_pos ++;
        ibus_my_engine_update (enchant);
        return TRUE;
    }
    return FALSE;
}

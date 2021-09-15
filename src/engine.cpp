//
// Created by dell on 2021/4/8.
//



#include <string.h>
#include "engine.h"
//#include "UI.h"


IBusMyEngine *myEngine=NULL;

IBusMyEngine* get_engine(){
    return myEngine;
}

G_DEFINE_TYPE (IBusMyEngine, ibus_my_engine, IBUS_TYPE_ENGINE)//绑定初始化

static void ibus_my_engine_class_init (IBusMyEngineClass *klass)//class初始化  绑定各函数
{
	g_print("class_init_start\n");
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);
    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_my_engine_destroy;//绑定destory函数
    engine_class->process_key_event = ibus_my_engine_process_key_event;//绑定事件函数

	engine_class->disable =  (ibus_engine_callback)ibus_my_engine_disable;
	engine_class->enable =  (ibus_engine_callback)ibus_my_engine_enable;

     //engine_class->focus_in= ibus_my_engine_focus_in;
     //engine_class->focus_out= ibus_my_engine_focus_out;
	//sg_print("class_init_end\n");
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
    myEngine=enchant;
}


static void ibus_my_engine_enable(IBusMyEngine *engine)
{
	g_print("enable start\n");
     	/* Go */
        XYVirtualKeyboard::getInstance()->show();
	g_print("enable end\n");
}


static void ibus_my_engine_disable(IBusMyEngine *engine)
{
        XYVirtualKeyboard::getInstance()->hide();
        QApplication::quit();
}


static void
ibus_my_engine_destroy (IBusMyEngine *enchant)//引擎销毁
{
   // UI_cancelui(enchant);
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

static void ibus_my_engine_update_lookup_table (IBusMyEngine *enchant)
{
  //  gchar ** sugs;
    gint n_sug, i;
    gboolean retval;

    if (enchant->preedit->len == 0) {//预输入为0
        ibus_engine_hide_lookup_table ((IBusEngine *) enchant);//隐藏托盘
        return;
    }
    ibus_lookup_table_clear (enchant->table);//清空托盘


    n_sug=3;
    gchar sugs[300];
	gchar add[100];
	//g_print("stringss");
    strcpy(sugs,enchant->preedit->str);
    strcpy(add,sugs);
    for (i = 0; i < n_sug; i++) {
        ibus_lookup_table_append_candidate(enchant->table, ibus_text_new_from_string (sugs));//附加一个短语
        strcat(sugs,add);
    }
	//g_print("stringend");
    ibus_engine_update_lookup_table ((IBusEngine *) enchant, enchant->table, TRUE);//更新
    /*TODO 测试*/
    ibus_engine_show_lookup_table       ((IBusEngine *)enchant); //显示
//    if (sugs)
//        enchant_dict_free_suggestions (dict, sugs);

}

void
ibus_my_engine_update_preedit (IBusMyEngine *enchant)//更新预输入
{
    IBusText *text;
    gint retval;

    text = ibus_text_new_from_static_string (enchant->preedit->str);
    text->attrs = ibus_attr_list_new ();//创建属性列表

    ibus_attr_list_append (text->attrs,//将ibus属性追加到text的属性列表 下划线
                           ibus_attr_underline_new (IBUS_ATTR_UNDERLINE_SINGLE, 0, enchant->preedit->len));
	


    ibus_engine_update_preedit_text ((IBusEngine *)enchant,text,enchant->cursor_pos,TRUE);

}


gboolean ibus_my_engine_commit_preedit (IBusMyEngine *enchant) //将得到的数据进行输出
{
    if (enchant->preedit->len == 0)
        return FALSE;

    ibus_my_engine_commit_string (enchant, enchant->preedit->str);//转到输入函数
    g_string_assign (enchant->preedit, "");
    enchant->cursor_pos = 0;

    ibus_my_engine_update (enchant);

    return TRUE;
}

void ibus_my_engine_commit_string(IBusMyEngine *enchant, const gchar *string)
{
    IBusText *text;
    text = ibus_text_new_from_static_string (string);
    ibus_engine_commit_text ((IBusEngine *)enchant, text); //输出
}


void ibus_my_engine_update(IBusMyEngine * enchant) {
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

//static void ibus_my_engine_focus_in(IBusEngine *engine)
//{
//     XYVirtualKeyboard::getInstance()->show();
//     g_print("_focus_in %d\n",engine->has_focus);
//     XYVirtualKeyboard *kbd =XYVirtualKeyboard::getInstance();
//     kbd->show();
//}
//static void ibus_my_engine_focus_out(IBusEngine *engine)
//{
//     XYVirtualKeyboard::getInstance()->hide();
//     g_print("_focus_%d\n",engine->has_focus);

//     XYVirtualKeyboard *kbd =XYVirtualKeyboard::getInstance();
//     kbd->hide();
//}

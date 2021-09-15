#include "engine.h"
const char* model_path ="/usr/libexec/ibus-engine-myengine/frozen_model.pb";
const char* modelE_path ="/usr/libexec/ibus-engine-myengine/frozen_modelE.pb";
const char* json_path = "/usr/libexec/ibus-engine-myengine/ddict.json";
TF_Session* session;
TF_Session* sessionE;
Document d;
TF_Graph* graph;
TF_Graph* graphE;
static IBusBus *bus = NULL;
static IBusFactory *factory = NULL;
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
/* command line options */
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;
static const GOptionEntry entries[] =
        {
                {"ibus",    'i', 0, G_OPTION_ARG_NONE, &ibus,    "component is executed by ibus", NULL},
                {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose",                       NULL},
                {NULL},
        };

static void  ibus_disconnected_cb(IBusBus *bus, gpointer user_data) {//是否断掉链接
    ibus_quit();
}

TF_Session* getSession()
{
     return session;
}

TF_Session* getSessionE()
{
     return sessionE;
}

Document &getDocument()
{
     return d;
}
TF_Graph* getGraph()
{
     return graph;
}
TF_Graph* getGraphE()
{
     return graphE;
}

static void init(void) {
    printf("123\n");
    ibus_init(); //主线初始化

    bus = ibus_bus_new();     //创建总线实例
    g_object_ref_sink(bus);    //线程安全
    g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnected_cb), NULL); //链接信号和函数

    factory = ibus_factory_new(ibus_bus_get_connection(bus));     //创建引擎工厂
    g_object_ref_sink(factory);                                 //线程安全

    ibus_factory_add_engine(factory, "myengine", IBUS_TYPE_MY_ENGINE);  //添加引擎至工厂

    if (ibus) {
        ibus_bus_request_name(bus, "org.freedesktop.IBus.Myengine", 0);     //自动添加
    } else {   //手动生成并添加
        IBusComponent *component;

        component = ibus_component_new("org.freedesktop.IBus.Myengine",
                                       "Myengine",
                                       "0.1.0",
                                       "GPL",
                                       "HCC <1787594808@qq.com>",
                                       "http://code.google.com/p/ibus/",
                                       "",
                                       "ibus-myengine");
        ibus_component_add_engine(component,
                                  ibus_engine_desc_new("myengine",
                                                       "Myengine",
                                                       "Myengine",
                                                       "zh",
                                                       "GPL",
                                                       "HCC <1787594808@qq.com>",
                                                       
        "/usr/share/ibus-myengine/icons/ibus-myengine.svg",
                "us"));
        g_object_ref_sink(component);
        ibus_bus_register_component(bus, component);//注册组件
    }
}

int main(int argc, char **argv) {
    //freopen("out.txt", "out",stdout);

    FILE* fp = fopen(json_path, "r");
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    d.ParseStream(is);
    fclose(fp);

	
    graph = tf_utils::LoadGraph(model_path);
    SCOPE_EXIT{ tf_utils::DeleteGraph(graph); }; // Auto-delete on scope exit.
    if (graph == nullptr) {
    std::cout << "Can't load graph" << std::endl;
   	return 1;
    }
    session = tf_utils::CreateSession(graph);
    SCOPE_EXIT{ tf_utils::DeleteSession(session); };

    
    graphE = tf_utils::LoadGraph(modelE_path);
    SCOPE_EXIT{ tf_utils::DeleteGraph(graphE); }; // Auto-delete on scope exit.
    if (graphE == nullptr) {
    std::cout << "Can't load graph" << std::endl;
    return 1;
    }
    sessionE = tf_utils::CreateSession(graphE);
    SCOPE_EXIT{ tf_utils::DeleteSession(sessionE); };   


	  
    GError *error = NULL;
    GOptionContext *context;
    /* Parse the command line */
    context = g_option_context_new("- ibus template engine");
    g_option_context_add_main_entries(context, entries, "ibus-myengine");
    if (!g_option_context_parse(context, &argc, &argv, &error)) {  //解析指令
        g_print("Option parsing failed: %s\n", error->message);
        g_error_free(error);
        return (-1);
    }
    init();
        printf(">>>>1\n");
        QApplication a(argc,argv);
        printf(">>>>2\n");
    	//XYSKIN->loadSkipWithFile(qApp->applicationDirPath() + "/skin/xyinput.skin");
        //XYVirtualKeyboard::getInstance()->show();
    	a.setQuitOnLastWindowClosed(true);
    	printf(">>>>3\n");
    	a.setWindowIcon(QIcon(":/Keyboard.ico"));
    	printf(">>>>4\n");
    	a.exec();
    	    	printf(">>>>5\n");
    	ibus_main();
    	    	printf(">>>>6\n");

}

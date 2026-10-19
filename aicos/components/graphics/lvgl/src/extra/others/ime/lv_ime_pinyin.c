/**
 * @file lv_ime_pinyin.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1888e4
#define LOG_MODULE    AMOD_LVGL

#include "lv_ime_pinyin.h"
#if LV_USE_IME_PINYIN != 0

#include <stdio.h>

#ifdef CONFIG_FP
#if 0
#include "app_log.h"
#define __inf           APP_LOGI
#else
#define __inf(fmt, ...)
#endif
#endif

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS    &lv_ime_pinyin_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_ime_pinyin_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_ime_pinyin_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_ime_pinyin_style_change_event(lv_event_t * e);
static void lv_ime_pinyin_kb_event(lv_event_t * e);
static void lv_ime_pinyin_cand_panel_event(lv_event_t * e);

static void init_pinyin_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict);
static void pinyin_input_proc(lv_obj_t * obj);
#ifdef CONFIG_FP
void pinyin_page_proc(lv_obj_t * obj, uint16_t btn);
#else
static void pinyin_page_proc(lv_obj_t * obj, uint16_t btn);
#endif
static char * pinyin_search_matching(lv_obj_t * obj, char * py_str, uint16_t * cand_num);
#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP
static int pinyin_search_matching_bihua(lv_obj_t * obj, char * py_str);
#endif
#endif
static void pinyin_ime_clear_data(lv_obj_t * obj);

#if LV_IME_PINYIN_USE_K9_MODE
    static void pinyin_k9_init_data(lv_obj_t * obj);
    static void pinyin_k9_get_legal_py(lv_obj_t * obj, char * k9_input, const char * py9_map[]);
#ifdef CONFIG_FP
    static void pinyin_k9_get_legal_py1(lv_obj_t * obj, char * k9_input, const char * py9_map[]);
#endif
    static bool pinyin_k9_is_valid_py(lv_obj_t * obj, char * py_str);
    static void pinyin_k9_fill_cand(lv_obj_t * obj);
    static void pinyin_k9_cand_page_proc(lv_obj_t * obj, uint16_t dir);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_ime_pinyin_class = {
    .constructor_cb = lv_ime_pinyin_constructor,
    .destructor_cb  = lv_ime_pinyin_destructor,
    .width_def      = LV_SIZE_CONTENT,
    .height_def     = LV_SIZE_CONTENT,
    .group_def      = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size  = sizeof(lv_ime_pinyin_t),
    .base_class     = &lv_obj_class
};

#if LV_IME_PINYIN_USE_K9_MODE
#ifdef CONFIG_FP
static const char * lv_btnm_def_pinyin_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 20] = {\
                                                                                ",\0", "1#\0",  "abc \0", "def\0",  LV_SYMBOL_BACKSPACE"\0", "\n\0",
                                                                                ".\0", "ghi\0", "jkl\0", "mno\0",  LV_SYMBOL_OK"\0", "\n\0",
                                                                                "?\0", "pqrs\0", "tuv\0", "wxyz\0",  LV_SYMBOL_NEW_LINE"\0", "\n\0",
                                                                                LV_SYMBOL_LEFT"\0", "\0"
                                                                               };
#else
static char * lv_btnm_def_pinyin_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 20] = {\
                                                                                ",\0", "1#\0",  "abc \0", "def\0",  LV_SYMBOL_BACKSPACE"\0", "\n\0",
                                                                                ".\0", "ghi\0", "jkl\0", "mno\0",  LV_SYMBOL_KEYBOARD"\0", "\n\0",
                                                                                "?\0", "pqrs\0", "tuv\0", "wxyz\0",  LV_SYMBOL_NEW_LINE"\0", "\n\0",
                                                                                LV_SYMBOL_LEFT"\0", "\0"
                                                                               };
#endif

static lv_btnmatrix_ctrl_t default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 16] = { 1 };
static char   lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 2][LV_IME_PINYIN_K9_MAX_INPUT] = {0};
#endif

static char   lv_pinyin_cand_str[LV_IME_PINYIN_CAND_TEXT_NUM][4];
static char * lv_btnm_def_pinyin_sel_map[LV_IME_PINYIN_CAND_TEXT_NUM + 3];

#if LV_IME_PINYIN_USE_DEFAULT_DICT
#ifdef CONFIG_FP
const lv_pinyin_dict_t lv_ime_pinyin_def_dict[] = {
    {"a","啊阿呵安按嗄"},
    {"ai","爱埃碍矮挨唉哎哀癌蔼艾隘"},
    {"an","安按暗岸案俺氨鞍庵黯鹌"},
    {"ang","昂"},
    {"ao","凹奥熬袄傲懊澳拗"},
    {"b","不部本鲍柏毕变比并别百报步办表"},
    {"ba","把八吧巴拔霸罢爸坝芭捌扒叭疤跋靶耙"},
    {"bai","百白败摆伯柏拜掰"},
    {"ban","办半板班般版拌搬斑扳伴颁扮瓣绊"},
    {"bang","帮棒邦榜梆膀绑磅蚌镑傍谤"},
    {"bao","报保包剥薄胞暴宝饱抱爆炮堡刨苞褒雹豹鲍曝瀑"},
    {"bei","北被倍备背辈贝杯卑臂悲碑狈惫悖"},
    {"ben","本奔笨"},
    {"beng","泵崩蚌绷甭蹦迸嘣"},
    {"bi","比必避闭辟笔壁臂毕彼逼币鼻蔽鄙碧毙庇痹弊拂泌匕璧"},
    {"bian","变边便编遍辩扁辨鞭贬辫匾缏蝙"},
    {"biao","表标彪"},
    {"bie","别鳖憋瘪"},
    {"bin","宾彬斌濒滨缤槟鬓"},
    {"bing","并病兵柄冰丙饼秉屏禀"},
    {"bo","波播剥伯拨薄博勃驳玻泊柏卜菠搏舶脖膊渤簸跛"},
    {"bu","不部步布补捕卜哺埠簿怖"},
    {"c","产成程陈蔡曹崔楚池仇从此处车场传厂才次查"},
    {"ca","擦拆嚓"},
    {"cai","采才材菜财裁彩猜睬踩蔡"},
    {"can","参残蚕灿餐惭惨璨"},
    {"cang","藏仓苍舱沧"},
    {"cao","草槽操糙曹"},
    {"ce","测策侧册厕"},
    {"cen","参"},
    {"ceng","层曾蹭"},
    {"ch","差长常昌超陈程城持重出初"},
    {"cha","查差插察茶叉茬岔刹"},
    {"chai","差柴拆豺"},
    {"chan","产单铲阐搀掺蝉馋缠颤禅"},
    {"chang","长常场厂唱肠昌倡偿畅猖尝敞"},
    {"chao","朝超潮巢抄钞嘲吵炒剿"},
    {"che","车彻撤扯澈"},
    {"chen","陈沉称衬尘臣晨沈辰趁"},
    {"cheng","成程称城承乘呈盛撑诚橙惩澄逞秤"},
    {"chi","持尺齿吃赤池迟翅斥耻痴匙弛驰侈"},
    {"chong","虫重充冲崇涌宠"},
    {"chou","抽仇臭酬畴稠愁筹绸瞅丑"},
    {"chu","出处除初础触楚锄储畜橱厨雏"},
    {"chuai","揣"},
    {"chuan","传船穿串川喘"},
    {"chuang","床创窗闯疮幢"},
    {"chui","吹垂锤椎炊捶"},
    {"chun","春纯醇椿唇淳蠢"},
    {"chuo","戳绰"},
    {"ci","此次刺磁雌词辞慈瓷赐伺兹祠"},
    {"cong","从丛聪葱囱匆"},
    {"cou","凑"},
    {"cu","粗促醋簇卒"},
    {"cuan","篡窜"},
    {"cui","催脆粹摧崔翠悴"},
    {"cun","存村寸"},
    {"cuo","错措撮搓挫"},
    {"d","的大地杜董段戴邓丁刁电党定等当队点代道"},
    {"da","大打达答搭瘩哒妲沓"},
    {"dai","大代带待袋戴呆歹贷逮怠"},
    {"dan","单但弹担蛋淡胆氮丹旦耽诞"},
    {"dang","党当档挡荡"},
    {"dao","到道导刀倒稻岛捣盗蹈祷悼叨"},
    {"de","的得地德底"},
    {"deng","等灯登邓澄蹬瞪凳"},
    {"di","的地第低敌底提帝抵滴弟递堤迪笛涤蒂缔嘀狄翟邸坻棣砥"},
    {"dian","电点垫典店颠淀掂滇碘佃甸惦奠殿巅玷"},
    {"diao","调掉吊叼雕刁钓"},
    {"die","迭跌爹碟蝶谍叠"},
    {"ding","定顶钉丁订盯叮鼎"},
    {"diu","丢"},
    {"dong","动东冬懂洞冻董栋"},
    {"dou","斗都豆读兜抖陡逗痘蚪窦"},
    {"du","度都毒独读渡杜堵镀督睹赌肚妒"},
    {"duan","断端段短锻缎"},
    {"dui","对队堆兑怼"},
    {"dun","盾吨顿蹲敦墩囤钝盹"},
    {"duo","多度夺朵哆躲跺舵惰堕咄踱"},
    {"e","恶额恩俄耳饵蛾饿峨鹅讹娥遏鄂噩愕鳄"},
    {"en","恩"},
    {"er","而二尔儿耳饵贰"},
    {"f","分发范樊方冯反放复风防非封富翻"},
    {"fa","发法阀乏伐罚筏"},
    {"fan","反翻范犯饭繁泛番凡烦返帆贩樊"},
    {"fang","方放防访房纺仿妨芳肪坊"},
    {"fei","非肥飞费废肺沸菲匪啡诽吠妃"},
    {"fen","分粉奋份粪纷芬愤吩氛坟焚忿"},
    {"feng","风封蜂丰缝峰锋疯奉枫逢冯讽凤"},
    {"fo","佛"},
    {"fou","否"},
    {"fu","复服副府夫负富附福伏符幅腐浮辅付腹妇孵覆扶辐傅佛缚父弗甫肤敷拂俘袱抚俯斧脯赴赋咐芙蝠"},
    {"g","个国工过高郭戈葛耿巩龚甘干古贾谷顾关管各革公根更改光广果格共搞"},
    {"ga","夹胳尬"},
    {"gai","改该盖概钙溉丐"},
    {"gan","干杆感敢赶甘肝秆柑竿赣尴橄"},
    {"gang","刚钢缸纲岗港杠冈肛"},
    {"gao","高搞告稿膏羔糕"},
    {"ge","个各革合格割歌隔盖哥葛阁戈搁鸽胳疙蛤"},
    {"gei","给"},
    {"gen","根跟"},
    {"geng","更耕庚羹埂耿梗"},
    {"gong","工公共供功攻红巩龚贡汞宫恭躬弓拱蚣"},
    {"gou","够构沟句狗钩勾购苟垢枸佝"},
    {"gu","鼓固古骨故顾股谷估雇孤姑贾辜菇咕"},
    {"gua","挂刮瓜括寡褂卦"},
    {"guai","怪乖拐"},
    {"guan","关管观官灌贯惯冠馆罐棺"},
    {"guang","光广逛"},
    {"gui","规贵归硅鬼轨龟桂瑰闺诡癸柜跪"},
    {"gun","滚棍"},
    {"guo","国过果锅郭裹"},
    {"h","和会后何黄胡洪侯贺霍韩华化好合还活很回号毫花或海话红火害"},
    {"ha","哈蛤"},
    {"hai","还海害孩亥骇"},
    {"han","含焊旱喊汉寒汗函韩酣憨涵罕翰撼捍憾悍"},
    {"hang","行航夯杭吭巷"},
    {"hao","好号毫耗豪浩镐嚎皓"},
    {"he","和合河何核赫荷褐喝贺呵禾盒鹤"},
    {"hei","黑嘿"},
    {"hen","很狠痕恨"},
    {"heng","横衡恒哼行"},
    {"hong","红洪轰烘哄虹鸿宏弘"},
    {"hou","后候厚侯喉猴吼"},
    {"hu","和护互湖呼户弧乎胡糊虎忽壶葫蝴狐唬沪浒惚琥扈"},
    {"hua","化花话划滑华画哗猾桦"},
    {"huai","坏怀划淮槐徊"},
    {"huan","还环换欢缓患幻焕唤痪涣宦"},
    {"huang","黄簧荒皇慌蝗凰惶煌晃幌恍谎"},
    {"hui","会回灰挥辉汇毁慧恢绘惠徽堕悔卉晦贿秽讳诲"},
    {"hun","混浑荤昏婚魂馄"},
    {"huo","和活或火货获伙霍豁惑祸"},
    {"j","级贾江蒋姜焦金晋就进加经家结阶间件及解较机军接教极九见"},
    {"ji","级及机极几积给基其记己系计集即际季激济技击继急剂既纪寄挤鸡迹绩齐吉脊辑籍疾肌棘畸圾稽箕饥讥缉嫉冀祭寂忌妓藉叽唧鲫"},
    {"jia","加家架价甲夹假钾贾稼驾嘉佳颊嫁茄挟"},
    {"jian","间件见建坚减检践尖简碱剪艰渐肩键健柬鉴剑歼监兼奸箭茧舰俭煎拣捡荐贱溅涧"},
    {"jiang","将降讲强江浆蒋奖疆僵姜桨匠酱缰犟"},
    {"jiao","较教交角叫脚胶浇焦觉校蕉搅酵郊窖椒礁骄娇嚼矫侥狡饺缴绞剿轿跤"},
    {"jie","结阶解接节家界截介借价届街揭洁杰竭皆秸劫桔捷睫姐戒芥诫"},
    {"jin","进金近紧斤今尽仅劲浸禁津筋锦晋巾襟谨烬靳缙矜觐"},
    {"jing","经精京径井静竟晶净境镜景警劲茎敬惊睛竞荆兢鲸靖"},
    {"jiong","窘炯炅囧"},
    {"jiu","就九旧究久救酒纠揪玖韭灸臼舅疚"},
    {"ju","具据局举车句聚距巨居锯剧矩拒桔鞠拘驹菊沮俱惧炬橘"},
    {"juan","卷捐鹃娟倦眷绢"},
    {"jue","决觉绝角脚掘嚼倔爵诀崛"},
    {"jun","军均菌君钧峻俊竣骏"},
    {"k","可开看克孔寇科口空苦快考刻靠况块困宽矿抗卡"},
    {"ka","卡咖"},
    {"kai","开凯揩楷慨"},
    {"kan","看刊坎槛堪勘砍"},
    {"kang","抗康炕慷糠扛亢"},
    {"kao","考靠拷烤铐"},
    {"ke","可克科刻客壳颗棵呵坷苛磕咳渴课蝌"},
    {"ken","肯啃垦恳"},
    {"keng","坑吭"},
    {"kong","孔空控恐"},
    {"kou","口扣抠寇叩"},
    {"ku","苦库枯酷哭窟裤"},
    {"kua","跨夸垮挎"},
    {"kuai","会快块筷"},
    {"kuan","宽款"},
    {"kuang","况矿狂框筐眶旷"},
    {"kui","溃馈亏盔窥葵魁愧"},
    {"kun","困昆坤捆"},
    {"kuo","扩括阔廓"},
    {"l","了来赖罗吕刘鲁李林雷卢梁厉廉陆龙黎鹿量理路里论两利领流力立料连老六轮类列"},
    {"la","拉落啦蜡腊垃喇辣"},
    {"lai","来赖莱睐"},
    {"lan","兰烂蓝览栏婪拦篮澜揽懒缆滥榄"},
    {"lang","浪朗郎狼琅榔廊"},
    {"lao","老劳落牢涝捞络姥酪烙唠"},
    {"le","了乐勒肋"},
    {"lei","类雷累勒垒泪蕾磊擂肋"},
    {"leng","冷棱愣"},
    {"li","理里利力立离例历粒厘礼李隶黎璃励犁梨丽厉篱狸漓鲤莉荔吏栗砾俐痢沥哩"},
    {"lia","俩"},
    {"lian","连联练炼脸链莲镰廉怜帘敛恋"},
    {"liang","量两粮良亮梁凉辆粱晾谅"},
    {"liao","了料疗辽僚撩聊寥嘹缭"},
    {"lie","列裂烈劣猎咧"},
    {"lin","林磷临邻淋琳鳞凛赁吝躏"},
    {"ling","领另零令灵岭铃龄凌棱陵拎玲菱伶羚聆"},
    {"liu","流六留刘硫柳陆馏瘤溜琉榴浏"},
    {"long","龙垄弄笼隆聋咙窿拢胧泷"},
    {"lou","漏露楼娄搂篓陋"},
    {"lu","路录陆炉露芦卢颅庐卤虏鲁碌赂鹿禄"},
    {"luan","卵乱峦"},
    {"lue","略掠"},
    {"lun","论轮伦抡仑沦"},
    {"luo","落罗螺洛络逻烙萝锣箩骡裸骆"},
    {"lv","率律旅虑驴吕铝侣履屡缕氯滤绿"},
    {"m","面命马孟穆梅们民毛茅苗莫没米明么每门名磨美密满亩目"},
    {"ma","马么麻吗妈骂嘛码摩抹玛蚂蟆"},
    {"mai","麦脉卖买埋迈"},
    {"man","满慢曼漫蔓瞒馒蛮"},
    {"mang","忙芒盲茫氓莽"},
    {"mao","毛矛冒貌贸帽猫茅锚卯茂"},
    {"me","么"},
    {"mei","没每美煤霉梅妹眉玫枚媒昧媚魅"},
    {"men","们门闷焖"},
    {"meng","孟猛蒙盟梦萌锰檬朦"},
    {"mi","米密迷蜜秘眯靡谜弥觅泌咪"},
    {"mian","面棉免绵眠勉缅"},
    {"miao","苗秒描庙妙瞄藐渺缪"},
    {"mie","灭蔑"},
    {"min","民敏皿悯闽"},
    {"ming","命明名鸣盟铭冥"},
    {"miu","谬"},
    {"mo","无磨没万末模膜摸墨摩脉莫抹冒默嘿摹蘑魔沫漠寞陌茉馍"},
    {"mou","某谋"},
    {"mu","亩目木母墓模幕牧姆穆拇牡暮募慕睦沐"},
    {"n","年能内那聂农你南难呢女泥哪拿念牛宁脑粘尼纳"},
    {"na","那南哪拿纳呢钠呐娜捺"},
    {"nai","耐哪奶乃奈"},
    {"nan","南难男楠"},
    {"nang","囊"},
    {"nao","脑闹挠恼瑙孬"},
    {"ne","呢哪呐"},
    {"nei","内那哪馁"},
    {"nen","嫩恁"},
    {"neng","能"},
    {"ni","你泥尼呢逆拟妮匿腻溺"},
    {"nian","年念粘碾撵捻廿"},
    {"niang","娘酿"},
    {"niao","尿鸟"},
    {"nie","捏聂孽"},
    {"nin","您"},
    {"ning","宁凝拧柠狞泞"},
    {"niu","牛扭钮纽拗"},
    {"nong","农弄浓脓"},
    {"nu","努奴怒"},
    {"nuan","暖"},
    {"nuo","诺娜挪懦糯"},
    {"nv","女"},
    {"nve","虐疟"},
    {"o","哦欧偶鸥殴藕呕"},
    {"ou","区欧偶鸥殴藕呕"},
    {"p","批平判潘彭庞蒲配片派破排品皮喷盘偏评怕培普贫跑炮"},
    {"pa","怕派爬帕扒耙啪趴"},
    {"pai","派排拍牌迫徘湃"},
    {"pan","判盘叛番潘攀盼畔"},
    {"pang","旁膀磅乓庞胖螃彷"},
    {"pao","跑炮刨抛泡袍疱"},
    {"pei","配培陪胚赔佩沛呸裴沛霈"},
    {"pen","喷盆"},
    {"peng","碰棚蓬朋捧膨砰烹澎彭篷鹏怦"},
    {"pi","批皮坏否辟坯脾疲霹披劈啤匹僻屁譬"},
    {"pian","片偏便篇骗翩"},
    {"piao","票漂朴飘瓢嫖"},
    {"pie","撇"},
    {"pin","品贫频拼苹聘"},
    {"ping","平评瓶凭苹乒坪萍屏"},
    {"po","破迫坡泼繁朴泊颇婆魄"},
    {"pou","剖"},
    {"pu","普谱扑铺暴葡朴蒲脯仆菩圃浦曝瀑"},
    {"q","起前其钱乔齐秦屈戚去气全期群情区强切取七求清确权且青"},
    {"qi","起其气期七器齐奇汽企漆欺旗启弃歧稽缉栖戚妻凄柒棋崎脐祈骑岂乞契砌迄泣岖鳍琪"},
    {"qia","卡恰掐洽"},
    {"qian","前千钱浅签迁铅潜牵钳纤谴谦乾黔遣嵌欠歉"},
    {"qiang","强将枪抢墙腔呛"},
    {"qiao","桥瞧壳巧敲乔锹悄侨撬翘峭俏窍憔跷"},
    {"qie","切且砌茄怯窃"},
    {"qin","亲侵勤秦钦琴芹擒禽寝沁"},
    {"qing","情清青轻倾请亲庆氢晴卿擎顷蜻"},
    {"qiong","穷琼"},
    {"qiu","求球秋丘囚蚯"},
    {"qu","去区取曲且渠屈趋驱趣躯娶戌岖"},
    {"quan","全权圈劝泉痊拳犬券"},
    {"que","确却缺瘸鹊雀"},
    {"qun","群裙"},
    {"r","人如日荣认入任热然容让润弱燃染壤绕仁溶刃仍"},
    {"ran","然燃染冉苒"},
    {"rang","让壤嚷瓤"},
    {"rao","绕扰饶娆"},
    {"re","热若惹"},
    {"ren","人认任仁刃忍壬韧纫"},
    {"reng","仍扔"},
    {"ri","日"},
    {"rong","容溶荣熔融绒戎茸蓉冗榕"},
    {"rou","肉揉柔蹂"},
    {"ru","如入儒乳蠕辱汝褥"},
    {"ruan","软阮"},
    {"rui","瑞锐蕊芮睿"},
    {"ruo","弱若偌"},
    {"s","是上时孙宋苏沈粟施申师帅沙隋石史生说社深水所三十使事数四实少式山手邵佘"},
    {"sa","撒萨洒飒"},
    {"sai","塞思赛腮鳃噻"},
    {"san","三散叁伞"},
    {"sang","桑丧嗓"},
    {"sao","扫梢搔骚嫂臊瘙"},
    {"se","色塞瑟涩啬"},
    {"sen","森"},
    {"seng","僧"},
    {"sh","数厦山删商少召社设甚声市师始手收守书属术输双水谁睡税顺"},
    {"sha","沙杀砂啥纱刹傻煞杉厦霎鲨嗄"},
    {"shai","色筛晒"},
    {"shan","山单闪善珊扇陕杉删煽衫擅赡栅禅膳汕讪姗跚鳝"},
    {"shang","上商伤尚汤赏晌裳"},
    {"shao","少烧稍绍召哨梢捎勺芍韶邵苕"},
    {"she","社设射摄舌折涉舍蛇奢赦"},
    {"shen","深身神什伸参甚渗沈肾审申慎呻绅婶蜃"},
    {"sheng","生胜声省升盛绳剩圣牲乘甥笙"},
    {"shi","是时十使事实式识世试石什示市史师始施士势湿适食失视室氏似蚀诗释殖拾饰驶匙狮尸矢屎柿拭誓逝嗜侍恃"},
    {"shou","手受收首守熟授寿兽售瘦狩"},
    {"shu","数书树属术输述熟束鼠疏殊舒蔬薯叔署枢梳抒淑赎暑曙蜀竖墅庶漱恕"},
    {"shua","刷耍唰"},
    {"shuai","衰帅摔甩蟀"},
    {"shuan","栓拴涮"},
    {"shuang","双霜爽"},
    {"shui","水说谁睡税"},
    {"shun","顺吮瞬舜"},
    {"shuo","说数硕烁朔铄"},
    {"si","四思死斯丝似司饲食私厕撕嘶肆寺伺巳祀嗣厮驷"},
    {"song","松送宋颂耸讼诵怂嵩悚"},
    {"sou","搜艘嗽擞叟嗖馊"},
    {"su","素速苏塑缩俗诉宿肃酥粟溯僳愫簌稣"},
    {"suan","算酸蒜"},
    {"sui","随穗碎虽岁隋髓遂隧祟"},
    {"sun","损孙笋"},
    {"suo","所缩锁索梭唆琐嗦娑"},
    {"t","他同它唐谭陶汤童涂田提题体图天头通条特土团统台她太套铁"},
    {"ta","他它她塔踏塌蹋遢榻沓"},
    {"tai","台太态胎抬泰苔汰"},
    {"tan","谈碳探炭弹坦贪滩摊瘫坛檀痰潭谭毯叹袒忐"},
    {"tang","堂糖唐塘汤棠膛倘躺淌趟烫"},
    {"tao","套讨逃陶萄桃掏涛滔淘叨"},
    {"te","特忒忑"},
    {"teng","腾疼藤誊"},
    {"ti","提题体替梯惕剔踢蹄啼涕剃屉"},
    {"tian","天田添填甜佃恬舔"},
    {"tiao","条调跳挑迢眺窕"},
    {"tie","铁贴帖"},
    {"ting","听停庭挺廷厅亭艇蜓"},
    {"tong","同通统铜痛筒童桶桐瞳彤捅"},
    {"tou","头投透偷"},
    {"tu","图土突途徒凸涂吐兔屠秃"},
    {"tuan","团"},
    {"tui","推退腿颓蜕褪"},
    {"tun","囤褪吞屯臀豚饨"},
    {"tuo","脱拖托妥椭舵鸵驮驼拓唾"},
    {"w","为我无王汪卫魏吴闻乌邬翁温伍问外五位物委完万文往武务围误晚"},
    {"wa","瓦挖凹哇蛙洼娃袜佤"},
    {"wai","外歪崴"},
    {"wan","完万晚弯碗顽湾挽玩蔓豌丸皖惋宛婉腕蜿娩"},
    {"wang","往王望网忘妄亡旺芒汪枉"},
    {"wei","为位委围维唯卫微伟未威危尾谓喂味胃魏伪遗违畏纬巍惟苇萎蔚尉慰偎薇帷猬韦炜"},
    {"wen","问温文稳纹闻蚊瘟吻刎汶雯"},
    {"weng","嗡翁"},
    {"wo","我握窝蜗涡沃卧挝莴喔幄龌"},
    {"wu","无五物武务误伍舞污悟雾午恶屋乌吴亡诬巫呜芜梧吾捂侮戊勿鹉蜈邬"},
    {"x","学下行许徐萧肖薛熊解项习小线性新些想心向相系席现西修象先形"},
    {"xi","系席西习细吸析喜洗稀戏隙希息袭锡牺悉惜溪腊栖昔熙晰嘻膝夕熄犀媳徙蟋茜奚玺禧"},
    {"xia","下夏吓狭霞唬瞎虾匣辖暇峡侠厦遐瑕"},
    {"xian","线现先县限见显鲜献险陷宪纤洗掀弦腺仙咸贤衔闲嫌馅羡"},
    {"xiang","想向相象响项箱乡降香像详橡享湘厢镶翔祥巷"},
    {"xiao","小消削效笑校销硝萧肖孝霄哮嚣宵淆晓啸潇箫"},
    {"xie","些解写斜谢协械叶血卸屑鞋歇邪胁蟹泄泻契蝎挟携谐懈"},
    {"xin","新心信锌芯辛欣薪衅馨"},
    {"xing","行性形型星兴省醒姓幸腥猩刑杏惺邢"},
    {"xiong","雄胸兄凶熊匈汹"},
    {"xiu","修锈休袖秀朽宿臭羞嗅绣"},
    {"xu","续许须需序虚絮畜叙蓄绪徐墟戌酗旭恤婿吁嘘诩圩浒溆栩煦"},
    {"xuan","选旋宣悬玄券轩喧癣绚渲炫眩萱漩璇"},
    {"xue","学血削雪穴靴薛"},
    {"xun","训旬迅讯寻循巡荤勋熏询驯殉汛逊"},
    {"y","一有杨严余阎姚颜袁燕岳乐叶羊尤用要以于也样义与由压员月意又运原油验尹"},
    {"ya","压亚呀牙芽雅鸭匹押鸦丫崖衙涯哑讶"},
    {"yan","验研严颜眼言盐演岩沿烟延掩宴炎燕衍铅焉咽淹蜒阎艳堰厌砚雁唁焰谚檐俨偃妍嫣琰胭腌焱"},
    {"yang","样养氧扬洋阳羊秧央杨仰殃鸯痒漾"},
    {"yao","要药约摇腰咬邀耀陶侥妖尧遥窑谣姚舀钥夭吆肴"},
    {"ye","也业页叶液夜野爷冶咽椰腋拽噎耶掖曳烨"},
    {"yi","一以义意已移医议依易乙艺益异宜仪亿遗伊役衣疑亦谊翼译抑忆疫壹夷胰姨椅蚁倚矣邑屹逸毅溢绎怡贻"},
    {"yin","因引阴印音银烟隐饮尹龈荫茵殷姻吟淫寅瘾蚓"},
    {"ying","应影硬营英映迎樱婴鹰莹萤荧蝇赢盈颖莺鹦滢瑛"},
    {"yo","育哟"},
    {"yong","用勇永拥涌庸佣踊咏泳"},
    {"you","有由又油右友优幼游尤诱犹幽悠忧邮酉佑釉宥"},
    {"yu","于与育鱼雨玉余遇预域语愈渔予羽愚御欲宇蔚尉迂淤榆舆逾愉渝隅娱屿禹芋郁吁喻狱誉浴寓裕豫虞俞毓瑜昱煜钰"},
    {"yuan","员原圆源元远愿院缘援园怨鸳渊冤袁猿苑"},
    {"yue","月说越约乐跃曰阅钥岳粤悦"},
    {"yun","运员云匀均允孕耘陨蕴酝晕韵芸熨"},
    {"z","在这主中张章赵曾郑周朱种着祝庄宗祖左展争战作正之制重组治只子自质指"},
    {"za","杂扎砸咋"},
    {"zai","在再载栽灾仔哉宰"},
    {"zan","赞咱暂攒簪"},
    {"zang","藏脏葬赃"},
    {"zao","造早遭燥凿糟枣皂藻澡蚤躁噪灶"},
    {"ze","则择责侧泽咋"},
    {"zei","贼"},
    {"zen","怎"},
    {"zeng","增曾憎赠"},
    {"zh","扎炸摘债战展长张章钟着照找这真争正政之制治只中周州主住抓转专撰赚装状准"},
    {"zha","查扎炸闸轧渣喳眨栅榨咋乍诈吒咤楂"},
    {"zhai","寨侧择摘窄斋祭宅债"},
    {"zhan","战展站占粘颤瞻沾盏斩辗崭栈绽"},
    {"zhang","长张章掌仗障胀涨账樟彰杖丈帐蟑"},
    {"zhao","着照找招朝召赵爪罩沼兆嘲昭肇钊"},
    {"zhe","这着者折哲浙遮辙蔗"},
    {"zhen","真针阵镇振震珍诊斟贞侦枕疹甄臻圳缜桢榛胗朕"},
    {"zheng","争正政整证征蒸症丁郑挣睁狰怔拯筝"},
    {"zhi","之制治只质指直支织止至置志值知识执职植纸致枝殖脂智氏肢秩址滞汁芝吱蜘侄趾旨挚掷帜稚窒"},
    {"zhong","中种重众钟终忠肿仲衷"},
    {"zhou","轴周洲州皱骤舟粥肘帚咒宙昼"},
    {"zhu","主注著住助猪朱铸属株筑柱术驻逐祝竹贮珠诸蛛烛煮拄瞩嘱蛀"},
    {"zhua","抓爪"},
    {"zhuai","转拽"},
    {"zhuan","转专传砖撰赚"},
    {"zhuang","装状壮庄撞幢桩妆"},
    {"zhui","追锥椎赘坠缀"},
    {"zhun","准谆"},
    {"zhuo","着著捉桌缴拙卓琢茁酌啄灼浊"},
    {"zi","子自资字紫仔籽姿吱兹咨滋姊淄孜渍呲"},
    {"zong","总纵宗综棕踪粽"},
    {"zou","走奏揍邹"},
    {"zu","组族足阻祖租卒"},
    {"zuan","钻赚"},
    {"zui","最罪嘴醉"},
    {"zun","尊遵"},
    {"zuo","作做左座坐昨凿琢佐"},
    {NULL,NULL},


};
#else
lv_pinyin_dict_t lv_ime_pinyin_def_dict[] = {
    { "a", "啊" },
    { "ai", "愛" },
    { "an", "安暗案" },
    { "ba", "吧把爸八" },
    { "bai", "百白敗" },
    { "ban", "半般辦" },
    { "bang", "旁" },
    { "bao", "保薄包報" },
    { "bei", "被背悲北杯備" },
    { "ben", "本" },
    { "bi", "必比避鼻彼筆秘閉" },
    { "bian", "便邊變変辺" },
    { "biao", "表標" },
    { "bie", "別" },
    { "bing", "病並氷" },
    { "bo", "波薄泊" },
    { "bu", "不布步部捕補歩" },
    { "ca", "察" },
    { "cai", "才材菜財採" },
    { "can", "参残參" },
    { "ce", "策側" },
    { "ceng", "曾" },
    { "cha", "差查茶" },
    { "chai", "差" },
    { "chan", "產産單" },
    { "chang", "場廠" },
    { "chao", "超朝" },
    { "che", "車" },
    { "cheng", "成程乗" },
    { "chi", "尺吃持赤池遅歯" },
    { "chong", "充种重種" },
    { "chu", "出初楚触處処" },
    { "chuan", "川船傳" },
    { "chuang", "創窓" },
    { "chun", "春" },
    { "ci", "此次辞差" },
    { "cong", "從従" },
    { "cu", "卒" },
    { "cun", "存村" },
    { "cuo", "錯" },
    { "da", "大打答達" },
    { "dai", "代待帯帶貸" },
    { "dan", "但担擔誕單単" },
    { "dang", "当党當黨" },
    { "dao", "到道盗導島辺" },
    { "de", "的得" },
    { "dei", "" },
    { "deng", "等" },
    { "di", "地得低底弟第締" },
    { "dian", "点电店點電" },
    { "diao", "調" },
    { "ding", "定町" },
    { "dong", "冬東動働凍" },
    { "du", "独度都渡読" },
    { "duan", "段断短斷" },
    { "dui", "對対" },
    { "duo", "多駄" },
    { "e", "嗯悪" },
    { "en", "嗯" },
    { "er", "而耳二兒" },
    { "fa", "乏法發発髪" },
    { "fan", "反返犯番仮販飯範払" },
    { "fang", "方放房坊訪" },
    { "fei", "非飛費" },
    { "fen", "分份" },
    { "feng", "風豐" },
    { "fou", "否不" },
    { "fu", "父夫富服符付附府幅婦復複負払" },
    { "gai", "改概該" },
    { "gan", "甘感敢" },
    { "gang", "港剛" },
    { "gao", "告高" },
    { "ge", "各格歌革割個" },
    { "gei", "給" },
    { "gen", "跟根" },
    { "geng", "更" },
    { "gong", "工共供功公" },
    { "gou", "夠構溝" },
    { "gu", "古故鼓" },
    { "guai", "掛" },
    { "guan", "官管慣館觀関關" },
    { "guang", "光広" },
    { "gui", "規帰" },
    { "guo", "果国裏菓國過" },
    { "hai", "孩海害還" },
    { "han", "寒漢" },
    { "hang", "航行" },
    { "hao", "好号" },
    { "he", "合和喝何荷" },
    { "hei", "黒" },
    { "hen", "很" },
    { "heng", "行横" },
    { "hou", "厚喉候後" },
    { "hu", "乎呼湖護" },
    { "hua", "化画花話畫劃" },
    { "huai", "壊劃" },
    { "huan", "緩環歡還換" },
    { "huang", "黄" },
    { "hui", "回会慧絵揮會" },
    { "hun", "混婚" },
    { "huo", "活或火獲" },
    { "i", "" },
    { "ji", "己计及机既急季寄技即集基祭系奇紀積計記済幾際極繼績機濟" },
    { "jia", "家加價" },
    { "jian", "件建健肩見減間検簡漸" },
    { "jiang", "降強講將港" },
    { "jiao", "叫教交角覚覺較學" },
    { "jie", "介借接姐皆届界解結階節價" },
    { "jin", "今近禁金僅進" },
    { "jing", "京境景静精經経" },
    { "jiu", "就久九酒究" },
    { "ju", "句具局居決挙據舉" },
    { "jue", "角覚覺" },
    { "jun", "均" },
    { "kai", "開" },
    { "kan", "看刊" },
    { "kang", "康" },
    { "kao", "考" },
    { "ke", "可刻科克客渇課" },
    { "ken", "肯" },
    { "kong", "空控" },
    { "kou", "口" },
    { "ku", "苦庫" },
    { "kuai", "快塊会會" },
    { "kuang", "況" },
    { "kun", "困" },
    { "kuo", "括拡適" },
    { "la", "拉啦落" },
    { "lai", "来來頼" },
    { "lao", "老絡落" },
    { "le", "了楽樂" },
    { "lei", "類" },
    { "leng", "冷" },
    { "li", "力立利理例礼離麗裡勵歷" },
    { "lian", "連練臉聯" },
    { "liang", "良量涼兩両" },
    { "liao", "料" },
    { "lie", "列" },
    { "lin", "林隣賃" },
    { "ling", "另令領" },
    { "liu", "六留流" },
    { "lu", "律路録緑陸履慮" },
    { "lv", "旅" },
    { "lun", "輪論" },
    { "luo", "落絡" },
    { "ma", "媽嗎嘛" },
    { "mai", "買売" },
    { "man", "滿" },
    { "mang", "忙" },
    { "mao", "毛猫貿" },
    { "me", "麼" },
    { "mei", "美妹每沒毎媒" },
    { "men", "們" },
    { "mi", "米密秘" },
    { "mian", "免面勉眠" },
    { "miao", "描" },
    { "min", "民皿" },
    { "ming", "命明名" },
    { "mo", "末模麼" },
    { "mou", "某" },
    { "mu", "母木目模" },
    { "na", "那哪拿內南" },
    { "nan", "男南難" },
    { "nao", "腦" },
    { "ne", "那哪呢" },
    { "nei", "内那哪內" },
    { "neng", "能" },
    { "ni", "你妳呢" },
    { "nian", "年念" },
    { "niang", "娘" },
    { "nin", "您" },
    { "ning", "凝" },
    { "niu", "牛" },
    { "nong", "農濃" },
    { "nu", "女努" },
    { "nuan", "暖" },
    { "o", "" },
    { "ou", "歐" },
    { "pa", "怕" },
    { "pian", "片便" },
    { "pai", "迫派排" },
    { "pan", "判番" },
    { "pang", "旁" },
    { "pei", "配" },
    { "peng", "朋" },
    { "pi", "疲否" },
    { "pin", "品貧" },
    { "ping", "平評" },
    { "po", "迫破泊頗" },
    { "pu", "普僕" },
    { "qi", "起其奇七气期泣企妻契気" },
    { "qian", "嵌浅千前鉛錢針" },
    { "qiang", "強將" },
    { "qiao", "橋繰" },
    { "qie", "且切契" },
    { "qin", "寝勤親" },
    { "qing", "青清情晴輕頃請軽" },
    { "qiu", "求秋球" },
    { "qu", "去取趣曲區" },
    { "quan", "全犬券" },
    { "que", "缺確卻" },
    { "ran", "然" },
    { "rang", "讓" },
    { "re", "熱" },
    { "ren", "人任認" },
    { "reng", "仍" },
    { "ri", "日" },
    { "rong", "容" },
    { "rou", "弱若肉" },
    { "ru", "如入" },
    { "ruan", "軟" },
    { "sai", "賽" },
    { "san", "三" },
    { "sao", "騒繰" },
    { "se", "色" },
    { "sen", "森" },
    { "sha", "砂" },
    { "shan", "善山單" },
    { "shang", "上尚商" },
    { "shao", "少紹" },
    { "shaung", "雙" },
    { "she", "社射設捨渉" },
    { "shei", "誰" },
    { "shen", "什申深甚身伸沈神" },
    { "sheng", "生声昇勝乗聲" },
    { "shi", "是失示食时事式十石施使世实史室市始柿氏士仕拭時視師試適実實識" },
    { "shou", "手首守受授" },
    { "shu", "束数暑殊樹書屬輸術" },
    { "shui", "水説說誰" },
    { "shuo", "数説說" },
    { "si", "思寺司四私似死価" },
    { "song", "送" },
    { "su", "速宿素蘇訴" },
    { "suan", "算酸" },
    { "sui", "隨雖歲歳" },
    { "sun", "孫" },
    { "suo", "所" },
    { "ta", "她他它牠" },
    { "tai", "太台態臺" },
    { "tan", "探談曇" },
    { "tang", "糖" },
    { "tao", "桃逃套討" },
    { "te", "特" },
    { "ti", "体提替題體戻" },
    { "tian", "天田" },
    { "tiao", "条條調" },
    { "tie", "鉄" },
    { "ting", "停庭聽町" },
    { "tong", "同童通痛统統" },
    { "tou", "投透頭" },
    { "tu", "土徒茶図" },
    { "tuan", "團" },
    { "tui", "推退" },
    { "tuo", "脱駄" },
    { "u", "" },
    { "v", "" },
    { "wai", "外" },
    { "wan", "完万玩晩腕灣" },
    { "wang", "忘望亡往網" },
    { "wei", "危位未味委為謂維違圍" },
    { "wen", "文温問聞" },
    { "wo", "我" },
    { "wu", "午物五無屋亡鳥務汚" },
    { "xi", "夕息西洗喜系昔席希析嬉膝細習係" },
    { "xia", "下夏狭暇" },
    { "xian", "先限嫌洗現見線顯" },
    { "xiang", "向相香像想象降項詳響" },
    { "xiao", "小笑消效校削咲" },
    { "xie", "写携些解邪械協謝寫契" },
    { "xin", "心信新辛" },
    { "xing", "行形性幸型星興" },
    { "xiong", "兄胸" },
    { "xiu", "休秀修" },
    { "xu", "須需許續緒続" },
    { "xuan", "選懸" },
    { "xue", "学雪削靴學" },
    { "xun", "訓訊" },
    { "ya", "呀押壓" },
    { "yan", "言顔研煙嚴厳験驗塩" },
    { "yang", "央洋陽樣様" },
    { "yao", "要揺腰薬曜" },
    { "ye", "也野夜邪業葉" },
    { "yi", "一已亦依以移意医易伊役異億義議藝醫訳" },
    { "yin", "因引音飲銀" },
    { "ying", "英迎影映應營営" },
    { "yong", "永用泳擁" },
    { "you", "又有右友由尤油遊郵誘優" },
    { "yu", "予育余雨浴欲愈御宇域語於魚與込" },
    { "yuan", "元原源院員円園遠猿願" },
    { "yue", "月越約楽" },
    { "yun", "雲伝運" },
    { "za", "雑" },
    { "zai", "在再載災" },
    { "zang", "蔵" },
    { "zao", "早造" },
    { "ze", "則擇責" },
    { "zen", "怎" },
    { "zeng", "曾增増" },
    { "zha", "札" },
    { "zhai", "宅擇" },
    { "zhan", "站展戰戦" },
    { "zhang", "丈長障帳張" },
    { "zhao", "找着朝招" },
    { "zhe", "者這" },
    { "zhen", "真震針" },
    { "zheng", "正整争政爭" },
    { "zhi", "之只知支止制至治直指值置智値紙製質誌織隻識職執" },
    { "zhong", "中种終重種眾" },
    { "zhou", "周州昼宙洲週" },
    { "zhu", "助主住柱株祝逐注著諸屬術" },
    { "zhuan", "专專転" },
    { "zhuang", "状狀" },
    { "zhui", "追" },
    { "zhun", "準" },
    { "zhuo", "着" },
    { "zi", "子自字姉資" },
    { "zong", "總" },
    { "zuo", "左做昨坐座作" },
    { "zu", "足祖族卒組" },
    { "zui", "最酔" },
    { "zou", "走" },
    {NULL, NULL}
};
#endif
#endif


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * lv_ime_pinyin_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}


/*=====================
 * Setter functions
 *====================*/

/**
 * Set the keyboard of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @param dict pointer to a Pinyin input method keyboard
 */
void lv_ime_pinyin_set_keyboard(lv_obj_t * obj, lv_obj_t * kb)
{
    if(kb) {
        LV_ASSERT_OBJ(kb, &lv_keyboard_class);
    }

    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->kb = kb;
    lv_obj_add_event_cb(pinyin_ime->kb, lv_ime_pinyin_kb_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_align_to(pinyin_ime->cand_panel, pinyin_ime->kb, LV_ALIGN_OUT_TOP_MID, 0, 0);
}

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @param dict pointer to a Pinyin input method dictionary
 */
void lv_ime_pinyin_set_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    init_pinyin_dict(obj, dict);
}

/**
 * Set mode, 26-key input(k26) or 9-key input(k9).
 * @param obj  pointer to a Pinyin input method object
 * @param mode   the mode from 'lv_keyboard_mode_t'
 */
void lv_ime_pinyin_set_mode(lv_obj_t * obj, lv_ime_pinyin_mode_t mode)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    LV_ASSERT_OBJ(pinyin_ime->kb, &lv_keyboard_class);

    pinyin_ime->mode = mode;

#if LV_IME_PINYIN_USE_K9_MODE
    if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
        pinyin_k9_init_data(obj);
        lv_keyboard_set_map(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1, lv_btnm_def_pinyin_k9_map,
                            (const lv_btnmatrix_ctrl_t*)default_kb_ctrl_k9_map);
        lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1);
    }
#endif
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin IME object
 * @return     pointer to the Pinyin IME keyboard
 */
lv_obj_t * lv_ime_pinyin_get_kb(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->kb;
}

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @return     pointer to the Pinyin input method candidate panel
 */
lv_obj_t * lv_ime_pinyin_get_cand_panel(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->cand_panel;
}

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @return     pointer to the Pinyin input method dictionary
 */
lv_pinyin_dict_t * lv_ime_pinyin_get_dict(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->dict;
}

/*=====================
 * Other functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_ime_pinyin_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
#ifdef CONFIG_FP
    for(btnm_i = 0; btnm_i < (LV_IME_PINYIN_CAND_TEXT_NUM); btnm_i++) {
            lv_pinyin_cand_str[py_str_i][0] = ' ';
            lv_btnm_def_pinyin_sel_map[btnm_i] = lv_pinyin_cand_str[py_str_i];
            py_str_i++;
    }
#else
    for(btnm_i = 0; btnm_i < (LV_IME_PINYIN_CAND_TEXT_NUM + 3); btnm_i++) {
        if(btnm_i == 0) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "<";
        }
        else if(btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 1)) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = ">";
        }
        else if(btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 2)) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "";
        }
        else {
            lv_pinyin_cand_str[py_str_i][0] = ' ';
            lv_btnm_def_pinyin_sel_map[btnm_i] = lv_pinyin_cand_str[py_str_i];
            py_str_i++;
        }
    }
#endif

    pinyin_ime->mode = LV_IME_PINYIN_MODE_K26;
    pinyin_ime->py_page = 0;
    pinyin_ime->ta_count = 0;
    pinyin_ime->cand_num = 0;
    lv_memset_00(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
    lv_memset_00(pinyin_ime->py_num, sizeof(pinyin_ime->py_num));
    lv_memset_00(pinyin_ime->py_pos, sizeof(pinyin_ime->py_pos));

    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(55));
    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, 0);

#if LV_IME_PINYIN_USE_DEFAULT_DICT
    init_pinyin_dict(obj, (lv_pinyin_dict_t *)lv_ime_pinyin_def_dict);
#endif

    /* Init pinyin_ime->cand_panel */
    pinyin_ime->cand_panel = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(pinyin_ime->cand_panel, (const char **)lv_btnm_def_pinyin_sel_map);
    lv_obj_set_size(pinyin_ime->cand_panel, LV_PCT(100), LV_PCT(5));
    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);

    lv_btnmatrix_set_one_checked(pinyin_ime->cand_panel, true);

    /* Set cand_panel style*/
    // Default style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, 0);
    lv_obj_set_style_border_width(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_all(pinyin_ime->cand_panel, 8, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_base_dir(pinyin_ime->cand_panel, LV_BASE_DIR_LTR, 0);

    // LV_PART_ITEMS style
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 12, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);

    // LV_PART_ITEMS | LV_STATE_PRESSED style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS | LV_STATE_PRESSED);

    /* event handler */
    lv_obj_add_event_cb(pinyin_ime->cand_panel, lv_ime_pinyin_cand_panel_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_add_event_cb(obj, lv_ime_pinyin_style_change_event, LV_EVENT_STYLE_CHANGED, NULL);

#if LV_IME_PINYIN_USE_K9_MODE
    pinyin_ime->k9_input_str_len = 0;
    pinyin_ime->k9_py_ll_pos = 0;
    pinyin_ime->k9_legal_py_count = 0;
    lv_memset_00(pinyin_ime->k9_input_str, LV_IME_PINYIN_K9_MAX_INPUT);

    pinyin_k9_init_data(obj);

    _lv_ll_init(&(pinyin_ime->k9_legal_py_ll), sizeof(ime_pinyin_k9_py_str_t));
#endif
}


static void lv_ime_pinyin_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(lv_obj_is_valid(pinyin_ime->kb))
        lv_obj_del(pinyin_ime->kb);

    if(lv_obj_is_valid(pinyin_ime->cand_panel))
        lv_obj_del(pinyin_ime->cand_panel);
}


static void lv_ime_pinyin_kb_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = lv_event_get_target(e);
    lv_obj_t * obj = lv_event_get_user_data(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

#if LV_IME_PINYIN_USE_K9_MODE
#ifdef CONFIG_FP
    static const char * k9_py_map1[8] = {"abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
    static const char * k9_py_map2[6] = {"a", "b", "c", "d", "e"};
    const char **k9_py_map;
    uint8_t k9_py_map_count;
    if(pinyin_ime->dict==lv_ime_pinyin_def_dict)
    {
        k9_py_map = k9_py_map1;
        k9_py_map_count = 8;
    }
    else
    {
        k9_py_map = k9_py_map2;
        k9_py_map_count = 6;
    }
#else
    static const char * k9_py_map[8] = {"abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
#endif
#endif

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint16_t btn_id  = lv_btnmatrix_get_selected_btn(kb);

#ifdef CONFIG_FP
        __inf("pinyin_ime->k9_input_str:btn_id:%d",btn_id);
#endif

        if(btn_id == LV_BTNMATRIX_BTN_NONE) return;

        const char * txt = lv_btnmatrix_get_btn_text(kb, lv_btnmatrix_get_selected_btn(kb));
        if(txt == NULL) return;

#if LV_IME_PINYIN_USE_K9_MODE
        if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
            lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
            uint16_t tmp_btn_str_len = strlen(pinyin_ime->input_char);
            if((btn_id >= 16) && (tmp_btn_str_len > 0) && (btn_id < (16 + LV_IME_PINYIN_K9_CAND_TEXT_NUM))) {
                tmp_btn_str_len = strlen(pinyin_ime->input_char);
                lv_memset_00(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
                strcat(pinyin_ime->input_char, txt);
                pinyin_input_proc(obj);

                for(int index = 0; index < (pinyin_ime->ta_count + tmp_btn_str_len); index++) {
                    lv_textarea_del_char(ta);
                }

                pinyin_ime->ta_count = tmp_btn_str_len;
                pinyin_ime->k9_input_str_len = tmp_btn_str_len;
                lv_textarea_add_text(ta, pinyin_ime->input_char);

                return;
            }
        }
#endif

        if(strcmp(txt, "Enter") == 0 || strcmp(txt, LV_SYMBOL_NEW_LINE) == 0) {
            pinyin_ime_clear_data(obj);
            lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
        }
        else if(strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
            // del input char
            if(pinyin_ime->ta_count > 0) {
                if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26)
                    pinyin_ime->input_char[pinyin_ime->ta_count - 1] = '\0';
#if LV_IME_PINYIN_USE_K9_MODE
                else
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count - 1] = '\0';
#endif

                pinyin_ime->ta_count = pinyin_ime->ta_count - 1;
                if(pinyin_ime->ta_count <= 0) {
                    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
#if LV_IME_PINYIN_USE_K9_MODE
#ifdef CONFIG_FP
                    pinyin_ime_clear_data(obj);
#else
                    lv_memset_00(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
                    strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
                    strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
#endif
#endif
                }
                else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) {
                    pinyin_input_proc(obj);
                }
#if LV_IME_PINYIN_USE_K9_MODE
                else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
#ifdef CONFIG_FP
                    pinyin_ime->k9_input_str_len = strlen(pinyin_ime->input_char);
                    for(int i=pinyin_ime->ta_count+1;i<LV_IME_PINYIN_K9_MAX_INPUT;i++)
                        pinyin_ime->k9_input_str[i] = 0;

                    pinyin_k9_get_legal_py1(obj, pinyin_ime->k9_input_str, k9_py_map);
#else
                    pinyin_ime->k9_input_str_len = strlen(pinyin_ime->input_char) - 1;
                    pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
#endif
                    pinyin_k9_fill_cand(obj);
                    pinyin_input_proc(obj);

#ifdef CONFIG_FP
                    __inf("pinyin_ime->k9_input_str1:%d,%d,%d,%d,%d,%d",pinyin_ime->k9_input_str[0],
                        pinyin_ime->k9_input_str[1],pinyin_ime->k9_input_str[2],pinyin_ime->k9_input_str[3],
                        pinyin_ime->k9_input_str[4],pinyin_ime->ta_count);
#endif
                }
#endif
            }
        }
        else if((strcmp(txt, "ABC") == 0) || (strcmp(txt, "abc") == 0) || (strcmp(txt, "1#") == 0)) {
            pinyin_ime->ta_count = 0;
            lv_memset_00(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
            return;
        }
        else if(strcmp(txt, LV_SYMBOL_KEYBOARD) == 0) {
            if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) {
                lv_ime_pinyin_set_mode((lv_obj_t *)pinyin_ime, LV_IME_PINYIN_MODE_K9);
            }
            else {
                lv_ime_pinyin_set_mode((lv_obj_t *)pinyin_ime, LV_IME_PINYIN_MODE_K26);
                lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_TEXT_LOWER);
            }
            pinyin_ime_clear_data(obj);
        }
        else if(strcmp(txt, LV_SYMBOL_OK) == 0) {
#ifdef CONFIG_FP
#if LV_IME_PINYIN_USE_K9_MODE
            if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
                pinyin_ime->k9_input_str_len = 0;
                pinyin_ime->k9_py_ll_pos = 0;
                pinyin_ime->k9_legal_py_count = 0;
                lv_memset_00(pinyin_ime->k9_input_str,  LV_IME_PINYIN_K9_MAX_INPUT);
            }
#endif

            pinyin_ime->ta_count = 0;
            lv_memset_00(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
#else
            pinyin_ime_clear_data(obj);
#endif
        }
        else if((pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) && ((txt[0] >= 'a' && txt[0] <= 'z') || (txt[0] >= 'A' &&
                                                                                                      txt[0] <= 'Z'))) {
            strcat(pinyin_ime->input_char, txt);
            pinyin_input_proc(obj);
            pinyin_ime->ta_count++;
        }
#if LV_IME_PINYIN_USE_K9_MODE
        else if((pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) && (txt[0] >= 'a' && txt[0] <= 'z')) {
#ifdef CONFIG_FP
            for(uint16_t i = 0; i < k9_py_map_count; i++) {
                if((strcmp(txt, k9_py_map1[i]) == 0) || (strcmp(txt, "abc ") == 0)) {
                    if(strcmp(txt, "abc ") == 0)    pinyin_ime->k9_input_str_len += strlen(k9_py_map[i]) + 1;
                    else                            pinyin_ime->k9_input_str_len += strlen(k9_py_map[i]);
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count] = 50 + i;

                    break;
                }
            }

            __inf("pinyin_ime->k9_input_str:%s",pinyin_ime->k9_input_str);
#else
            for(uint16_t i = 0; i < 8; i++) {
                if((strcmp(txt, k9_py_map[i]) == 0) || (strcmp(txt, "abc ") == 0)) {
                    if(strcmp(txt, "abc ") == 0)    pinyin_ime->k9_input_str_len += strlen(k9_py_map[i]) + 1;
                    else                            pinyin_ime->k9_input_str_len += strlen(k9_py_map[i]);
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count] = 50 + i;

                    break;
                }
            }
#endif
            pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
            pinyin_k9_fill_cand(obj);
            pinyin_input_proc(obj);
        }
        else if(strcmp(txt, LV_SYMBOL_LEFT) == 0) {
            pinyin_k9_cand_page_proc(obj, 0);
        }
        else if(strcmp(txt, LV_SYMBOL_RIGHT) == 0) {
            pinyin_k9_cand_page_proc(obj, 1);
        }
#endif
    }
}


static void lv_ime_pinyin_cand_panel_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * cand_panel = lv_event_get_target(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_user_data(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t id = lv_btnmatrix_get_selected_btn(cand_panel);
        if(id == 0) {
            pinyin_page_proc(obj, 0);
            return;
        }
        if(id == (LV_IME_PINYIN_CAND_TEXT_NUM + 1)) {
            pinyin_page_proc(obj, 1);
            return;
        }

        const char * txt = lv_btnmatrix_get_btn_text(cand_panel, id);
        lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
        uint16_t index = 0;
        for(index = 0; index < pinyin_ime->ta_count; index++)
            lv_textarea_del_char(ta);

        lv_textarea_add_text(ta, txt);

        pinyin_ime_clear_data(obj);
    }
}


static void pinyin_input_proc(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;
#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP
    if(pinyin_ime->bihua_enable)
    {
        uint8_t cand_index,num;
        if(0==pinyin_search_matching_bihua(obj, pinyin_ime->input_char)){
            return;
        }

        pinyin_ime->py_page = 0;

        for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
            memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
            lv_pinyin_cand_str[i][0] = ' ';
        }

        cand_index  = 0;
        num         = 0;
        for(uint8_t i = 0;  i< LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
            if(num>=pinyin_ime->bihua_cand_count[cand_index])
            {
                num = 0;
                cand_index++;
                if(cand_index>=BIHUA_CAND_STR_COUNT)
                    break;
                if(pinyin_ime->bihua_cand_count[cand_index]==0)
                    break;
                }
                for(uint8_t j = 0; j < 3; j++) {
                lv_pinyin_cand_str[i][j] = pinyin_ime->bihua_cand_str[cand_index][num * 3 + j];
            }
            num++;
        }

    }
    else
    {
#endif
#endif
    pinyin_ime->cand_str = pinyin_search_matching(obj, pinyin_ime->input_char, &pinyin_ime->cand_num);
    if(pinyin_ime->cand_str == NULL) {
        return;
    }

    pinyin_ime->py_page = 0;

    for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
        memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    for(uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++) {
        for(uint8_t j = 0; j < 3; j++) {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[i * 3 + j];
        }
    }
#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP
    }
#endif
#endif
    lv_obj_clear_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
}

#ifdef CONFIG_FP
void pinyin_page_proc(lv_obj_t * obj, uint16_t dir)
#else
static void pinyin_page_proc(lv_obj_t * obj, uint16_t dir)
#endif
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP
    if(pinyin_ime->bihua_enable)
    {
        uint16_t total_count;
        uint16_t page_num;
        uint16_t sur;
        uint8_t cand_index,num;
        uint16_t tmp,tmp_start;

        total_count	    = 0;
        for(uint8_t i = 0; i < BIHUA_CAND_STR_COUNT; i++)
            total_count	=  total_count + pinyin_ime->bihua_cand_count[i];
        page_num	    = total_count / LV_IME_PINYIN_CAND_TEXT_NUM;
        sur	            = total_count % LV_IME_PINYIN_CAND_TEXT_NUM;

        if(dir == 0) {
            if(pinyin_ime->py_page) {
                pinyin_ime->py_page--;
            }
        }
        else {
            if(sur == 0) {
                page_num -= 1;
            }
            if(pinyin_ime->py_page < page_num) {
                pinyin_ime->py_page++;
            }
            else return;
        }


        for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
            memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
            lv_pinyin_cand_str[i][0] = ' ';
        }

        tmp	        = 0;
        cand_index	= BIHUA_CAND_STR_COUNT;
        tmp_start	= pinyin_ime->py_page*LV_IME_PINYIN_CAND_TEXT_NUM;
        for(uint8_t i = 0; i < BIHUA_CAND_STR_COUNT; i++) {
            if(tmp + pinyin_ime->bihua_cand_count[i]>tmp_start)
            {
                cand_index = i;
                num = tmp_start-tmp;
                break;
            }
            tmp+=pinyin_ime->bihua_cand_count[i];
        }

        if(cand_index>=BIHUA_CAND_STR_COUNT)
            return;

        for(uint8_t i = 0;  i< LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
            if(num>=pinyin_ime->bihua_cand_count[cand_index])
            {
                num = 0;
                cand_index++;
                if(cand_index>=BIHUA_CAND_STR_COUNT)
                    break;
                if(pinyin_ime->bihua_cand_count[cand_index]==0)
                    break;
                }
            for(uint8_t j = 0; j < 3; j++) {
               lv_pinyin_cand_str[i][j] = pinyin_ime->bihua_cand_str[cand_index][num * 3 + j];
            }
            num++;
        }

    }
    else
    {
#endif
#endif
    uint16_t page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
    uint16_t sur = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

    if(dir == 0) {
        if(pinyin_ime->py_page) {
            pinyin_ime->py_page--;
        }
    }
    else {
        if(sur == 0) {
            page_num -= 1;
        }
        if(pinyin_ime->py_page < page_num) {
            pinyin_ime->py_page++;
        }
        else return;
    }

    for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
        memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    uint16_t offset = pinyin_ime->py_page * (3 * LV_IME_PINYIN_CAND_TEXT_NUM);
    for(uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++) {
        if((sur > 0) && (pinyin_ime->py_page == page_num)) {
            if(i > sur)
                break;
        }
        for(uint8_t j = 0; j < 3; j++) {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[offset + (i * 3) + j];
        }
    }
#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP
    }
#endif
#endif
}


static void lv_ime_pinyin_style_change_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(code == LV_EVENT_STYLE_CHANGED) {
        const lv_font_t * font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_obj_set_style_text_font(pinyin_ime->cand_panel, font, 0);
    }
}


static void init_pinyin_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    char headletter = 'a';
    uint16_t offset_sum = 0;
    uint16_t offset_count = 0;
    uint16_t letter_calc = 0;
#ifdef CONFIG_FP
    uint16_t pre_letter_calc = 0;
#endif

    pinyin_ime->dict = dict;

    for(uint16_t i = 0; ; i++) {
        if((NULL == (dict[i].py)) || (NULL == (dict[i].py_mb))) {
            headletter = dict[i - 1].py[0];
            letter_calc = headletter - 'a';
            pinyin_ime->py_num[letter_calc] = offset_count;
#ifdef CONFIG_FP
            __inf("letter_calc:%d,offset_count:%d",letter_calc,offset_count);
#endif
            break;
        }

        if(headletter == (dict[i].py[0])) {
            offset_count++;
        }
        else {
            headletter = dict[i].py[0];
            letter_calc = headletter - 'a';
#ifdef CONFIG_FP
            pinyin_ime->py_num[pre_letter_calc] = offset_count; //pinyin_ime->py_num[letter_calc - 1] = offset_count;
#else
            pinyin_ime->py_num[letter_calc - 1] = offset_count;
#endif
            offset_sum += offset_count;
            pinyin_ime->py_pos[letter_calc] = offset_sum;
#ifdef CONFIG_FP
            pre_letter_calc = letter_calc;
            __inf("dict[%d].py[0]:%d,letter_calc:%d,offset_count:%d,offset_sum:%d",i,dict[i].py[0],letter_calc,offset_count,offset_sum);
#endif

            offset_count = 1;
        }
    }
}


static char * pinyin_search_matching(lv_obj_t * obj, char * py_str, uint16_t * cand_num)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_pinyin_dict_t * cpHZ;
    uint8_t index, len = 0, offset;
    volatile uint8_t count = 0;

    if(*py_str == '\0')    return NULL;
    if(*py_str == 'i')     return NULL;
    if(*py_str == 'u')     return NULL;
    if(*py_str == 'v')     return NULL;

    offset = py_str[0] - 'a';
    len = strlen(py_str);

#ifdef CONFIG_FP
    __inf("pinyin_ime->py_pos[%d]:%d,%s",offset,pinyin_ime->py_pos[offset],py_str);
#endif
    cpHZ  = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    while(count--) {
        for(index = 0; index < len; index++) {
            if(*(py_str + index) != *((cpHZ->py) + index)) {
                break;
            }
        }

        // perfect match
        if(len == 1 || index == len) {
            // The Chinese character in UTF-8 encoding format is 3 bytes
            * cand_num = strlen((const char *)(cpHZ->py_mb)) / 3;
            return (char *)(cpHZ->py_mb);
        }
        cpHZ++;
    }
    return NULL;
}

#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP
static int pinyin_search_matching_bihua(lv_obj_t * obj, char * py_str)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_pinyin_dict_t * cpHZ;
    uint8_t index, len = 0, offset,py_len;
    volatile uint16_t count = 0;

    offset = py_str[0] - 'a';
    len = strlen(py_str);

    cpHZ  = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    pinyin_ime->cand_num = 0;
    for(uint8_t i=0;i<BIHUA_CAND_STR_COUNT;i++)
        pinyin_ime->bihua_cand_count[i] = 0;

    while(count--) {
        for(index = 0; index < len; index++) {
            if(*(py_str + index) != *((cpHZ->py) + index)) {
                break;
            }
        }

        // perfect match
        if(index == len) {
            uint8_t cand_index = 0;
            pinyin_ime->bihua_cand_str[cand_index]        = cpHZ->py_mb;
            pinyin_ime->bihua_cand_count[cand_index]    = strlen((const char *)(cpHZ->py_mb)) / 3;
            pinyin_ime->cand_num                        = pinyin_ime->bihua_cand_count[cand_index];
            cand_index++;
            if(count==0)
                return cand_index;
            count--;
            cpHZ++;
            while(count--)
            {
                py_len = strlen(cpHZ->py);
                if((py_len<5)&&(len<py_len))
                {
                    for(index = 0; index < len; index++) {
                        if(*(py_str + index) != *((cpHZ->py) + index)) {
                            break;
                        }
                    }
                    if(index != len)
                        break;
                    pinyin_ime->bihua_cand_str[cand_index]        = cpHZ->py_mb;
                    pinyin_ime->bihua_cand_count[cand_index]    = strlen((const char *)(cpHZ->py_mb)) / 3;
                    pinyin_ime->cand_num	                    += pinyin_ime->bihua_cand_count[cand_index];
                    cand_index++;
                    if(cand_index>=BIHUA_CAND_STR_COUNT)
                        break;
                }
                cpHZ++;
            }

            return cand_index;
        }

        cpHZ++;
    }
    return 0;
}
#endif
#endif

static void pinyin_ime_clear_data(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;


#if LV_IME_PINYIN_USE_K9_MODE
    if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
        pinyin_ime->k9_input_str_len = 0;
        pinyin_ime->k9_py_ll_pos = 0;
        pinyin_ime->k9_legal_py_count = 0;
        lv_memset_00(pinyin_ime->k9_input_str,  LV_IME_PINYIN_K9_MAX_INPUT);
        lv_memset_00(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
#ifdef CONFIG_FP
        _lv_ll_clear(&pinyin_ime->k9_legal_py_ll);
#endif
    }
#endif

    pinyin_ime->ta_count = 0;
    lv_memset_00(lv_pinyin_cand_str, (sizeof(lv_pinyin_cand_str)));
    lv_memset_00(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));

    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
}


#if LV_IME_PINYIN_USE_K9_MODE
static void pinyin_k9_init_data(lv_obj_t * obj)
{
    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
    for(btnm_i = 19; btnm_i < (LV_IME_PINYIN_K9_CAND_TEXT_NUM + 20); btnm_i++) {
        if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM) {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], LV_SYMBOL_RIGHT"\0");
        }
        else if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1) {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], "\0");
        }
        else {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], " \0");
        }

        lv_btnm_def_pinyin_k9_map[btnm_i] = lv_pinyin_k9_cand_str[py_str_i];
        py_str_i++;
    }

    default_kb_ctrl_k9_map[0]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[4]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[5]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[9]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[10] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[14] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[15] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 16 - 1] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
}

static void pinyin_k9_get_legal_py(lv_obj_t * obj, char * k9_input, const char * py9_map[])
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t len = strlen(k9_input);

    if((len == 0) || (len >= LV_IME_PINYIN_K9_MAX_INPUT)) {
        return;
    }

    char py_comp[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int mark[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int index = 0;
    int flag = 0;
    int count = 0;

    uint32_t ll_len = 0;
    ime_pinyin_k9_py_str_t * ll_index = NULL;

    ll_len = _lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);
    ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);

    while(index != -1) {
        if(index == len) {
            if(pinyin_k9_is_valid_py(obj, py_comp)) {
                if((count >= ll_len) || (ll_len == 0)) {
                    ll_index = _lv_ll_ins_tail(&pinyin_ime->k9_legal_py_ll);
                    strcpy(ll_index->py_str, py_comp);
                }
                else if((count < ll_len)) {
                    strcpy(ll_index->py_str, py_comp);
                    ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index);
                }
                count++;
#ifdef CONFIG_FP
                __inf("count:%d,py_comp:%s",count,py_comp);
#endif
            }
            index--;
        }
        else {
            flag = mark[index];
            if(flag < strlen(py9_map[k9_input[index] - '2'])) {
                py_comp[index] = py9_map[k9_input[index] - '2'][flag];
                mark[index] = mark[index] + 1;
                index++;
            }
            else {
                mark[index] = 0;
                index--;
            }
#ifdef CONFIG_FP
            __inf("index:%d,len:%d,py_comp:%s",index,len,py_comp);
#endif
        }
    }
#ifdef CONFIG_FP
    __inf("count:%d,py_comp:%s",count,py_comp);
#endif

    if(count > 0) {
        pinyin_ime->ta_count++;
        pinyin_ime->k9_legal_py_count = count;
    }
}

#ifdef CONFIG_FP
/*

pinyin_k9_get_legal_py1（）函数中
inyin_ime->ta_count变量不变化
*/
static void pinyin_k9_get_legal_py1(lv_obj_t * obj, char * k9_input, const char * py9_map[])
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t len = strlen(k9_input);

    if((len == 0) || (len >= LV_IME_PINYIN_K9_MAX_INPUT)) {
        return;
    }

    char py_comp[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int mark[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int index = 0;
    int flag = 0;
    int count = 0;

    uint32_t ll_len = 0;
    ime_pinyin_k9_py_str_t * ll_index = NULL;

    _lv_ll_clear(&pinyin_ime->k9_legal_py_ll);
    ll_len = _lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);
    ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);

    while(index != -1) {
        if(index == len) {
            if(pinyin_k9_is_valid_py(obj, py_comp)) {
                if((count >= ll_len) || (ll_len == 0)) {
                    ll_index = _lv_ll_ins_tail(&pinyin_ime->k9_legal_py_ll);
                    strcpy(ll_index->py_str, py_comp);
                }
                else if((count < ll_len)) {
                    strcpy(ll_index->py_str, py_comp);
                    ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index);
                }
                count++;
            }
            index--;
        }
        else {
            flag = mark[index];
            if(flag < strlen(py9_map[k9_input[index] - '2'])) {
                py_comp[index] = py9_map[k9_input[index] - '2'][flag];
                mark[index] = mark[index] + 1;
                index++;
            }
            else {
                mark[index] = 0;
                index--;
            }
        }
    }

    if(count > 0) {
        //pinyin_ime->ta_count++;
        pinyin_ime->k9_legal_py_count = count;
    }
}
#endif

/*true: visible; false: not visible*/
static bool pinyin_k9_is_valid_py(lv_obj_t * obj, char * py_str)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_pinyin_dict_t * cpHZ = NULL;
    uint8_t index = 0, len = 0, offset = 0;
    volatile uint8_t count = 0;

    if(*py_str == '\0')    return false;
    if(*py_str == 'i')     return false;
    if(*py_str == 'u')     return false;
    if(*py_str == 'v')     return false;

    offset = py_str[0] - 'a';
    len = strlen(py_str);

    cpHZ  = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

#ifdef CONFIG_FP
    __inf("*py_str:%d,count:%d,offset:%d",*py_str,count,offset);
#endif

    while(count--) {
        for(index = 0; index < len; index++) {
            if(*(py_str + index) != *((cpHZ->py) + index)) {
                break;
            }

#ifdef CONFIG_FP
            __inf("*(py_str + index):%d,*((cpHZ->py) + index:%d",*py_str,*((cpHZ->py) + index));
#endif
        }

        // perfect match
        if(len == 1 || index == len) {
            return true;
        }
        cpHZ++;
    }
    return false;
}


static void pinyin_k9_fill_cand(lv_obj_t * obj)
{
    static uint16_t len = 0;
    uint16_t index = 0, tmp_len = 0;
    ime_pinyin_k9_py_str_t * ll_index = NULL;

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    tmp_len = pinyin_ime->k9_legal_py_count;

    if(tmp_len != len) {
        lv_memset_00(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
        len = tmp_len;
    }

    ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
    strcpy(pinyin_ime->input_char, ll_index->py_str);
    while(ll_index) {
        if((index >= LV_IME_PINYIN_K9_CAND_TEXT_NUM) || \
           (index >= pinyin_ime->k9_legal_py_count))
            break;

        strcpy(lv_pinyin_k9_cand_str[index], ll_index->py_str);
        ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
        index++;
    }
    pinyin_ime->k9_py_ll_pos = index;

    lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    for(index = 0; index < pinyin_ime->k9_input_str_len; index++) {
        lv_textarea_del_char(ta);
    }
    pinyin_ime->k9_input_str_len = strlen(pinyin_ime->input_char);
    lv_textarea_add_text(ta, pinyin_ime->input_char);
}


static void pinyin_k9_cand_page_proc(lv_obj_t * obj, uint16_t dir)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    uint16_t ll_len =  _lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);

    if((ll_len > LV_IME_PINYIN_K9_CAND_TEXT_NUM) && (pinyin_ime->k9_legal_py_count > LV_IME_PINYIN_K9_CAND_TEXT_NUM)) {
        ime_pinyin_k9_py_str_t * ll_index = NULL;
        int count = 0;

        ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
        while(ll_index) {
            if(count >= pinyin_ime->k9_py_ll_pos)   break;

            ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
            count++;
        }

        if((NULL == ll_index) && (dir == 1))   return;

        lv_memset_00(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");

        // next page
        if(dir == 1) {
            count = 0;
            while(ll_index) {
                if(count >= (LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1))
                    break;

                strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
                count++;
            }
            pinyin_ime->k9_py_ll_pos += count - 1;

        }
        // previous page
        else {
            count = LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1;
            ll_index = _lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index);
            while(ll_index) {
                if(count < 0)  break;

                strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = _lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the previous list*/
                count--;
            }

            if(pinyin_ime->k9_py_ll_pos > LV_IME_PINYIN_K9_CAND_TEXT_NUM)
                pinyin_ime->k9_py_ll_pos -= 1;
        }

        lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
    }
}

#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP

char * lv_pinyin_k9_cand_str_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 3];

#if 0
lv_obj_t *  pinyin_k9_cand_panel_create()
{
    lv_obj_t * cand_panel;
    /* Init pinyin_ime->cand_panel */
    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;

    for(btnm_i = 19; btnm_i < (LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21); btnm_i++) {
        if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM) {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], "\0");//strcpy(lv_pinyin_k9_cand_str[py_str_i], LV_SYMBOL_RIGHT"\0");
        }
        else if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1) {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], "\0");
        }
        else {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], " \0");
        }

        //lv_pinyin_k9_cand_str_map[btnm_i] = lv_pinyin_k9_cand_str[py_str_i];
        lv_pinyin_k9_cand_str_map[py_str_i] = lv_pinyin_k9_cand_str[py_str_i];
        py_str_i++;
    }


    cand_panel = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(cand_panel, (const char **)lv_pinyin_k9_cand_str_map);
    lv_obj_set_size(cand_panel, LV_PCT(100), LV_PCT(5));
    //lv_obj_add_flag(cand_panel, LV_OBJ_FLAG_HIDDEN);

    lv_btnmatrix_set_one_checked(cand_panel, true);

    /* Set cand_panel style*/
    // Default style
    lv_obj_set_style_bg_opa(cand_panel, LV_OPA_0, 0);
    lv_obj_set_style_border_width(cand_panel, 0, 0);
    lv_obj_set_style_pad_all(cand_panel, 8, 0);
    lv_obj_set_style_pad_gap(cand_panel, 0, 0);
    lv_obj_set_style_radius(cand_panel, 0, 0);
    lv_obj_set_style_pad_gap(cand_panel, 0, 0);
    lv_obj_set_style_base_dir(cand_panel, LV_BASE_DIR_LTR, 0);

    // LV_PART_ITEMS style
    lv_obj_set_style_radius(cand_panel, 12, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(cand_panel, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(cand_panel, LV_OPA_0, LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(cand_panel, LV_OPA_0, LV_PART_ITEMS);

    // LV_PART_ITEMS | LV_STATE_PRESSED style
    lv_obj_set_style_bg_opa(cand_panel, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(cand_panel, lv_color_white(), LV_PART_ITEMS | LV_STATE_PRESSED);

    return cand_panel;
}
#else
void  pinyin_k9_cand_panel_str_init()
{
    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;

    for(btnm_i = 19; btnm_i < (LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21); btnm_i++) {
        if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM) {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], "\0");//strcpy(lv_pinyin_k9_cand_str[py_str_i], LV_SYMBOL_RIGHT"\0");
        }
        else if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1) {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], "\0");
        }
        else {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], " \0");
        }

        //lv_pinyin_k9_cand_str_map[btnm_i] = lv_pinyin_k9_cand_str[py_str_i];
        lv_pinyin_k9_cand_str_map[py_str_i] = lv_pinyin_k9_cand_str[py_str_i];
        py_str_i++;
    }

}
#endif


void pinyin_k9_enable_bihua(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->bihua_enable = 1;
}

void pinyin_k9_disable_bihua(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->bihua_enable = 0;
}
#endif
#endif

#endif  /*LV_IME_PINYIN_USE_K9_MODE*/

#endif  /*LV_USE_IME_PINYIN*/


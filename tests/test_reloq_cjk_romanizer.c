#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vendor/openevv61/src/eci/lang/eci_rom.h"

extern EvvRom *reloq_chinese_romanizer(const char *dir);

static void require_reading(const char *text, const char *reading)
{
    EvvRom *r = reloq_chinese_romanizer(NULL);
    char *out = NULL;
    int32_t status;

    if (!r) {
        fputs("could not create the Chinese romanizer\n", stderr);
        exit(1);
    }
    status = r->ops->addText(r, text, (int32_t)strlen(text), 0);
    if (status != 0)
        status = r->ops->processSentence(r, &out, 0);
    if (status != 2 || !out || !strstr(out, reading)) {
        fprintf(stderr, "reading failed: expected %s in %s\n", reading,
                out ? out : "(none)");
        if (out) r->ops->release(r);
        exit(1);
    }
    r->ops->release(r);
}

int main(void)
{
    /* 测试了 */
    require_reading("\xe6\xb5\x8b\xe8\xaf\x95\xe4\xba\x86", " `gT_le0");
    /* 了 */
    require_reading("\xe4\xba\x86", " `gT_le0");
    /* 血战到底 */
    require_reading("\xe8\xa1\x80\xe6\x88\x98\xe5\x88\xb0\xe5\xba\x95",
                    " `gN_xue4zhan4");
    /* 空格 */
    require_reading("\xe7\xa9\xba\xe6\xa0\xbc", " `gN_kong4ge2");
    /* 东西 */
    require_reading("\xe4\xb8\x9c\xe8\xa5\xbf", " `gN_dong1xi0");
    /* 单选按钮 */
    require_reading("\xe5\x8d\x95\xe9\x80\x89\xe6\x8c\x89\xe9\x92\xae",
                    " `gN_dan1xuan3an4niu3");
    /* 可恶 */
    require_reading("\xe5\x8f\xaf\xe6\x81\xb6", " `gA_wu4");
    /* 呢 */
    require_reading("\xe5\x91\xa2", " `gH_ne0");
    /* 采样率 */
    require_reading("\xe9\x87\x87\xe6\xa0\xb7\xe7\x8e\x87",
                    " `gV_luu4");
    /* 哔哩哔哩 */
    require_reading("\xe5\x93\x94\xe5\x93\xa9\xe5\x93\x94\xe5\x93\xa9",
                    " `gN_bi4li3bi4li3");
    /* 地理 */
    require_reading("\xe5\x9c\xb0\xe7\x90\x86", " `gN_di4li3");
    /* 大写 */
    require_reading("\xe5\xa4\xa7\xe5\x86\x99", "xie3");
    /* 大写字母 */
    require_reading("\xe5\xa4\xa7\xe5\x86\x99\xe5\xad\x97\xe6\xaf\x8d",
                    "zi4mu3");
    /* 大黄蜂 */
    require_reading("\xe5\xa4\xa7\xe9\xbb\x84\xe8\x9c\x82",
                    "da4huang2feng1");
    /* 开启了 */
    require_reading("\xe5\xbc\x80\xe5\x90\xaf\xe4\xba\x86", " `gT_le0");
    /* 粘贴 */
    require_reading("\xe7\xb2\x98\xe8\xb4\xb4", " `gV_zhan1tie1");
    /* 拿着 */
    require_reading("\xe6\x8b\xbf\xe7\x9d\x80", " `gT_zhe0");
    /* 看着 */
    require_reading("\xe7\x9c\x8b\xe7\x9d\x80", " `gT_zhe0");
    /* 着急 keeps its lexical reading. */
    require_reading("\xe7\x9d\x80\xe6\x80\xa5", "zhao2ji2");
    /* Sleep-result zhao2 differs from the ongoing-aspect zhe0. */
    require_reading("\xe7\x9d\xa1\xe7\x9d\x80", "shui4zhao2");
    /* 照片 uses the standard compound reading. */
    require_reading("\xe7\x85\xa7\xe7\x89\x87", "zhao4pian4");
    /* 合并好了 uses the aspect-particle reading after a verb phrase. */
    require_reading("\xe5\x90\x88\xe5\xb9\xb6\xe5\xa5\xbd\xe4\xba\x86", "le0");
    /* Sentence-final particles use the neutral tone after a spoken word. */
    require_reading("\xe5\xaf\xb9\xe5\x91\x80", "ya0");
    require_reading("\xe5\xa5\xbd\xe5\x90\x97", "ma0");
    /* The verb-complement particle is neutral. */
    require_reading("\xe8\xb7\x91\xe5\xbe\x97", "de0");
    /* The 丶 radical is not the spoken word 点. */
    require_reading("\xe4\xb8\xb6", "zhu3");
    /* Common particles and compounds that the legacy dictionary gets wrong. */
    require_reading("\xe5\x93\x8e\xe5\x91\x80", "ya0");
    require_reading("\xe5\xa5\xbd\xe5\x95\xa6", "la0");
    require_reading("\xe5\x88\x86\xe5\xbc\x80", "fen1kai1");
    require_reading("\xe5\x9c\xa8\xe4\xba\x8e", "zai4yu2");
    require_reading("\xe7\xbc\x98\xe5\x88\x86", "yuan2fen4");
    /* 一号 is a lexical exception to ordinary 一 tone sandhi. */
    require_reading("\xe4\xb8\x80\xe5\x8f\xb7", "yi1hao4");
    /* 小北 */
    require_reading("\xe5\xb0\x8f\xe5\x8c\x97", "xiao2");
    /* 小米 does not chain its second syllable into the next word. */
    require_reading(
        "\xe5\xb0\x8f\xe7\xb1\xb3\xe4\xba\xa7\xe5\x93\x81\xe7\xa7\x91"
        "\xe6\x8a\x80\xe4\xba\xa4\xe6\xb5\x81\xe7\xbe\xa4", " `gQ_mi3");
    /* 你好 */
    require_reading("\xe4\xbd\xa0\xe5\xa5\xbd", "ni2hao3");
    /* 不会 */
    require_reading("\xe4\xb8\x8d\xe4\xbc\x9a", "bu2");
    /* 一定 */
    require_reading("\xe4\xb8\x80\xe5\xae\x9a", "yi2");
    require_reading("\xe9\x87\x8d\xe7\xbd\xae", "chong2");
    require_reading("\xe9\x87\x8d\xe6\x9d\xa5", "chong2");
    require_reading("\xe9\x87\x8d\xe8\xaf\x95", "chong2");
    require_reading("\xe9\x87\x8d\xe8\xbe\x93", "chong2");
    require_reading("\xe9\x87\x8d\xe5\xa4\x8d", "chong2");
    /* Lexical corrections must use the matched word, not trailing lookahead. */
    require_reading("缘分让我们相遇", "yuan2fen4");
    require_reading("这是缘分不是巧合", "yuan2fen4");
    require_reading("緣分讓我們相遇", "yuan2fen4");
    require_reading("分开以后再说", "fen1kai1");
    require_reading("合并好了再试", "hao3le0");
    require_reading("大家去酒吧", "da4");
    require_reading("名分", "ming2fen4");
    require_reading("命中", "ming4zhong4");
    require_reading("命中目标", "ming4zhong4");
    require_reading("命中率很高", "ming4zhong4lv4");
    require_reading("中间", "zhong1jian1");
    require_reading("命中目标以后继续", "ji4xu4");
    require_reading("天分很高", "tian1fen4");
    require_reading("养分充足", "yang3fen4");
    require_reading("充分准备", "chong1fen1");
    require_reading("分内的事", "fen4");
    require_reading("本分", "ben3fen4");
    require_reading("情分", "qing2fen4");
    require_reading("过分", "guo4fen4");
    require_reading("分外", "fen4wai4");
    require_reading("分钟", "fen1zhong1");
    require_reading("分数", "fen1shu4");
    require_reading("分别", "fen1bie2");
    static const struct {const char *word,*reading;} contexts[]={
        {"\xe5\x91\xbd\xe4\xb8\xad","ming4zhong4"},
        {"\xe5\x91\xbd\xe4\xb8\xad\xe7\x9b\xae\xe6\xa0\x87","ming4zhong4"},
        {"\xe5\x91\xbd\xe4\xb8\xad\xe7\x8e\x87","ming4zhong4lv4"},
        {"\xe8\xa1\x8c\xe5\x8f\xb7","hang2hao4"},
        {"\xe8\xa1\x8c\xe6\x95\xb0","hang2shu4"},
        {"\xe5\xbd\x93\xe5\x89\x8d\xe8\xa1\x8c","hang2"},
        {"\xe4\xb8\x80\xe8\xa1\x8c\xe6\x96\x87\xe5\xad\x97","yi4hang2"},
        {"\xe4\xb8\xa4\xe8\xa1\x8c\xe6\x96\x87\xe5\xad\x97","liang3hang2"},
        {"\xe9\x93\xb6\xe8\xa1\x8c","yin2hang2"},
        {"\xe8\xa1\x8c\xe4\xb8\xba","xing2wei2"},
        {"\xe8\xa1\x8c\xe8\xb5\xb0","xing2zou3"},
        {"\xe8\xbf\x90\xe8\xa1\x8c","yun4xing2"},
        {"\xe4\xb8\x8d\xe8\xa1\x8c","bu4xing2"},
        {"得去","dei3qu4"},
        {"着手","zhuo2shou3"},
        {"着重","zhuo2zhong4"},
        {"着想","zhuo2xiang3"},
        {"着眼","zhuo2yan3"},
        {"着装","zhuo2zhuang1"},
        {"着落","zhuo2luo4"},
        {"着实","zhuo2shi2"},
        {"了解","liao2jie3"},
        {"重创","zhong4chuang1"},
        {"重复","chong2fu4"},
        {"重量","zhong4liang4"},
        {"折本","she2ben3"},
        {"下载","xia4zai3"},
        {"文件夹","wen2jian4jia1"},
        {"复选框","fu4xuan3kuang4"},
        {"边框","bian1kuang4"},
        {"酒吧","jiu3ba1"},
        {"睡着","shui4zhao2"},
        {"框","kuang4"},
        {"还款","huan2"},
        {"还贷","huan2"},
        {"编辑框","kuang4"},
        {"输入框","kuang4"},
        {"对话框","kuang4"},
        {"选择框","kuang4"},
        {"银行","yin2hang2"},
        {"行业","hang2ye4"},
        {"行为","xing2wei2"},
        {"音乐","yin1yue4"},
        {"快乐","kuai4le4"},
        {"还有","hai2you3"},
        {"还是","hai2shi0"},
        {"着急","zhao2ji2"},
        {"着火","zhao2huo3"},
        {"着力","zhuo2li4"},
        {"获得","huo4de2"},
        {"觉得","jue2de0"},
        {"网吧","wang3ba1"},
        {"记载","ji4zai3"},
        {"载入","zai4ru4"},
        {"折叠","zhe2die2"},
        {"折腾","zhe1teng0"},
        {"重要","zhong4yao4"},
        {"重启","chong2"},
    };
    for(size_t i=0;i<sizeof contexts/sizeof *contexts;++i) {
        char sentence[256];
        require_reading(contexts[i].word,contexts[i].reading);
        snprintf(sentence,sizeof sentence,"测试%s后继续",contexts[i].word);
        require_reading(sentence,contexts[i].reading);
        require_reading(sentence,"ji4xu4");
    }
    require_reading("请看着屏幕", "zhe0");
    /* Modal 得 (děi) must not fall through the preceding-word particle rule. */
    require_reading("都得", "dei3");
    require_reading("都得完", "dei3");
    require_reading("都得死", "dei2");
    require_reading("他们都得死", "dei2");
    require_reading("全都得完后继续", "dei3");
    require_reading("全都得完后继续", "ji4xu4");
    require_reading("都得快", "dei3");
    require_reading("都得这么做", "dei3");
    require_reading("大家都得参加", "dei3");
    require_reading("你得注意", "dei3");
    require_reading("必须得做", "dei3");
    require_reading("得先处理", "dei3");
    require_reading("我得走", "dei3");
    require_reading("你还得去", "hai2dei3");
    require_reading("还得", "hai2dei3");
    require_reading("总得解决", "dei3");
    require_reading("总得", "dei3");
    require_reading("非得做", "dei3");
    /* Complement, lexical and neutral-verb uses stay unchanged. */
    require_reading("得到", "de2dao4");
    require_reading("觉得", "de0");
    require_reading("跑得快", "de0");
    require_reading("看得懂", "de0");
    require_reading("跑得真快", "de0");
    require_reading("笑得开心", "de0");
    require_reading("气得发抖", "de0");
    require_reading("急得跳脚", "de0");
    require_reading("不得不去", "bu4de2bu2");
    require_reading("长得高", "de0");
    require_reading("心得", "xin1de2");
    require_reading("心得体会", "xin1de2");
    require_reading("恨不得", "hen4bu4de2");
    require_reading("测试都得继续", "dei3");
    require_reading("测试得先处理后继续", "dei3");
    require_reading("测试得先处理后继续", "ji4xu4");
    require_reading("命令行", "ming4ling4hang2");
    require_reading("打开命令行窗口后继续", "ming4ling4hang2");
    require_reading("打开命令行窗口后继续", "ji4xu4");
    require_reading("命令行工具", "ming4ling4hang2");
    require_reading("指令行", "hang2");
    require_reading("命令行走", "ming4ling4xing2");
    require_reading("命令行走", "zou3");
    require_reading("运行命令行", "yun4xing2");
    require_reading("运行命令行", "ming4ling4hang2");
    require_reading("行首", "hang2shou3");
    require_reading("行尾", "hang2wei3");
    require_reading("移动到行首后继续", "hang2shou3");
    require_reading("移动到行尾后继续", "hang2wei3");
    require_reading("移动到行尾后继续", "ji4xu4");
    require_reading("解锁", "jie2suo3");
    require_reading("解好", "jie2");
    require_reading("解好", "hao3");
    require_reading("解送", "jie4song4");
    require_reading("押解", "ya1jie4");
    require_reading("解数", "xie4shu4");
    require_reading("手机解锁", "jie2suo3");
    require_reading("锁定和解锁后继续", "jie2suo3");
    require_reading("锁定和解锁后继续", "ji4xu4");
    require_reading("了解锁定状态", "liao2");
    require_reading("了解锁定状态", "ding4");
    /* Standalone @ in Chinese context uses the English property. */
    require_reading("\xe4\xbd\xa0\xe5\xa5\xbd@\xe4\xb8\x96\xe7\x95\x8c", " `g9_at");
    /* Email addresses remain a single English token. */
    require_reading("abc@example.com", "abc@example.com");
    /* @ between Chinese and Latin uses the English property. */
    require_reading("\xe4\xbd\xa0\xe5\xa5\xbd@world", " `g9_at");
    /* Keep the native symbol handler after the at-sign change. */
    require_reading("你好|世界", "\\|");
    puts("reloq_cjk_romanizer: OK");
    return 0;
}

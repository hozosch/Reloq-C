package org.reloq.tts;

public final class NumberReadingTest {
    private static int checks;
    private static void require(boolean value,String name)
        {++checks;if(!value)throw new AssertionError("check "+checks+": "+name);}
    public static void main(String[] arguments) {
        for (Catalog.Language language:Catalog.LANGUAGES) {
            if (language.id==0x60000||language.id==0x60001) continue;
            for(int digit=0;digit<=9;++digit) {
                String value=Integer.toString(digit);
                require(value.equals(NumberReading.prepare(value,NumberReading.AUTO,language.id)),
                    "non-Mandarin auto digit "+value+" in "+language.name);
            }
            for (String text:new String[]{"2","200","2 items","２","102"})
                require(text.equals(NumberReading.prepare(text,NumberReading.AUTO,language.id)),
                    "non-Mandarin number "+text+" in "+language.name);
            require(NumberReading.prepare("13800138000",NumberReading.DIGITS,language.id)
                .equals(NumberReading.groupDigits("13800138000",language.id,true)),
                "non-Mandarin phone digits in "+language.name);
        }
        require(NumberReading.prepare("13800138000",NumberReading.VALUE,0x60000)
            .equals("13800138000"),"value mode");
        require(NumberReading.prepare("13800138000",NumberReading.DIGITS,0x60000)
            .equals("\u5E7A 3 8\uFF0C0 0 \u5E7A 3\uFF0C8 0 0 0"),"phone digits");
        require(NumberReading.prepare("13800138000",NumberReading.AUTO,0x60000)
            .equals("\u5E7A 3 8\uFF0C0 0 \u5E7A 3\uFF0C8 0 0 0"),"auto phone keeps its grouping");
        require(NumberReading.prepare("1234,5678",NumberReading.AUTO,0x60000)
            .equals("幺二三四,五六七八"),"local landline keeps a 4+4 boundary");
        require(NumberReading.prepare("\u5EA7\u673A\uFF1A12345678",NumberReading.AUTO,0x60000)
            .equals("\u5EA7\u673A\uFF1A幺二三四\uFF0C五六七八"),"labeled local landline is digit mode");
        require(NumberReading.prepare("\u5EA7\u673A\u53F71234567",NumberReading.AUTO,0x60000)
            .equals("\u5EA7\u673A\u53F7幺二三\uFF0C四五六七"),"labeled seven-digit local landline uses 3+4 groups");
        require(NumberReading.prepare("0898,1234,5678 座机号12345678",NumberReading.AUTO,0x60000)
            .equals("零八九八,幺二三四,五六七八 座机号幺二三四，五六七八"),
            "earlier landline replacement does not shift later label matching");
        require(NumberReading.prepare("0898,1234,5678",NumberReading.AUTO,0x60000)
            .equals("零八九八,幺二三四,五六七八"),"four-digit area code landline preserves groups");
        require(NumberReading.prepare("010,1234567",NumberReading.AUTO,0x60000)
            .equals("零幺零,幺二三，四五六七"),"three-digit area code plus seven-digit local line");
        require(NumberReading.prepare("01012345678",NumberReading.AUTO,0x60000)
            .equals("零幺零\uFF0C幺二三四\uFF0C五六七八"),"compact three-digit area code landline");
        require(NumberReading.prepare("0101234567",NumberReading.AUTO,0x60000)
            .equals("零幺零\uFF0C幺二三\uFF0C四五六七"),"compact three-digit area code with seven local digits");
        require(NumberReading.prepare("02112345678",NumberReading.AUTO,0x60000)
            .equals("零二幺\uFF0C幺二三四\uFF0C五六七八"),"another three-digit area code is detected structurally");
        require(NumberReading.prepare("089812345678",NumberReading.AUTO,0x60000)
            .equals("零八九八\uFF0C幺二三四\uFF0C五六七八"),"compact four-digit area code landline");
        require(NumberReading.prepare("4008201166",NumberReading.AUTO,0x60000)
            .equals("四零零\uFF0C八二零\uFF0C幺幺六六"),
            "400 service number uses 3+3+4 groups");
        require(NumberReading.prepare("8008201166",NumberReading.AUTO,0x60000)
            .equals("八零零\uFF0C八二零\uFF0C幺幺六六"),
            "800 service number uses 3+3+4 groups");
        require(NumberReading.prepare("客服：4008201166。",NumberReading.AUTO,0x60000)
            .equals("客服：四零零\uFF0C八二零\uFF0C幺幺六六。"),
            "400 service number is grouped inside surrounding text");
        require(NumberReading.prepare("QQ号4008201166",NumberReading.AUTO,0x60000)
            .equals("QQ号4 0\uFF0C0 8 2 0\uFF0C1 1 6 6"),
            "QQ identifier is not reclassified as a 400 service number");
        require(NumberReading.prepare("0898,1234,5678",NumberReading.VALUE,0x60000)
            .equals("零八九八,幺二三四,五六七八"),"landline overrides value mode");
        require(NumberReading.prepare("0898,1234,5678",NumberReading.DIGITS,0x60000)
            .equals("零八九八,幺二三四,五六七八"),"landline digit mode has no cardinal reading");
        require(NumberReading.prepare("12345678",NumberReading.VALUE,0x60000)
            .equals("12345678"),"unlabeled local integer stays numeric in value mode");
        require(NumberReading.prepare("12345566",NumberReading.AUTO,0x60000)
            .equals("\u5E7A\u4E8C\u4E09\u56DB\uFF0C\u4E94\u4E94\u516D\u516D"),
            "standalone local landline needs no label or comma");
        require(NumberReading.prepare("12345678",NumberReading.AUTO,0x60000)
            .equals("\u5E7A\u4E8C\u4E09\u56DB\uFF0C\u4E94\u516D\u4E03\u516B"),
            "standalone eight-digit number defaults to 4+4 local landline groups");
        require(NumberReading.prepare("\u6D4B\u8BD5\uFF1A12345678\u3002",NumberReading.AUTO,0x60000)
            .equals("\u6D4B\u8BD5\uFF1A\u5E7A\u4E8C\u4E09\u56DB\uFF0C\u4E94\u516D\u4E03\u516B\u3002"),
            "unlabeled eight-digit local number uses landline groups inside punctuation");
        require(NumberReading.prepare(" 87654321 ",NumberReading.AUTO,0x60001)
            .equals(" \u516B\u4E03\u516D\u4E94\uFF0C\u56DB\u4E09\u4E8C\u5E7A "),
            "standalone local landline groups other numbers with surrounding whitespace");
        require(NumberReading.prepare("12345566",NumberReading.DIGITS,0x60000)
            .equals("1 2 3 4\uFF0C5 5 6 6"),"explicit digit mode keeps its digit names");
        require(NumberReading.prepare("QQ\u53F712345566",NumberReading.AUTO,0x60000)
            .equals("QQ\u53F71 2 3 4 5 5 6 6"),"eight-digit QQ identifier stays continuous");
        require(NumberReading.prepare("12345566\u5143",NumberReading.AUTO,0x60000)
            .equals("12345566\u5143"),"eight-digit amount is not a landline");
        require(NumberReading.prepare("10000000",NumberReading.AUTO,0x60000)
            .equals("10000000"),"round eight-digit quantity is not a landline");
        require(NumberReading.prepare("12345566",NumberReading.AUTO,0x10000)
            .equals("12345566"),"non-Mandarin base keeps its auto number behavior");
        require(NumberReading.prepare("QQ\u53F71234,5678",NumberReading.AUTO,0x60000)
            .equals("QQ\u53F71 2 3 4,5 6 7 8"),"QQ separators are not reclassified as a landline");
        require(NumberReading.prepare("\u624b\u673a\u53f713800138001",NumberReading.AUTO,0x60000)
            .equals("\u624b\u673a\u53f7\u5E7A 3 8\uFF0C0 0 \u5E7A 3\uFF0C8 0 0 \u5E7A"),
            "labeled phone cannot be captured by the bare-ID heuristic");
        require(NumberReading.prepare("QQ\u7fa4\u53f7\uff1a123456789",
            NumberReading.AUTO,0x60000)
            .equals("QQ\u7fa4\u53f7\uff1a1 2\uFF0C3 4 5 6\uFF0C7 8 9"),"nine-digit QQ number uses 2+4+3 groups");
        require(NumberReading.prepare("2026\u5e74",NumberReading.AUTO,0x60000)
            .equals("2026\u5e74"),"ordinary value");
        require(NumberReading.prepare("\u5feb\u9012\u5355\u53f7SF1234567890123",
            NumberReading.AUTO,0x60000)
            .equals("\u5feb\u9012\u5355\u53f7SF1 2 3 4\uFF0C5 6 7 8\uFF0C9 0 1 2\uFF0C3"),
            "tracking digits");
        require(NumberReading.endsWithPausePunctuation(
            "\u81ea\u52a8\u8bed\u8a00\u6d4b\u8bd5\uff1a\u4f60\u597d\u4e16\u754c\uff0c"),
            "boundary comma");
        require(NumberReading.prepare("https://example.com/path",NumberReading.VALUE,0x60000)
            .equals("https: \u659c\u6760 \u659c\u6760 example \u70B9 com \u659c\u6760 path"),
            "url slashes");
        require(NumberReading.prepare("QQ\u53f712345",NumberReading.AUTO,0x60000)
            .equals("QQ\u53f71 2 3 4 5"),"short qq digits");
        require(NumberReading.prepare("QQ\u53f71234567890",NumberReading.AUTO,0x60000)
            .equals("QQ\u53f71 2\uFF0C3 4 5 6\uFF0C7 8 9 0"),"ten-digit QQ number uses 2+4+4 groups");
        require(NumberReading.prepare("712345678\u8fd9\u662fQQ\u53f7",NumberReading.AUTO,0x60000)
            .equals("7 1\uFF0C2 3 4 5\uFF0C6 7 8\u8fd9\u662fQQ\u53f7"),"trailing QQ label uses 2+4+3 groups");
        require(NumberReading.prepare("3654321098\uFF0C\u8fd9\u4e9bQQ\u53f7",NumberReading.AUTO,0x60000)
            .equals("3 6\uFF0C5 4 3 2\uFF0C1 0 9 8\uFF0C\u8fd9\u4e9bQQ\u53f7"),"trailing plural QQ label uses 2+4+4 groups");
        require(NumberReading.prepare("QQ\u53f7712345678\uFF0C\u6709200\u4e2a\u4eba",NumberReading.AUTO,0x60000)
            .equals("QQ\u53f77 1\uFF0C2 3 4 5\uFF0C6 7 8\uFF0C\u6709\u4E24\u767E\u4E2A\u4eba"),
            "QQ identifier context does not leak to later quantity");
        require(NumberReading.prepare("712345678\u5143",NumberReading.AUTO,0x60000)
            .equals("712345678\u5143"),"quantity remains a quantity");
        require(NumberReading.prepare("712345678 \u5143",NumberReading.AUTO,0x60000)
            .equals("712345678 \u5143"),"spaced currency remains a quantity");
        require(NumberReading.prepare("712345678 \u4eba",NumberReading.AUTO,0x60000)
            .equals("712345678 \u4eba"),"spaced people count remains a quantity");
        require(NumberReading.prepare("\u7B2C 712345678",NumberReading.AUTO,0x60000)
            .equals("\u7B2C 712345678"),"ordinal is not inferred as a bare identifier");
        require(NumberReading.prepare("20261120",NumberReading.AUTO,0x60000)
            .equals("20261120"),"compact date remains numeric");
        require(NumberReading.prepare("712345678",NumberReading.AUTO,0x60000)
            .equals("7 1\uFF0C2 3 4 5\uFF0C6 7 8"),"bare nine-digit identifier uses 2+4+3 groups");
        require(NumberReading.prepare("3654321098",NumberReading.AUTO,0x60000)
            .equals("3 6\uFF0C5 4 3 2\uFF0C1 0 9 8"),"bare ten-digit identifier uses 2+4+4 groups");
        require(NumberReading.prepare("QQ\u53f712",NumberReading.AUTO,0x60000)
            .equals("QQ\u53f71 2"),"two-digit QQ identifier");
        require(NumberReading.prepare("QQ\u53f7123",NumberReading.AUTO,0x60000)
            .equals("QQ\u53f71 2 3"),"three-digit QQ identifier");
        require(NumberReading.prepare("QQ\u53f7123456",NumberReading.AUTO,0x60000)
            .equals("QQ\u53f71 2 3 4 5 6"),"six-digit QQ identifier");
        require(NumberReading.prepare("QQ群号13800138000",NumberReading.AUTO,0x60000)
            .equals("QQ群号1 3 8 0 0 1 3 8 0 0 0"),"QQ number is not mistaken for phone");
        require(NumberReading.prepare("QQ号712345678",NumberReading.VALUE,0x60000)
            .equals("QQ号7 1\uFF0C2 3 4 5\uFF0C6 7 8"),"QQ label overrides value mode");
        require(NumberReading.prepare("QQ群号13800138000",NumberReading.DIGITS,0x60000)
            .equals("QQ群号1 3 8 0 0 1 3 8 0 0 0"),"QQ label overrides phone grouping in digit mode");
        require(NumberReading.prepare("712345678",NumberReading.VALUE,0x60000)
            .equals("712345678"),"value mode preserves unlabelled integer");
        require(NumberReading.prepare("群号123456789012",NumberReading.AUTO,0x60000)
            .equals("群号1 2 3 4 5 6 7 8 9 0 1 2"),"long group number has no fixed grouping");
        require(NumberReading.prepare("QQ群号123456789012",NumberReading.DIGITS,0x60000)
            .equals("QQ群号1 2 3 4 5 6 7 8 9 0 1 2"),"digit mode keeps QQ numbers continuous");
        require(NumberReading.prepare("QQ号123，456",NumberReading.AUTO,0x60000)
            .equals("QQ号1 2 3，4 5 6"),"source comma remains a pause boundary");
        require(NumberReading.prepare("712345678，后续",NumberReading.AUTO,0x60000)
            .equals("7 1，2 3 4 5，6 7 8，后续"),"bare ID keeps source punctuation");
        require(NumberReading.prepare("a\u2022",NumberReading.AUTO,0x10001).equals("a"),
            "trailing decorative bullet is silent");
        require(NumberReading.prepare("a\u2022b",NumberReading.AUTO,0x10001).equals("a b"),
            "inline decorative bullet separates words without speaking");
        require(NumberReading.prepare("Operit, • 10 \u5206\u949F\u524D, Operit \u6b63\u5728\u8fd0\u884c",
            NumberReading.AUTO,0x10001).equals("Operit, 10 \u5206\u949F\u524D, Operit \u6b63\u5728\u8fd0\u884c"),
            "notification bullet is silent");
        require(NumberReading.prepare("200",NumberReading.AUTO,0x60000)
            .equals("\u4E24\u767E"),"two hundred");
        require(NumberReading.prepare("2000",NumberReading.AUTO,0x60000)
            .equals("\u4E24\u5343"),"two thousand");
        require(NumberReading.prepare("20000",NumberReading.AUTO,0x60000)
            .equals("\u4E24\u4E07"),"two ten thousand");
        require(NumberReading.prepare("2000000",NumberReading.AUTO,0x60000)
            .equals("\u4E24\u767E\u4E07"),"two million");
        require(NumberReading.prepare("200000000",NumberReading.AUTO,0x60000)
            .equals("\u4E24\u4EBF"),"two hundred million");
        require(NumberReading.prepare("20",NumberReading.AUTO,0x60000)
            .equals("\u4E8C\u5341"),"twenty");
        require(NumberReading.prepare("2\u9879",NumberReading.AUTO,0x60000)
            .equals("\u4E24\u9879"),"two items");
        require(NumberReading.prepare("2\u7FA4",NumberReading.AUTO,0x60000)
            .equals("\u4E8C\u7FA4"),"group number uses two");
        require(NumberReading.prepare("2\u53F7",NumberReading.AUTO,0x60000)
            .equals("\u4E8C\u53F7"),"number label uses two");
        require(NumberReading.prepare("20\u7FA4",NumberReading.AUTO,0x60000)
            .equals("\u4E8C\u5341\u7FA4"),"twenty group number");
        require(NumberReading.prepare("20\u9879",NumberReading.AUTO,0x60000)
            .equals("\u4E8C\u5341\u9879"),"twenty items");
        require(NumberReading.prepare("200\u9879",NumberReading.AUTO,0x60000)
            .equals("\u4E24\u767E\u9879"),"two hundred items");
        require(NumberReading.prepare("200\u4E07",NumberReading.AUTO,0x60000)
            .equals("\u4E24\u767E\u4E07"),"two million scale");
        require(NumberReading.prepare("\u7B2C2\u5C4F",NumberReading.AUTO,0x60000)
            .equals("\u7B2C\u4E8C\u5C4F"),"second screen ordinal");
        require(NumberReading.prepare("\u7B2C20\u5C4F",NumberReading.AUTO,0x60000)
            .equals("\u7B2C\u4E8C\u5341\u5C4F"),"twentieth screen ordinal");
        require(NumberReading.prepare("\u7B2C200\u9879",NumberReading.AUTO,0x60000)
            .equals("\u7B2C\u4E8C\u767E\u9879"),"two hundredth item ordinal");
        require(NumberReading.prepare("z-ai/glm-5.3-flash",NumberReading.AUTO,0x60000)
            .equals("z-ai \u659c\u6760 glm-5 \u70B9 3-flash"),"generic slash and dots");
        require(NumberReading.prepare("org.reloq.tts.apk.1",NumberReading.AUTO,0x60000)
            .equals("org \u70B9 reloq \u70B9 tts \u70B9 apk \u70B9 1"),"generic package dots");
        require(NumberReading.prepare("Hello world.",NumberReading.AUTO,0x60000)
            .equals("Hello world."),"sentence period unchanged");
        require(NumberReading.prepare("a=b",NumberReading.AUTO,0x60000)
            .equals("a \u7b49\u4e8e b"),"equals sign");
        require(NumberReading.prepare("A&B",NumberReading.AUTO,0x60000)
            .equals("A \u548c B"),"ampersand sign");
        require(NumberReading.prepare("=\u51711\u4e2a\u5b57\u7b26",NumberReading.AUTO,0x60000)
            .equals(" \u7b49\u4e8e \u51711\u4e2a\u5b57\u7b26"),"equals expansion keeps tail");
        require(NumberReading.prepare("2026\u5E74",NumberReading.AUTO,0x60000)
            .equals("2026\u5E74"),"year stays native");
        require(!NumberReading.endsWithPausePunctuation("\u4f60\u597d\u4e16\u754c"),
            "boundary text");
        require(NumberReading.endsWithPausePunctuation(
            "\u7cfb\u7edf, LSPosed \u5df2\u52a0\u8f7d, "), "comma before space");
        require(NumberReading.endsWithPausePunctuation("https. "), "url dot pause");
        require(NumberReading.prepare("22:02",NumberReading.AUTO,0x60000)
            .equals("\u4E8C\u5341\u4E8C\u70B9\u96F6\u4E8C\u5206"),"time 22:02");
        require(NumberReading.prepare("2026-11-17",NumberReading.AUTO,0x60000)
            .equals("2026\u5E7411\u670817\u65E5"),"date 2026-11-17");
        require(NumberReading.prepare("149.96MB",NumberReading.AUTO,0x60000)
            .equals("149\u70B9\u4E5D\u516DMB"),"file size decimal digits");
        require(NumberReading.prepare("2\u533A",NumberReading.AUTO,0x60000)
            .equals("\u4E8C\u533A"),"game zone uses two");
        require(NumberReading.prepare("2\u5C48",NumberReading.AUTO,0x60000)
            .equals("\u4E8C\u5C48"),"non-measure suffix uses two");
        require(NumberReading.prepare("00:08",NumberReading.AUTO,0x60000)
            .equals("\u96F6\u70B9\u96F6\u516B\u5206"),"zero hour is spoken");
        require(NumberReading.prepare("1dm+",NumberReading.AUTO,0x60000)
            .equals("1dm \u52A0 "),"plus in mixed text");
        require(NumberReading.prepare("1DM+",NumberReading.AUTO,0x60000)
            .equals("1DM \u52A0 "),"uppercase plus in mixed text");
        require(NumberReading.prepare("#v1.0.24",NumberReading.AUTO,0x60000)
            .equals(" \u4E95\u53F7 v1 \u70B9 0 \u70B9 \u4E8C\u5341\u56DB"),"hash in mixed text");
        require(NumberReading.prepare("test2-x",NumberReading.AUTO,0x60000).equals("test2-x"),"hyphen preserves text");
        require(NumberReading.prepare("a-b",NumberReading.AUTO,0x60000).equals("a-b"),"hyphen preserves text");
        require(NumberReading.prepare("\u8FF7\u96FE\u4E16\u754C2\u533A-\u4E07\u91CC\u957F\u9014",NumberReading.AUTO,0x60000)
            .equals("\u8FF7\u96FE\u4E16\u754C\u4E8C\u533A\uFF0C\u4E07\u91CC\u957F\u9014"),
            "Chinese hyphen is a punctuation boundary");
        require(NumberReading.prepare("\u8FF7\u96FE\u4E16\u754C2\u533A-\u4E07\u91CC\u957F\u9014",NumberReading.AUTO,0x60000)
            .equals("\u8FF7\u96FE\u4E16\u754C\u4E8C\u533A\uFF0C\u4E07\u91CC\u957F\u9014"),
            "Chinese hyphen is a punctuation boundary");
        require(NumberReading.prepare("@",NumberReading.AUTO,0x60000).equals(" at "),"at symbol expansion");
        require(NumberReading.prepare("\u4f60\u597d@\u4e16\u754c",NumberReading.AUTO,0x60000)
            .equals("\u4f60\u597d at \u4e16\u754c"),"at keeps neighboring Chinese");
        require(NumberReading.prepare("\uFF20",NumberReading.VALUE,0x60001).equals(" at "),"fullwidth at in value mode");
        require(NumberReading.prepare("@",NumberReading.DIGITS,0x60000).equals(" at "),"at in digits mode");
        require(NumberReading.prepare("abc@example.com",NumberReading.AUTO,0x60000)
            .equals("abc at example \u70B9 com"),"email at expands without joining words");
        require(NumberReading.prepare("@",NumberReading.AUTO,0x10000).equals("@"),"English symbol processing unchanged");
        require(NumberReading.prepare("G:\\ReLoq C",NumberReading.AUTO,0x60000)
            .equals("G: \u53cd\u659c\u6760 ReLoq C"),"Windows path backslash");
        require(NumberReading.prepare("\uFF3C",NumberReading.VALUE,0x60001)
            .equals(" \u53cd\u659c\u7dda "),"fullwidth Taiwanese backslash");
        require(NumberReading.prepare("G:\\path",NumberReading.VALUE,0x10000)
            .equals("G:\\path"),"English path backslash");
        System.out.println("NumberReadingTest: "+checks+" checks passed");
    }
}

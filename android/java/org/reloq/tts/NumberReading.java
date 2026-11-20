package org.reloq.tts;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/** Bounded local handling of identifier-style digit runs. */
final class NumberReading {
    static final int VALUE=0, DIGITS=1, AUTO=2;
    private static final String[] TRIGGERS={
        "phone number", "mobile number", "telephone number", "qq number",
        "group number", "tracking number", "tracking code", "order number",
        "account number", "card number", "verification code",
        "confirmation code",
        "\u624b\u673a\u53f7", "\u624b\u673a\u53f7\u7801",
        "\u7535\u8bdd\u53f7\u7801", "\u8054\u7cfb\u7535\u8bdd",
        "qq\u53f7", "qq\u7fa4\u53f7", "\u7fa4\u53f7",
        "\u5feb\u9012\u5355\u53f7", "\u8fd0\u5355\u53f7",
        "\u8ba2\u5355\u53f7", "\u5355\u53f7", "\u7f16\u53f7",
        "\u8d26\u53f7", "\u5361\u53f7", "\u9a8c\u8bc1\u7801"
    };
    private NumberReading() {}

    static String prepare(String text,int mode,int baseDialect) {
        if(text==null || text.isEmpty())return text;
        String prepared=prepareDateTimes(prepareUrls(text,baseDialect),baseDialect);
        prepared=prepareSymbols(prepared,baseDialect);
        prepared=prepareLandlines(prepared,baseDialect,mode==AUTO);
        StringBuilder out=new StringBuilder(prepared.length()+32);
        for(int at=0;at<prepared.length();) {
            int start=at;
            while(start<prepared.length() && !isDigit(prepared.charAt(start)))++start;
            if(start>at)out.append(prepared,at,start);
            if(start==prepared.length())break;
            int end=start;
            while(end<prepared.length() && isDigit(prepared.charAt(end)))++end;
            boolean qq=qqIdentifier(prepared,start,end);
            if(mode==VALUE&&!qq) {
                out.append(prepared,start,end);
                at=end;continue;
            }
            if(mode==AUTO) {
                String digits=prepared.substring(start,end);
                boolean labeled=identifier(prepared,start,end);
                boolean bare=isMandarin(baseDialect)&&bareIdentifier(prepared,start,end);
                if(labeled||bare) {
                    appendIdentifier(out,digits,baseDialect,phone(digits)&&!qq,
                        bare||qq);
                    at=end;continue;
                }
                if((baseDialect==0x60000||baseDialect==0x60001) &&
                   shouldConvertChineseValue(prepared,start,end)) {
                    char next=end<prepared.length()?prepared.charAt(end):'\0';
                    out.append(chineseValue(digits,next,baseDialect,ordinal(prepared,start)));
                    at=end;continue;
                }
                out.append(prepared,start,end);
                at=end;continue;
            }
            String digits=prepared.substring(start,end);
            appendIdentifier(out,digits,baseDialect,phone(digits)&&!qq,qq);
            at=end;
        }
        return out.toString();
    }

    private static String prepareLandlines(String text,int baseDialect,boolean allowBareLocal) {
        if(!isMandarin(baseDialect))return text;
        final String labelSource=text;
        text=mapPattern(text,Pattern.compile("(?i)(\u5EA7\u673A\u53F7|\u5EA7\u673A|\u56FA\u5B9A\u7535\u8BDD\u53F7|\u56FA\u5B9A\u7535\u8BDD|\u56FA\u8BDD\u53F7|\u56FA\u8BDD|landline|land line)([\\s:：,，=]*)([0-9]{7,8})(?![0-9])"),
            m->qqIdentifier(labelSource,m.start(3),m.end(3))?m.group():
                m.group(1)+m.group(2)+localGroups(m.group(3)));
        final String serviceNumberSource=text;
        text=mapPattern(text,Pattern.compile("(?<![0-9])(400|800)([0-9]{3})([0-9]{4})(?![0-9])"),
            m->qqIdentifier(serviceNumberSource,m.start(1),m.end(3))?m.group():
                spokenDigits(m.group(1))+(char)0xFF0C+spokenDigits(m.group(2))+(char)0xFF0C+spokenDigits(m.group(3)));
        final String groupedSource=text;
        text=mapPattern(text,Pattern.compile("(?<![0-9])(0[0-9]{2,3})([,，])([0-9]{4})([,，])([0-9]{3,4})(?![0-9])"),
            m->qqIdentifier(groupedSource,m.start(1),m.end(5))?m.group():
                spokenDigits(m.group(1))+m.group(2)+spokenDigits(m.group(3))+m.group(4)+spokenDigits(m.group(5)));
        final String areaGroupSource=text;
        text=mapPattern(text,Pattern.compile("(?<![0-9])(0[0-9]{2,3})([,，])([0-9]{7,8})(?![0-9])"),
            m->qqIdentifier(areaGroupSource,m.start(1),m.end(3))?m.group():
                spokenDigits(m.group(1))+m.group(2)+localGroups(m.group(3)));
        final String compactSource=text;
        text=mapPattern(text,Pattern.compile("(?<![0-9])(0[0-9]{9,11})(?![0-9])"),m->{
            String number=m.group(1);
            if(qqIdentifier(compactSource,m.start(1),m.end(1)))return number;
            if(phone(number))return number;
            int area=(number.charAt(1)=='1'||number.charAt(1)=='2')?3:4;
            String local=number.substring(area);
            if(local.length()<7||local.length()>8)return number;
            return spokenDigits(number.substring(0,area))+"，"+localGroups(local);
        });
        final String localGroupSource=text;
        text=mapPattern(text,Pattern.compile("(?<![0-9])([0-9]{4})([,，])([0-9]{3,4})(?![0-9])"),
            m->qqIdentifier(localGroupSource,m.start(1),m.end(3))?m.group():
                spokenDigits(m.group(1))+m.group(2)+spokenDigits(m.group(3)));
        final String bareLocalSource=text;
        text=mapPattern(text,Pattern.compile("(?<![0-9])([0-9]{8})(?![0-9])"),m->{
            String digits=m.group(1);
            if(!allowBareLocal||!bareIdentifier(bareLocalSource,m.start(1),m.end(1))||
               digits.endsWith("0000")||compactDate(digits)||qqIdentifier(bareLocalSource,m.start(1),m.end(1)))
                return m.group();
            return localGroups(digits);
        });
        return text;
    }

    private static String localGroups(String local) {
        int first=local.length()==7?3:4;
        return spokenDigits(local.substring(0,first))+"，"+spokenDigits(local.substring(first));
    }

    private interface Replacement {String apply(Matcher matcher);}

    private static String mapPattern(String text,Pattern pattern,Replacement replacement) {
        Matcher matcher=pattern.matcher(text);
        StringBuffer out=new StringBuffer();
        while(matcher.find())matcher.appendReplacement(out,Matcher.quoteReplacement(replacement.apply(matcher)));
        matcher.appendTail(out);
        return out.toString();
    }

    private static String spokenDigits(String digits) {
        StringBuilder out=new StringBuilder(digits.length());
        for(int i=0;i<digits.length();++i) {
            char digit=digits.charAt(i);
            if(digit=='1')out.append("幺");
            else out.append(digitName(digit-'0',0x60000));
        }
        return out.toString();
    }

    static String prepareUrls(String text,int baseDialect) {
        StringBuilder out=new StringBuilder(text.length());
        for(int at=0;at<text.length();) {
            int start=urlStart(text,at);
            if(start<0) {
                out.append(text,at,text.length());
                break;
            }
            out.append(text,at,start);
            int end=urlEnd(text,start);
            out.append(urlReadable(text.substring(start,end),baseDialect));
            at=end;
        }
        return out.toString();
    }

    private static String prepareSymbols(String text,int baseDialect) {
        text=removeDecorativeBullets(text);
        if(baseDialect!=0x60000 && baseDialect!=0x60001)return text;
        String slash=baseDialect==0x60001 ? " \u659c\u7dda " : " \u659c\u6760 ";
        String backslash=baseDialect==0x60001 ? " \u53cd\u659c\u7dda " : " \u53cd\u659c\u6760 ";
        String dot=baseDialect==0x60001 ? " \u9ede " : " \u70b9 ";
        String equals=baseDialect==0x60001 ? " \u7b49\u65bc " : " \u7b49\u4e8e ";
        String ampersand=" \u548c ";
        String underscore=baseDialect==0x60001 ? " \u5e95\u7dda " : " \u4e0b\u5283\u7dda ";
        String plus=" \u52a0 ";
        String hash=baseDialect==0x60001 ? " \u4e95\u865f " : " \u4e95\u53f7 ";
        StringBuilder out=new StringBuilder(text.length());
        for(int at=0;at<text.length();++at) {
            char c=text.charAt(at);
            if(c=='/' || c=='\uFF0F')out.append(slash);
            else if(c=='\\' || c=='\uFF3C')out.append(backslash);
            else if(c=='@' || c=='\uFF20')out.append(" at ");
            else if(c=='=' || c=='\uFF1D')out.append(equals);
            else if(c=='&')out.append(ampersand);
            else if(c=='+' || c=='\uFF0B')out.append(plus);
            else if(c=='#' || c=='\uFF03')out.append(hash);
            else if(c=='_' || c=='\uFF3F')out.append(underscore);
            else if((c=='-' || c=='\uFF0D') && at>0 && at+1<text.length() &&
                    chineseCharacter(text.charAt(at-1)) && chineseCharacter(text.charAt(at+1)))
                out.append('\uFF0C');
            else if((c=='.' || c=='\uFF0E') && at>0 && at+1<text.length() &&
                    Character.isLetterOrDigit(text.charAt(at-1)) &&
                    Character.isLetterOrDigit(text.charAt(at+1)))out.append(dot);
            else out.append(c);
        }
        return out.toString();
    }

    private static String removeDecorativeBullets(String text) {
        StringBuilder out=new StringBuilder(text.length());
        for(int at=0;at<text.length();++at) {
            char c=text.charAt(at);
            if(c!='\u2022') {out.append(c);continue;}
            boolean left=out.length()>0&&!Character.isWhitespace(out.charAt(out.length()-1));
            boolean right=at+1<text.length()&&!Character.isWhitespace(text.charAt(at+1));
            if(left&&right)out.append(' ');
            if(!left)while(at+1<text.length()&&Character.isWhitespace(text.charAt(at+1)))++at;
        }
        return out.toString();
    }

    private static boolean chineseCharacter(char c) {
        return (c>='\u3400'&&c<='\u4DBF') || (c>='\u4E00'&&c<='\u9FFF') ||
            (c>='\uF900'&&c<='\uFAFF');
    }

    private static int urlStart(String text,int from) {
        int best=-1;
        for(String prefix:new String[]{"https://","http://","www."}) {
            int at=text.indexOf(prefix,from);
            if(at>=0 && (best<0 || at<best))best=at;
        }
        return best;
    }

    private static int urlEnd(String text,int start) {
        int at=start;
        while(at<text.length()) {
            char c=text.charAt(at);
            if(Character.isWhitespace(c) || "<>\"')]}\uFF0C\u3002\uFF1B\uFF1A\uFF01\uFF1F".indexOf(c)>=0)break;
            ++at;
        }
        return at;
    }

    private static String urlReadable(String url,int baseDialect) {
        return prepareSymbols(url.replace("/"," \u659c\u6760 ").replaceAll(" +"," "),baseDialect);
    }

    static boolean identifier(String text,int start,int end) {
        String digits=text.substring(start,end);
        if(phone(digits))return true;
        return identifierLabelBefore(text,start)||identifierLabelAfter(text,end);
    }

    private static boolean qqIdentifier(String text,int start,int end) {
        return identifierLabelBefore(text,start,QQ_TRIGGERS)||
            identifierLabelAfter(text,end,QQ_TRIGGERS);
    }

    private static boolean bareIdentifier(String text,int start,int end) {
        String digits=text.substring(start,end);
        // shortcut: bare 7-12 digit values are ambiguous; labels and value context take priority.
        if(phone(digits))return false;
        if(digits.length()<7||digits.length()>12||digits.matches("[0-9]*0{2,}"))return false;
        int beforeAt=start;
        while(beforeAt>0&&Character.isWhitespace(text.charAt(beforeAt-1)))--beforeAt;
        int nextAt=end;
        while(nextAt<text.length()&&Character.isWhitespace(text.charAt(nextAt)))++nextAt;
        char before=beforeAt>0?text.charAt(beforeAt-1):'\0';
        char next=nextAt<text.length()?text.charAt(nextAt):'\0';
        if(before=='.'||before=='\uFF0E'||before=='-'||before=='/'||before==':'||
           next=='.'||next=='\uFF0E'||next=='-'||next=='/'||next==':'||
           Character.isLetter(before)||Character.isLetter(next)||before=='_'||next=='_')return false;
        if("\u5143\u4E2A\u4EBA\u53EA\u6761\u9879\u6B21\u5C81\u5E74\u6708\u65E5\u65F6\u5206\u79D2\u7C73\u5146\u4E07\u4EBF".indexOf(next)>=0)return false;
        if(compactDate(digits))return false;
        String beforeText=text.substring(Math.max(0,beforeAt-3),beforeAt);
        return !beforeText.endsWith("\u6709")&&!beforeText.endsWith("\u5171")&&
            !beforeText.endsWith("\u7B2C")&&!beforeText.endsWith("\u7EA6");
    }

    private static boolean compactDate(String digits) {
        if(digits.length()!=8)return false;
        int year=Integer.parseInt(digits.substring(0,4));
        int month=Integer.parseInt(digits.substring(4,6));
        int day=Integer.parseInt(digits.substring(6,8));
        return year>=1900&&year<=2099&&month>=1&&month<=12&&day>=1&&day<=31;
    }

    private static boolean identifierLabelBefore(String text,int start) {
        return identifierLabelBefore(text,start,TRIGGERS);
    }

    private static boolean identifierLabelBefore(String text,int start,String[] triggers) {
        String before=text.substring(Math.max(0,start-32),start).toLowerCase(java.util.Locale.ROOT);
        for(String trigger:triggers) {
            int at=before.lastIndexOf(trigger);
            while(at>=0) {
                if(identifierBridge(before.substring(at+trigger.length())))return true;
                at=before.lastIndexOf(trigger,at-1);
            }
        }
        return false;
    }

    private static boolean identifierLabelAfter(String text,int end) {
        return identifierLabelAfter(text,end,TRIGGERS);
    }

    private static boolean identifierLabelAfter(String text,int end,String[] triggers) {
        String after=text.substring(end,Math.min(text.length(),end+32)).toLowerCase(java.util.Locale.ROOT);
        for(String trigger:triggers) {
            int at=after.indexOf(trigger);
            while(at>=0) {
                if(identifierBridge(after.substring(0,at)))return true;
                at=after.indexOf(trigger,at+1);
            }
        }
        return false;
    }

    private static boolean identifierBridge(String text) {
        if(text.length()>18)return false;
        String bridge=text.replaceAll("[\\s,，:：=]+","");
        return bridge.isEmpty()||bridge.equals("是")||bridge.equals("这是")||
            bridge.equals("这些")||bridge.equals("就是")||bridge.equals("为")||
            bridge.equals("号码为")||bridge.equals("叫做")||
            bridge.matches("(?i)[a-z0-9_-]{1,8}");
    }

    static boolean isPhone(String digits) {return phone(digits);}

    static String groupDigits(String digits,int baseDialect) {
        return groupDigits(digits,baseDialect,false);
    }

    static String groupDigits(String digits,int baseDialect,boolean phone) {
        return groupDigits(digits,baseDialect,phone,false);
    }

    private static String groupDigits(String digits,int baseDialect,boolean phone,boolean continuous) {
        int[] groups;
        if(continuous&&digits.length()==9)groups=new int[]{2,4,3};
        else if(continuous&&digits.length()==10)groups=new int[]{2,4,4};
        else if(continuous)groups=new int[]{digits.length()};
        else if(phone)groups=new int[]{3,4,4};
        else if(digits.length()<=5)groups=new int[]{digits.length()};
        else if(digits.length()<=8) {
            int first=(digits.length()+1)/2;
            groups=new int[]{first,digits.length()-first};
        }
        else if(digits.length()==9)groups=new int[]{3,3,3};
        else {
            groups=new int[(digits.length()+3)/4];
            for(int i=0;i<groups.length;++i)groups[i]=Math.min(4,digits.length()-4*i);
        }
        StringBuilder out=new StringBuilder(digits.length()+groups.length);
        int at=0;
        for(int i=0;i<groups.length;++i) {
            if(i>0)out.append(groupSeparator(baseDialect));
            for(int j=0;j<groups[i];++j) {
                if(j>0)out.append(' ');
                char digit=digits.charAt(at++);
                out.append(phone&&digit=='1'?'\u5E7A':digit);
            }
        }
        return out.toString();
    }

    static boolean endsWithPausePunctuation(String text) {
        return pauseMilliseconds(text)>0;
    }

    static int pauseMilliseconds(String text) {
        if(text==null)return 0;
        int at=text.length();
        while(at>0 && Character.isWhitespace(text.charAt(at-1)))--at;
        if(at==0)return 0;
        switch(text.charAt(at-1)) {
        case ',': case '\uFF0C': case '\u3001': case ';': case '\uFF1B':
        case ':': case '\uFF1A':
            return 80;
        case '.': case '!': case '?': case '\u3002': case '\uFF01':
        case '\uFF1F':
            return 140;
        default:return 0;
        }
    }

    static boolean shouldConvertChineseValue(String text,int start,int end) {
        int length=end-start;
        if(length<1 || length>16 || text.charAt(start)!='2')return false;
        if(compactDate(text.substring(start,end)))return false;
        char before=start>0?text.charAt(start-1):'\0';
        if(before=='.' || before=='\uFF0E' || before=='-' || before=='/')return false;
        char next=end<text.length()?text.charAt(end):'\0';
        if(next=='.' || next=='\uFF0E' || next=='-' || next=='/' || next==':'
           || next=='\uFF1A')return false;
        return !(length==4 && next=='\u5E74');
    }

    static String chineseValue(String digits,char next,int baseDialect,boolean ordinal) {
        boolean measure=measureNext(next,digits.length());
        String value=cardinal(digits,baseDialect);
        if(shouldUseLiang(digits.length(),measure) && !ordinal) {
            char liang=baseDialect==0x60001?'\u5169':'\u4E24';
            return liang+value.substring(1);
        }
        return value;
    }

    private static boolean ordinal(String text,int start) {
        int at=start;
        while(at>0 && Character.isWhitespace(text.charAt(at-1)))--at;
        return at>0 && text.charAt(at-1)=='\u7B2C';
    }

    private static String prepareDateTimes(String text,int baseDialect) {
        if(baseDialect!=0x60000 && baseDialect!=0x60001)return text;
        text=prepareTimes(text,baseDialect);
        text=prepareDates(text,baseDialect);
        return prepareFileSizes(text,baseDialect);
    }

    private static String prepareTimes(String text,int baseDialect) {
        Pattern time=Pattern.compile("(?<![0-9:])([01]?[0-9]|2[0-3]):([0-5][0-9])(?::([0-5][0-9]))?(?![0-9:])");
        Matcher m=time.matcher(text);
        StringBuffer out=new StringBuffer();
        while(m.find()) {
            int hour=Integer.parseInt(m.group(1));
            int minute=Integer.parseInt(m.group(2));
            String seconds=m.group(3);
            StringBuilder replacement=new StringBuilder();
            replacement.append(hourChinese(hour,baseDialect)).append('\u70B9');
            if(minute==0)replacement.append('\u6574');
            else replacement.append(minuteChinese(minute,baseDialect)).append('\u5206');
            if(seconds!=null)replacement.append(chineseValue(seconds,'\0',baseDialect,false)).append('\u79D2');
            m.appendReplacement(out,replacement.toString());
        }
        m.appendTail(out);
        return out.toString();
    }

    private static String prepareDates(String text,int baseDialect) {
        Pattern date=Pattern.compile("(?<![0-9])(\\d{4})[-/.](\\d{1,2})[-/.](\\d{1,2})(?![0-9])");
        Matcher m=date.matcher(text);
        StringBuffer out=new StringBuffer();
        while(m.find()) {
            String replacement=m.group(1)+"\u5E74"+m.group(2)+"\u6708"+m.group(3)+"\u65E5";
            m.appendReplacement(out,replacement);
        }
        m.appendTail(out);
        return out.toString();
    }

    private static String prepareFileSizes(String text,int baseDialect) {
        Pattern size=Pattern.compile("(?i)(\\d+)\\.(\\d+)(\\s?(?:MB|GB|KB|B))");
        Matcher m=size.matcher(text);
        StringBuffer out=new StringBuffer();
        while(m.find()) {
            StringBuilder replacement=new StringBuilder();
            replacement.append(m.group(1)).append('\u70B9');
            for(int i=0;i<m.group(2).length();++i)
                replacement.append(digitName(m.group(2).charAt(i)-'0',baseDialect));
            replacement.append(m.group(3));
            m.appendReplacement(out,replacement.toString());
        }
        m.appendTail(out);
        return out.toString();
    }

    private static String hourChinese(int hour,int baseDialect) {
        if(hour==0)return zeroName(baseDialect)+"";
        if(hour==2)return twoName(baseDialect)+"";
        return chineseValue(Integer.toString(hour),'\0',baseDialect,false);
    }

    private static String minuteChinese(int minute,int baseDialect) {
        if(minute<10)return zeroName(baseDialect)+""+digitName(minute,baseDialect);
        return chineseValue(Integer.toString(minute),'\0',baseDialect,false);
    }

    private static boolean measureNext(char next,int length) {
        if(next=='\u5E74' && length==4)return false;
        if(isLabelSuffix(next))return false;
        return next=='\u4E07' || next=='\u4EBF' || next=='\u842C' || next=='\u5104'
            || isMeasureSuffix(next);
    }

    private static boolean isMeasureSuffix(char next) {
        return "\u4E2A\u53EA\u6761\u672C\u9879\u4EFD\u7BC7\u5F20\u5757\u8F86\u53F0\u5957\u6279\u6B21\u904D\u573A\u987F\u4EF6\u5305\u74F6\u676F\u76D2\u7B52\u8282\u8BFE\u9875\u6B65".indexOf(next)>=0;
    }

    private static boolean isLabelSuffix(char next) {
        return next=='\u7FA4' || next=='\u53F7' || next=='\u73ED'
           || next=='\u7EC4' || next=='\u961F' || next=='\u533A';
    }

    private static boolean shouldUseLiang(int length,boolean measure) {
        int exponent=length-1;
        if(exponent==0)return measure;
        return exponent%4!=1;
    }

    private static String cardinal(String digits,int baseDialect) {
        int groups=(digits.length()+3)/4;
        String[] chunks=new String[groups];
        int at=digits.length();
        for(int i=0;i<groups;++i) {
            int take=Math.min(4,at);
            chunks[i]=digits.substring(at-take,at);
            at-=take;
        }
        StringBuilder out=new StringBuilder();
        boolean pendingZero=false;
        for(int i=groups-1;i>=0;--i) {
            String chunk=chunks[i];
            int value=Integer.parseInt(chunk);
            if(value==0) {
                pendingZero=true;
                continue;
            }
            if(out.length()>0 && (value<1000 || pendingZero))
                out.append(zeroName(baseDialect));
            out.append(groupCardinal(value,baseDialect));
            if(i>0)out.append(scaleName(i,baseDialect));
            pendingZero=false;
        }
        return out.toString();
    }

    private static String groupCardinal(int value,int baseDialect) {
        int[] digits={value/1000,(value/100)%10,(value/10)%10,value%10};
        StringBuilder out=new StringBuilder();
        boolean pendingZero=false;
        for(int i=0;i<4;++i) {
            int digit=digits[i];
            if(digit==0) {
                if(i<3)pendingZero=true;
                continue;
            }
            if(pendingZero && out.length()>0)out.append(zeroName(baseDialect));
            if(i==2 && digit==1)out.append(tenName(baseDialect));
            else {
                out.append(digitName(digit,baseDialect));
                if(i<3)out.append(unitName(i,baseDialect));
            }
            pendingZero=false;
        }
        return out.toString();
    }

    private static char zeroName(int baseDialect) {return '\u96F6';}
    private static char digitName(int digit,int baseDialect) {
        return "\u96F6\u4E00\u4E8C\u4E09\u56DB\u4E94\u516D\u4E03\u516B\u4E5D".charAt(digit);
    }
    private static char twoName(int baseDialect)
        {return baseDialect==0x60001?'\u5169':'\u4E24';}
    private static char tenName(int baseDialect) {return '\u5341';}
    private static char hundredName(int baseDialect) {return '\u767E';}
    private static char thousandName(int baseDialect) {return '\u5343';}
    private static String scaleName(int index,int baseDialect) {
        switch(index) {
        case 1:return baseDialect==0x60001?"\u842C":"\u4E07";
        case 2:return baseDialect==0x60001?"\u5104":"\u4EBF";
        case 3:return baseDialect==0x60001?"\u842C\u5104":"\u4E07\u4EBF";
        default:return "";
        }
    }
    private static String unitName(int index,int baseDialect) {
        switch(index) {
        case 0:return thousandName(baseDialect)+"";
        case 1:return hundredName(baseDialect)+"";
        case 2:return tenName(baseDialect)+"";
        default:return "";
        }
    }

    private static void appendIdentifier(StringBuilder out,String digits,int baseDialect,boolean phone)
        {appendIdentifier(out,digits,baseDialect,phone,false);}

    private static void appendIdentifier(StringBuilder out,String digits,int baseDialect,
                                         boolean phone,boolean continuous)
        {out.append(groupDigits(digits,baseDialect,phone,continuous));}

    private static boolean isMandarin(int baseDialect)
        {return baseDialect==0x60000||baseDialect==0x60001;}

    private static boolean phone(String digits) {
        return digits.length()==11 && digits.charAt(0)=='1' &&
            digits.charAt(1)>='3' && digits.charAt(1)<='9';
    }

    private static char groupSeparator(int baseDialect)
        {return baseDialect==0x60000 || baseDialect==0x60001 ? '\uFF0C' : ' ';}

    private static final String[] QQ_TRIGGERS={
        "qq number", "group number", "qq\u53f7", "qq\u7fa4\u53f7", "\u7fa4\u53f7"
    };

    private static boolean isDigit(char c) {return c>='0' && c<='9';}
}

package org.reloq.tts;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.CodingErrorAction;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.regex.*;

/** Imported NVDA-format dictionaries, applied before native pronunciation. */
final class SpeechDictionary {
    static final int LIMIT=8*1024*1024;
    private static final Pattern FILENAME=Pattern.compile("nvda-([0-9]{3})-([0-9a-f]{8})-([0-8])\\.nvda");
    private static final int UNICODE_FLAGS=unicodeFlags();
    private final ArrayList<Entry> entries=new ArrayList<>();
    private static int unicodeFlags() {
        // Android always uses Unicode character classes but older Android
        // Pattern implementations reject the corresponding Java flag.
        try{Pattern.compile("",Pattern.UNICODE_CHARACTER_CLASS);return Pattern.UNICODE_CASE|Pattern.UNICODE_CHARACTER_CLASS;}
        catch(IllegalArgumentException e){return Pattern.UNICODE_CASE;}
    }
    private static final class Entry {
        final Pattern pattern;final String replacement;
        Entry(Pattern p,String r){pattern=p;replacement=r;}
    }
    static final class Result {
        final String text;final boolean matched;
        Result(String text,boolean matched){this.text=text;this.matched=matched;}
    }
    private static String text(byte[] data) throws IOException {
        if(data.length>LIMIT)throw new IOException("Dictionary exceeds 8 MiB");
        for(byte b:data)if(b==0)throw new IOException("Dictionary contains a NUL character");
        try {
            String result=StandardCharsets.UTF_8.newDecoder().onMalformedInput(CodingErrorAction.REPORT)
                .onUnmappableCharacter(CodingErrorAction.REPORT).decode(ByteBuffer.wrap(data)).toString();
            return result.startsWith("\ufeff")?result.substring(1):result;
        } catch(CharacterCodingException e){throw new IOException("Invalid UTF-8 speech dictionary",e);}
    }
    /** Detect complete four-column files, rejecting mixtures and bad options. */
    static boolean looksNvda(byte[] data) throws IOException {
        final String value;
        try{value=text(data);}catch(IOException e){
            if(data.length>LIMIT)throw e;
            for(byte b:data)if(b==0)throw e;
            return false; // Native Eloquence files may be Windows-1252.
        }
        boolean candidate=false,other=false;
        for(String line:value.split("\\R",-1)) {
            if(line.trim().isEmpty()||line.startsWith("#"))continue;
            String[] fields=line.split("\\t",-1);
            boolean row=fields.length==4&&(fields[2].equals("0")||fields[2].equals("1"))&&fields[3].matches("[0-9]+");
            if(row)candidate=true;else other=true;
        }
        if(candidate&&other)throw new IOException("Mixed NVDA and Eloquence dictionary entries");
        if(candidate)parse(data);
        return candidate;
    }
    static SpeechDictionary parse(byte[] data) throws IOException {
        String value=text(data);SpeechDictionary dictionary=new SpeechDictionary();
        int lineNumber=0;
        for(String line:value.split("\\R",-1)) {
            ++lineNumber;
            if(line.trim().isEmpty()||line.startsWith("#"))continue;
            String[] fields=line.split("\\t",-1);
            try {
                if(fields.length!=4||!(fields[2].equals("0")||fields[2].equals("1")))throw new IllegalArgumentException();
                int kind=Integer.parseInt(fields[3]);if(kind<0||kind>6)throw new IllegalArgumentException();
                String pattern=fields[0].replace("\\#","#"),replacement=fields[1].replace("\\#","#");
                HashMap<String,String> names=new HashMap<>();
                String expression;
                if(kind==1)expression=pythonPattern(pattern,names);
                else if(kind==6)expression=wildcard(pattern);
                else {
                    String quoted=Pattern.quote(pattern);
                    expression=kind==2?"\\b"+quoted+"\\b":
                        kind==3?"(?<=\\w)"+quoted+"|"+quoted+"(?=\\w)":
                        kind==4?"\\b"+quoted+"(?=\\w)":
                        kind==5?"(?<=\\w)"+quoted+"\\b":quoted;
                }
                int flags=UNICODE_FLAGS;
                if(fields[2].equals("0"))flags|=Pattern.CASE_INSENSITIVE;
                Pattern compiled=Pattern.compile(expression,flags);
                String output=kind==1?pythonReplacement(replacement,names,compiled.matcher("").groupCount()):Matcher.quoteReplacement(replacement);
                dictionary.entries.add(new Entry(compiled,output));
            } catch(IllegalArgumentException e){throw new IOException("Invalid speech dictionary entry on line "+lineNumber,e);}
        }
        return dictionary;
    }
    private static String pythonPattern(String pattern,Map<String,String> names) {
        StringBuilder out=new StringBuilder();
        boolean inClass=false;
        for(int i=0;i<pattern.length();) {
            char c=pattern.charAt(i);
            if(c=='\\'&&i+1<pattern.length()){
                char next=pattern.charAt(i+1);
                if(next=='U') {
                    if(i+10>pattern.length())throw new IllegalArgumentException("Invalid Unicode escape");
                    String hex=pattern.substring(i+2,i+10);if(!hex.matches("[0-9a-fA-F]{8}"))throw new IllegalArgumentException("Invalid Unicode escape");
                    out.append("\\x{").append(hex).append('}');i+=10;continue;
                }
                out.append('\\').append(!inClass&&next=='Z'?'z':next);i+=2;continue;
            }
            if(c=='[')inClass=true;else if(c==']')inClass=false;
            if(!inClass&&pattern.startsWith("(?P<",i)) {
                int end=pattern.indexOf('>',i+4);if(end<0)throw new IllegalArgumentException("Invalid group name");
                String name=pattern.substring(i+4,end);
                if(!name.matches("[A-Za-z_][A-Za-z0-9_]*")||names.containsKey(name))throw new IllegalArgumentException("Invalid group name");
                String mapped="r"+(names.size()+1);names.put(name,mapped);out.append("(?<").append(mapped).append('>');i=end+1;continue;
            }
            if(!inClass&&pattern.startsWith("(?P=",i)) {
                int end=pattern.indexOf(')',i+4);if(end<0)throw new IllegalArgumentException("Invalid group reference");
                String mapped=names.get(pattern.substring(i+4,end));if(mapped==null)throw new IllegalArgumentException("Unknown group");
                out.append("\\k<").append(mapped).append('>');i=end+1;continue;
            }
            out.append(c);++i;
        }
        return out.toString();
    }
    private static String wildcard(String value) {
        StringBuilder out=new StringBuilder("(?s:");
        for(int i=0;i<value.length();++i) {
            char c=value.charAt(i);
            if(c=='*'){while(i+1<value.length()&&value.charAt(i+1)=='*')++i;out.append(".*");}
            else if(c=='?')out.append('.');
            else if(c=='[') {
                int end=i+1;if(end<value.length()&&value.charAt(end)=='!')++end;
                if(end<value.length()&&value.charAt(end)==']')++end;
                while(end<value.length()&&value.charAt(end)!=']')++end;
                if(end==value.length())out.append("\\[");
                else {
                    String inside=value.substring(i+1,end);out.append('[');
                    if(inside.startsWith("!")){out.append('^');inside=inside.substring(1);}
                    else if(inside.startsWith("^"))out.append('\\');
                    out.append(inside.replace("\\","\\\\").replace("&&","\\&\\&"));out.append(']');i=end;
                }
            } else if(Character.isHighSurrogate(c)&&i+1<value.length()&&Character.isLowSurrogate(value.charAt(i+1))) {
                out.append(Pattern.quote(value.substring(i,i+2)));++i;
            } else out.append(Pattern.quote(String.valueOf(c)));
        }
        return out.append(')').toString();
    }
    private static String pythonReplacement(String value,Map<String,String> names,int groups) {
        StringBuilder out=new StringBuilder(),literal=new StringBuilder();
        for(int i=0;i<value.length();++i) {
            char c=value.charAt(i);if(c!='\\'){literal.append(c);continue;}
            if(++i>=value.length())throw new IllegalArgumentException("Trailing replacement escape");
            c=value.charAt(i);String reference=null;
            if(c=='g'&&i+1<value.length()&&value.charAt(i+1)=='<') {
                int end=value.indexOf('>',i+2);if(end<0)throw new IllegalArgumentException("Invalid group reference");
                reference=value.substring(i+2,end);i=end;
            } else if(c>='1'&&c<='9') {
                reference=String.valueOf(c);
                if(i+1<value.length()&&Character.isDigit(value.charAt(i+1)))reference+=value.charAt(++i);
            } else {
                switch(c){case '0':throw new IllegalArgumentException("Embedded NUL is not supported");
                    case 'n':literal.append('\n');break;case 'r':literal.append('\r');break;
                    case 't':literal.append('\t');break;case 'f':literal.append('\f');break;
                    case 'v':literal.append('\u000b');break;case 'a':literal.append('\u0007');break;
                    case 'b':literal.append('\b');break;case '\\':literal.append('\\');break;
                    default:if(Character.isLetter(c))throw new IllegalArgumentException("Unknown replacement escape");
                        literal.append('\\').append(c);}
            }
            if(reference!=null) {
                out.append(Matcher.quoteReplacement(literal.toString()));literal.setLength(0);
                if(reference.matches("[0-9]+")) {
                    int group=Integer.parseInt(reference);if(group>groups)throw new IllegalArgumentException("Unknown group");
                    out.append('$').append(group);
                } else {
                    String name=names.get(reference);if(name==null)throw new IllegalArgumentException("Unknown group");
                    out.append("${").append(name).append('}');
                }
            }
        }
        return out.append(Matcher.quoteReplacement(literal.toString())).toString();
    }
    Result apply(String input) {
        String result=input;boolean matched=false;
        for(Entry entry:entries) {
            Matcher m=entry.pattern.matcher(result);
            if(m.find()){matched=true;result=m.replaceAll(entry.replacement);}
        }
        return new Result(result,matched);
    }
    static SpeechDictionary load(File folder,int language,int voice) throws IOException {
        SpeechDictionary combined=new SpeechDictionary();if(folder==null)return combined;
        File[] files=folder.listFiles((dir,name) -> FILENAME.matcher(name).matches());
        if(files==null)throw new IOException("Dictionary snapshot unavailable");
        Arrays.sort(files,Comparator.comparing(File::getName));
        File parent=folder.getCanonicalFile();
        for(File file:files) {
            Matcher match=FILENAME.matcher(file.getName());if(!match.matches())continue;
            int lang=(int)Long.parseLong(match.group(2),16),profile=Integer.parseInt(match.group(3));
            if(lang!=language||(profile!=0&&profile!=voice))continue;
            if(!parent.equals(file.getCanonicalFile().getParentFile()))throw new IOException("Invalid speech dictionary file");
            byte[] data;try(InputStream in=new FileInputStream(file)){data=DictionaryData.read(in,LIMIT);}
            combined.entries.addAll(parse(data).entries);
        }
        return combined;
    }
    static String snapshotName(int index,int language,int voice) {
        if(index<0||index>999||voice<0||voice>8)throw new IllegalArgumentException("Invalid speech dictionary scope");
        return String.format(Locale.ROOT,"nvda-%03d-%08x-%d.nvda",index,language,voice);
    }
    static boolean isSnapshotName(String name){return FILENAME.matcher(name).matches();}
}

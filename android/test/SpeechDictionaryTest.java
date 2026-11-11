package org.reloq.tts;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public final class SpeechDictionaryTest {
    private static byte[] bytes(String s){return s.getBytes(StandardCharsets.UTF_8);}
    private static void same(SpeechDictionary dict,String input,String expected,boolean matched){
        SpeechDictionary.Result result=dict.apply(input);
        if(!result.text.equals(expected)||result.matched!=matched)
            throw new AssertionError(input+" -> "+result.text+" (expected "+expected+")");
    }
    private static void invalid(String value) throws Exception {
        try {SpeechDictionary.parse(bytes(value));throw new AssertionError("invalid file accepted");}
        catch(IOException expected){}
    }
    public static void main(String[] args) throws Exception {
        String content="\ufeff# comment\r\n"
            +"youtube\tbanana\t0\t0\r\n"
            +"literalprobe\t\\\\1\t1\t0\r\n"
            +"(?P<some_name>regprobe)([0-9]+)\t\\g<some_name>-\\2\t1\t1\r\n"
            +"wholeprobe\tWeg\t0\t2\r\n"
            +"teilprobe\tTeil\t1\t3\r\n"
            +"anfangprobe\tAnfang\t1\t4\r\n"
            +"endeprobe\tEnde\t1\t5\r\n"
            +"wild?card\tPlatzhalter\t1\t6\r\n"
            +"\\#zeichen\t\\#Ersatz\t1\t0\r\n";
        byte[] data=bytes(content);if(!SpeechDictionary.looksNvda(data))throw new AssertionError();
        if(SpeechDictionary.looksNvda(bytes("youtube\tbanana\n")))throw new AssertionError();
        SpeechDictionary dict=SpeechDictionary.parse(data);
        same(dict,"YouTube","banana",true);
        same(dict,"literalprobe","\\\\1",true);
        same(dict,"regprobe42","regprobe-42",true);
        same(dict,"WHOLEPROBE","Weg",true);
        same(dict,"xwholeprobe","xwholeprobe",false);
        same(dict,"teilprobe","teilprobe",false);same(dict,"xteilprobe","xTeil",true);
        same(dict,"anfangprobe","anfangprobe",false);same(dict,"anfangprobex","Anfangx",true);
        same(dict,"endeprobe","endeprobe",false);same(dict,"xendeprobe","xEnde",true);
        same(dict,"wildXcard","Platzhalter",true);same(dict,"#zeichen","#Ersatz",true);
        SpeechDictionary escaped=SpeechDictionary.parse(bytes("\\u00e4\\Z\tVokal\t1\t1\n"));
        same(escaped,"ä","Vokal",true);same(escaped,"ä\n","ä\n",false);
        same(SpeechDictionary.parse(bytes("abc\tabc\t1\t0\n")),"abc","abc",true);
        invalid("bad\tvalue\t0\t99\n");invalid("(\tvalue\t1\t1\n");invalid("a\tx\\2\t1\t1\n");
        try{SpeechDictionary.looksNvda(bytes("native\tword\nfoo\tbar\t0\t0\n"));throw new AssertionError();}
        catch(IOException expected){}
        Path directory=Files.createTempDirectory("reloq-dict");
        try {
            Files.write(directory.resolve(SpeechDictionary.snapshotName(0,0x10000,0)),bytes("foo\tbar\t0\t0\n"));
            Files.write(directory.resolve(SpeechDictionary.snapshotName(1,0x10000,2)),bytes("bar\tbaz\t0\t0\n"));
            Files.write(directory.resolve(SpeechDictionary.snapshotName(2,0x10001,0)),bytes("fileswap\tbanana\t0\t0\n"));
            same(SpeechDictionary.load(directory.toFile(),0x10000,1),"foo","bar",true);
            same(SpeechDictionary.load(directory.toFile(),0x10000,2),"foo","baz",true);
            same(SpeechDictionary.load(directory.toFile(),0x10001,2),"foo","foo",false);
            same(SpeechDictionary.load(directory.toFile(),0x10001,2),"fileswap","banana",true);
        } finally {try(java.util.stream.Stream<Path> paths=Files.list(directory)){paths.forEach(p -> {try{Files.delete(p);}catch(IOException e){throw new UncheckedIOException(e);}});}Files.delete(directory);}
        System.out.println("NVDA speech dictionaries: types, options, captures, scope, precedence and validation passed");
    }
}

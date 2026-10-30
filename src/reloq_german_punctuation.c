#include "reloq_german_punctuation.h"
#include <string.h>
static int space(unsigned char c) {return c==' '||c=='\t'||c=='\n'||c=='\r'||c==0xa0;}
static int letter(unsigned char c) {return (c>='A'&&c<='Z')||(c>='a'&&c<='z')||c==0xc4||c==0xd6||c==0xdc||c==0xe4||c==0xf6||c==0xfc||c==0xdf;}
static int quote(unsigned char c) {return c=='"'||c==0x82||c==0x84||c==0x93||c==0x94||c==0x8b||c==0x9b||c==0xab||c==0xbb;}
/* A compact punctuation token inside balanced wrappers names its inner
 * marks. Spaces terminate this class, so '( . )' retains normal punctuation.
 * Bound the look-around; long punctuation runs keep their existing path. */
static int enclosed(const unsigned char *s,size_t n,size_t at)
{
 size_t first=at,limit=at>255?at-255:0;
 while(first>limit&&strchr("()[]{}\".,:;?!",s[first-1]))--first;
 for(size_t left=first;left<=at;++left) {
  unsigned char close=s[left]=='('?')':s[left]=='['?']':s[left]=='{'?'}':s[left]=='"'?'"':0;
  if(!close)continue;
  unsigned depth=0;size_t end=n-left>256?left+256:n;
  for(size_t right=left+1;right<end&&strchr("()[]{}\".,:;?!",s[right]);++right) {
   if(s[left]!='"'&&s[right]==s[left])++depth;
   if(s[right]==close) {
    if(depth){--depth;continue;}
    /* Quotation marks name a single punctuation mark. Brackets can
     * enclose a compact run; quoted runs keep sentence punctuation. */
    if(left<=at&&at<right&&(s[left]!='"'||right==left+2))return 1;
    break;
   }
  }
 }
 return 0;
}
/* Normal-text character classes, observed across isolated, word and number
 * contexts. These bytes delimit tokens without acquiring a spoken name. */
int reloq_german_silent_separator(unsigned char c)
{
 if(c<=0x20 || c==0x7f || c==0xa0 || c==0xff)return 1;
 switch(c) {
 case 0x83:case 0x86:case 0x87:case 0x88:case 0x89:case 0x8e:
 case 0x95:case 0x99:case 0x9e:case 0x9f:return 1;
 default:return 0;
 }
}
int reloq_german_apostrophe(unsigned char c)
{return c=='\''||c==0x91||c==0x92||c==0xb4;}
int reloq_german_punctuation_read(const unsigned char *text,size_t length,size_t at,
 int have_word,struct reloq_german_punctuation *out)
{
 if(!text||!out||at>=length)return -1;
 unsigned char c=text[at],prev=at?text[at-1]:0,next=at+1<length?text[at+1]:0;
 struct reloq_german_punctuation p={RELOQ_DE_PUNCT_UNKNOWN,1,0,NULL,0};
 if(quote(c)||c=='['||c==']'||c=='{'||c=='}')p.action=RELOQ_DE_PUNCT_SILENT;
 else if(c=='<'&&(letter(next)||next=='<'||prev=='<'||
     ((next>='0'&&next<='9')&&!(prev>='0'&&prev<='9'))))p.action=RELOQ_DE_PUNCT_SILENT;
 else if(c=='&'||c==0xd7) {
  int context=have_word;
  for(size_t i=at+1;i<length&&!strchr(".?!:;",text[i]);++i)
   if(letter(text[i])||(text[i]>='0'&&text[i]<='9')) {context=1;break;}
  p.action=RELOQ_DE_PUNCT_SPOKEN;p.weak=context;
  p.spoken=c=='&'?(context?".0Unt":".1Unt"):(context?".0mal":".1mal");
 }
 else if(reloq_german_apostrophe(c)) {
  p.action=(!at||space(prev))&&(at+1==length||space(next))?RELOQ_DE_PUNCT_SPOKEN:RELOQ_DE_PUNCT_SILENT;
  p.spoken=".2A.0pOs.1trof";
 } else if(c==0xad) {
  p.action=RELOQ_DE_PUNCT_SILENT;
 } else if(c=='('||c==')') {
  if(c==')'&&letter(next)) {p.action=RELOQ_DE_PUNCT_SPOKEN;p.spoken=".1klA.0mR] `00 `[.0Tu";}
  else {p.action=c=='('&&enclosed(text,length,at)?RELOQ_DE_PUNCT_SILENT:RELOQ_DE_PUNCT_BOUNDARY;
      p.mark=c;}
 } else if(c=='-'||c==0x96||c==0x97) {
  size_t end=at+1;
  if(c=='-')while(end<length&&text[end]=='-')++end;
  /* longdash and doubled hyphens create a phrase boundary. A short dash
   * requires blanks on both sides (or the end of input); one-sided spacing
   * merely separates tokens. Public SPR omits this boundary, so its pause
   * was checked independently in the original acoustic schedule. */
  int phrase=c==0x97||end-at>1||
      (at&&space(prev)&&(end==length||space(text[end])));
  p.consumed=end-at;
  p.action=have_word&&phrase?RELOQ_DE_PUNCT_BOUNDARY:RELOQ_DE_PUNCT_SILENT;p.mark=c;
  if((c=='-'||c==0x96)&&end==at+1&&at&&letter(prev)&&next&&
      !letter(next)&&!(next>='0'&&next<='9')&&!space(next)&&!quote(next)&&!strchr(".,:;?!",next)) {
   p.action=RELOQ_DE_PUNCT_SPOKEN;p.spoken=".1bIn.0d@.2StrIX";
  }
 } else if(c==0x85) {p.action=have_word?RELOQ_DE_PUNCT_BOUNDARY:RELOQ_DE_PUNCT_SILENT;p.mark=0x85;}
 else if(c=='.'||c==','||c==':'||c==';'||c=='?'||c=='!') {
  size_t end=at+1;
  if(c=='.')while(end<length&&text[end]=='.')++end;
  else if(c=='?'||c=='!')while(end<length&&(text[end]=='?'||text[end]=='!'))++end;
  p.consumed=end-at;p.mark=c;
  if((enclosed(text,length,at)&&!(c=='.'&&p.consumed>=3))||(!have_word&&c=='.'&&p.consumed==1&&
      ((next>='0'&&next<='9')||(letter(next)&&!((next>='A'&&next<='Z')||next==0xc4||next==0xd6||next==0xdc))))) {
   p.action=RELOQ_DE_PUNCT_SPOKEN;p.consumed=1;
   p.spoken=c=='.'?".1pUGkt":c==','?".1kO.0mA":c==':'?".1dO.0p@l.2pUGkt":
    c==';'?".2ze.0mi.0ko.1lon":c=='?'?".1fra.0g@.2T'aj'.0X@n":".1'aw's.2ru.0f@.2T'aj'.0X@n";
  } else if(c=='.'&&p.consumed==1&&letter(prev)&&
      (at==1||(!letter(text[at-2])&&!(text[at-2]>='0'&&text[at-2]<='9'))))
   p.action=RELOQ_DE_PUNCT_SILENT;
  else if(!have_word)p.action=RELOQ_DE_PUNCT_SILENT;
  else if(c=='.'&&p.consumed>=3) {p.action=RELOQ_DE_PUNCT_BOUNDARY;p.mark=0x85;}
  else if(end<length&&letter(text[end])&&c!=','&&c!=';') {
   p.action=RELOQ_DE_PUNCT_SPOKEN;p.consumed=1;
   p.spoken=c=='.'?".1pUGkt":c==':'?".1dO.0p@l.2pUGkt":
    c==';'?".2ze.0mi.1ko.0lon":c=='?'?".1fra.0g@.2T'aj'.0X@n":".1'aw's.2ru.0f@.2T'aj'.0X@n";
  } else {
   p.action=RELOQ_DE_PUNCT_BOUNDARY;
   if(c=='?'||c=='!') {
    if(memchr(text+at,'?',p.consumed))p.mark='?';
    size_t following=end;
    while(following<length&&(space(text[following])||quote(text[following])||text[following]=='('||text[following]=='['))++following;
    if(following<length&&letter(text[following])&&
      !((text[following]>='A'&&text[following]<='Z')||text[following]==0xc4||text[following]==0xd6||text[following]==0xdc))
      p.mark=p.mark=='?'?RELOQ_DE_TEXT_QUESTION_CONTINUE:RELOQ_DE_TEXT_EXCLAMATION_CONTINUE;
   }
  }
 }
 *out=p;return 0;
}

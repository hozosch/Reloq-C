/* The built-in Korean ASCII-word dictionary. Phone plans continue through
 * normal Korean syllable, duration, prosody and voice-model processing. */
struct korean_source_word { const char *key; unsigned char count,phones[16]; };
static const struct korean_source_word korean_source_words[]={
 {"apec",5,{32,31,1,32,11}}, /* action 1 */
 {"apollo",6,{36,1,40,25,25,40}}, /* action 2 */
 {"apt",5,{36,1,36,5,34}}, /* action 3 */
 {"at",2,{33,7}}, /* action 4 */
 {"chosun",6,{8,20,40,18,35,23}}, /* action 5 */
 {"daum",4,{7,36,34,22}}, /* action 6 */
 {"donga",4,{7,40,24,36}}, /* action 7 */
 {"during",6,{7,27,39,26,31,24}}, /* action 8 */
 {"european",8,{27,39,26,40,1,31,35,23}}, /* action 9 */
 {"gnu",4,{11,34,23,39}}, /* action 10 */
 {"hankyoreh",8,{30,36,23,12,27,35,26,32}}, /* action 11 */
 {"hinet",6,{30,36,31,23,32,7}}, /* action 12 */
 {"jaguar",7,{8,20,33,12,27,39,35}}, /* action 13 */
 {"joongang",6,{8,20,39,24,36,24}}, /* action 14 */
 {"kaist",7,{9,36,31,18,34,5,34}}, /* action 15 */
 {"kedo",4,{9,32,8,40}}, /* action 17 */
 {"ketel",5,{9,32,5,32,25}}, /* action 16 */
 {"korea",5,{9,40,26,31,36}}, /* action 18 */
 {"korean",6,{9,40,26,31,35,23}}, /* action 19 */
 {"netian",6,{23,32,6,31,36,24}}, /* action 20 */
 {"new",3,{23,27,39}}, /* action 21 */
 {"news",5,{23,27,39,18,34}}, /* action 22 */
 {"newspaper",10,{23,27,39,18,34,1,32,31,1,35}}, /* action 23 */
 {"nimby",5,{23,31,22,4,31}}, /* action 24 */
 {"radio",5,{26,36,8,31,40}}, /* action 25 */
 {"roora",5,{26,39,25,25,36}}, /* action 26 */
 {"simmani",7,{20,31,22,22,36,23,31}}, /* action 27 */
 {"tachyon",6,{5,36,9,31,40,23}}, /* action 28 */
 {"timor",6,{5,31,22,40,26,34}}, /* action 29 */
};
static size_t korean_source_word(const char *text,unsigned char *out,size_t capacity)
{
 char key[10];size_t n=0;
 while(text[n]&&n+1<sizeof key){unsigned char c=(unsigned char)text[n];key[n]=(char)(c>='A'&&c<='Z'?c+32:c);++n;}
 if(text[n])return 0;key[n]=0;
 size_t lo=0,hi=sizeof korean_source_words/sizeof *korean_source_words;
 while(lo<hi){size_t mid=lo+(hi-lo)/2;int c=strcmp(key,korean_source_words[mid].key);
  if(c<0)hi=mid;else if(c>0)lo=mid+1;else{
   size_t count=korean_source_words[mid].count;
   if(out){if(count>capacity)return 0;memcpy(out,korean_source_words[mid].phones,count);}return count;
  }}
 return 0;
}

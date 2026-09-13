#include <tinyvm/artifact_v2.h>
#include <tinyvm/isa_v1.h>

#include <openssl/sha.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { V2_HEADER=512, V2_DIRECTORY_ENTRY=32, V2_WORD=32 };
enum { SEC_CODE=1, SEC_CONSTANTS=2, SEC_STRINGS=3, SEC_STORAGE=4, SEC_IMPORTS=5, SEC_PROVENANCE=6 };
typedef struct { uint32_t type; uint64_t offset,size,count; } Section;
static const uint8_t v2_magic[8]={'F','L','O','W','T','V','M',0};

static void diag(char *out,size_t cap,const char *message){if(out&&cap)snprintf(out,cap,"%s",message);}
static void p32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>(8*i));}
static void p64(uint8_t *p,uint64_t v){for(unsigned i=0;i<8;++i)p[i]=(uint8_t)(v>>(8*i));}
static uint32_t g32(const uint8_t *p){uint32_t v=0;for(unsigned i=0;i<4;++i)v|=(uint32_t)p[i]<<(8*i);return v;}
static uint64_t g64(const uint8_t *p){uint64_t v=0;for(unsigned i=0;i<8;++i)v|=(uint64_t)p[i]<<(8*i);return v;}
static int64_t gi64(const uint8_t *p){uint64_t bits=g64(p);int64_t v;memcpy(&v,&bits,8);return v;}
static bool zeros(const uint8_t *p,size_t n){for(size_t i=0;i<n;++i)if(p[i])return false;return true;}
static bool text64(const char field[64]){
    const char *end=memchr(field,0,64);if(!end||!field[0])return false;
    for(const unsigned char *p=(const unsigned char*)field;*p;++p)
        if(!((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')||*p=='.'||*p=='_'||*p==':'||*p=='/'||*p=='+'||*p=='-'||*p==','))return false;
    for(const char *p=end;p<field+64;++p)if(*p)return false;
    return true;
}
static bool add_size(size_t *total,size_t add){if(add>SIZE_MAX-*total)return false;*total+=add;return true;}
static bool mul_size(size_t count,size_t width,size_t *out){if(count&&width>SIZE_MAX/count)return false;*out=count*width;return true;}
static bool ids_sorted_u64(const uint64_t *ids,size_t count,size_t stride){uint64_t prev=0;for(size_t i=0;i<count;++i){uint64_t id;memcpy(&id,(const uint8_t*)ids+i*stride,8);if(id==0||(i&&id<=prev))return false;prev=id;}return true;}

void tinyvm_artifact_v2_init(TinyvmArtifactV2 *a){memset(a,0,sizeof(*a));a->data_words=256;a->stack_words=256;}
void tinyvm_artifact_v2_destroy(TinyvmArtifactV2 *a){
    if(!a)return;
    for(size_t i=0;i<a->string_count;++i)free(a->strings[i].bytes);
    free(a->code);free(a->constants);free(a->strings);free(a->storage);free(a->imports);free(a->provenance);tinyvm_artifact_v2_init(a);
}

bool tinyvm_artifact_v2_validate(const TinyvmArtifactV2 *a,char *d,size_t cap){
    if(!a){diag(d,cap,"artifact is null");return false;}
    if(a->isa_version>2){diag(d,cap,"unsupported ISA version");return false;}
    if(!text64(a->artifact_id)||!text64(a->source_id)||!text64(a->target_policy_id)||!text64(a->lowering_plan_id)||!text64(a->optimization_id)){diag(d,cap,"invalid required identity");return false;}
    if(a->isa_version==0&&!tinyvm_validate_recovered_code(a->code,a->code_count,a->entrypoint,a->data_words,a->stack_words,d,cap))return false;
    if((a->isa_version==1||a->isa_version==2)&&!tinyvm_isa_v1_validate(a,d,cap))return false;
    if(a->provenance_count!=a->code_count||(!a->provenance&&a->code_count)){diag(d,cap,"provenance count does not match code");return false;}
    if(a->constant_count&&(!a->constants||!ids_sorted_u64(&a->constants[0].id,a->constant_count,sizeof(*a->constants)))){diag(d,cap,"constant identities are not unique and sorted");return false;}
    for(size_t i=0;i<a->constant_count;++i){const TinyvmConstant *c=&a->constants[i];if(c->carrier<1||c->carrier>5||(c->carrier==1&&c->bits>1)||(c->carrier==2&&(int64_t)(int32_t)c->bits!=(int64_t)c->bits)||(c->carrier==4&&c->bits)||(c->carrier==5&&c->bits)){diag(d,cap,"invalid canonical constant");return false;}}
    if(a->string_count&&(!a->strings||!ids_sorted_u64(&a->strings[0].id,a->string_count,sizeof(*a->strings)))){diag(d,cap,"string identities are not unique and sorted");return false;}
    for(size_t i=0;i<a->string_count;++i)if(a->strings[i].length&&!a->strings[i].bytes){diag(d,cap,"string bytes are absent");return false;}
    if(a->storage_count&&(!a->storage||!ids_sorted_u64(&a->storage[0].id,a->storage_count,sizeof(*a->storage)))){diag(d,cap,"storage identities are not unique and sorted");return false;}
    for(size_t i=0;i<a->storage_count;++i){const TinyvmStorage *s=&a->storage[i];if(!s->bytes||!s->alignment||(s->alignment&(s->alignment-1))||s->kind<1||s->kind>4){diag(d,cap,"invalid storage declaration");return false;}}
    if(a->import_count&&(!a->imports||!ids_sorted_u64(&a->imports[0].id,a->import_count,sizeof(*a->imports)))){diag(d,cap,"import identities are not unique and sorted");return false;}
    for(size_t i=0;i<a->import_count;++i){const TinyvmImport *x=&a->imports[i];if(!text64(x->contract)||!text64(x->library)||!text64(x->convention)||!text64(x->symbol)||!text64(x->effect)||!text64(x->parameters)||!text64(x->result)||!text64(x->evidence)){diag(d,cap,"invalid import authority tuple");return false;}}
    for(size_t i=0;i<a->provenance_count;++i){const TinyvmProvenance *p=&a->provenance[i];if(p->instruction!=i||!p->operation||!p->block||!p->line||!p->column||!text64(p->source)||!text64(p->derivation)||strcmp(p->source,a->source_id)){diag(d,cap,"invalid instruction provenance");return false;}}
    diag(d,cap,"valid");return true;
}

static size_t sections_for(const TinyvmArtifactV2 *a,Section s[6],char *d,size_t cap){
    size_t n=0,offset=0,bytes=0,string_bytes=0;
    if(!mul_size(a->code_count,32,&bytes)){diag(d,cap,"code section overflow");return 0;}s[n++]=(Section){SEC_CODE,0,bytes,a->code_count};
    if(a->constant_count){if(!mul_size(a->constant_count,24,&bytes)){diag(d,cap,"constant section overflow");return 0;}s[n++]=(Section){SEC_CONSTANTS,0,bytes,a->constant_count};}
    if(a->string_count){if(!mul_size(a->string_count,24,&bytes)){diag(d,cap,"string table overflow");return 0;}for(size_t i=0;i<a->string_count;++i)if(!add_size(&string_bytes,a->strings[i].length)){diag(d,cap,"string bytes overflow");return 0;}if(!add_size(&bytes,string_bytes)){diag(d,cap,"string section overflow");return 0;}s[n++]=(Section){SEC_STRINGS,0,bytes,a->string_count};}
    if(a->storage_count){if(!mul_size(a->storage_count,32,&bytes)){diag(d,cap,"storage section overflow");return 0;}s[n++]=(Section){SEC_STORAGE,0,bytes,a->storage_count};}
    if(a->import_count){if(!mul_size(a->import_count,576,&bytes)){diag(d,cap,"import section overflow");return 0;}s[n++]=(Section){SEC_IMPORTS,0,bytes,a->import_count};}
    if(!mul_size(a->provenance_count,168,&bytes)){diag(d,cap,"provenance section overflow");return 0;}s[n++]=(Section){SEC_PROVENANCE,0,bytes,a->provenance_count};
    offset=V2_HEADER+n*V2_DIRECTORY_ENTRY;for(size_t i=0;i<n;++i){if(offset>SIZE_MAX-7){diag(d,cap,"artifact alignment overflow");return 0;}offset=(offset+7)&~(size_t)7;s[i].offset=offset;if(!add_size(&offset,s[i].size)){diag(d,cap,"artifact size overflow");return 0;}}
    return n;
}

static void encode_header(const TinyvmArtifactV2 *a,uint8_t *b,size_t size,const Section *s,size_t n){
    memset(b,0,size);memcpy(b,v2_magic,8);p32(b+8,2);p32(b+12,a->isa_version);p32(b+16,V2_HEADER);b[24]=1;b[25]=V2_WORD;
    p64(b+32,a->entrypoint);p64(b+40,a->code_count);p64(b+48,a->data_words);p64(b+56,a->stack_words);
    memcpy(b+64,a->artifact_id,64);memcpy(b+128,a->source_id,64);memcpy(b+192,a->target_policy_id,64);memcpy(b+256,a->lowering_plan_id,64);memcpy(b+320,a->optimization_id,64);
    p64(b+416,V2_HEADER);p32(b+424,(uint32_t)n);p32(b+428,V2_DIRECTORY_ENTRY);p64(b+432,size);
    for(size_t i=0;i<n;++i){uint8_t *e=b+V2_HEADER+i*32;p32(e,s[i].type);p64(e+8,s[i].offset);p64(e+16,s[i].size);p64(e+24,s[i].count);}
}

static void encode_sections(const TinyvmArtifactV2 *a,uint8_t *b,const Section *s,size_t n){
    for(size_t k=0;k<n;++k){uint8_t *p=b+s[k].offset;
        if(s[k].type==SEC_CODE)for(size_t i=0;i<a->code_count;++i){p64(p+i*32,(uint64_t)a->code[i].opcode);p64(p+i*32+8,(uint64_t)a->code[i].a);p64(p+i*32+16,(uint64_t)a->code[i].b);p64(p+i*32+24,(uint64_t)a->code[i].pad);}
        else if(s[k].type==SEC_CONSTANTS)for(size_t i=0;i<a->constant_count;++i){p64(p+i*24,a->constants[i].id);p32(p+i*24+8,a->constants[i].carrier);p64(p+i*24+16,a->constants[i].bits);}
        else if(s[k].type==SEC_STRINGS){size_t data=a->string_count*24,cursor=0;for(size_t i=0;i<a->string_count;++i){p64(p+i*24,a->strings[i].id);p64(p+i*24+8,cursor);p64(p+i*24+16,a->strings[i].length);memcpy(p+data+cursor,a->strings[i].bytes,a->strings[i].length);cursor+=a->strings[i].length;}}
        else if(s[k].type==SEC_STORAGE)for(size_t i=0;i<a->storage_count;++i){p64(p+i*32,a->storage[i].id);p64(p+i*32+8,a->storage[i].bytes);p64(p+i*32+16,a->storage[i].alignment);p32(p+i*32+24,a->storage[i].kind);}
        else if(s[k].type==SEC_IMPORTS)for(size_t i=0;i<a->import_count;++i){uint8_t *r=p+i*576;p64(r,a->imports[i].id);memcpy(r+8,a->imports[i].contract,64);memcpy(r+72,a->imports[i].library,64);memcpy(r+136,a->imports[i].convention,64);memcpy(r+200,a->imports[i].symbol,64);memcpy(r+264,a->imports[i].effect,64);memcpy(r+328,a->imports[i].parameters,64);memcpy(r+392,a->imports[i].result,64);memcpy(r+456,a->imports[i].evidence,64);}
        else if(s[k].type==SEC_PROVENANCE)for(size_t i=0;i<a->provenance_count;++i){uint8_t *r=p+i*168;const TinyvmProvenance *v=&a->provenance[i];p64(r,v->instruction);p64(r+8,v->operation);p64(r+16,v->block);p64(r+24,v->symbol);p32(r+32,v->line);p32(r+36,v->column);memcpy(r+40,v->source,64);memcpy(r+104,v->derivation,64);}
    }
}

bool tinyvm_artifact_v2_write(const char *path,TinyvmArtifactV2 *a,char *d,size_t cap){
    if(!tinyvm_artifact_v2_validate(a,d,cap))return false;
    Section s[6];size_t n=sections_for(a,s,d,cap);if(!n)return false;size_t size=s[n-1].offset+s[n-1].size;uint8_t *b=malloc(size);if(!b){diag(d,cap,"allocation failed");return false;}
    encode_header(a,b,size,s,n);encode_sections(a,b,s,n);SHA256(b,size,b+384);memcpy(a->digest,b+384,32);
    FILE *f=fopen(path,"wb");bool ok=false;if(f){ok=fwrite(b,1,size,f)==size;if(fclose(f))ok=false;}free(b);diag(d,cap,ok?"valid":"artifact write failed");return ok;
}

static bool parse_directory(const uint8_t *b,size_t size,Section s[6],size_t *count,char *d,size_t cap){
    uint32_t n=g32(b+424);if(n<2||n>6||g64(b+416)!=V2_HEADER||g32(b+428)!=32||size<V2_HEADER+(size_t)n*32){diag(d,cap,"invalid section directory header");return false;}
    uint64_t expected=V2_HEADER+(uint64_t)n*32;uint32_t previous=0;
    for(uint32_t i=0;i<n;++i){const uint8_t *e=b+V2_HEADER+i*32;s[i]=(Section){g32(e),g64(e+8),g64(e+16),g64(e+24)};expected=(expected+7)&~UINT64_C(7);if(g32(e+4)||s[i].type<=previous||s[i].type>6||s[i].offset!=expected||s[i].offset>size||s[i].size>size-s[i].offset){diag(d,cap,"invalid section directory entry");return false;}if(!zeros(b+(size_t)(i? s[i-1].offset+s[i-1].size : V2_HEADER+n*32),(size_t)(s[i].offset-(i? s[i-1].offset+s[i-1].size : V2_HEADER+n*32)))){diag(d,cap,"section alignment padding is nonzero");return false;}expected=s[i].offset+s[i].size;previous=s[i].type;}
    if(expected!=size||s[0].type!=SEC_CODE||s[n-1].type!=SEC_PROVENANCE){diag(d,cap,"required sections or exact coverage missing");return false;}*count=n;return true;
}

static const Section *find_section(const Section *s,size_t n,uint32_t type){for(size_t i=0;i<n;++i)if(s[i].type==type)return &s[i];return NULL;}

bool tinyvm_artifact_v2_read(const char *path,TinyvmArtifactV2 *a,char *d,size_t cap){
    tinyvm_artifact_v2_init(a);FILE *f=fopen(path,"rb");if(!f){diag(d,cap,"artifact open failed");return false;}if(fseek(f,0,SEEK_END)){fclose(f);diag(d,cap,"artifact seek failed");return false;}long end=ftell(f);rewind(f);if(end<V2_HEADER){fclose(f);diag(d,cap,"artifact is truncated");return false;}size_t size=(size_t)end;uint8_t *b=malloc(size);if(!b){fclose(f);diag(d,cap,"allocation failed");return false;}bool read_ok=fread(b,1,size,f)==size;if(fclose(f))read_ok=false;if(!read_ok){free(b);diag(d,cap,"artifact read failed");return false;}
    if(memcmp(b,v2_magic,8)||g32(b+8)!=2||g32(b+16)!=V2_HEADER||g32(b+20)||b[24]!=1||b[25]!=32||!zeros(b+26,6)||g64(b+432)!=size||!zeros(b+440,72)){free(b);diag(d,cap,"unsupported or noncanonical header");return false;}
    uint8_t stored[32],actual[32];memcpy(stored,b+384,32);memset(b+384,0,32);SHA256(b,size,actual);if(memcmp(stored,actual,32)){free(b);diag(d,cap,"artifact digest mismatch");return false;}memcpy(b+384,stored,32);
    Section s[6];size_t n;if(!parse_directory(b,size,s,&n,d,cap)){free(b);return false;}const Section *code=find_section(s,n,SEC_CODE),*prov=find_section(s,n,SEC_PROVENANCE);if(code->count>SIZE_MAX/32||prov->count>SIZE_MAX/168||code->size!=code->count*32||prov->size!=prov->count*168||code->count!=prov->count||g64(b+40)!=code->count){free(b);diag(d,cap,"section record size/count mismatch");return false;}
    a->isa_version=g32(b+12);a->entrypoint=g64(b+32);a->data_words=g64(b+48);a->stack_words=g64(b+56);a->code_count=(size_t)code->count;memcpy(a->artifact_id,b+64,64);memcpy(a->source_id,b+128,64);memcpy(a->target_policy_id,b+192,64);memcpy(a->lowering_plan_id,b+256,64);memcpy(a->optimization_id,b+320,64);memcpy(a->digest,stored,32);
    a->code=calloc(a->code_count,sizeof(*a->code));a->provenance_count=(size_t)prov->count;a->provenance=calloc(a->provenance_count,sizeof(*a->provenance));if(!a->code||!a->provenance){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"allocation failed");return false;}
    for(size_t i=0;i<a->code_count;++i){const uint8_t *p=b+code->offset+i*32;a->code[i]=(InstrWord){gi64(p),gi64(p+8),gi64(p+16),gi64(p+24)};}
    for(size_t i=0;i<a->provenance_count;++i){const uint8_t *p=b+prov->offset+i*168;TinyvmProvenance *v=&a->provenance[i];v->instruction=g64(p);v->operation=g64(p+8);v->block=g64(p+16);v->symbol=g64(p+24);v->line=g32(p+32);v->column=g32(p+36);memcpy(v->source,p+40,64);memcpy(v->derivation,p+104,64);}
    const Section *x=find_section(s,n,SEC_CONSTANTS);if(x){if(x->count>SIZE_MAX/24||x->size!=x->count*24){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"constant section malformed");return false;}a->constant_count=(size_t)x->count;a->constants=calloc(a->constant_count,sizeof(*a->constants));if(!a->constants){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"allocation failed");return false;}for(size_t i=0;i<a->constant_count;++i){const uint8_t *p=b+x->offset+i*24;a->constants[i]=(TinyvmConstant){g64(p),g32(p+8),g64(p+16)};}}
    x=find_section(s,n,SEC_STORAGE);if(x){if(x->count>SIZE_MAX/32||x->size!=x->count*32){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"storage section malformed");return false;}a->storage_count=(size_t)x->count;a->storage=calloc(a->storage_count,sizeof(*a->storage));if(!a->storage){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"allocation failed");return false;}for(size_t i=0;i<a->storage_count;++i){const uint8_t *p=b+x->offset+i*32;a->storage[i]=(TinyvmStorage){g64(p),g64(p+8),g64(p+16),g32(p+24)};}}
    x=find_section(s,n,SEC_IMPORTS);if(x){if(x->count>SIZE_MAX/576||x->size!=x->count*576){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"import section malformed");return false;}a->import_count=(size_t)x->count;a->imports=calloc(a->import_count,sizeof(*a->imports));if(!a->imports){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"allocation failed");return false;}for(size_t i=0;i<a->import_count;++i){const uint8_t *p=b+x->offset+i*576;TinyvmImport *v=&a->imports[i];v->id=g64(p);memcpy(v->contract,p+8,64);memcpy(v->library,p+72,64);memcpy(v->convention,p+136,64);memcpy(v->symbol,p+200,64);memcpy(v->effect,p+264,64);memcpy(v->parameters,p+328,64);memcpy(v->result,p+392,64);memcpy(v->evidence,p+456,64);if(!zeros(p+520,56)){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"import reserved bytes are nonzero");return false;}}}
    x=find_section(s,n,SEC_STRINGS);if(x){if(x->count>SIZE_MAX/24||x->count>(x->size/24)){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"string section malformed");return false;}a->string_count=(size_t)x->count;a->strings=calloc(a->string_count,sizeof(*a->strings));if(!a->strings){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"allocation failed");return false;}size_t table=a->string_count*24,cursor=0;for(size_t i=0;i<a->string_count;++i){const uint8_t *p=b+x->offset+i*24;uint64_t off=g64(p+8),len=g64(p+16);if(off!=cursor||len>SIZE_MAX||len>x->size-table-cursor){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"string ranges are noncanonical");return false;}a->strings[i].id=g64(p);a->strings[i].length=(size_t)len;if(len){a->strings[i].bytes=malloc((size_t)len);if(!a->strings[i].bytes){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"allocation failed");return false;}memcpy(a->strings[i].bytes,b+x->offset+table+cursor,(size_t)len);}cursor+=(size_t)len;}if(cursor!=x->size-table){free(b);tinyvm_artifact_v2_destroy(a);diag(d,cap,"string bytes not fully covered");return false;}}
    free(b);if(!tinyvm_artifact_v2_validate(a,d,cap)){tinyvm_artifact_v2_destroy(a);return false;}return true;
}

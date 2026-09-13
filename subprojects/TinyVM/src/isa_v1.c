#include <tinyvm/isa_v1.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void diagnose(char *out,size_t capacity,const char *message){if(out&&capacity)snprintf(out,capacity,"%s",message);}
static bool slot(const TinyvmArtifactV2 *a,int64_t value){return value>=0&&(uint64_t)value<a->data_words;}
static const TinyvmConstant *constant(const TinyvmArtifactV2 *a,uint64_t id){for(size_t i=0;i<a->constant_count;++i)if(a->constants[i].id==id)return &a->constants[i];return NULL;}
static const TinyvmString *string_value(const TinyvmArtifactV2 *a,uint64_t id){for(size_t i=0;i<a->string_count;++i)if(a->strings[i].id==id)return &a->strings[i];return NULL;}
static const TinyvmStorage *storage_value(const TinyvmArtifactV2 *a,uint64_t id){for(size_t i=0;i<a->storage_count;++i)if(a->storage[i].id==id)return &a->storage[i];return NULL;}
static const TinyvmImport *import_value(const TinyvmArtifactV2 *a,uint64_t id){for(size_t i=0;i<a->import_count;++i)if(a->imports[i].id==id)return &a->imports[i];return NULL;}
static size_t import_parameter_count(const TinyvmImport *value){if(!value->parameters[0]||strcmp(value->parameters,"none")==0)return 0;size_t count=1;for(const char *p=value->parameters;*p;++p)if(*p==',')++count;return count;}
static bool zero2(const InstrWord *w){return w->b==0&&w->pad==0;}
static bool zero3(const InstrWord *w){return w->a==0&&w->b==0&&w->pad==0;}

bool tinyvm_isa_v1_validate(const TinyvmArtifactV2 *a,char *d,size_t capacity){
    if(!a||(a->isa_version!=1&&a->isa_version!=2)){diagnose(d,capacity,"typed ISA validator received a different ISA");return false;}
    if(!a->code||!a->code_count){diagnose(d,capacity,"ISA v1 code is empty");return false;}
    if(a->entrypoint>=a->code_count){diagnose(d,capacity,"ISA v1 entrypoint is outside code");return false;}
    if(a->isa_version==2&&!a->data_words){diagnose(d,capacity,"ISA v2 execution-input slot is absent");return false;}
    if(a->data_words>UINT32_MAX){diagnose(d,capacity,"ISA v1 virtual slot capacity is unsupported");return false;}
    for(size_t pc=0;pc<a->code_count;++pc){const InstrWord *w=&a->code[pc];
        if(w->opcode<0||w->opcode>=TV1_OPCODE_COUNT){diagnose(d,capacity,"ISA v1 opcode is unsupported");return false;}
        switch(w->opcode){
        case TV1_NOP:case TV1_HALT:if(!zero3(w)){diagnose(d,capacity,"ISA v1 zero operands are nonzero");return false;}break;
        case TV1_CONST:if(!slot(a,w->a)||w->b<=0||!constant(a,(uint64_t)w->b)||w->pad){diagnose(d,capacity,"ISA v1 constant reference is invalid");return false;}break;
        case TV1_MOVE:if(!slot(a,w->a)||!slot(a,w->b)||w->pad){diagnose(d,capacity,"ISA v1 move operand is invalid");return false;}break;
        case TV1_CONVERT:if(!slot(a,w->a)||!slot(a,w->b)||w->pad<TINYVM_CARRIER_I1||w->pad>TINYVM_CARRIER_I64){diagnose(d,capacity,"ISA v1 conversion is invalid");return false;}break;
        case TV1_ADD:case TV1_SUB:case TV1_MUL:case TV1_SDIV:
        case TV1_CMP_EQ:case TV1_CMP_NE:case TV1_CMP_LT:case TV1_CMP_LE:case TV1_CMP_GT:case TV1_CMP_GE:
            if(!slot(a,w->a)||!slot(a,w->b)||!slot(a,w->pad)){diagnose(d,capacity,"ISA v1 slot operand is invalid");return false;}break;
        case TV1_JMP:if(w->a<0||(uint64_t)w->a>=a->code_count||!zero2(w)){diagnose(d,capacity,"ISA v1 jump target is invalid");return false;}break;
        case TV1_BRANCH:if(!slot(a,w->a)||w->b<0||w->pad<0||(uint64_t)w->b>=a->code_count||(uint64_t)w->pad>=a->code_count){diagnose(d,capacity,"ISA v1 branch is invalid");return false;}break;
        case TV1_RETURN:if(!slot(a,w->a)||!zero2(w)){diagnose(d,capacity,"ISA v1 return is invalid");return false;}break;
        case TV1_TRAP:if(w->a<=0||!zero2(w)){diagnose(d,capacity,"ISA v1 trap is invalid");return false;}break;
        case TV1_STRING_HANDLE:if(!slot(a,w->a)||w->b<=0||(uint64_t)w->b>UINT64_C(0x00ffffffffffffff)||!string_value(a,(uint64_t)w->b)||w->pad){diagnose(d,capacity,"ISA v1 string handle is invalid");return false;}break;
        case TV1_STORAGE_HANDLE:if(!slot(a,w->a)||w->b<=0||(uint64_t)w->b>UINT64_C(0x00ffffffffffffff)||!storage_value(a,(uint64_t)w->b)||w->pad){diagnose(d,capacity,"ISA v1 storage handle is invalid");return false;}break;
        case TV1_CALL_IMPORT:{const TinyvmImport *x=w->b>0?import_value(a,(uint64_t)w->b):NULL;if(!slot(a,w->a)||!x||w->pad<0||(uint64_t)w->pad>a->data_words||import_parameter_count(x)>a->data_words-(uint64_t)w->pad){diagnose(d,capacity,"ISA v1 import call is invalid");return false;}break;}
        case TV1_TEXT_OUTCOME_CODE:case TV1_TEXT_OUTCOME_VALUE:if(!slot(a,w->a)||!slot(a,w->b)||w->pad){diagnose(d,capacity,"ISA v1 TextOutcome projection is invalid");return false;}break;
        }
    }
    const int64_t terminal=a->code[a->code_count-1].opcode;if(terminal!=TV1_RETURN&&terminal!=TV1_TRAP&&terminal!=TV1_HALT){diagnose(d,capacity,"ISA v1 code has no terminal final instruction");return false;}
    diagnose(d,capacity,"valid");return true;
}

bool tinyvm_isa_v1_context_init(TinyvmIsaV1Context *ctx,size_t slots,uint64_t step_limit){
    if(!ctx||!step_limit)return false;
    memset(ctx,0,sizeof(*ctx));if(slots){ctx->slots=calloc(slots,sizeof(*ctx->slots));if(!ctx->slots)return false;}ctx->slot_count=slots;ctx->step_limit=step_limit;ctx->running=true;return true;
}
void tinyvm_isa_v1_context_destroy(TinyvmIsaV1Context *ctx){if(!ctx)return;free(ctx->slots);memset(ctx,0,sizeof(*ctx));}
static bool trap(TinyvmIsaV1Context *ctx,uint32_t code,const char *fault,uint64_t instruction){ctx->trap=code;ctx->fault=fault;ctx->trap_instruction=instruction;ctx->running=false;return false;}
static TinyvmValue *read_slot(TinyvmIsaV1Context *ctx,uint64_t index,uint64_t instruction){TinyvmValue *v=&ctx->slots[index];if(!v->initialized){trap(ctx,TV1_TRAP_UNINITIALIZED_SLOT,"uninitialized virtual slot",instruction);return NULL;}return v;}
static uint64_t canonical_i32(int32_t value){return (uint64_t)(int64_t)value;}
static int64_t signed_bits(uint64_t bits){int64_t value;memcpy(&value,&bits,8);return value;}
static bool integer_carrier(uint32_t carrier){return carrier==TINYVM_CARRIER_I32||carrier==TINYVM_CARRIER_I64;}
static bool binary_arithmetic(TinyvmIsaV1Context *ctx,int64_t opcode,uint64_t dst,uint64_t left,uint64_t right,uint64_t instruction){
    TinyvmValue *x=read_slot(ctx,left,instruction),*y=read_slot(ctx,right,instruction);if(!x||!y)return false;if(x->carrier!=y->carrier||!integer_carrier(x->carrier))return trap(ctx,TV1_TRAP_TYPE_MISMATCH,"arithmetic carrier mismatch",instruction);
    TinyvmValue result={x->carrier,0,true};
    if(x->carrier==TINYVM_CARRIER_I32){int32_t a=(int32_t)x->bits,b=(int32_t)y->bits,r=0;if(opcode==TV1_SDIV){if(!b)return trap(ctx,TV1_TRAP_DIVISION_BY_ZERO,"division by zero",instruction);if(a==INT32_MIN&&b==-1)return trap(ctx,TV1_TRAP_DIVISION_OVERFLOW,"signed division overflow",instruction);r=a/b;}else{uint32_t ua=(uint32_t)a,ub=(uint32_t)b,ur=opcode==TV1_ADD?ua+ub:opcode==TV1_SUB?ua-ub:ua*ub;memcpy(&r,&ur,4);}result.bits=canonical_i32(r);
    }else{int64_t a=signed_bits(x->bits),b=signed_bits(y->bits),r=0;if(opcode==TV1_SDIV){if(!b)return trap(ctx,TV1_TRAP_DIVISION_BY_ZERO,"division by zero",instruction);if(a==INT64_MIN&&b==-1)return trap(ctx,TV1_TRAP_DIVISION_OVERFLOW,"signed division overflow",instruction);r=a/b;result.bits=(uint64_t)r;}else{uint64_t ua=x->bits,ub=y->bits;result.bits=opcode==TV1_ADD?ua+ub:opcode==TV1_SUB?ua-ub:ua*ub;}}
    ctx->slots[dst]=result;return true;
}
static bool comparison(TinyvmIsaV1Context *ctx,int64_t opcode,uint64_t dst,uint64_t left,uint64_t right,uint64_t instruction){
    TinyvmValue *x=read_slot(ctx,left,instruction),*y=read_slot(ctx,right,instruction);if(!x||!y)return false;if(x->carrier!=y->carrier||(!integer_carrier(x->carrier)&&x->carrier!=TINYVM_CARRIER_I1))return trap(ctx,TV1_TRAP_TYPE_MISMATCH,"comparison carrier mismatch",instruction);int64_t a=x->carrier==TINYVM_CARRIER_I32?(int32_t)x->bits:signed_bits(x->bits),b=y->carrier==TINYVM_CARRIER_I32?(int32_t)y->bits:signed_bits(y->bits);bool r=opcode==TV1_CMP_EQ?a==b:opcode==TV1_CMP_NE?a!=b:opcode==TV1_CMP_LT?a<b:opcode==TV1_CMP_LE?a<=b:opcode==TV1_CMP_GT?a>b:a>=b;ctx->slots[dst]=(TinyvmValue){TINYVM_CARRIER_I1,r?1:0,true};return true;
}
static bool call_import(const TinyvmArtifactV2 *a,TinyvmIsaV1Context *ctx,const InstrWord *w,uint64_t instruction){const TinyvmImport *x=import_value(a,(uint64_t)w->b);const size_t count=import_parameter_count(x);for(size_t i=0;i<count;++i)if(!read_slot(ctx,(uint64_t)w->pad+i,instruction))return false;if(!ctx->import_resolver)return trap(ctx,TV1_TRAP_UNRESOLVED_IMPORT,"authorized import has no runtime resolver",instruction);TinyvmValue result={0};const char *fault="runtime provider rejected import";if(!ctx->import_resolver(ctx->import_user,a,x,&ctx->slots[w->pad],count,&result,&fault))return trap(ctx,TV1_TRAP_UNRESOLVED_IMPORT,fault?fault:"runtime provider rejected import",instruction);if(!result.initialized)return trap(ctx,TV1_TRAP_UNRESOLVED_IMPORT,"runtime provider returned an uninitialized value",instruction);ctx->slots[w->a]=result;return true;}
static bool text_outcome_projection(TinyvmIsaV1Context *ctx,uint64_t destination,uint64_t source,bool value_field,uint64_t instruction){TinyvmValue *outcome=read_slot(ctx,source,instruction);if(!outcome)return false;if(outcome->carrier!=TINYVM_CARRIER_TEXT_OUTCOME||!ctx->text_outcome_resolver)return trap(ctx,TV1_TRAP_TYPE_MISMATCH,"TextOutcome projection is unavailable",instruction);TinyvmValue result={0};const char *fault="TextOutcome projection failed";if(!ctx->text_outcome_resolver(ctx->text_outcome_user,outcome,value_field,&result,&fault))return trap(ctx,TV1_TRAP_UNRESOLVED_IMPORT,fault,instruction);if(!result.initialized)return trap(ctx,TV1_TRAP_UNRESOLVED_IMPORT,"TextOutcome projection returned an uninitialized value",instruction);ctx->slots[destination]=result;return true;}

static void execution_inputs(const TinyvmArtifactV2 *a,TinyvmIsaV1Context *ctx){if(a->isa_version!=2||!ctx->slot_count)return;ctx->slots[0]=(TinyvmValue){TINYVM_CARRIER_I32,canonical_i32((int32_t)ctx->argument_count),true};for(size_t i=0;i<ctx->argument_count&&i+1<ctx->slot_count;++i)ctx->slots[i+1]=(TinyvmValue){TINYVM_CARRIER_OPAQUE_HANDLE,UINT64_C(0x0300000000000000)|i,true};}

bool tinyvm_isa_v1_run_switch(const TinyvmArtifactV2 *a,TinyvmIsaV1Context *ctx){
    if(!a||!ctx||(a->isa_version!=1&&a->isa_version!=2)||ctx->slot_count<a->data_words)return false;
    execution_inputs(a,ctx);
    ctx->pc=a->entrypoint;
    while(ctx->running){if(ctx->steps++>=ctx->step_limit)return trap(ctx,TV1_TRAP_STEP_LIMIT,"instruction step limit exceeded",ctx->pc);uint64_t at=ctx->pc++;const InstrWord *w=&a->code[at];
        switch(w->opcode){
        case TV1_NOP:break;
        case TV1_CONST:{const TinyvmConstant *v=constant(a,(uint64_t)w->b);ctx->slots[w->a]=(TinyvmValue){v->carrier,v->bits,true};break;}
        case TV1_MOVE:{TinyvmValue *v=read_slot(ctx,(uint64_t)w->b,at);if(v)ctx->slots[w->a]=*v;break;}
        case TV1_CONVERT:{TinyvmValue *v=read_slot(ctx,(uint64_t)w->b,at);if(!v)break;if(v->carrier==TINYVM_CARRIER_OPAQUE_HANDLE){trap(ctx,TV1_TRAP_TYPE_MISMATCH,"opaque handle conversion is forbidden",at);break;}uint64_t bits=v->bits;if(w->pad==TINYVM_CARRIER_I1&&bits>1){trap(ctx,TV1_TRAP_TYPE_MISMATCH,"noncanonical boolean conversion",at);break;}if(w->pad==TINYVM_CARRIER_I32)bits=canonical_i32((int32_t)bits);else if(w->pad==TINYVM_CARRIER_I64&&v->carrier==TINYVM_CARRIER_I32)bits=(uint64_t)(int64_t)(int32_t)bits;ctx->slots[w->a]=(TinyvmValue){(uint32_t)w->pad,bits,true};break;}
        case TV1_ADD:case TV1_SUB:case TV1_MUL:case TV1_SDIV:binary_arithmetic(ctx,w->opcode,(uint64_t)w->a,(uint64_t)w->b,(uint64_t)w->pad,at);break;
        case TV1_CMP_EQ:case TV1_CMP_NE:case TV1_CMP_LT:case TV1_CMP_LE:case TV1_CMP_GT:case TV1_CMP_GE:comparison(ctx,w->opcode,(uint64_t)w->a,(uint64_t)w->b,(uint64_t)w->pad,at);break;
        case TV1_JMP:ctx->pc=(uint64_t)w->a;break;
        case TV1_BRANCH:{TinyvmValue *v=read_slot(ctx,(uint64_t)w->a,at);if(!v)break;if(v->carrier!=TINYVM_CARRIER_I1){trap(ctx,TV1_TRAP_TYPE_MISMATCH,"branch condition is not i1",at);break;}ctx->pc=v->bits?(uint64_t)w->b:(uint64_t)w->pad;break;}
        case TV1_RETURN:{TinyvmValue *v=read_slot(ctx,(uint64_t)w->a,at);if(v){ctx->result=*v;ctx->returned=true;ctx->running=false;}break;}
        case TV1_TRAP:trap(ctx,(uint32_t)w->a,"explicit artifact trap",at);break;
        case TV1_HALT:ctx->result=(TinyvmValue){TINYVM_CARRIER_I32,0,true};ctx->returned=true;ctx->running=false;break;
        case TV1_STRING_HANDLE:ctx->slots[w->a]=(TinyvmValue){TINYVM_CARRIER_OPAQUE_HANDLE,UINT64_C(0x0100000000000000)|(uint64_t)w->b,true};break;
        case TV1_STORAGE_HANDLE:ctx->slots[w->a]=(TinyvmValue){TINYVM_CARRIER_OPAQUE_HANDLE,UINT64_C(0x0200000000000000)|(uint64_t)w->b,true};break;
        case TV1_CALL_IMPORT:call_import(a,ctx,w,at);break;
        case TV1_TEXT_OUTCOME_CODE:text_outcome_projection(ctx,(uint64_t)w->a,(uint64_t)w->b,false,at);break;
        case TV1_TEXT_OUTCOME_VALUE:text_outcome_projection(ctx,(uint64_t)w->a,(uint64_t)w->b,true,at);break;
        default:return trap(ctx,TV1_TRAP_EXPLICIT,"invalid validated opcode",at);
        }
    }
    return ctx->returned&&ctx->trap==0;
}

bool tinyvm_isa_v1_run_computed(const TinyvmArtifactV2 *a,TinyvmIsaV1Context *ctx){
    static void *dispatch[TV1_OPCODE_COUNT]={
        [TV1_NOP]=&&do_nop,[TV1_CONST]=&&do_const,[TV1_MOVE]=&&do_move,
        [TV1_CONVERT]=&&do_convert,[TV1_ADD]=&&do_arithmetic,[TV1_SUB]=&&do_arithmetic,
        [TV1_MUL]=&&do_arithmetic,[TV1_SDIV]=&&do_arithmetic,[TV1_CMP_EQ]=&&do_compare,
        [TV1_CMP_NE]=&&do_compare,[TV1_CMP_LT]=&&do_compare,[TV1_CMP_LE]=&&do_compare,
        [TV1_CMP_GT]=&&do_compare,[TV1_CMP_GE]=&&do_compare,[TV1_JMP]=&&do_jump,
        [TV1_BRANCH]=&&do_branch,[TV1_RETURN]=&&do_return,[TV1_TRAP]=&&do_trap,
        [TV1_HALT]=&&do_halt,[TV1_STRING_HANDLE]=&&do_string,
        [TV1_STORAGE_HANDLE]=&&do_storage,[TV1_CALL_IMPORT]=&&do_import,
        [TV1_TEXT_OUTCOME_CODE]=&&do_outcome_code,[TV1_TEXT_OUTCOME_VALUE]=&&do_outcome_value};
    if(!a||!ctx||(a->isa_version!=1&&a->isa_version!=2)||ctx->slot_count<a->data_words)return false;
    execution_inputs(a,ctx);
    ctx->pc=a->entrypoint;uint64_t at=0;const InstrWord *w=NULL;
next:
    if(!ctx->running)goto done;
    if(ctx->steps++>=ctx->step_limit){trap(ctx,TV1_TRAP_STEP_LIMIT,"instruction step limit exceeded",ctx->pc);goto done;}
    at=ctx->pc++;w=&a->code[at];if(w->opcode<0||w->opcode>=TV1_OPCODE_COUNT){trap(ctx,TV1_TRAP_EXPLICIT,"invalid validated opcode",at);goto done;}goto *dispatch[w->opcode];
do_nop:goto next;
do_const:{const TinyvmConstant *v=constant(a,(uint64_t)w->b);ctx->slots[w->a]=(TinyvmValue){v->carrier,v->bits,true};goto next;}
do_move:{TinyvmValue *v=read_slot(ctx,(uint64_t)w->b,at);if(v)ctx->slots[w->a]=*v;goto next;}
do_convert:{TinyvmValue *v=read_slot(ctx,(uint64_t)w->b,at);if(!v)goto next;if(v->carrier==TINYVM_CARRIER_OPAQUE_HANDLE){trap(ctx,TV1_TRAP_TYPE_MISMATCH,"opaque handle conversion is forbidden",at);goto next;}uint64_t bits=v->bits;if(w->pad==TINYVM_CARRIER_I1&&bits>1){trap(ctx,TV1_TRAP_TYPE_MISMATCH,"noncanonical boolean conversion",at);goto next;}if(w->pad==TINYVM_CARRIER_I32)bits=canonical_i32((int32_t)bits);else if(w->pad==TINYVM_CARRIER_I64&&v->carrier==TINYVM_CARRIER_I32)bits=(uint64_t)(int64_t)(int32_t)bits;ctx->slots[w->a]=(TinyvmValue){(uint32_t)w->pad,bits,true};goto next;}
do_arithmetic:binary_arithmetic(ctx,w->opcode,(uint64_t)w->a,(uint64_t)w->b,(uint64_t)w->pad,at);goto next;
do_compare:comparison(ctx,w->opcode,(uint64_t)w->a,(uint64_t)w->b,(uint64_t)w->pad,at);goto next;
do_jump:ctx->pc=(uint64_t)w->a;goto next;
do_branch:{TinyvmValue *v=read_slot(ctx,(uint64_t)w->a,at);if(!v)goto next;if(v->carrier!=TINYVM_CARRIER_I1){trap(ctx,TV1_TRAP_TYPE_MISMATCH,"branch condition is not i1",at);goto next;}ctx->pc=v->bits?(uint64_t)w->b:(uint64_t)w->pad;goto next;}
do_return:{TinyvmValue *v=read_slot(ctx,(uint64_t)w->a,at);if(v){ctx->result=*v;ctx->returned=true;ctx->running=false;}goto next;}
do_trap:trap(ctx,(uint32_t)w->a,"explicit artifact trap",at);goto next;
do_halt:ctx->result=(TinyvmValue){TINYVM_CARRIER_I32,0,true};ctx->returned=true;ctx->running=false;goto next;
do_string:ctx->slots[w->a]=(TinyvmValue){TINYVM_CARRIER_OPAQUE_HANDLE,UINT64_C(0x0100000000000000)|(uint64_t)w->b,true};goto next;
do_storage:ctx->slots[w->a]=(TinyvmValue){TINYVM_CARRIER_OPAQUE_HANDLE,UINT64_C(0x0200000000000000)|(uint64_t)w->b,true};goto next;
do_import:call_import(a,ctx,w,at);goto next;
do_outcome_code:text_outcome_projection(ctx,(uint64_t)w->a,(uint64_t)w->b,false,at);goto next;
do_outcome_value:text_outcome_projection(ctx,(uint64_t)w->a,(uint64_t)w->b,true,at);goto next;
done:
    return ctx->returned&&ctx->trap==0;
}

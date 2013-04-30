#include "mruby.h"
#include "mruby/class.h"
#include "mruby/compile.h"
#include "mruby/data.h"

#include <ipc.h>
#include <stdlib.h>


static mrb_value f_thread_pass(mrb_state *mrb, mrb_value self)
{
    yield();

    return mrb_nil_value();
}


static mrb_value f_exit(mrb_state *mrb, mrb_value self)
{
    mrb_int status = 0;

    mrb_get_args(mrb, "|i", &status);

    exit(status);

    return mrb_nil_value();
}


void mrb_thread_gem_init(mrb_state *mrb)
{
    struct RClass *pc = mrb_define_class(mrb, "Thread", mrb->object_class);

    mrb_define_class_method(mrb, pc, "pass", f_thread_pass, ARGS_NONE());

    mrb_define_method(mrb, mrb->kernel_module, "exit", f_exit, ARGS_OPT(1));
}


void mrb_thread_gem_final(mrb_state *mrb)
{
}

#include "mruby.h"
#include "mruby/class.h"
#include "mruby/compile.h"
#include "mruby/data.h"

#include <errno.h>
#include <string.h>


static struct RClass *errno_class;


static mrb_value f_to_i(mrb_state *mrb, mrb_value self)
{
    return mrb_fixnum_value(*(int *)DATA_PTR(self));
}


static mrb_value f_to_s(mrb_state *mrb, mrb_value self)
{
    return mrb_str_new_cstr(mrb, strerror(*(int *)DATA_PTR(self)));
}


static void mrb_errno_free(mrb_state *mrb, void *ptr)
{
    mrb_free(mrb, ptr);
}


static struct mrb_data_type mrb_errno_type = { "Errno", mrb_errno_free };


static mrb_value f_errno(mrb_state *mrb, mrb_value self)
{
    int *ep = mrb_malloc(mrb, sizeof(*ep));
    *ep = errno;

    return mrb_obj_value(mrb_data_object_alloc(mrb, errno_class, ep, &mrb_errno_type));
}


static mrb_value f_set_errno(mrb_state *mrb, mrb_value self)
{
    mrb_int val;

    mrb_get_args(mrb, "i", &val);

    errno = val;

    return mrb_nil_value();
}


void mrb_errno_gem_init(mrb_state *mrb)
{
    errno_class = mrb_define_class(mrb, "Errno", mrb->object_class);

    mrb_define_method(mrb, errno_class, "to_i", f_to_i, ARGS_NONE());
    mrb_define_method(mrb, errno_class, "to_s", f_to_s, ARGS_NONE());

    mrb_define_class_method(mrb, errno_class, "set", f_set_errno, ARGS_REQ(1));
    mrb_define_class_method(mrb, errno_class, "=", f_set_errno, ARGS_REQ(1));

    mrb_define_method(mrb, mrb->kernel_module, "errno", f_errno, ARGS_NONE());
}


void mrb_errno_gem_final(mrb_state *mrb)
{
}
